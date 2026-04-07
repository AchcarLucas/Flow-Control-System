#include <data_monitor.h>
#include <esp_task_wdt.h>

DataMonitor::DataMonitor(const std::string fileName, std::string cleaningTime) :
    dao(nullptr),
    fileName(fileName),
    cleaningTime(cleaningTime),
    __lock(false) {
    this->createDatabase();
}

DataMonitor::~DataMonitor() {
    delete this->dao;
}

void DataMonitor::createDatabase() {
    if (this->dao != nullptr) return;
    this->__lock = true;

    Serial.println("Creating SQLiteDAO");
    this->dao = new SQLiteDAO(fileName);

    /*
    dao->SQLiteExec("PRAGMA synchronous = OFF;");
    dao->SQLiteExec("PRAGMA journal_mode = TRUNCATE;");
    dao->SQLiteExec("PRAGMA auto_vacuum = NONE;");
    dao->SQLiteExec("PRAGMA cache_size = 100;");
    dao->SQLiteExec("PRAGMA temp_store = MEMORY;");
    */

    dao->SQLiteExec(
        "CREATE TABLE "
        "IF NOT EXISTS " 
        "sample ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
                "sampling_time INTEGER,"
                "in_flow INTEGER,"
                "out_flow INTEGER"
        ");"
    );

    // dao->SQLiteExec("CREATE INDEX IF NOT EXISTS index_timestamp ON sample (timestamp);");
    this->__lock = false;
}

bool DataMonitor::insertSamples(std::list<Sample> samples) {
    this->__lock = true;
    for (const auto& sample : samples) {
        std::string SQL = dao->SQLiteQuery(
            "INSERT INTO sample (timestamp, sampling_time, in_flow, out_flow) VALUES (datetime('now'), %u, %u, %u);",
            sample.sampling_time, 
            sample.in, 
            sample.out
        );

        Serial.println(SQL.c_str());

        if(!dao->SQLiteExec(SQL)) return false;

        // reset do watchdog para não derrubar a aplicação
        esp_task_wdt_reset();
    }

    this->__lock = false;
    return true;
}

uint32_t DataMonitor::getTotalPages(uint16_t limit) {
    this->__lock = true;
    std::string SQL = dao->SQLiteQuery("SELECT COUNT(*) FROM sample;");

    Serial.println(SQL.c_str());

    uint32_t rows = 0;

    SQLitePrepareObject *prepare = dao->SQLitePrepare(SQL);

    if (prepare != nullptr) {
        if(dao->SQLiteStep(prepare)) {
            rows = sqlite3_column_int(prepare->getRes(), 0);
        }
    }

    dao->SQLiteFinalize(prepare);

    this->__lock = false;
    return (rows + limit - 1) / limit;
}

std::list<Sample> DataMonitor::selectSamples(uint16_t page, uint16_t limit) {
    this->__lock = true;
    std::list<Sample> samples;

    sqlite3_stmt *res;
    const char *tail;

    uint32_t offset = (page - 1) * limit;

    std::string SQL = dao->SQLiteQuery(
        "SELECT id, "
        "strftime('%%d/%%m/%%Y %%H:%%M:%%S', timestamp, 'localtime') AS timestamp_s, "
        "sampling_time, "
        "in_flow, "
        "out_flow "
        "FROM sample "
        "ORDER BY id DESC "
        "LIMIT %u OFFSET %u;",
        limit,
        offset
    );

    Serial.println(SQL.c_str());

    SQLitePrepareObject *prepare = dao->SQLitePrepare(SQL);
    
    if (prepare != nullptr) {
        while (dao->SQLiteStep(prepare)) {
            samples.push_back(
                Sample(
                    sqlite3_column_int(prepare->getRes(), 0),
                    std::string(reinterpret_cast<const char*>(sqlite3_column_text(prepare->getRes(), 1))),
                    sqlite3_column_int(prepare->getRes(), 2),
                    sqlite3_column_int(prepare->getRes(), 3),
                    sqlite3_column_int(prepare->getRes(), 4)
                )
            );

            esp_task_wdt_reset();
        }
    }

    dao->SQLiteFinalize(prepare);

    this->__lock = false;
    return samples;
}

std::list<Sample> DataMonitor::selectSamples(String startDatetime, String endDatetime) {
    this->__lock = true;
    std::list<Sample> samples;

    sqlite3_stmt *res;
    const char *tail;

    std::string SQL = dao->SQLiteQuery(
        "SELECT id, "
        "strftime('%%d/%%m/%%Y %%H:%%M:%%S', timestamp, 'localtime') AS timestamp_s, "
        "sampling_time, "
        "in_flow, "
        "out_flow "
        "FROM sample "
        "WHERE timestamp BETWEEN '%s' AND '%s' "
        "ORDER BY timestamp ASC;",
        startDatetime.c_str(),
        endDatetime.c_str()
    );

    Serial.println(SQL.c_str());

    SQLitePrepareObject *prepare = dao->SQLitePrepare(SQL);
    
    if (prepare != nullptr) {
        while (dao->SQLiteStep(prepare)) {
            samples.push_back(
                Sample(
                    sqlite3_column_int(prepare->getRes(), 0),
                    std::string(reinterpret_cast<const char*>(sqlite3_column_text(prepare->getRes(), 1))),
                    sqlite3_column_int(prepare->getRes(), 2),
                    sqlite3_column_int(prepare->getRes(), 3),
                    sqlite3_column_int(prepare->getRes(), 4)
                )
            );

            esp_task_wdt_reset();
        }
    }

    dao->SQLiteFinalize(prepare);

    this->__lock = false;
    return samples;
}

bool DataMonitor::removeSamplesByID(uint32_t id) {
    this->__lock = true;
    std::string SQL = dao->SQLiteQuery(
        "DELETE FROM sample WHERE id = %u;",
        id
    );

    Serial.println(SQL.c_str());

    if(!dao->SQLiteExec(SQL)) return false;

    this->__lock = false;
    return true;
}

bool DataMonitor::removeSamplesByTimestamp(uint64_t timestamp) {
    this->__lock = true;
    std::string SQL = dao->SQLiteQuery(
        "DELETE FROM sample WHERE timestamp = datetime(%llu, 'unixepoch');",
        timestamp
    );

    Serial.println(SQL.c_str());

    if(!dao->SQLiteExec(SQL)) return false;

    this->__lock = false;
    return true;
}

bool DataMonitor::cleanup(std::string cleaningTime) {
    this->__lock = true;

    std::string SQL = dao->SQLiteQuery(
        "DELETE FROM sample WHERE timestamp < datetime('now', '%s');",
        cleaningTime.empty() ? this->cleaningTime.c_str() : cleaningTime.c_str()
    );

    Serial.println(SQL.c_str());

    if(!dao->SQLiteExec(SQL)) return false;

    if(!dao->SQLiteExec("PRAGMA incremental_vacuum(100);")) return false;

    this->__lock = false;
    return true;
}

bool DataMonitor::reset() {
    this->__lock = true;

    delete this->dao; this->dao = nullptr;

    Serial.printf("Deleting database %s file.\n", DATABASE);

    if (!(LittleFS.exists("/" DATABASE) && LittleFS.remove("/" DATABASE))) {
        Serial.printf("Failed to remove database %s file.\n", DATABASE);
        return false;
    }

    Serial.printf("Recreating database %s file.\n", DATABASE);

    this->createDatabase();

    this->__lock = false;
    return true;
}
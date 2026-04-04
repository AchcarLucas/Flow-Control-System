#include <data_monitor.h>

DataMonitor::DataMonitor(const std::string fileName, std::string cleaningTime) 
    : fileName(fileName), cleaningTime(cleaningTime) {
    this->dao = new SQLiteDAO(fileName);
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
}

DataMonitor::~DataMonitor() {
    delete this->dao;
}

bool DataMonitor::insertSamples(std::list<Sample> samples) {
    for (const auto& sample : samples) {
        std::string SQL = dao->SQLiteQuery(
            "INSERT INTO sample (timestamp, sampling_time, in_flow, out_flow) VALUES (datetime('now'), %u, %u, %u);",
            sample.sampling_time, 
            sample.in, 
            sample.out
        );

        if(!dao->SQLiteExec(SQL)) return false;
    }

    return true;
}

std::list<Sample> DataMonitor::selectSamples(uint16_t limit) {
    std::list<Sample> samples;

    sqlite3_stmt *res;
    const char *tail;

    std::string SQL = dao->SQLiteQuery(
        "SELECT id, "
        "strftime('%%d/%%m/%%Y %%H:%%M:%%S', timestamp, 'localtime') AS timestamp_s, "
        "sampling_time, "
        "in_flow, "
        "out_flow "
        "FROM sample ORDER BY id DESC LIMIT %u;",
        limit
    );

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
        }
    }

    dao->SQLiteFinalize(prepare);

    return samples;
}

bool DataMonitor::removeSamplesByID(uint32_t id) {
    std::string SQL = dao->SQLiteQuery(
        "DELETE FROM sample WHERE id = {%u};",
        id
    );
    if(!dao->SQLiteExec(SQL)) return false;

    return true;
}

bool DataMonitor::removeSamplesByTimestamp(uint64_t timestamp) {
    std::string SQL = dao->SQLiteQuery(
        "DELETE FROM sample WHERE timestamp = datetime(%u);",
        timestamp
    );

    if(!dao->SQLiteExec(SQL)) return false;

    return true;
}

bool DataMonitor::cleaning() {
    std::string SQL = dao->SQLiteQuery(
        "DELETE FROM sample WHERE timestamp < datetime('now', %u);",
        this->cleaningTime
    );

    if(!dao->SQLiteExec(SQL)) return false;

    return true;
}
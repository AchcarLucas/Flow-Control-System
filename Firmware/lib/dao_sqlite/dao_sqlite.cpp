#include <dao_sqlite.h>
#include <esp_task_wdt.h>

SQLiteDAO::SQLiteDAO(const std::string fileName)
{
    sqlite3_initialize();

    std::string path_file = "/littlefs/" + fileName;

    if (sqlite3_open(path_file.c_str(), &db) != SQLITE_OK) {
        Serial.printf("Error to opening <%s> SQLite file\n", path_file.c_str());
        return;
    }
}

SQLiteDAO::~SQLiteDAO() {
    for (auto slpo : this->slpoList) {
        this->SQLiteFinalize(slpo);
    }

    this->slpoList.clear();
}

bool SQLiteDAO::SQLiteExec(const std::string sql) {
    char *zErrMsg = 0;

    esp_task_wdt_delete(NULL);
    int resultExec = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, &zErrMsg);
    esp_task_wdt_add(NULL);

    if (resultExec != SQLITE_OK) {
        Serial.printf("Error <%s> when did you try to execute the SQL command <%s>\n", zErrMsg, sql.c_str());
        sqlite3_free(zErrMsg);
        return false;
    }

    return true;
}

SQLitePrepareObject* SQLiteDAO::SQLitePrepare(const std::string sql) {
    sqlite3_stmt *res = nullptr;
    const char *tail = nullptr;

    esp_task_wdt_delete(NULL);
    int resultPrepare = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &res, &tail);
    esp_task_wdt_add(NULL);

    if (resultPrepare == SQLITE_OK) {
        SQLitePrepareObject *slpo = new SQLitePrepareObject(res, tail);
        this->slpoList.push_back(slpo);
        return slpo;
    }

    return nullptr;
}

bool SQLiteDAO::SQLiteStep(SQLitePrepareObject *slpo) {
    if (slpo == nullptr)
        return false;

    return sqlite3_step(slpo->getRes()) == SQLITE_ROW;
}

bool SQLiteDAO::SQLiteFinalize(SQLitePrepareObject *slpo) {
    if (slpo == nullptr)
        return false;

    esp_task_wdt_delete(NULL);
    int resultFinalize = sqlite3_finalize(slpo->getRes());
    esp_task_wdt_add(NULL);

    if (resultFinalize == SQLITE_OK) {
        this->slpoList.remove(slpo);
        delete slpo;
        return true;
    }

    return false;
}
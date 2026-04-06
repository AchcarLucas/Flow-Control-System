#include <dao_sqlite.h>
#include <esp_task_wdt.h>

HandlerCallback myHandlerCallback = nullptr;

uint16_t tickCount = 0;

void SQLiteHandlerReset() {
    tickCount = 0;
}

int SQLiteHandler(void *arg) {
    // watchdog reset
    esp_task_wdt_reset();
    vTaskDelay(1 / portTICK_PERIOD_MS);

    Serial.printf(" - [%u] SQLite is still running. - Watchdog Reset\n", tickCount);

    if (myHandlerCallback != nullptr) {
        myHandlerCallback(tickCount);
    }

    return 0; 
}

void SQLiteDAO::handlerCallback(HandlerCallback &handlerCallback) {
    myHandlerCallback = handlerCallback;
}

SQLiteDAO::SQLiteDAO(const std::string fileName) {
    sqlite3_initialize();

    std::string path_file = "/littlefs/" + fileName;

    if (sqlite3_open(path_file.c_str(), &db) != SQLITE_OK) {
        Serial.printf("Error to opening <%s> SQLite file\n", path_file.c_str());
        return;
    }

    sqlite3_progress_handler(db, 100, SQLiteHandler, NULL);
}

SQLiteDAO::~SQLiteDAO() {
    for (auto slpo : this->slpoList) {
        this->SQLiteFinalize(slpo);
    }

    this->slpoList.clear();
}

bool SQLiteDAO::SQLiteExec(const std::string sql) {
    char *zErrMsg = 0;

    SQLiteHandlerReset();
    int resultExec = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, &zErrMsg);

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

    SQLiteHandlerReset();
    int resultPrepare = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &res, &tail);

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

    SQLiteHandlerReset();
    return sqlite3_step(slpo->getRes()) == SQLITE_ROW;
}

bool SQLiteDAO::SQLiteFinalize(SQLitePrepareObject *slpo) {
    if (slpo == nullptr)
        return false;

    SQLiteHandlerReset();
    int resultFinalize = sqlite3_finalize(slpo->getRes());

    if (resultFinalize == SQLITE_OK) {
        this->slpoList.remove(slpo);
        delete slpo;
        return true;
    }

    return false;
}
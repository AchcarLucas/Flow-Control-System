#include <dao_sqlite.h>
#include <esp_task_wdt.h>

#define OPS_LIMIT 250
#define MUTEX_TIMEBLOCK 5000

HandlerCallback myHandlerCallback = nullptr;

uint16_t handlerCount = 0;

void SQLiteResetHandlerCount() {
    handlerCount = 0;
}

int SQLiteHandler(void *arg) {
    // watchdog reset
    esp_task_wdt_reset();

    Serial.printf(" - [%u] SQLite is still running. - Watchdog Reset\n", handlerCount);

    if (myHandlerCallback != nullptr) {
        myHandlerCallback(handlerCount);
    }

    handlerCount++;

    vTaskDelay(1 / portTICK_PERIOD_MS);
    return 0; 
}

void SQLiteDAO::handlerCallback(HandlerCallback &handlerCallback) {
    myHandlerCallback = handlerCallback;
}

SQLiteDAO::SQLiteDAO(const std::string fileName) {
    this->sqliteMutex = xSemaphoreCreateMutex();

    if (this->sqliteMutex == NULL) {
        Serial.printf("An error occurred while trying to create the mutex.\n");
        return;
    }

    sqlite3_initialize();

    std::string path_file = "/littlefs/" + fileName;

    int flags =
            SQLITE_OPEN_READWRITE |
            SQLITE_OPEN_CREATE    |
            SQLITE_OPEN_FULLMUTEX;

    if (sqlite3_open_v2(path_file.c_str(), &this->db, flags, NULL) != SQLITE_OK) {
        Serial.printf("An error occurred while opening the SQLite file <%s> \n", path_file.c_str());
        return;
    }

    sqlite3_progress_handler(this->db, OPS_LIMIT, SQLiteHandler, NULL);
}

SQLiteDAO::~SQLiteDAO() {
    this->close();
}

void SQLiteDAO::close() {
    printf("DAO closing process\n");

    for (auto slpo : this->slpoList) {
        this->SQLiteFinalize(slpo);
    }

    sqlite3_close(this->db);

    this->slpoList.clear();

    Serial.printf("DAO closed\n");
}

bool SQLiteDAO::SQLiteExec(const std::string sql) {
    while (xSemaphoreTake(this->sqliteMutex, pdMS_TO_TICKS(MUTEX_TIMEBLOCK)) != pdTRUE) {
        Serial.printf("[%s] Waiting for Mutex to be released.\n", __func__);
        esp_task_wdt_reset();
    }

    Serial.printf("[%s] Mutex locked.\n", __func__);

    char *zErrMsg = 0;
    bool __success = true;

    SQLiteResetHandlerCount();
    int resultExec = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, &zErrMsg);

    if (resultExec != SQLITE_OK) {
        Serial.printf("[SQL Error] <%s> when did you try to execute the SQL command <%s>\n", zErrMsg, sql.c_str());
        sqlite3_free(zErrMsg);
        __success = false;
    }

    Serial.printf("[%s] Mutex released.\n", __func__);
    xSemaphoreGive(this->sqliteMutex);

    return __success;
}

SQLitePrepareObject* SQLiteDAO::SQLitePrepare(const std::string sql) {
    while (xSemaphoreTake(this->sqliteMutex, pdMS_TO_TICKS(MUTEX_TIMEBLOCK)) != pdTRUE) {
        Serial.printf("[%s] Waiting for Mutex to be released.\n", __func__);
        esp_task_wdt_reset();
    }

    Serial.printf("[%s] Mutex locked.\n", __func__);

    SQLitePrepareObject *slpo = nullptr;
    sqlite3_stmt *res = nullptr;
    const char *tail = nullptr;

    SQLiteResetHandlerCount();
    int resultPrepare = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &res, &tail);

    if (resultPrepare == SQLITE_OK) {
        slpo = new SQLitePrepareObject(res, tail);
        this->slpoList.push_back(slpo);
    }

    Serial.printf("[%s] Mutex released.\n", __func__);
    xSemaphoreGive(this->sqliteMutex);

    return slpo;
}

bool SQLiteDAO::SQLiteStep(SQLitePrepareObject *slpo) {
    if (slpo == nullptr)
        return false;

    while (xSemaphoreTake(this->sqliteMutex, pdMS_TO_TICKS(MUTEX_TIMEBLOCK)) != pdTRUE) {
        Serial.printf("[%s] Waiting for Mutex to be released.\n", __func__);
        esp_task_wdt_reset();
    }

    bool __success = false;

    __success = sqlite3_step(slpo->getRes()) == SQLITE_ROW;

    xSemaphoreGive(this->sqliteMutex);

    return __success;
}

bool SQLiteDAO::SQLiteFinalize(SQLitePrepareObject *slpo) {
    if (slpo == nullptr)
        return false;

    while (xSemaphoreTake(this->sqliteMutex, pdMS_TO_TICKS(MUTEX_TIMEBLOCK)) != pdTRUE) {
        Serial.printf("[%s] Waiting for Mutex to be released.\n", __func__);
        esp_task_wdt_reset();
    }

    Serial.printf("[%s] Mutex locked.\n", __func__);

    bool __success = true;

    int resultFinalize = sqlite3_finalize(slpo->getRes());

    if (resultFinalize != SQLITE_OK) {
        Serial.printf("[%s] An error occurred while executing sqlite3_finalize..\n", __func__);
        __success = false;
    }

    this->slpoList.remove(slpo);
    delete slpo;

    Serial.printf("[%s] Mutex released.\n", __func__);
    xSemaphoreGive(this->sqliteMutex);

    return __success;
}
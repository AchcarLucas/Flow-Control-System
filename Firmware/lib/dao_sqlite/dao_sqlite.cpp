#include <dao_sqlite.h>
#include <esp_task_wdt.h>

#define OPS_LIMIT 100

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

    if (sqlite3_open(path_file.c_str(), &this->db) != SQLITE_OK) {
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
    bool __success = true;

    // Faz o Mutex aguardar até ser possível chamar o 'sqlite3_exec' novamente
    if (xSemaphoreTake(this->sqliteMutex, portMAX_DELAY) == pdTRUE) {
        SQLiteResetHandlerCount();

        char *zErrMsg = 0;
        int resultExec = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, &zErrMsg);

        if (resultExec != SQLITE_OK) {
            Serial.printf("[SQL Error] <%s> when did you try to execute the SQL command <%s>\n", zErrMsg, sql.c_str());
            sqlite3_free(zErrMsg);
            __success = false;
        }

        // Liberação do Mutex
        xSemaphoreGive(this->sqliteMutex);
    } else {
        Serial.printf("[Mutex Error] when did you try to execute the SQL command <%s>\n", sql.c_str());
        return false;
    }

    return __success;
}

SQLitePrepareObject* SQLiteDAO::SQLitePrepare(const std::string sql) {
    sqlite3_stmt *res = nullptr;
    const char *tail = nullptr;

    SQLiteResetHandlerCount();
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

    SQLiteResetHandlerCount();
    return sqlite3_step(slpo->getRes()) == SQLITE_ROW;
}

bool SQLiteDAO::SQLiteFinalize(SQLitePrepareObject *slpo) {
    if (slpo == nullptr)
        return false;

    SQLiteResetHandlerCount();
    int resultFinalize = sqlite3_finalize(slpo->getRes());

    if (resultFinalize == SQLITE_OK) {
        this->slpoList.remove(slpo);
        delete slpo;
        return true;
    }

    return false;
}
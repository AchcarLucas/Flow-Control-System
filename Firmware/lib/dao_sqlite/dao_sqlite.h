#ifndef DAO_SQLITE_H
#define DAO_SQLITE_H

#include <string>
#include <list>

#include <sqlite3.h>
#include <LittleFS.h>

typedef void (*HandlerCallback)(int progresso);

class SQLitePrepareObject {
    private:
        sqlite3_stmt *res;
        const char *tail;
    public:
        SQLitePrepareObject(sqlite3_stmt *res, const char *tail) 
            : res(res), tail(tail) {}

        sqlite3_stmt *getRes() {
            return this->res; 
        }

        const char *getTail() { 
            return this->tail; 
        }
};

class SQLiteDAO {
    protected:
        sqlite3 *db;
        std::list<SQLitePrepareObject *> slpoList;
    private:
    public:
        SQLiteDAO(const std::string fileName);
        ~SQLiteDAO();

        sqlite3 *getDB() { return this->db; }

        bool SQLiteExec(const std::string);

        SQLitePrepareObject *SQLitePrepare(const std::string sql);
        bool SQLiteStep(SQLitePrepareObject *);
        bool SQLiteFinalize(SQLitePrepareObject *);

        void handlerCallback(HandlerCallback &handlerCallback);

        template <typename... Args>
        std::string SQLiteQuery(const char* fmt, Args... args) {
            int size = snprintf(nullptr, 0, fmt, args...) + 1;
            if (size <= 0) return "";
            
            char* buf = new char[size];
            snprintf(buf, size, fmt, args...);
            std::string res(buf);

            delete[] buf;

            return res;
        }
};

#endif
#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <Arduino.h>

#include <list>

class FileSystem {
    private:
    public:
        FileSystem() { };

        std::list<String> listFile(String, uint8_t);
        bool deleteFile(std::list<String>, std::list<String>);
        bool deleteFile(String file);

}

#endif
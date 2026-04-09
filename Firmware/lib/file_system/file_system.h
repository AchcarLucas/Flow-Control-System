#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <Arduino.h>

#include <list>
#include <utility>

class FileSystem {
    private:
        // Private constructor to prevent direct instantiation
        FileSystem() = default;
    public:
        // Delete copy constructor and assignment operator to prevent cloning
        FileSystem(const FileSystem&) = delete;
        FileSystem& operator=(const FileSystem&) = delete;

        // Static method to provide global access to the instance
        static FileSystem& getInstance() {
            // Guaranteed to be thread-safe in C++11+
            static FileSystem instance;
            return instance;
        }

        std::list<std::pair<String, size_t>> listFile(String, uint8_t levels = 3);
        bool deleteFile(std::list<String>, std::list<String>);
        bool deleteFile(String file);
        bool fileExist(String file);
};

#endif
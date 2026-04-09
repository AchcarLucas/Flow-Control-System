#include <file_system.h>

#include <LittleFS.h>

std::list<String> FileSystem::listFile(String directory, uint8_t levels) {
    std::list<String> dirFile;

    Serial.printf("\nExploring directory: %s\n", directory);

    File root = LittleFS.open(directory);

    if (!root) {
        Serial.println(" - Failed to open directory.");
        return std::list<String>();
    }

    if (!root.isDirectory()) {
        Serial.println(" - Not a directory");
        return std::list<String>();
    }

    File file = root.openNextFile();

    while (file) {
        if (file.isDirectory()) {
            // Serial.printf("  [DIR]  %s\n", file.name());
            dirFile.push_back(" [DIR] " + String(file.name()));
            
            if (levels) {
                std::list<String> _dirFile = listFile(file.path(), levels - 1);
                dirFile.splice(dirFile.end(), _dirFile);
            }
        }
        else {
            // Serial.printf("  [FILE] %s  |  SIZE: %u bytes\n", file.name(), file.size());
            dirFile.push_back("[FILE] " + directory + String(file.name()) + "  |  SIZE " + String(file.size()) + " bytes");
        }
        file = root.openNextFile();
    }

    return dirFile;
}

bool FileSystem::deleteFile(std::list<String> lStartsWith, std::list<String> lEndsWith) {
    bool resultDeleteFile = true;

    File root = LittleFS.open("/");
    File file = root.openNextFile();

    while (file) {
        String fileName = file.name();

        bool isStartWith = false;
        for (auto startsWith : lStartsWith) {
            if (fileName.startsWith(startsWith)) {
                isStartWith = true;
                break;
            }
        }

        bool isEndsWith = false;
        for (auto endsWith : lEndsWith) {
            if (fileName.startsWith(endsWith)) {
                isEndsWith = true;
                break;
            }
        }

        if (isStartWith || isEndsWith) {
            String path = "/" + fileName;
            file.close();

            if(!deleteFile(path)) {
                resultDeleteFile = false;
                break;
            }
        }

        file = root.openNextFile();
    }

    return resultDeleteFile;
}

bool FileSystem::deleteFile(String file) {
    if (!(LittleFS.exists(file) && LittleFS.remove(file))) {
        Serial.printf("Failed to remove file %s.\n", file.c_str());
        return false;
    }
    Serial.printf(" - Delete: %s\n", file.c_str());
    return true;
}

bool FileSystem::fileExist(String file) {
    return LittleFS.exists(file);
}
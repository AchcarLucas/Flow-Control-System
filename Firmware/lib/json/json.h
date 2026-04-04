#ifndef JSON_H
#define JSON_H

#include <Arduino.h>

class JSON {
    public:
        virtual String serialize() = 0;
};

#endif
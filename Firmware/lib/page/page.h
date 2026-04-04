#ifndef PAGE_H
#define PAGE_H

#include <Arduino.h>

class Page {
    public:
        virtual String page() = 0;
};

#endif
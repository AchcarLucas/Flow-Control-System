#ifndef INDEX_PAGE_H
#define INDEX_PAGE_H

#include <Arduino.h>

#include <page.h>
#include <data_monitor.h>

#include <list>

class IndexPage : public Page {
    protected:
        String header();
    private:
    public:
        IndexPage() {};
        String style();
        String body();
        String page() override;
};

#endif
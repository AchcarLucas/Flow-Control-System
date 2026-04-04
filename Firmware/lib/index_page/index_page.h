#ifndef TEST_PAGE_H
#define TEST_PAGE_H

#include <Arduino.h>

#include <page.h>
#include <data_monitor.h>

#include <list>

class IndexPage : public Page {
    protected:
        String header();
        String table();
        String pagination();
        String tools();
    private:
        String database;

        uint32_t currentPage;

        uint16_t totalPages;
        uint16_t limit;

        std::list<Sample> samples;
    public:
        IndexPage(
                String database,
                uint32_t currentPage,
                uint16_t totalPages,
                uint16_t limit,
                std::list<Sample> samples,
                bool debug = false
        ) : database(database),
            currentPage(currentPage),
            totalPages(totalPages),
            limit(limit),
            samples(samples) {};

        String page() override;
};

#endif
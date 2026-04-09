#ifndef RAW_PAGE_H
#define RAW_PAGE_H

#include <Arduino.h>

#include <page.h>
#include <data_monitor.h>

#include <list>

class RawPage : public Page {
    protected:
        String style();
        String script();

        String table();
        String pagination();
        String tools();

        String header();
        String body();        
        String footer();
    private:
        String database;

        uint32_t currentPage;

        uint16_t totalPages;
        uint16_t limit;

        std::list<Sample> *samples;
    public:
        RawPage(
                String database,
                uint32_t currentPage,
                uint16_t totalPages,
                uint16_t limit,
                std::list<Sample> *samples
        ) : database(database),
            currentPage(currentPage),
            totalPages(totalPages),
            limit(limit),
            samples(samples) {};

        String page() override;
};

#endif
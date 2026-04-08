#ifndef STATS_PAGE_H
#define STATS_PAGE_H

#include <Arduino.h>

#include <page.h>

class StatsPage : public Page {
    protected:
        String style();
        String script();

        String header();
        String body();
        String footer();
    private:
    public:
        StatsPage();
        
        String page() override;
};

#endif
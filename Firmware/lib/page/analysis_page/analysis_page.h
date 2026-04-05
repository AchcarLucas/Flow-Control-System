#ifndef ANALYSIS_PAGE_H
#define ANALYSIS_PAGE_H

#include <Arduino.h>

#include <page.h>
#include <data_monitor.h>

#include <list>

class AnalysisPage : public Page {
    protected:
        String header();
        String footer();
        String body();
    private:
        String database;
    public:
        AnalysisPage(String database) : database(database) {};
        String page() override;
};

#endif
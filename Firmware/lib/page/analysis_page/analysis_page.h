#ifndef ANALYSIS_PAGE_H
#define ANALYSIS_PAGE_H

#include <Arduino.h>

#include <page.h>

#include <list>

class AnalysisPage : public Page {
    protected:
        String style();
        String script();

        String header();
        String body();
        String footer();
    private:
        String database;
    public:
        AnalysisPage(String database) : database(database) {};
        String page() override;
};

#endif
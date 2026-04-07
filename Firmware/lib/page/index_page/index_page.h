#ifndef INDEX_PAGE_H
#define INDEX_PAGE_H

#include <Arduino.h>

#include <page.h>

#include <list>

class IndexPage : public Page {
    protected:
        String style();
        String script();

        String header();
        String body();
        String footer();
    private:
    public:
        IndexPage() {};
        
        String page() override;
};

#endif
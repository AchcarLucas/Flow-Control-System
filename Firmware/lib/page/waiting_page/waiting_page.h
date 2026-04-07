#ifndef WAITING_PAGE_H
#define WAITING_PAGE_H

#include <Arduino.h>

#include <page.h>

class WaitingPage : public Page {
    protected:
        String style();
        String script();

        String header();
        String body();        
        String footer();
    private:
        String text;
        String url;
        uint16_t timeout;
    public:
        WaitingPage(String text = "", String url = "/", uint16_t timeout = 1000) : 
            text(text), url(url), timeout(timeout) {};

        void setText(String text) { 
            this->text = text; 
        }

        void setRedirection(String url, uint16_t timeout) {
            this->url = url;
            this->timeout = timeout;
        }

        String page() override;
};

#endif
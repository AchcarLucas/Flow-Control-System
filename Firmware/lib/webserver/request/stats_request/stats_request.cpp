#include <stats_request.h>

#include <page.h>
#include <stats_page.h>
#include <config.h>

AsyncCallbackWebHandler& StatsRequest::onServer() {
    // PAGE 'stats data' http://[IP-DO-ESP]/stats
    return this->webServer->on("/raw", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        Page *statsPage = new StatsPage();

        request->send(200, "text/html", statsPage->page());

        delete statsPage;

        FINISH_SERVER_PROCESSING();
    });
}
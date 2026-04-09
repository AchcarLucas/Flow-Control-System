#include <stats_request.h>

#include <page.h>
#include <stats_page.h>
#include <config.h>

AsyncCallbackWebHandler& StatsRequest::onServer() {
    // PAGE 'stats data' http://[IP-DO-ESP]/stats
    return this->webServer->on("/stats", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        StatsPage statsPage = StatsPage();

        request->send(200, "text/html", statsPage.page());

        FINISH_SERVER_PROCESSING();
    });
}
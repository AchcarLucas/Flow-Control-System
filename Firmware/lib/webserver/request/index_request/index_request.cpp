#include <index_request.h>

#include <page.h>
#include <index_page.h>
#include <config.h>

AsyncCallbackWebHandler& IndexRequest::onServer() {
    // PAGE 'index' http://[IP-DO-ESP]/index
    return this->webServer->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        IndexPage indexPage = IndexPage();

        request->send(200, "text/html", indexPage.page());

        FINISH_SERVER_PROCESSING();
    });
}
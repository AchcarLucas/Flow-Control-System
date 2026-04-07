#include <index_request.h>

#include <page.h>
#include <index_page.h>
#include <config.h>

AsyncCallbackWebHandler& IndexRequest::onServer() {
    // PAGE 'index' http://[IP-DO-ESP]/index
    return this->server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        Page *indexPage = new IndexPage();

        request->send(200, "text/html", indexPage->page());

        delete indexPage;

        FINISH_SERVER_PROCESSING();
    });
}
#include <raw_request.h>

#include <page.h>
#include <raw_page.h>
#include <config.h>

AsyncCallbackWebHandler& RawRequest::onServer() {
    // PAGE 'raw data' http://[IP-DO-ESP]/raw?page={page}
    return this->server->on("/raw", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        uint16_t limit = 10;
        uint16_t currentPage = 1;

        if (request->hasParam("page")) {
            currentPage = request->getParam("page")->value().toInt();
            if (currentPage < 1) {
                currentPage = 1;
            }
        }

        uint32_t totalPages = this->getMonitor()->getTotalPages(limit);
        std::list<Sample> samples = this->getMonitor()->selectSamples(currentPage, limit);

        Serial.println("Current Page: " + String(currentPage) + " Total Pages: " + String(totalPages));

        Page *rawPage = new RawPage(
            DATABASE,
            currentPage,
            totalPages,
            limit,
            samples
        );

        request->send(200, "text/html", rawPage->page());

        delete rawPage;

        FINISH_SERVER_PROCESSING();
    });
}
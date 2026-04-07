#include <analysis_request.h>

#include <page.h>
#include <analysis_page.h>
#include <config.h>

AsyncCallbackWebHandler& AnalysisRequest::onServer() {
    // PAGE 'analysis data' http://[IP-DO-ESP]/analysis?start={datetime}&end={datetime}
    return this->server->on("/analysis", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        Page *analysisPage = new AnalysisPage(DATABASE);

        request->send(200, "text/html", analysisPage->page());

        delete analysisPage;

        FINISH_SERVER_PROCESSING();
    });
}
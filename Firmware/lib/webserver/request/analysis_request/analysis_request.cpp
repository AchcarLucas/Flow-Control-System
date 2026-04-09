#include <analysis_request.h>

#include <page.h>
#include <analysis_page.h>
#include <config.h>

AsyncCallbackWebHandler& AnalysisRequest::onServer() {
    // PAGE 'analysis data' http://[IP-DO-ESP]/analysis?start={datetime}&end={datetime}
    return this->webServer->on("/analysis", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        AnalysisPage analysisPage = AnalysisPage(DATABASE);

        request->send(200, "text/html", analysisPage.page());

        FINISH_SERVER_PROCESSING();
    });
}
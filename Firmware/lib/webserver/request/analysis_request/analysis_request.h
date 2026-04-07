#ifndef ANALYSIS_REQUEST_H
#define ANALYSIS_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class AnalysisRequest : public WebServer {
    public:
        AnalysisRequest(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
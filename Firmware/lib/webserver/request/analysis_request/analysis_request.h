#ifndef ANALYSIS_REQUEST_H
#define ANALYSIS_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class AnalysisRequest : public WebServer {
    public:
        AnalysisRequest(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
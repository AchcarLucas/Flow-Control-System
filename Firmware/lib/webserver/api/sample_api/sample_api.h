#ifndef SAMPLE_API_H
#define SAMPLE_API_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class SampleAPI : public WebServer {
    public:
        SampleAPI(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
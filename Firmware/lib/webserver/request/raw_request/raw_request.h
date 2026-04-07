#ifndef RAW_REQUEST_H
#define RAW_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class RawRequest : public WebServer {
    public:
        RawRequest(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
#ifndef RESET_REQUEST_H
#define RESET_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class ResetRequest : public WebServer {
    public:
        ResetRequest(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
#ifndef RAW_REQUEST_H
#define RAW_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class RawRequest : public WebServer {
    public:
        RawRequest(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
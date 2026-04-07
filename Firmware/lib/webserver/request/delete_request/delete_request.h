#ifndef DELETE_REQUEST_H
#define DELETE_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class DeleteRequest : public WebServer {
    public:
        DeleteRequest(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
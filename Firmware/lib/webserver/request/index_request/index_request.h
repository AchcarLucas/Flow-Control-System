#ifndef INDEX_REQUEST_H
#define INDEX_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class IndexRequest : public WebServer {
    public:
        IndexRequest(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
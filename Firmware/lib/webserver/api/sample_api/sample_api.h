#ifndef SAMPLE_API_H
#define SAMPLE_API_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class SampleAPI : public WebServer {
    public:
        SampleAPI(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
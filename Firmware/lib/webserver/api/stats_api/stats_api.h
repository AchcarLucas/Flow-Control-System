#ifndef STATS_API_H
#define STATS_API_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class StatsAPI : public WebServer {
    public:
        StatsAPI(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
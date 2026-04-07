#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <data_monitor.h>

class WebServer {
    protected:
        AsyncWebServer *server;
        DataMonitor *monitor;
    public:
        WebServer(AsyncWebServer *server, DataMonitor *monitor) :
            server(server), monitor(monitor) {};
        virtual AsyncCallbackWebHandler& onServer() = 0;
        DataMonitor *getMonitor() { return monitor; }
        AsyncWebServer *getServer() { return server; }
};

#endif
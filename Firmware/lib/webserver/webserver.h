#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <data_monitor.h>

class WebServer {
    protected:
        AsyncWebServer *server;
        DataMonitor *dataMonitor;
    public:
        WebServer(AsyncWebServer *server, DataMonitor *dataMonitor) :
            server(server), dataMonitor(dataMonitor) {};
        virtual AsyncCallbackWebHandler& onServer() = 0;
        DataMonitor *getMonitor() { return dataMonitor; }
        AsyncWebServer *getServer() { return server; }
};

#endif
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <data_monitor.h>

class WebServer {
    protected:
        AsyncWebServer *webServer;
        DataMonitor *dataMonitor;
    public:
        WebServer(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            webServer(webServer), dataMonitor(dataMonitor) {};
        virtual AsyncCallbackWebHandler& onServer() = 0;
        DataMonitor *getDataMonitor() { return dataMonitor; }
        AsyncWebServer *getWebServer() { return webServer; }
};

#endif
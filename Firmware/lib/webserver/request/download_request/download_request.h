#ifndef DOWNLOAD_REQUEST_H
#define DOWNLOAD_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class DownloadRequest : public WebServer {
    public:
        DownloadRequest(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
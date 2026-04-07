#ifndef DOWNLOAD_REQUEST_H
#define DOWNLOAD_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class DownloadRequest : public WebServer {
    public:
        DownloadRequest(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor) {};
        AsyncCallbackWebHandler& onServer() override;
};

#endif
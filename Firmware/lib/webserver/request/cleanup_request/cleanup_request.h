#ifndef CLEANUP_REQUEST_H
#define CLEANUP_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class CleanupRequest : public WebServer {
    private:
        bool taskRunning;
        bool taskStarted;
    protected:
        static void Task(void *pvParameters);
    public:
        CleanupRequest(AsyncWebServer *webServer, DataMonitor *dataMonitor) :
            WebServer(webServer, dataMonitor), taskRunning(false) {};
        AsyncCallbackWebHandler& onServer() override;

        void setTaskRunning(bool taskRunning) { this->taskRunning = taskRunning; }
        bool getTaskRunning() { return this->taskRunning; }

        void setTaskStarted(bool taskStarted) { this->taskStarted = taskStarted; }
        bool getTaskStarted() { return this->taskStarted; }

};

#endif
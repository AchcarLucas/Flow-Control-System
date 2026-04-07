#ifndef SIMULATE_REQUEST_H
#define SIMULATE_REQUEST_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <webserver.h>

class SimulateRequest : public WebServer {
    private:
        bool taskRunning;
        bool taskStarted;
    protected:
        static void Task(void *pvParameters);
    public:
        SimulateRequest(AsyncWebServer *server, DataMonitor *monitor) :
            WebServer(server, monitor), taskRunning(false) {};
        AsyncCallbackWebHandler& onServer() override;

        void setTaskRunning(bool taskRunning) { this->taskRunning = taskRunning; }
        bool getTaskRunning() { return this->taskRunning; }

        void setTaskStarted(bool taskStarted) { this->taskStarted = taskStarted; }
        bool getTaskStarted() { return this->taskStarted; }

};

#endif
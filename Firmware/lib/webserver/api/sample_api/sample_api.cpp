#include <sample_api.h>

#include <json.h>
#include <sample_json.h>
#include <config.h>

AsyncCallbackWebHandler& SampleAPI::onServer() {
    // t_start ('YYYY-MM-DD HH:MM:SS') and t_end ('YYYY-MM-DD HH:MM:SS')
    return this->webServer->on("/api/samples", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        PRINT_FREE_HEAP_SIZE();

        if (!(request->hasParam("t_start") && request->hasParam("t_end"))) {
            request->send(404, "application/json", "{ \"status\": \"failed\" }");
            FINISH_SERVER_PROCESSING();
            return;
        }

        String tStart = request->getParam("t_start")->value();
        String tEnd = request->getParam("t_end")->value();
        
        Serial.printf("t_start %s - t_end %s\n", tStart.c_str(), tEnd.c_str());

        std::list<Sample> samples = this->getDataMonitor()->selectSamples(tStart, tEnd);

        SampleJson json = SampleJson(&samples);
        
        request->send(200, "application/json", json.serialize());

        PRINT_FREE_HEAP_SIZE();

        FINISH_SERVER_PROCESSING();
    });
}
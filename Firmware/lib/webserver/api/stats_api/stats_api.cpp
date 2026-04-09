#include <stats_api.h>

#include <json.h>
#include <stats_json.h>
#include <config.h>

#include <esp_heap_caps.h>
#include <esp_system.h>
#include <LittleFS.h>

AsyncCallbackWebHandler& StatsAPI::onServer() {
    return this->webServer->on("/api/stats", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        uint32_t freeHeap = esp_get_free_heap_size();
        uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
        uint32_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
        
        size_t flashTotal = LittleFS.totalBytes();
        size_t flashUsed = LittleFS.usedBytes();

        Serial.printf(" - freeHeap %u - minFreeHeap %u - largestBlock %u - flashTotal %u - flashUsed %u\n",
            freeHeap,
            minFreeHeap,
            largestBlock,
            flashTotal,
            flashUsed
        );

        JSON *json = new StatsJson(
            freeHeap,
            minFreeHeap,
            largestBlock,
            flashTotal,
            flashUsed
        );
        
        request->send(200, "application/json", json->serialize());

        delete json;
    
        FINISH_SERVER_PROCESSING();
    });
}
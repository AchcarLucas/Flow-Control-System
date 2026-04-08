#include <stats_json.h>

#include <ArduinoJson.h>

String StatsJson::serialize() {
    JsonDocument doc;
    
    JsonObject ram = doc.createNestedObject("ram");
    ram["free"] = this->freeHeap;
    ram["min_ever"] = this->minFreeHeap;
    ram["largest_block"] = this->largestBlock; // Se esse valor baixar muito, o JSON do SELECT vai falhar
    ram["fragmentation_stable"] = (this->largestBlock > (this->freeHeap * 0.5)); // Exemplo de indicador de saúde

    JsonObject flash = doc.createNestedObject("flash");
    flash["total"] = this->flashTotal;
    flash["used"] = this->flashUsed;
    flash["free"] = this->flashTotal - this->flashUsed;

    doc["uptime_ms"] = millis();
    doc["status"] = "online";

    // 3. Serialização e Envio
    String response;
    serializeJson(doc, response);

    return response;
}
#include <sample_json.h>

#include <ArduinoJson.h>

String SampleJson::serialize() {
    if (this->samples == nullptr) return String("");

    JsonDocument doc;
    JsonArray array = doc.createNestedArray("data");

    for (const auto& sample : *this->samples) {
        JsonObject obj = array.add<JsonObject>();

        obj["id"] = sample.id;
        obj["timestamp"] = sample.timestamp; // Se for string ou uint64
        obj["sampling_time"] = sample.sampling_time;
        obj["in"] = sample.in;
        obj["out"] = sample.out;
    }

    doc["total"] = this->samples->size();
    doc["status"] = "success";

    // Serializar para string
    String response;
    serializeJson(doc, response);

    return response;
}
#include <sample_json.h>

#include <ArduinoJson.h>

String SampleJson::serialize() {
    JsonDocument doc; // Ajuste o tamanho conforme necessário
    JsonArray array = doc.createNestedArray("data");

    for (const auto& sample : this->samples) {
        JsonObject obj = doc['data'].createNestedObject();

        obj["id"] = sample.id;
        obj["timestamp"] = sample.timestamp; // Se for string ou uint64
        obj["sampling_time"] = sample.sampling_time;
        obj["in"] = sample.in;
        obj["out"] = sample.out;
    }

    doc["total"] = this->samples.size();
    doc["status"] = "success";

    // Serializar para string
    String response;
    serializeJson(doc, response);

    return response;
}
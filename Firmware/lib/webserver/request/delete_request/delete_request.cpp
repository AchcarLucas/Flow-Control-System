#include <delete_request.h>
#include <config.h>

AsyncCallbackWebHandler& DeleteRequest::onServer() {
    // Deleta um dado especifico com base no seu ID: http://[IP-DO-ESP]/delete?id={id}
    return this->server->on("/delete", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        if (!request->hasParam("id")) {
            request->send(404, "text/plain", "An error occurred while trying to delete; the id parameter is not present.");
            FINISH_SERVER_PROCESSING();
            return;
        }

        uint32_t id = request->getParam("id")->value().toInt();

        bool result = this->getMonitor()->removeSamplesByID(id);

        if(!result) {
            request->send(404, "text/plain", "An error occurred while trying to delete the id " + String(id) + ".");
            FINISH_SERVER_PROCESSING();
            return;
        }

        request->redirect("/raw");

        FINISH_SERVER_PROCESSING();
    });
}
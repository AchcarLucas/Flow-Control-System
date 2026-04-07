#include <reset_request.h>
#include <config.h>

AsyncCallbackWebHandler& ResetRequest::onServer() {
    // Reset do banco de dados: http://[IP-DO-ESP]/reset
    return this->server->on("/reset", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        bool result = this->getMonitor()->reset();

        if(!result) {
            request->send(404, "text/plain", "An error occurred while attempting to perform an database reset on the system.");
            FINISH_SERVER_PROCESSING();
            return;
        }

        request->redirect("/raw");

        FINISH_SERVER_PROCESSING();
    });
}
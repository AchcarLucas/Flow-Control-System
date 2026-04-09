#include <download_request.h>
#include <file_system.h>
#include <config.h>

AsyncCallbackWebHandler& DownloadRequest::onServer() {
    // Rota para baixar o banco de dados: http://[IP-DO-ESP]/download
    return this->webServer->on("/download", HTTP_GET, [this](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();
        
        if (!FileSystem::getInstance().fileExist("/" DATABASE)) {
            request->send(404, "text/plain", "An error occurred while trying to download the database " DATABASE " file not found");
            FINISH_SERVER_PROCESSING();
            return;
        }

        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/" DATABASE, "application/octet-stream");

        String attachment = "attachment; filename=\"" + String(DATABASE) + "\"";
        response->addHeader("Content-Disposition", attachment.c_str());

        request->send(response);

        FINISH_SERVER_PROCESSING();
    });
}
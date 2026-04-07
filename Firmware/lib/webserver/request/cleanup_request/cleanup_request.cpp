#include <cleanup_request.h>

#include <page.h>
#include <waiting_page.h>
#include <config.h>

void CleanupRequest::Task(void *pvParameters) {
    CleanupRequest* instance = (CleanupRequest*)pvParameters;

    instance->setTaskRunning(true);

    Serial.println("[Task CleanupRequest] Starting background cleanup....");

    bool result = instance->getDataMonitor()->cleanup();

    if(!result) {
        Serial.println("[Task CleanupRequest] An error occurred while attempting to perform an database optimized cleanup on the system.");
    } else {
        Serial.println("[Task CleanupRequest] Cleaning completed successfully.");
    }

    instance->setTaskRunning(false);
    vTaskDelete(NULL);
}

AsyncCallbackWebHandler& CleanupRequest::onServer() {
    // Limpeza de otimização do banco de dados: http://[IP-DO-ESP]/cleanup
    return this->webServer->on("/cleanup", HTTP_GET, [this](AsyncWebServerRequest *request) {
        CHECK_DEBUG();

        STARTING_SERVER_PROCESSING();

        // Se iniciou o processo e já finalizou, redireciona para '/raw'
        if (this->getTaskStarted() && this->getTaskRunning() == false) {
            this->setTaskStarted(false);
            request->redirect("/raw");
            return;
        }

        if (this->getTaskRunning()) {
            WaitingPage *waitingPage = new WaitingPage(
                "O processo de limpeza já está em andamento. Por favor, aguarde.", 
                "/cleanup"
            );

            request->send(422, "text/html", waitingPage->page());
            delete waitingPage;

            FINISH_SERVER_PROCESSING();
            return;
        }

        xTaskCreatePinnedToCore(
            CleanupRequest::Task,          // Função da task
            "cleanupTask",                 // Nome
            4096,                           // Tamanho da Stack
            this,                           // Parâmetros
            1,                              // Prioridade
            NULL,                           // Handle
            1                               // Core (1 é o padrão do Arduino)
        );

        this->setTaskStarted(true);

        WaitingPage *waitingPage = new WaitingPage(
            "Cleanup Optimization Database iniciada! O sistema estará pronto em instantes. Por favor, aguarde.",
            "/cleanup"
        );

        request->send(200, "text/html", waitingPage->page());
        delete waitingPage;

        FINISH_SERVER_PROCESSING();
    });
}
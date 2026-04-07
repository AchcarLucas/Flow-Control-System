#include <simulate_request.h>

#include <page.h>
#include <waiting_page.h>
#include <config.h>

void SimulateRequest::Task(void *pvParameters) {
    SimulateRequest* instance = (SimulateRequest*)pvParameters;

    instance->setTaskRunning(true);

    Serial.println("[Task SimulateRequest] Starting background simulate....");

    bool result = instance->getDataMonitor()->insertSamples({
        Sample(10, esp_random() % 2000, esp_random() % 2000)
    });

    if(!result) {
        Serial.println("[Task SimulateRequest] An error occurred while inserting a simulation into the database.");
    } else {
        Serial.println("[Task SimulateRequest] Simulate completed successfully.");
    }

    instance->setTaskRunning(false);
    vTaskDelete(NULL);
}

AsyncCallbackWebHandler& SimulateRequest::onServer() {
    // Rota para simulação de inserção de dados: http://[IP-DO-ESP]/simulate
    return this->webServer->on("/simulate", HTTP_GET, [this](AsyncWebServerRequest *request) {
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
                "O processo de inserção já está em andamento. Por favor, aguarde.", 
                "/simulate"
            );

            request->send(422, "text/html", waitingPage->page());
            delete waitingPage;

            FINISH_SERVER_PROCESSING();
            return;
        }

        xTaskCreatePinnedToCore(
            SimulateRequest::Task,          // Função da task
            "simulateTask",                 // Nome
            4096,                           // Tamanho da Stack
            this,                           // Parâmetros
            1,                              // Prioridade
            NULL,                           // Handle
            1                               // Core (1 é o padrão do Arduino)
        );

        this->setTaskStarted(true);

        WaitingPage *waitingPage = new WaitingPage(
            "Simulate iniciada! A inserção estará pronto em instantes. Por favor, aguarde.",
            "/simulate"
        );

        request->send(200, "text/html", waitingPage->page());
        delete waitingPage;

        FINISH_SERVER_PROCESSING();
    });
}
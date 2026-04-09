#include <routine_monitor.h>
#include <config.h>

void IRAM_ATTR RoutineMonitor::interruptionHandler(void* arg) {
    RoutineMonitor* instance = static_cast<RoutineMonitor*>(arg);
    instance->interruptionRoutine();
}

void IRAM_ATTR RoutineMonitor::interruptionRoutine() {
    this->setInterruptionRunning(true);

    if (digitalRead(this->pInt)) {
        this->inFlow++;
    }

    if (digitalRead(this->pOut)) {
        this->outFlow++;
    }

    this->setInterruptionRunning(false);
}

void RoutineMonitor::insertionTask(void *pvParameters) {
    RoutineMonitor* instance = (RoutineMonitor*)pvParameters;
    instance->setInsertionTaskRunning(true);

    Serial.println("[Task " + String(__func__) + "] Starting insertionTask.");

    VISUAL_INDICATOR_ON();

    bool result = instance->dataMonitor->insertSamples({
        Sample(instance->step, instance->lastInFlow, instance->lastOutFlow)
    });

    if(!result) {
        Serial.println("[Task " + String(__func__) + "] An error occurred while inserting into the database.");
    } else {
        Serial.println("[Task " + String(__func__) + "] Inserted completed successfully.");
    }

    VISUAL_INDICATOR_OFF();

    instance->setInsertionTaskRunning(false);
    vTaskDelete(NULL);
}

void RoutineMonitor::cleanupTask(void *pvParameters) {
    RoutineMonitor* instance = (RoutineMonitor*)pvParameters;
    instance->setCleanupTaskRunning(true);

    Serial.println("[Task " + String(__func__) + "] Starting cleanupTask.");

    VISUAL_INDICATOR_ON();

    bool result = instance->dataMonitor->cleanup();

    if(!result) {
        Serial.println("[Task " + String(__func__) + "] An error occurred while attempting to perform an database optimized cleanup on the system.");
    } else {
        Serial.println("[Task " + String(__func__) + "] Cleaning completed successfully.");
    }
    VISUAL_INDICATOR_OFF();

    instance->setCleanupTaskRunning(false);
    vTaskDelete(NULL);
}

void RoutineMonitor::insertionRoutine(struct tm timeinfo) {
    Serial.println("[Routine " + String(__func__) + "] Starting insertionRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The insertion task is still in progress.");
        return;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The cleanup task is still in progress.");
        return;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("[Routine " + String(__func__) + "] Data Monitor is locked");
        return;
    }

    this->setInsertionRoutineRunning(true);

    VISUAL_INDICATOR_ON();

    this->lastInFlow = this->inFlow;
    this->lastOutFlow = this->outFlow;

    this->inFlow = this->outFlow = 0;

    Serial.printf("[Routine %s] (%02d/%02d/%d - %02d:%02d:%02d) InFlow {%u} OutFlow {%u}\n",
        __func__,
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        this->lastInFlow,
        this->lastOutFlow
    );

    xTaskCreatePinnedToCore(
        RoutineMonitor::insertionTask,           // Função da task
        "insertionTask",                        // Nome
        4096,                                   // Tamanho da Stack
        this,                                   // Parâmetros
        1,                                      // Prioridade
        NULL,                                   // Handle
        1                                       // Core (1 é o padrão do Arduino)
    );

    VISUAL_INDICATOR_OFF();
    this->setInsertionRoutineRunning(false);
}

void RoutineMonitor::cleanupRoutine(struct tm timeinfo) {
    Serial.println("[Routine " + String(__func__) + "] Starting cleanupRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The insertion task is still in progress.");
        return;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The cleanup task is still in progress.");
        return;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("[Routine " + String(__func__) + "] Data Monitor is locked");
        return;
    }

    this->setCleanupRoutineRunning(true);

    VISUAL_INDICATOR_ON();

    Serial.printf("[Routine %s] (%02d/%02d/%d - %02d:%02d:%02d)\n",
        __func__,
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );

    xTaskCreatePinnedToCore(
        RoutineMonitor::cleanupTask,           // Função da task
        "cleanupTask",                        // Nome
        4096,                                   // Tamanho da Stack
        this,                                   // Parâmetros
        1,                                      // Prioridade
        NULL,                                   // Handle
        1                                       // Core (1 é o padrão do Arduino)
    );

    VISUAL_INDICATOR_OFF();
    this->setCleanupRoutineRunning(false);
}

void RoutineMonitor::running() {
    struct tm timeinfo;

    // Lógica do Relógio (NTP)
    if (getLocalTime(&timeinfo)) {
        int currentHour = timeinfo.tm_hour;
        int currentMinute = timeinfo.tm_min;
        int currentSecond = timeinfo.tm_sec;

        // Chama a rotina de cleanup em um horário especifico
        if (currentHour == 7 && currentMinute == 35) {
            if (this->processedCleanup.canProcessed(currentHour, currentMinute, -1)) {
                this->processedCleanup.trigger();
                this->processedCleanup.setLastProcessed(currentHour, currentMinute, -1);
            }
        }

        // Chama a routine a cada X minutos, configurável
        if (currentMinute % this->step == 0 && currentSecond == 0) {
            if (this->processedInsertion.canProcessed(-1, currentMinute, currentSecond)) {
                this->processedInsertion.trigger();
                this->processedInsertion.setLastProcessed(-1, currentMinute, currentSecond);
            }
        }
    }

    static uint16_t __inFlow = 0;
    static uint16_t __outFlow = 0;

    if (
        this->inFlow != __inFlow ||
        this->outFlow != __outFlow
    ) {
        Serial.printf("[Interruption]: InFlow {%u} OutFlow {%u}\n", this->inFlow, this->outFlow);
        __inFlow = this->inFlow;
        __outFlow = this->outFlow;
    }

    // Executa as routine se nenhuma task e interrupção estiver rodando
    if 
    (
            !this->getInsertionTaskRunning() ||
            !this->getCleanupTaskRunning() ||
            !this->getInterruptionRunning()
    ) {
        if (!this->getInsertionRoutineRunning() && this->processedCleanup.isTrigger()) {
            this->cleanupRoutine(timeinfo);
            this->processedCleanup.resetTrigger();
        } 
        else if (!this->getCleanupRoutineRunning() && this->processedInsertion.isTrigger()) {
            this->insertionRoutine(timeinfo);
            this->processedInsertion.resetTrigger();
        }
    }
}
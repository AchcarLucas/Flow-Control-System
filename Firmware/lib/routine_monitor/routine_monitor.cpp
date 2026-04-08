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

    Serial.println("Starting insertionTask.");

    VISUAL_INDICATOR_ON();

    bool result = instance->dataMonitor->insertSamples({
        Sample(instance->step, instance->lastInFlow, instance->lastOutFlow)
    });

    if(!result) {
        Serial.println("[Task RoutineMonitor] An error occurred while inserting into the database.");
    } else {
        Serial.println("[Task RoutineMonitor] Inserted completed successfully.");
    }

    VISUAL_INDICATOR_OFF();

    instance->setInsertionTaskRunning(false);
    vTaskDelete(NULL);
}

void RoutineMonitor::cleanupTask(void *pvParameters) {
    RoutineMonitor* instance = (RoutineMonitor*)pvParameters;
    instance->setCleanupTaskRunning(true);

    Serial.println("Starting cleanupTask.");

    VISUAL_INDICATOR_ON();

    bool result = instance->dataMonitor->cleanup();

    if(!result) {
        Serial.println("[Task RoutineMonitor] An error occurred while attempting to perform an database optimized cleanup on the system.");
    } else {
        Serial.println("[Task RoutineMonitor] Cleaning completed successfully.");
    }
    VISUAL_INDICATOR_OFF();

    instance->setCleanupTaskRunning(false);
    vTaskDelete(NULL);
}

void RoutineMonitor::insertionRoutine(struct tm timeinfo) {
    this->setInsertionRoutineRunning(true);

    Serial.println("Starting insertionRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("The insertion task is still in progress.");
        return;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("The cleanup task is still in progress.");
        return;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("Data Monitor is locked");
        return;
    }

    VISUAL_INDICATOR_ON();

    this->lastInFlow = this->inFlow;
    this->lastOutFlow = this->outFlow;

    this->inFlow = this->outFlow = 0;

    Serial.printf("[InsertionRoutine]: (%02d/%02d/%d - %02d:%02d:%02d) InFlow {%u} OutFlow {%u}\n",
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
    this->setCleanupRoutineRunning(true);

    Serial.println("Starting cleanupRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("The insertion task is still in progress.");
        return;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("The cleanup task is still in progress.");
        return;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("Data Monitor is locked");
        return;
    }

    VISUAL_INDICATOR_ON();

    Serial.printf("[CleanupRoutine]: (%02d/%02d/%d - %02d:%02d:%02d)\n",
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

    static int lastMinuteProcessed = -1;
    static int lastHourProcessed = -1;

    static bool cleanupCanRun = false;
    static bool insertionCanRun = false;

    static uint16_t __inFlow = 0;
    static uint16_t __outFlow = 0;

    // Lógica do Relógio (NTP)
    if (getLocalTime(&timeinfo)) {
        int currentMinute = timeinfo.tm_min;
        int currentHour = timeinfo.tm_hour;

        // Chama a rotina de cleanup em um horário especifico
        if (currentHour == 0) {
            if (currentHour != lastHourProcessed) {
                cleanupCanRun = true;
                lastHourProcessed = currentHour;
            }
        } else {
            lastHourProcessed = -1;
        }

        // Chama a routine a cada X minutos, configurável
        if (currentMinute % this->step == 0) {
            if (currentMinute != lastMinuteProcessed) {
                insertionCanRun = true;
                lastMinuteProcessed = currentMinute;
            }
        } else {
            lastMinuteProcessed = -1;
        }
    }

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
        if (!this->getInsertionRoutineRunning() && cleanupCanRun) {
            this->cleanupRoutine(timeinfo);
            cleanupCanRun = false;
        } 
        else if (!this->getCleanupRoutineRunning() && insertionCanRun) {
            this->insertionRoutine(timeinfo);
            insertionCanRun = false;
        }
    }
}
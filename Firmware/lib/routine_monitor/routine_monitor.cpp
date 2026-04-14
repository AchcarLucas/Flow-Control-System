#include <routine_monitor.h>
#include <file_system.h>
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
    VISUAL_INDICATOR_ON();

    Serial.println("[Task " + String(__func__) + "] Starting insertionTask.");

    bool result = instance->dataMonitor->insertSamples({
        Sample(instance->step, instance->lastInFlow, instance->lastOutFlow)
    });

    if(!result) {
        Serial.println("[Task " + String(__func__) + "] An error occurred while inserting into the database.");
    } else {
        Serial.println("[Task " + String(__func__) + "] Inserted completed successfully.");
    }

    instance->setInsertionTaskRunning(false);
    VISUAL_INDICATOR_OFF();

    vTaskDelete(NULL);
}

void RoutineMonitor::cleanupTask(void *pvParameters) {
    RoutineMonitor* instance = (RoutineMonitor*)pvParameters;

    instance->setCleanupTaskRunning(true);
    VISUAL_INDICATOR_ON();

    Serial.println("[Task " + String(__func__) + "] Starting cleanupTask.");

    bool result = instance->dataMonitor->cleanup();

    if(!result) {
        Serial.println("[Task " + String(__func__) + "] An error occurred while attempting to perform an database optimized cleanup on the system.");
    } else {
        Serial.println("[Task " + String(__func__) + "] Cleaning completed successfully.");
    }

    instance->setCleanupTaskRunning(false);
    VISUAL_INDICATOR_OFF();

    vTaskDelete(NULL);
}

void RoutineMonitor::systemTask(void *pvParameters) {
    RoutineMonitor* instance = (RoutineMonitor*)pvParameters;

    instance->setSystemTaskRunning(true);
    VISUAL_INDICATOR_ON();

    Serial.println("[Task " + String(__func__) + "] Starting systemTask.");

    uint32_t freeHeap = esp_get_free_heap_size();
    uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
    uint32_t largestBlock = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    size_t flashTotal = LittleFS.totalBytes();
    size_t flashUsed = LittleFS.usedBytes();

    Serial.printf(" - freeHeap %u - minFreeHeap %u - largestBlock %u - flashTotal %u - flashUsed %u\n",
        freeHeap,
        minFreeHeap,
        largestBlock,
        flashTotal,
        flashUsed
    );

    FileSystem::getInstance().deleteFileWith(
        {
            "tmp_"
        },
        {}
    );

    std::list<std::pair<String, size_t>> _file = FileSystem::getInstance().listFile("/");

    Serial.printf("Exploring directory: /\n");

    for (auto file : _file) {
        if(file.second == -1) {
            Serial.printf(" [DIR] %s\n", file.first.c_str());
        } else {
            Serial.printf(" [FILE] %s | SIZE %u\n", file.first.c_str(), file.second);
        }
    }

    _file.clear();

    instance->setSystemTaskRunning(false);
    VISUAL_INDICATOR_OFF();

    vTaskDelete(NULL);
}

bool RoutineMonitor::insertionRoutine(struct tm timeinfo) {
    Serial.println("[Routine " + String(__func__) + "] Starting insertionRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The insertion task is still in progress.");
        return false;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The cleanup task is still in progress.");
        return false;
    }

    if (this->getSystemRoutineRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The system task is still in progress.");
        return false;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("[Routine " + String(__func__) + "] Data Monitor is locked");
        return false;
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
        RoutineMonitor::insertionTask,          // Função da task
        "insertionTask",                        // Nome
        4096,                                   // Tamanho da Stack
        this,                                   // Parâmetros
        1,                                      // Prioridade
        NULL,                                   // Handle
        1                                       // Core (1 é o padrão do Arduino)
    );

    this->setInsertionRoutineRunning(false);
    VISUAL_INDICATOR_OFF();

    return true;
}

bool RoutineMonitor::cleanupRoutine(struct tm timeinfo) {
    Serial.println("[Routine " + String(__func__) + "] Starting cleanupRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The insertion task is still in progress.");
        return false;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The cleanup task is still in progress.");
        return false;
    }

    if (this->getSystemRoutineRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The system task is still in progress.");
        return false;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("[Routine " + String(__func__) + "] Data Monitor is locked");
        return false;
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
        RoutineMonitor::cleanupTask,            // Função da task
        "cleanupTask",                          // Nome
        4096,                                   // Tamanho da Stack
        this,                                   // Parâmetros
        1,                                      // Prioridade
        NULL,                                   // Handle
        1                                       // Core (1 é o padrão do Arduino)
    );

    this->setCleanupRoutineRunning(false);
    VISUAL_INDICATOR_OFF();

    return true;
}

bool RoutineMonitor::systemRoutine(struct tm timeinfo) {
    Serial.println("[Routine " + String(__func__) + "] Starting systemRoutine.");

    if (this->getInsertionTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The insertion task is still in progress.");
        return false;
    }

    if (this->getCleanupTaskRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The cleanup task is still in progress.");
        return false;
    }

    if (this->getSystemRoutineRunning()) {
        Serial.println("[Routine " + String(__func__) + "] The system task is still in progress.");
        return false;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("[Routine " + String(__func__) + "] Data Monitor is locked");
        return false;
    }

    this->setSystemRoutineRunning(true);
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
        RoutineMonitor::systemTask,             // Função da task
        "systemTask",                           // Nome
        4096,                                   // Tamanho da Stack
        this,                                   // Parâmetros
        1,                                      // Prioridade
        NULL,                                   // Handle
        1                                       // Core (1 é o padrão do Arduino)
    );

    this->setSystemRoutineRunning(false);
    VISUAL_INDICATOR_OFF();

    return true;
}

void RoutineMonitor::running() {
    struct tm timeinfo;

    // Lógica do Relógio (NTP)
    if (getLocalTime(&timeinfo)) {
        int currentHour = timeinfo.tm_hour;
        int currentMinute = timeinfo.tm_min;
        int currentSecond = timeinfo.tm_sec;

        // Chama a rotina de cleanup em um horário especifico
        if (currentHour % 1 == 0 && currentMinute == 0 && currentSecond == 0) {
            if (this->processedCleanup.canProcessed(currentHour, currentMinute, currentSecond)) {
                this->processedCleanup.trigger();
                this->processedCleanup.setLastProcessed(currentHour, currentMinute, currentSecond);
            }
        }

        // Chama a routine a cada X minutos, configurável
        if (currentMinute % this->step == 0 && currentSecond == 0) {
            if (this->processedInsertion.canProcessed(-1, currentMinute, currentSecond)) {
                this->processedInsertion.trigger();
                this->processedInsertion.setLastProcessed(-1, currentMinute, currentSecond);
            }
        }

        if (currentMinute % 15 == 0 && currentSecond == 0) {
            if (this->processedSystem.canProcessed(-1, currentMinute, currentSecond)) {
                this->processedSystem.trigger();
                this->processedSystem.setLastProcessed(-1, currentMinute, currentSecond);
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
        !(
            this->getInsertionTaskRunning() ||
            this->getCleanupTaskRunning() ||
            this->getSystemTaskRunning() ||
            this->getInterruptionRunning()
        )
    ) {
        if (!this->getInsertionRoutineRunning() && this->processedCleanup.isTrigger()) {
            if (this->cleanupRoutine(timeinfo))
                this->processedCleanup.resetTrigger();
        } 
        else if (!this->getCleanupRoutineRunning() && this->processedInsertion.isTrigger()) {
            if (this->insertionRoutine(timeinfo))
                this->processedInsertion.resetTrigger();
        }
        else if(!this->getSystemRoutineRunning() && this->processedSystem.isTrigger()) {
            if (this->systemRoutine(timeinfo))
                this->processedSystem.resetTrigger();
        }
    }
}
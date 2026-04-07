#include <sensor_monitor.h>

void IRAM_ATTR SensorMonitor::interruptionHandler(void* arg) {
    SensorMonitor* instance = static_cast<SensorMonitor*>(arg);
    instance->interruptionSensor();
}

void IRAM_ATTR SensorMonitor::interruptionSensor() {
    this->setInterruptionRunning(true);

    /*
        if(this->getTaskRunning()) {
            Serial.println("The task is still in progress.");
        }
    */

    if (digitalRead(this->pInt)) {
        this->inFlow++;
    }

    if (digitalRead(this->pOut)) {
        this->outFlow++;
    }

    // Serial.printf("[Interruption]: InFlow {%u} OutFlow {%u}\n", this->inFlow, this->outFlow);

    this->setInterruptionRunning(false);
}

void SensorMonitor::Task(void *pvParameters) {
    SensorMonitor* instance = (SensorMonitor*)pvParameters;
    instance->setTaskRunning(true);

    bool result;

    result = instance->dataMonitor->insertSamples({
        Sample(instance->step, instance->lastInFlow, instance->lastOutFlow)
    });

    if(!result) {
        Serial.println("[Task SensorMonitor] An error occurred while inserting into the database.");
    } else {
        Serial.println("[Task SensorMonitor] Inserted completed successfully.");
    }

    /*
        result = instance->dataMonitor->cleanup();

        if(!result) {
            Serial.println("[Task SensorMonitor] An error occurred while attempting to perform an database optimized cleanup on the system.");
        } else {
            Serial.println("[Task SensorMonitor] Cleaning completed successfully.");
        }
    */

    instance->setTaskRunning(false);
    vTaskDelete(NULL);
}

void SensorMonitor::routineSensor(struct tm timeinfo) {
    Serial.println("Starting routineSensor.");

    if (this->getTaskRunning()) {
        Serial.println("The task is still in progress.");
        return;
    }

    if (this->dataMonitor->lock()) {
        Serial.println("Data Monitor is locked");
        return;
    }

    this->lastInFlow = this->inFlow;
    this->lastOutFlow = this->outFlow;

    this->inFlow = this->outFlow = 0;

    Serial.printf("[Routine]: (%02d/%02d/%d - %02d:%02d:%02d) InFlow {%u} OutFlow {%u}\n",
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
        SensorMonitor::Task,          // Função da task
        "sensorMonitorTask",          // Nome
        4096,                         // Tamanho da Stack
        this,                         // Parâmetros
        1,                            // Prioridade
        NULL,                         // Handle
        1                             // Core (1 é o padrão do Arduino)
    );
}

void SensorMonitor::running() {
    struct tm timeinfo;

    static int lastMinutesProcessed = -1;
    static uint16_t __inFlow = 0;
    static uint16_t __outFlow = 0;

    // 1. Lógica do Relógio (NTP)
    if (getLocalTime(&timeinfo)) {
        int currentMinutes = timeinfo.tm_min;

        // Chama a routine a cada STEP minutos, configurável
        if ((currentMinutes % this->step) == 0) {
            if (currentMinutes != lastMinutesProcessed) {
                this->setRoutineRunning(true);
                lastMinutesProcessed = currentMinutes;
            }
        } else {
            lastMinutesProcessed = -1;
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

    // acessa o método routineSensor se tiver no tempo correto e não tiver task rodando
    if (
        this->getRoutineRunning() &&
        !(
            this->getTaskRunning() ||
            this->getInterruptionRunning()
        )
    ) {
        this->setRoutineRunning(false);
        this->routineSensor(timeinfo);
    }
}
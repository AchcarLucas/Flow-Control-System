#include <sensor_monitor.h>

void IRAM_ATTR SensorMonitor::interruptionHandler(void* arg) {
    SensorMonitor* instance = static_cast<SensorMonitor*>(arg);
    instance->interruptionSensor();
}

void IRAM_ATTR SensorMonitor::interruptionSensor() {
    if (digitalRead(this->pInt)) {
        this->inFlow++;
    }

    if (digitalRead(this->pOut)) {
        this->outFlow++;
    }

    Serial.printf("[Interruption]: InFlow {%u} OutFlow {%u}\n", this->inFlow, this->outFlow);
}

void SensorMonitor::routineSensor(struct tm timeinfo) {
    uint16_t _inFlow = this->inFlow;
    uint16_t _outFlow = this->outFlow;

    this->inFlow = this->outFlow = 0;

    Serial.printf("[Routine]: (%02d/%02d/%d - %02d:%02d:%02d) InFlow {%u} OutFlow {%u}\n",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        _inFlow,
        _outFlow
    );
}

void SensorMonitor::runningSensor() {
    struct tm timeinfo;

    static int lastMinutesProcessed = -1;

    // 1. Lógica do Relógio (NTP)
    if (getLocalTime(&timeinfo)) {
        int currentMinutes = timeinfo.tm_min;

        // Chama a routine a cada 10 minutos
        if (currentMinutes % 1 == 0) {
            if (currentMinutes != lastMinutesProcessed) {
                this->routineSensor(timeinfo);
                lastMinutesProcessed = currentMinutes;
            }
        } else {
            lastMinutesProcessed = -1;
        }
    }
}
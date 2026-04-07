#ifndef SENSOR_MONITOR_H
#define SENSOR_MONITOR_H

#include <data_monitor.h>
#include <config.h>

class SensorMonitor {
    protected:
        DataMonitor *dataMonitor;
        int pInt, pOut;
        volatile uint16_t inFlow, outFlow;

    private:
    public:
        SensorMonitor(DataMonitor *dataMonitor, const int pIn, const int pOut, const int pInterruption) : 
            dataMonitor(dataMonitor), inFlow(0), outFlow(0) {
                attachInterruptArg(digitalPinToInterrupt(pInterruption), this->interruptionHandler, this, FALLING);
                this->pInt = pIn;
                this->pOut = pOut;
        };

        static void IRAM_ATTR interruptionHandler(void* arg);
        void IRAM_ATTR interruptionSensor();

        void routineSensor(struct tm);
        void runningSensor();
};

#endif
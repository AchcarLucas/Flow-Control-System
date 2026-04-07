#ifndef SENSOR_MONITOR_H
#define SENSOR_MONITOR_H

#include <data_monitor.h>
#include <config.h>

class SensorMonitor {
    protected:
        DataMonitor *dataMonitor;

        int pInt, pOut;
        uint16_t step;

        bool taskRunning;
        bool routineRunning;

        volatile bool interruptionRunning;
    private:
        volatile uint16_t inFlow, outFlow;
        u_int16_t lastInFlow, lastOutFlow;
    public:
        SensorMonitor(DataMonitor *dataMonitor, const int pIn, const int pOut, const int pInterruption, uint16_t step = 10) : 
            dataMonitor(dataMonitor), inFlow(0), outFlow(0), step(step), taskRunning(false), routineRunning(false) {
                attachInterruptArg(digitalPinToInterrupt(pInterruption), this->interruptionHandler, this, FALLING);
                this->pInt = pIn;
                this->pOut = pOut;
        };

        void setStep(uint16_t step) { this->step = step; }

        void setTaskRunning(bool taskRunning) { this->taskRunning = taskRunning; }
        bool getTaskRunning() { return this->taskRunning; }

        void setRoutineRunning(bool routineRunning) { this->routineRunning = routineRunning; }
        bool getRoutineRunning() { return this->routineRunning; }

        void setInterruptionRunning(bool interruptionRunning) { this->interruptionRunning = interruptionRunning; }
        bool getInterruptionRunning() { return this->interruptionRunning; }

        static void IRAM_ATTR interruptionHandler(void* arg);
        static void Task(void *pvParameters);

        void IRAM_ATTR interruptionSensor();

        void routineSensor(struct tm);
        void running();
};

#endif
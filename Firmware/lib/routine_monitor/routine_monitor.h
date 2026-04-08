#ifndef ROUTINE_MONITOR_H
#define ROUTINE_MONITOR_H

#include <data_monitor.h>
#include <config.h>

class RoutineMonitor {
    protected:
        DataMonitor *dataMonitor;

        int pInt, pOut;
        uint16_t step;

        volatile bool interruptionRunning;

        bool insertionTaskRunning;
        bool insertionRoutineRunning;

        bool cleanupTaskRunning;
        bool cleanupRoutineRunning;
    private:
        volatile uint16_t inFlow, outFlow;
        u_int16_t lastInFlow, lastOutFlow;
    public:
        RoutineMonitor(DataMonitor *dataMonitor, const int pIn, const int pOut, const int pInterruption, uint16_t step = 10) : 
            dataMonitor(dataMonitor),
            inFlow(0),
            outFlow(0),
            step(step),
            interruptionRunning(false),
            insertionTaskRunning(false),
            cleanupTaskRunning(false),
            insertionRoutineRunning(false),
            cleanupRoutineRunning(false)

        {
                attachInterruptArg(digitalPinToInterrupt(pInterruption), this->interruptionHandler, this, FALLING);
                this->pInt = pIn;
                this->pOut = pOut;
        };

        static void IRAM_ATTR interruptionHandler(void* arg);
        void IRAM_ATTR interruptionRoutine();

        void setInterruptionRunning(bool interruptionRunning) { this->interruptionRunning = interruptionRunning; }
        bool getInterruptionRunning() { return this->interruptionRunning; }

        // INSERTION
        void setInsertionTaskRunning(bool insertionTaskRunning) { this->insertionTaskRunning = insertionTaskRunning; }
        bool getInsertionTaskRunning() { return this->insertionTaskRunning; }

        void setInsertionRoutineRunning(bool insertionRoutineRunning) { this->insertionRoutineRunning = insertionRoutineRunning; }
        bool getInsertionRoutineRunning() { return this->insertionRoutineRunning; }

        // CLEANUP
        void setCleanupTaskRunning(bool cleanupTaskRunning) { this->cleanupTaskRunning = cleanupTaskRunning; }
        bool getCleanupTaskRunning() { return this->cleanupTaskRunning; }

        void setCleanupRoutineRunning(bool cleanupRoutineRunning) { this->cleanupRoutineRunning = cleanupRoutineRunning; }
        bool getCleanupRoutineRunning() { return this->cleanupRoutineRunning; }

        
        static void insertionTask(void *pvParameters);
        void insertionRoutine(struct tm);

        static void cleanupTask(void *pvParameters);
        void cleanupRoutine(struct tm);

        void running();
};

#endif
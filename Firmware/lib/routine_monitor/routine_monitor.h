#ifndef ROUTINE_MONITOR_H
#define ROUTINE_MONITOR_H

#include <data_monitor.h>
#include <config.h>

class ProcessedRoutine {
    private:
        int lastHourProcessed;
        int lastMinuteProcessed;
        int lastSecondProcessed;
        bool __trigger;
    public:
        ProcessedRoutine() :
            lastHourProcessed(-1), lastMinuteProcessed(-1), lastSecondProcessed(-1) { }
        void setLastHourProcessed(int lastHourProcessed) { this->lastHourProcessed = lastHourProcessed; }
        void setLastMinuteProcessed(int lastMinuteProcessed) { this->lastMinuteProcessed = lastMinuteProcessed; }
        void setLastSecondProcessed(int lastSecondProcessed) { this->lastSecondProcessed = lastSecondProcessed; }

        int getLastHourProcessed() { return this->lastHourProcessed; }
        int getLastMinuteProcessed() { return this->lastMinuteProcessed; }
        int getLastSecondProcessed() { return this->lastSecondProcessed; }

        void setLastProcessed(int hour, int minute, int second) {
            this->lastHourProcessed = hour;
            this->lastMinuteProcessed = minute;
            this->lastSecondProcessed = second;
        }

        bool canProcessed(int hour, int minute, int second) {
            return  (this->lastHourProcessed != hour) ||
                    (this->lastMinuteProcessed != minute) ||
                    (this->lastSecondProcessed != second);
        }

        void trigger(bool _trigger = true) { this->__trigger = _trigger; }

        bool isTrigger() { return this->__trigger; }

        void resetLastProcessed() {
            this->lastHourProcessed = this->lastMinuteProcessed = this->lastSecondProcessed = -1;
            this->__trigger = false;
        }

        void resetTrigger() {
            this->trigger(false);
        }
};

class RoutineMonitor {
    protected:
        DataMonitor *dataMonitor;

        ProcessedRoutine processedCleanup;
        ProcessedRoutine processedInsertion;
        ProcessedRoutine processedSystem;

        int pInt, pOut;
        uint16_t step;

        volatile bool interruptionRunning;

        bool insertionTaskRunning;
        bool insertionRoutineRunning;

        bool cleanupTaskRunning;
        bool cleanupRoutineRunning;

        bool systemTaskRunning;
        bool systemRoutineRunning;
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

        // SYSTEM
        void setSystemTaskRunning(bool systemTaskRunning) { this->systemTaskRunning = systemTaskRunning; }
        bool getSystemTaskRunning() { return this->systemTaskRunning; }

        void setSystemRoutineRunning(bool systemRoutineRunning) { this->systemRoutineRunning = systemRoutineRunning; }
        bool getSystemRoutineRunning() { return this->systemRoutineRunning; }

        static void insertionTask(void *pvParameters);
        bool insertionRoutine(struct tm);

        static void cleanupTask(void *pvParameters);
        bool cleanupRoutine(struct tm);

        static void systemTask(void *pvParameters);
        bool systemRoutine(struct tm);

        void running();
};

#endif
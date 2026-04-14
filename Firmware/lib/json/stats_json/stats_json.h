#ifndef STATS_JSON_H
#define STATS_JSON_H

#include <Arduino.h>
#include <json.h>

#include <data_monitor.h>

class StatsJson : public JSON {
    private:
        uint32_t
                freeHeap,
                minFreeHeap,
                largestBlock,
                flashTotal,
                flashUsed;
        ulong uptimeMillis;
    public:
        StatsJson(
            uint32_t freeHeap,
            uint32_t minFreeHeap,
            uint32_t largestBlock,
            uint32_t flashTotal,
            uint32_t flashUsed,
            ulong uptimeMillis
        ) : 
            freeHeap(freeHeap),
            minFreeHeap(minFreeHeap),
            largestBlock(largestBlock),
            flashTotal(flashTotal),
            flashUsed(flashUsed),
            uptimeMillis(uptimeMillis) {}
        virtual String serialize() override;
};

#endif
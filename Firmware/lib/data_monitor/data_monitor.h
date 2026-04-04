#ifndef DATA_MONITOR_H
#define DATA_MONITOR_H

#include <string>
#include <cstdint>

#include <dao_sqlite.h>

struct Sample {
    uint32_t id;                // sample 
    std::string timestamp;         // datetime timestamp
    uint16_t sampling_time;     // sampling time in second
    uint16_t in;                // input sample 
    uint16_t out;               // output sample
    Sample() = default;
    Sample(
        uint32_t id,
        std::string timestamp,
        uint16_t sampling_time, 
        uint16_t in, 
        uint16_t out
    ) : id(id), 
        timestamp(timestamp), 
        sampling_time(sampling_time),
        in(in),
        out(out) {}
    Sample(
        uint16_t sampling_time, 
        uint16_t in, 
        uint16_t out) 
            : id(0), 
            timestamp(std::string()),
            sampling_time(sampling_time), 
            in(in), 
            out(out) {}
};

class DataMonitor {
    protected:
        SQLiteDAO *dao;
        std::string fileName;
        std::string cleaningTime;
    private:
    public:
        DataMonitor(const std::string fileName="database.db", std::string cleaningTime="-3 months");
        ~DataMonitor();

        bool insertSamples(std::list<Sample>);
        std::list<Sample> selectSamples(uint16_t limit = 100);

        bool removeSamplesByID(uint32_t id);
        bool removeSamplesByTimestamp(uint64_t timestamp);

        bool cleanup(std::string cleaningTime=std::string());
        bool reset();
};

#endif
#ifndef SAMPLE_JSON_H
#define SAMPLE_JSON_H

#include <Arduino.h>
#include <json.h>

#include <data_monitor.h>

class SampleJson : public JSON {
    private:
        std::list<Sample> samples;
    public:
        SampleJson(std::list<Sample> samples) : samples(samples) {}
        virtual String serialize() override;
};

#endif
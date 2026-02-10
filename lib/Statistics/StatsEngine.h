#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H

#include <Arduino.h>
#include <vector>

class StatsEngine {
private:
    std::vector<float> _buffer;
    size_t _windowSize;
    size_t _index;
    bool _bufferFull;
    float _sum;
    float _sqSum;

    void updateSums(float oldVal, float newVal);

    float _rateOfChange;
    unsigned long _tickRateMs;

public:
    StatsEngine(size_t windowSize, unsigned long tickRateMs = 500);
    void addSample(float value);
    float getMean();
    float getStdDev();
    float getZScore(float value);
    float getRateOfChange();   // Returns units/sec rise rate
    float getOldestSample();   // Returns oldest value in buffer
    bool isBaselineReady();
};

#endif

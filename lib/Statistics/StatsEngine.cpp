#include "StatsEngine.h"

StatsEngine::StatsEngine(size_t windowSize, unsigned long tickRateMs) {
    _windowSize = windowSize;
    _buffer.resize(windowSize, 0.0);
    _index = 0;
    _bufferFull = false;
    _sum = 0.0;
    _sqSum = 0.0;
    _rateOfChange = 0.0;
    _tickRateMs = tickRateMs;
}

void StatsEngine::addSample(float value) {
    // Compute rate of change before overwriting oldest sample
    if (_bufferFull) {
        float oldest = _buffer[_index]; // About to be overwritten
        float windowDurationSec = (_windowSize * _tickRateMs) / 1000.0f;
        if (windowDurationSec > 0) {
            _rateOfChange = (value - oldest) / windowDurationSec;
        }
    }

    _buffer[_index] = value;
    _index = (_index + 1) % _windowSize;
    if (_index == 0) _bufferFull = true;
}

float StatsEngine::getMean() {
    if (!_bufferFull && _index == 0) return 0.0;
    
    size_t count = _bufferFull ? _windowSize : _index;
    float sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += _buffer[i];
    }
    return sum / count;
}

float StatsEngine::getStdDev() {
    float mean = getMean();
    size_t count = _bufferFull ? _windowSize : _index;
    if (count < 2) return 0.0; // Avoid div by zero

    float varianceSum = 0.0;
    for (size_t i = 0; i < count; i++) {
        float diff = _buffer[i] - mean;
        varianceSum += diff * diff;
    }
    return sqrt(varianceSum / count);
}

float StatsEngine::getZScore(float value) {
    float mean = getMean();
    float stdDev = getStdDev();
    if (stdDev < 0.1) stdDev = 0.1; // Avoid division by zero/noise
    return (value - mean) / stdDev;
}

float StatsEngine::getRateOfChange() {
    return _rateOfChange;
}

float StatsEngine::getOldestSample() {
    if (_bufferFull) {
        return _buffer[_index]; // Next to be overwritten = oldest
    } else if (_index > 0) {
        return _buffer[0];
    }
    return 0.0;
}

bool StatsEngine::isBaselineReady() {
    return _bufferFull;
}

#include "StatsEngine.h"

StatsEngine::StatsEngine(size_t windowSize) {
    _windowSize = windowSize;
    _buffer.resize(windowSize, 0.0);
    _index = 0;
    _bufferFull = false;
    _sum = 0.0;
    _sqSum = 0.0;
}

// O(1) rolling update of sum and squared sum is precise enough for this application
// But floating point errors can accumulate over time.
// For robustness, we'll recompute full sums occasionally or just accept drift?
// Better: circular buffer for "window" approach, recompute mean/stddev when needed?
// No, calculating mean/stddev every loop on 600 items is fast on ESP32 (240MHz).
// Let's optimize: maintain sum, but recompute sqSum?
// Actually, simple O(N) calculation on demand is safest against drift.
// 600 iterations is ~3-4 microseconds on ESP32. Totally fine.

void StatsEngine::addSample(float value) {
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

bool StatsEngine::isBaselineReady() {
    return _bufferFull;
}

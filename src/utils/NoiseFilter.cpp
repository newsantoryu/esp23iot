#include "NoiseFilter.h"
#include "Logger.h"

NoiseFilter::NoiseFilter() {
    for (int i = 0; i < AMP_HISTORY_SIZE; i++) {
        ampHistory[i] = 0;
    }
}

bool NoiseFilter::isSignalActive(float amplitude, float normalized) {
    // Atualiza histórico de amplitude
    ampHistory[ampIndex] = amplitude;
    ampIndex = (ampIndex + 1) % AMP_HISTORY_SIZE;
    
    // Atualiza noise floor
    updateNoiseFloor(amplitude);
    
    // Obtém amplitude média filtrada
    float filteredAmp = getAverageAmplitude();
    
    // Aplica threshold com histerese
    bool shouldBeActive = normalized > activityThreshold;
    
    if (shouldBeActive != currentState) {
        // Aplica histerese para mudanças de estado
        if (shouldBeActive) {
            currentState = normalized > (activityThreshold + hysteresis);
        } else {
            currentState = normalized < (activityThreshold - hysteresis);
        }
    }
    
    Logger::audio("Amp: " + String(amplitude) + 
                  " Norm: " + String(normalized) + 
                  " Filtered: " + String(filteredAmp) + 
                  " Active: " + String(currentState ? "YES" : "NO") +
                  " NoiseFloor: " + String(noiseFloor));
    
    return currentState;
}

float NoiseFilter::getFilteredAmplitude() {
    return getAverageAmplitude();
}

void NoiseFilter::setActivityThreshold(float threshold) {
    activityThreshold = threshold;
    Logger::info("Activity threshold set to: " + String(threshold));
}

void NoiseFilter::setHysteresis(float hysteresisValue) {
    hysteresis = hysteresisValue;
    Logger::info("Hysteresis set to: " + String(hysteresisValue));
}

bool NoiseFilter::getActiveState() {
    return currentState;
}

float NoiseFilter::getNoiseFloor() {
    return noiseFloor;
}

void NoiseFilter::updateNoiseFloor(float amplitude) {
    // Atualiza noise floor lentamente
    if (amplitude < noiseFloor * 2) {
        noiseFloor += noiseFloorAlpha * (amplitude - noiseFloor);
    }
}

float NoiseFilter::getAverageAmplitude() {
    float sum = 0;
    int count = 0;
    
    for (int i = 0; i < AMP_HISTORY_SIZE; i++) {
        if (ampHistory[i] > 0) {
            sum += ampHistory[i];
            count++;
        }
    }
    
    return count > 0 ? sum / count : 0;
}

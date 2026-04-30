#pragma once
#include <Arduino.h>

class NoiseFilter {
public:
    NoiseFilter();
    
    // Filtragem de amplitude
    bool isSignalActive(float amplitude, float normalized);
    float getFilteredAmplitude();
    
    // Configuração de thresholds
    void setActivityThreshold(float threshold);
    void setHysteresis(float hysteresis);
    
    // Estado atual
    bool getActiveState();
    float getNoiseFloor();
    
private:
    float activityThreshold = 0.35;  // Aumentado de 0.25
    float hysteresis = 0.05;         // Histerese para evitar flicker
    bool currentState = false;
    
    // Filtro de amplitude média
    static const int AMP_HISTORY_SIZE = 10;
    float ampHistory[AMP_HISTORY_SIZE];
    int ampIndex = 0;
    
    // Detecção de noise floor
    float noiseFloor = 0;
    float noiseFloorAlpha = 0.001;
    
    void updateNoiseFloor(float amplitude);
    float getAverageAmplitude();
};

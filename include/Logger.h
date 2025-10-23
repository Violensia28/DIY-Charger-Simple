#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <Wire.h>
#include <INA226_WE.h>
#include "Config.h"
#include "BatteryTypes.h"

// ============================================
// LOGGER CLASS
// ============================================

class BatteryLogger {
private:
    INA226_WE ina226[NUM_PORTS];
    PortData* portData;
    
    // Filter buffers for median filtering
    float voltageBuffer[NUM_PORTS][FILTER_SAMPLES];
    float currentBuffer[NUM_PORTS][FILTER_SAMPLES];
    int bufferIndex[NUM_PORTS];
    
    // Timing
    unsigned long lastSampleTime;
    
    // Helper functions
    float medianFilter(float* buffer, int size);
    void updateAccumulators(int port, float voltage, float current, unsigned long deltaTime);
    bool validateReading(int port, float voltage, float current);

public:
    BatteryLogger(PortData* data);
    bool begin();
    bool initPort(int port);
    void update();
    void updatePort(int port);
    bool isPortReady(int port);
    void calibratePort(int port);
    
    // CSV logging
    String getCSVHeader();
    String getCSVLine(int port);
};

#endif // LOGGER_H

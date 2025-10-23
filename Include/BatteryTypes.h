#ifndef BATTERY_TYPES_H
#define BATTERY_TYPES_H

#include <Arduino.h>

// ============================================
// ENUMERATIONS
// ============================================

enum BatteryType {
    LIION = 0,
    LIFEPO4 = 1,
    LIPO = 2
};

enum OperationMode {
    SAFETY = 0,
    CHARGING = 1,
    DISCHARGING = 2
};

enum PortStatus {
    IDLE = 0,
    ACTIVE = 1,
    COMPLETE = 2,
    ERROR = 3
};

// ============================================
// BATTERY CONFIGURATION STRUCT
// ============================================

struct BatteryConfig {
    float cutoffVoltage;
    float maxVoltage;
    float nominalVoltage;
    const char* name;
};

const BatteryConfig BATTERY_CONFIGS[] = {
    {3.0f, 4.2f, 3.7f, "Li-ion"},      // LIION
    {2.5f, 3.65f, 3.2f, "LiFePO4"},    // LIFEPO4
    {3.0f, 4.2f, 3.7f, "LiPo"}         // LIPO
};

// ============================================
// PORT DATA STRUCTURE
// ============================================

struct PortData {
    // Measurements
    float voltage;
    float current;
    float mAh;
    float Wh;
    float power;
    
    // Configuration
    OperationMode mode;
    BatteryType batteryType;
    float customCutoff;
    bool useCustomCutoff;
    
    // Status
    PortStatus status;
    bool active;
    unsigned long startTime;
    unsigned long lastUpdate;
    
    // Error tracking
    int errorCount;
    char errorMsg[64];
    
    // Constructor with defaults
    PortData() : 
        voltage(0), current(0), mAh(0), Wh(0), power(0),
        mode(SAFETY), batteryType(LIION), 
        customCutoff(3.0), useCustomCutoff(false),
        status(IDLE), active(false), 
        startTime(0), lastUpdate(0),
        errorCount(0) {
        errorMsg[0] = '\0';
    }
    
    // Get effective cutoff voltage
    float getCutoffVoltage() const {
        if (useCustomCutoff) {
            return customCutoff;
        }
        return BATTERY_CONFIGS[batteryType].cutoffVoltage;
    }
    
    // Get battery type name
    const char* getBatteryName() const {
        return BATTERY_CONFIGS[batteryType].name;
    }
    
    // Get mode name
    const char* getModeName() const {
        switch(mode) {
            case CHARGING: return "Charging";
            case DISCHARGING: return "Discharging";
            case SAFETY: return "Safety";
            default: return "Unknown";
        }
    }
    
    // Get status name
    const char* getStatusName() const {
        switch(status) {
            case IDLE: return "Idle";
            case ACTIVE: return "Active";
            case COMPLETE: return "Complete";
            case ERROR: return "Error";
            default: return "Unknown";
        }
    }
    
    // Check if discharge should stop
    bool shouldStopDischarge() const {
        if (mode != DISCHARGING) return false;
        return voltage <= getCutoffVoltage();
    }
    
    // Reset accumulated data
    void reset() {
        mAh = 0;
        Wh = 0;
        startTime = millis();
        errorCount = 0;
        errorMsg[0] = '\0';
        status = IDLE;
    }
};

#endif // BATTERY_TYPES_H

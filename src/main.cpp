#include <Arduino.h>
#include "Config.h"
#include "BatteryTypes.h"
#include "Logger.h"
#include "WebUI.h"
#include "UI.h"

// ============================================
// GLOBAL OBJECTS
// ============================================

PortData portData[NUM_PORTS];
BatteryLogger* logger;
WebUI* webUI;
PhysicalUI* physicalUI;

// ============================================
// MOSFET CONTROL
// ============================================

void initMOSFETs() {
    for (int i = 0; i < NUM_PORTS; i++) {
        pinMode(MOSFET_PINS[i], OUTPUT);
        digitalWrite(MOSFET_PINS[i], LOW); // OFF by default
    }
    DEBUG_PRINTLN("MOSFETs initialized");
}

void updateMOSFETs() {
    for (int i = 0; i < NUM_PORTS; i++) {
        bool shouldBeOn = false;
        PortStatus previousStatus = portData[i].status;
        
        // MOSFET ON only during discharge mode
        if (portData[i].mode == DISCHARGING && portData[i].active) {
            // ✅ FIX: Use the new method with hysteresis
            if (!portData[i].shouldStopDischarge()) {
                shouldBeOn = true;
                portData[i].status = ACTIVE;
            } else {
                // Discharge complete
                portData[i].status = COMPLETE;
                portData[i].active = false;
                DEBUG_PRINTF("Port %d: Discharge complete (%.3fV)\n", i, portData[i].voltage);
                
                // Notify UI if status changed
                if (previousStatus != COMPLETE) {
                    physicalUI->notifyComplete(i);
                }
            }
        }
        
        // Charging mode - MOSFET should be OFF
        if (portData[i].mode == CHARGING && portData[i].active) {
            shouldBeOn = false;
            
            // ✅ FIX: More realistic charging detection (10% tolerance and lower current threshold)
            float maxV = BATTERY_CONFIGS[portData[i].batteryType].maxVoltage;
            if (portData[i].voltage >= maxV - 0.1 && abs(portData[i].current) < 0.05) {
                portData[i].status = COMPLETE;
                portData[i].active = false;
                DEBUG_PRINTF("Port %d: Charging complete (%.3fV)\n", i, portData[i].voltage);
                
                if (previousStatus != COMPLETE) {
                    physicalUI->notifyComplete(i);
                }
            } else {
                portData[i].status = ACTIVE;
            }
        }
        
        // Safety mode - everything OFF
        if (portData[i].mode == SAFETY) {
            shouldBeOn = false;
            portData[i].active = false;
            portData[i].status = IDLE;
        }
        
        // ✅ FIX: Better safety checks with different error messages
        // Safety check - critical low voltage
        if (portData[i].voltage < MIN_VOLTAGE && portData[i].voltage > 0.1) {
            shouldBeOn = false;
            if (portData[i].status != ERROR) {
                portData[i].status = ERROR;
                portData[i].active = false;
                snprintf(portData[i].errorMsg, 64, "Voltage critically low: %.2fV", portData[i].voltage);
                physicalUI->notifyError(i);
                DEBUG_PRINTF("Port %d: ERROR - %s\n", i, portData[i].errorMsg);
            }
        }
        
        // Safety check - overvoltage
        if (portData[i].voltage > MAX_VOLTAGE) {
            shouldBeOn = false;
            if (portData[i].status != ERROR) {
                portData[i].status = ERROR;
                portData[i].active = false;
                snprintf(portData[i].errorMsg, 64, "Overvoltage: %.2fV", portData[i].voltage);
                physicalUI->notifyError(i);
                DEBUG_PRINTF("Port %d: ERROR - %s\n", i, portData[i].errorMsg);
            }
        }
        
        // ✅ FIX: Safety check - overcurrent
        if (abs(portData[i].current) > MAX_DISCHARGE_CURRENT) {
            shouldBeOn = false;
            if (portData[i].status != ERROR) {
                portData[i].status = ERROR;
                portData[i].active = false;
                snprintf(portData[i].errorMsg, 64, "Overcurrent: %.2fA", portData[i].current);
                physicalUI->notifyError(i);
                DEBUG_PRINTF("Port %d: ERROR - %s\n", i, portData[i].errorMsg);
            }
        }
        
        // Apply MOSFET state
        digitalWrite(MOSFET_PINS[i], shouldBeOn ? HIGH : LOW);
    }
}

// ============================================
// SYSTEM MONITORING
// ============================================

void printSystemStatus() {
    static unsigned long lastPrint = 0;
    
    if (millis() - lastPrint < 10000) return; // Print every 10 seconds
    lastPrint = millis();
    
    DEBUG_PRINTLN("\n===== System Status =====");
    DEBUG_PRINTF("Uptime: %lu seconds\n", millis() / 1000);
    DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
    DEBUG_PRINTF("WiFi clients: %d\n", WiFi.softAPgetStationNum());
    
    for (int i = 0; i < NUM_PORTS; i++) {
        DEBUG_PRINTF("\nPort %d: %s\n", i, portData[i].getStatusName());
        DEBUG_PRINTF("  Mode: %s\n", portData[i].getModeName());
        DEBUG_PRINTF("  Battery: %s\n", portData[i].getBatteryName());
        DEBUG_PRINTF("  Voltage: %.3fV\n", portData[i].voltage);
        DEBUG_PRINTF("  Current: %.3fA\n", portData[i].current);
        DEBUG_PRINTF("  Power: %.2fW\n", portData[i].power);
        DEBUG_PRINTF("  Capacity: %.1f mAh\n", portData[i].mAh);
        DEBUG_PRINTF("  Energy: %.2f Wh\n", portData[i].Wh);
        DEBUG_PRINTF("  Cutoff: %.1fV\n", portData[i].getCutoffVoltage());
        
        if (portData[i].errorMsg[0] != '\0') {
            DEBUG_PRINTF("  Error: %s\n", portData[i].errorMsg);
        }
    }
    DEBUG_PRINTLN("========================\n");
}

// ============================================
// SYSTEM SYNCHRONIZATION
// ============================================

void syncUIStates() {
    // This function ensures Web UI and Physical UI stay in sync
    // Called periodically in main loop
    
    static unsigned long lastSync = 0;
    if (millis() - lastSync < 1000) return;
    lastSync = millis();
    
    // Physical UI will force redraw if any state changed
    static PortStatus lastStatus[NUM_PORTS] = {IDLE, IDLE, IDLE, IDLE};
    
    for (int i = 0; i < NUM_PORTS; i++) {
        if (portData[i].status != lastStatus[i]) {
            physicalUI->forceRedraw();
            lastStatus[i] = portData[i].status;
        }
    }
}

// ============================================
// SETUP
// ============================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    DEBUG_PRINTLN("\n\n");
    DEBUG_PRINTLN("=====================================");
    DEBUG_PRINTLN("  DIY Charger Simple v2.0");
    DEBUG_PRINTLN("  ESP32 Battery Sorting System");
    DEBUG_PRINTLN("  with OLED + Rotary Encoder UI");
    DEBUG_PRINTLN("=====================================\n");
    
    // Initialize hardware
    DEBUG_PRINTLN("Initializing hardware...");
    
    // Initialize MOSFETs first (safety)
    initMOSFETs();
    
    // Initialize all ports to safety mode
    for (int i = 0; i < NUM_PORTS; i++) {
        portData[i].mode = SAFETY;
        portData[i].batteryType = LIION;
        portData[i].active = false;
        portData[i].status = IDLE;
        portData[i].customCutoff = LIION_CUTOFF;
        portData[i].useCustomCutoff = false;
    }
    
    // Initialize Physical UI (OLED + Encoder + Buzzer)
    DEBUG_PRINTLN("Initializing Physical UI...");
    physicalUI = new PhysicalUI(portData);
    if (!physicalUI->begin()) {
        DEBUG_PRINTLN("WARNING: Physical UI failed to initialize");
    } else {
        DEBUG_PRINTLN("Physical UI ready!");
    }
    
    delay(500);
    
    // Initialize Logger (INA226)
    DEBUG_PRINTLN("Initializing Logger (INA226)...");
    logger = new BatteryLogger(portData);
    if (!logger->begin()) {
        DEBUG_PRINTLN("WARNING: Some INA226 sensors failed");
    } else {
        DEBUG_PRINTLN("Logger ready!");
    }
    
    delay(500);
    
    // Initialize Web UI (WiFi AP + HTTP Server)
    DEBUG_PRINTLN("Initializing Web UI...");
    webUI = new WebUI(portData);
    if (!webUI->begin()) {
        DEBUG_PRINTLN("ERROR: Web UI failed to start");
    } else {
        DEBUG_PRINTLN("\nWeb UI started successfully!");
        DEBUG_PRINT("  SSID: ");
        DEBUG_PRINTLN(AP_SSID);
        DEBUG_PRINT("  Password: ");
        DEBUG_PRINTLN(AP_PASSWORD);
        DEBUG_PRINT("  IP Address: http://");
        DEBUG_PRINTLN(WiFi.softAPIP());
        DEBUG_PRINTLN("  Open browser to access dashboard");
    }
    
    DEBUG_PRINTLN("\n=====================================");
    DEBUG_PRINTLN("System ready!");
    DEBUG_PRINTLN("=====================================");
    DEBUG_PRINTLN("\nControls:");
    DEBUG_PRINTLN("  - Rotate encoder: Navigate menu");
    DEBUG_PRINTLN("  - Press encoder: Select/Confirm");
    DEBUG_PRINTLN("  - Web UI: http://192.168.4.1");
    DEBUG_PRINTLN("=====================================\n");
}

// ============================================
// MAIN LOOP
// ============================================

void loop() {
    // Update measurements from INA226
    logger->update();
    
    // Update MOSFET states based on mode and voltage
    updateMOSFETs();
    
    // Update Physical UI (OLED + Encoder + Buzzer)
    physicalUI->update();
    
    // Update Web UI (WebSocket broadcasts)
    webUI->update();
    
    // Sync UI states
    syncUIStates();
    
    // Print status to serial (debug)
    printSystemStatus();
    
    // Small delay to prevent watchdog timeout
    delay(10);
}

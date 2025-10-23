#include "Logger.h"

// ============================================
// CONSTRUCTOR
// ============================================

BatteryLogger::BatteryLogger(PortData* data) {
    portData = data;
    lastSampleTime = 0;
    
    // Initialize buffers
    for (int i = 0; i < NUM_PORTS; i++) {
        bufferIndex[i] = 0;
        for (int j = 0; j < FILTER_SAMPLES; j++) {
            voltageBuffer[i][j] = 0;
            currentBuffer[i][j] = 0;
        }
    }
}

// ============================================
// INITIALIZATION
// ============================================

bool BatteryLogger::begin() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);
    
    DEBUG_PRINTLN("Initializing INA226 sensors...");
    
    bool allOk = true;
    for (int i = 0; i < NUM_PORTS; i++) {
        if (!initPort(i)) {
            allOk = false;
        }
    }
    
    return allOk;
}

bool BatteryLogger::initPort(int port) {
    if (port < 0 || port >= NUM_PORTS) return false;
    
    if (!ina226[port].begin(INA226_ADDR[port], &Wire)) {
        DEBUG_PRINTF("Port %d: INA226 not found at 0x%02X\n", port, INA226_ADDR[port]);
        portData[port].status = ERROR;
        snprintf(portData[port].errorMsg, 64, "Sensor not found");
        return false;
    }
    
    // Configure INA226
    ina226[port].setAveragingCount(INA226_COUNT_16);
    ina226[port].setConversionTime(INA226_TIME_1100us);
    
    // Calibrate for 0.1 ohm shunt, 3.2A max
    ina226[port].calibrate(SHUNT_RESISTOR, MAX_CURRENT);
    
    DEBUG_PRINTF("Port %d: INA226 initialized (0x%02X)\n", port, INA226_ADDR[port]);
    return true;
}

// ============================================
// UPDATE FUNCTIONS
// ============================================

void BatteryLogger::update() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastSampleTime < SAMPLE_INTERVAL_MS) {
        return;
    }
    
    unsigned long deltaTime = currentTime - lastSampleTime;
    lastSampleTime = currentTime;
    
    // Update all active ports
    for (int i = 0; i < NUM_PORTS; i++) {
        if (portData[i].active) {
            updatePort(i);
        }
    }
}

void BatteryLogger::updatePort(int port) {
    if (port < 0 || port >= NUM_PORTS) return;
    if (!isPortReady(port)) return;
    
    // Read raw values from INA226
    float rawVoltage = ina226[port].getBusVoltage();
    float rawCurrent = ina226[port].getCurrent_mA() / 1000.0; // Convert to A
    
    // Validate readings
    if (!validateReading(port, rawVoltage, rawCurrent)) {
        portData[port].errorCount++;
        if (portData[port].errorCount > 10) {
            portData[port].status = ERROR;
            snprintf(portData[port].errorMsg, 64, "Invalid readings");
        }
        return;
    }
    
    portData[port].errorCount = 0;
    
    // Add to filter buffer
    int idx = bufferIndex[port];
    voltageBuffer[port][idx] = rawVoltage;
    currentBuffer[port][idx] = rawCurrent;
    bufferIndex[port] = (idx + 1) % FILTER_SAMPLES;
    
    // Apply median filter
    float filteredVoltage = medianFilter(voltageBuffer[port], FILTER_SAMPLES);
    float filteredCurrent = medianFilter(currentBuffer[port], FILTER_SAMPLES);
    
    // Update port data
    portData[port].voltage = filteredVoltage;
    portData[port].current = filteredCurrent;
    portData[port].power = filteredVoltage * filteredCurrent;
    
    // ✅ FIX: Initialize lastUpdate on first run
    if (portData[port].lastUpdate == 0) {
        portData[port].lastUpdate = millis();
        return; // Skip accumulator update on first sample
    }
    
    // Update accumulators (mAh and Wh)
    unsigned long deltaTime = millis() - portData[port].lastUpdate;
    
    // ✅ FIX: Sanity check for deltaTime (max 10 seconds)
    if (deltaTime > 10000) {
        DEBUG_PRINTF("Port %d: Warning - deltaTime too large (%lu ms), resetting\n", port, deltaTime);
        portData[port].lastUpdate = millis();
        return;
    }
    
    updateAccumulators(port, filteredVoltage, filteredCurrent, deltaTime);
    
    portData[port].lastUpdate = millis();
    
    #if DEBUG_LOGGER
    if (millis() % 5000 < 100) { // Print every 5 seconds
        DEBUG_PRINTF("Port %d: %.3fV %.3fA %.1fmAh %.2fWh\n", 
                    port, filteredVoltage, filteredCurrent, 
                    portData[port].mAh, portData[port].Wh);
    }
    #endif
}

// ============================================
// HELPER FUNCTIONS
// ============================================

float BatteryLogger::medianFilter(float* buffer, int size) {
    // ✅ FIX: Use selection sort (faster for small arrays)
    float sorted[FILTER_SAMPLES];
    memcpy(sorted, buffer, size * sizeof(float));
    
    // Selection sort (O(n²) but fast for small n)
    for (int i = 0; i < size - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < size; j++) {
            if (sorted[j] < sorted[minIdx]) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            float temp = sorted[i];
            sorted[i] = sorted[minIdx];
            sorted[minIdx] = temp;
        }
    }
    
    // Return median
    return sorted[size / 2];
}

void BatteryLogger::updateAccumulators(int port, float voltage, float current, unsigned long deltaTime) {
    if (deltaTime == 0) return;
    
    // Convert deltaTime to hours
    float deltaHours = deltaTime / 3600000.0;
    
    // ✅ FIX: Sanity check - ignore unrealistic values
    if (deltaHours > 0.1) { // More than 6 minutes = error
        DEBUG_PRINTF("Port %d: deltaHours too large (%.4f), skipping accumulation\n", port, deltaHours);
        return;
    }
    
    // Update mAh (current in A, so multiply by 1000)
    portData[port].mAh += (current * 1000.0) * deltaHours;
    
    // Update Wh
    portData[port].Wh += (voltage * current) * deltaHours;
    
    // ✅ FIX: Prevent negative accumulation
    if (portData[port].mAh < 0) portData[port].mAh = 0;
    if (portData[port].Wh < 0) portData[port].Wh = 0;
}

bool BatteryLogger::validateReading(int port, float voltage, float current) {
    // Check for reasonable voltage range
    if (voltage < MIN_VOLTAGE || voltage > MAX_VOLTAGE) {
        DEBUG_PRINTF("Port %d: Invalid voltage %.3fV\n", port, voltage);
        return false;
    }
    
    // Check for reasonable current range (can be negative for charging)
    if (abs(current) > MAX_DISCHARGE_CURRENT) {
        DEBUG_PRINTF("Port %d: Invalid current %.3fA\n", port, current);
        return false;
    }
    
    // ✅ FIX: Check for NaN or Inf
    if (isnan(voltage) || isinf(voltage) || isnan(current) || isinf(current)) {
        DEBUG_PRINTF("Port %d: NaN/Inf detected\n", port);
        return false;
    }
    
    return true;
}

bool BatteryLogger::isPortReady(int port) {
    if (port < 0 || port >= NUM_PORTS) return false;
    return portData[port].status != ERROR;
}

void BatteryLogger::calibratePort(int port) {
    if (port < 0 || port >= NUM_PORTS) return;
    
    ina226[port].calibrate(SHUNT_RESISTOR, MAX_CURRENT);
    DEBUG_PRINTF("Port %d: Calibrated\n", port);
}

// ============================================
// CSV LOGGING
// ============================================

String BatteryLogger::getCSVHeader() {
    return "Timestamp,Port,Voltage(V),Current(A),Power(W),mAh,Wh,Mode,Battery,Status\n";
}

String BatteryLogger::getCSVLine(int port) {
    if (port < 0 || port >= NUM_PORTS) return "";
    
    char line[256];
    unsigned long timestamp = millis() - portData[port].startTime;
    
    snprintf(line, 256, "%lu,%d,%.3f,%.3f,%.3f,%.1f,%.2f,%s,%s,%s\n",
             timestamp / 1000, // seconds
             port,
             portData[port].voltage,
             portData[port].current,
             portData[port].power,
             portData[port].mAh,
             portData[port].Wh,
             portData[port].getModeName(),
             portData[port].getBatteryName(),
             portData[port].getStatusName());
    
    return String(line);
}

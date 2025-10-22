/*
 * I2C Scanner for DIY Charger Simple
 * 
 * This utility scans the I2C bus and reports all detected devices.
 * Upload this before main firmware to verify hardware connections.
 * 
 * Expected devices:
 * - 0x3C or 0x3D: OLED SSD1306
 * - 0x40: INA226 Port 1
 * - 0x41: INA226 Port 2
 * - 0x42: INA226 Port 3
 * - 0x43: INA226 Port 4
 */

#include <Arduino.h>
#include <Wire.h>

// Pin definitions (must match Config.h)
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 400000

// Device name lookup
const char* getDeviceName(uint8_t address) {
    switch(address) {
        case 0x3C:
        case 0x3D:
            return "OLED SSD1306";
        case 0x40:
            return "INA226 Port 1 (A1=GND, A0=GND)";
        case 0x41:
            return "INA226 Port 2 (A1=GND, A0=VS+)";
        case 0x42:
            return "INA226 Port 3 (A1=VS+, A0=GND)";
        case 0x43:
            return "INA226 Port 4 (A1=VS+, A0=VS+)";
        case 0x68:
            return "MPU6050 / DS1307 RTC";
        case 0x76:
        case 0x77:
            return "BMP280 / BME280";
        default:
            return "Unknown device";
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n\n========================================");
    Serial.println("   I2C Scanner - DIY Charger Simple");
    Serial.println("========================================\n");
    
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_FREQ);
    
    Serial.printf("I2C initialized:\n");
    Serial.printf("  SDA = GPIO%d\n", I2C_SDA);
    Serial.printf("  SCL = GPIO%d\n", I2C_SCL);
    Serial.printf("  Frequency = %d Hz\n\n", I2C_FREQ);
}

void loop() {
    byte error, address;
    int devicesFound = 0;
    
    Serial.println("Scanning I2C bus...\n");
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("✓ Device found at 0x%02X - %s\n", 
                         address, getDeviceName(address));
            devicesFound++;
        }
        else if (error == 4) {
            Serial.printf("✗ Error at address 0x%02X\n", address);
        }
    }
    
    Serial.println("\n----------------------------------------");
    
    if (devicesFound == 0) {
        Serial.println("❌ No I2C devices found!");
        Serial.println("\nTroubleshooting:");
        Serial.println("  1. Check SDA/SCL wiring");
        Serial.println("  2. Verify 4.7kΩ pull-up resistors");
        Serial.println("  3. Check device power (3.3V or 5V)");
        Serial.println("  4. Try lower frequency (100kHz)");
    }
    else {
        Serial.printf("✓ Found %d device(s)\n", devicesFound);
        
        // Check for expected devices
        Serial.println("\nExpected devices checklist:");
        
        // OLED
        Wire.beginTransmission(0x3C);
        if (Wire.endTransmission() == 0) {
            Serial.println("  ✓ OLED Display (0x3C)");
        } else {
            Wire.beginTransmission(0x3D);
            if (Wire.endTransmission() == 0) {
                Serial.println("  ✓ OLED Display (0x3D)");
            } else {
                Serial.println("  ✗ OLED Display NOT FOUND");
            }
        }
        
        // INA226 sensors
        for (int i = 0; i < 4; i++) {
            uint8_t addr = 0x40 + i;
            Wire.beginTransmission(addr);
            if (Wire.endTransmission() == 0) {
                Serial.printf("  ✓ INA226 Port %d (0x%02X)\n", i + 1, addr);
            } else {
                Serial.printf("  ✗ INA226 Port %d NOT FOUND (0x%02X)\n", i + 1, addr);
            }
        }
    }
    
    Serial.println("\n========================================");
    Serial.println("Scan complete. Scanning again in 5s...\n");
    
    delay(5000);
}

/*
 * Expected Output (all devices connected):
 * 
 * ========================================
 *    I2C Scanner - DIY Charger Simple
 * ========================================
 * 
 * I2C initialized:
 *   SDA = GPIO21
 *   SCL = GPIO22
 *   Frequency = 400000 Hz
 * 
 * Scanning I2C bus...
 * 
 * ✓ Device found at 0x3C - OLED SSD1306
 * ✓ Device found at 0x40 - INA226 Port 1 (A1=GND, A0=GND)
 * ✓ Device found at 0x41 - INA226 Port 2 (A1=GND, A0=VS+)
 * ✓ Device found at 0x42 - INA226 Port 3 (A1=VS+, A0=GND)
 * ✓ Device found at 0x43 - INA226 Port 4 (A1=VS+, A0=VS+)
 * 
 * ----------------------------------------
 * ✓ Found 5 device(s)
 * 
 * Expected devices checklist:
 *   ✓ OLED Display (0x3C)
 *   ✓ INA226 Port 1 (0x40)
 *   ✓ INA226 Port 2 (0x41)
 *   ✓ INA226 Port 3 (0x42)
 *   ✓ INA226 Port 4 (0x43)
 * 
 * ========================================
 * Scan complete. Scanning again in 5s...
 */

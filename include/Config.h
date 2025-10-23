#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// HARDWARE CONFIGURATION - LOCKED GPIO MAPPING
// ============================================

// Number of battery ports
#define NUM_PORTS 4

// MOSFET Control Pins (for discharge) - LOCKED
const int MOSFET_PINS[NUM_PORTS] = {26, 14, 12, 13};

// INA226 I2C Addresses (0x40, 0x41, 0x42, 0x43)
const uint8_t INA226_ADDR[NUM_PORTS] = {0x40, 0x41, 0x42, 0x43};

// I2C Configuration (shared by INA226 and OLED) - LOCKED
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 400000

// Rotary Encoder Pins - LOCKED
#define ENCODER_CLK 32
#define ENCODER_DT 33
#define ENCODER_SW 25

// Buzzer Pin - LOCKED
#define BUZZER_PIN 27

// OLED Configuration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
#define OLED_RESET -1

// ============================================
// LOGGER CONFIGURATION
// ============================================

// Sampling rate (2Hz = 500ms interval)
#define SAMPLE_INTERVAL_MS 500

// Median filter samples
#define FILTER_SAMPLES 5

// INA226 Calibration
#define SHUNT_RESISTOR 0.1  // 100mOhm
#define MAX_CURRENT 3.2     // 3.2A max

// ============================================
// BATTERY CONFIGURATION
// ============================================

// Default cutoff voltages (can be overridden)
#define LIION_CUTOFF 3.0
#define LIFEPO4_CUTOFF 2.5
#define LIPO_CUTOFF 3.0

// Safety limits
#define MIN_VOLTAGE 2.0
#define MAX_VOLTAGE 4.5
#define MAX_DISCHARGE_CURRENT 3.0

// ============================================
// WEB SERVER CONFIGURATION
// ============================================

// WiFi AP Configuration
#define AP_SSID "DIY-Charger"
#define AP_PASSWORD "charger123"
#define AP_CHANNEL 6
#define AP_MAX_CLIENTS 4

// Web server port
#define WEB_PORT 80

// WebSocket update interval (ms)
#define WS_UPDATE_INTERVAL 1000

// ============================================
// UI CONFIGURATION
// ============================================

// OLED refresh rate (ms)
#define UI_REFRESH_INTERVAL 200

// Encoder debounce (ms)
#define ENCODER_DEBOUNCE 50

// Menu timeout (ms) - return to main screen
#define MENU_TIMEOUT 30000

// Buzzer tones (Hz) and durations (ms)
#define BUZZER_FREQ_MENU 2000
#define BUZZER_FREQ_SELECT 2500
#define BUZZER_FREQ_COMPLETE 3000
#define BUZZER_FREQ_ERROR 1500
#define BUZZER_DURATION_SHORT 50
#define BUZZER_DURATION_LONG 200
#define BUZZER_DURATION_COMPLETE 500

// ============================================
// STORAGE CONFIGURATION
// ============================================

// CSV log file path
#define LOG_PATH "/logs"
#define MAX_LOG_SIZE 1048576  // 1MB per file

// ============================================
// DEBUG CONFIGURATION
// ============================================

#define DEBUG_SERIAL 1
#define DEBUG_LOGGER 1
#define DEBUG_WEBUI 1
#define DEBUG_UI 1

#if DEBUG_SERIAL
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

#endif // CONFIG_H

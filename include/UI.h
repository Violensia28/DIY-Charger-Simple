#ifndef UI_H
#define UI_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "BatteryTypes.h"

// ============================================
// ENUMERATIONS
// ============================================

enum MenuState {
    MENU_MAIN = 0,      // Main status display
    MENU_PORT_SELECT,   // Select which port to configure
    MENU_MODE_SELECT,   // Select mode for port
    MENU_BATTERY_SELECT,// Select battery type
    MENU_CUTOFF_ADJUST, // Adjust cutoff voltage
    MENU_CONFIRM        // Confirm action
};

enum BuzzerPattern {
    BEEP_NONE = 0,
    BEEP_MENU,          // Short beep for menu navigation
    BEEP_SELECT,        // Medium beep for selection
    BEEP_COMPLETE,      // Long beep for task complete
    BEEP_ERROR          // Error beep
};

// ============================================
// UI CLASS
// ============================================

class PhysicalUI {
private:
    Adafruit_SSD1306* display;
    PortData* portData;
    
    // Menu state
    MenuState currentMenu;
    int selectedPort;
    int menuIndex;
    int maxMenuIndex;
    unsigned long lastMenuActivity;
    
    // Encoder state
    volatile int encoderPos;
    int lastEncoderPos;
    volatile bool buttonPressed;
    unsigned long lastButtonPress;
    
    // Display state
    unsigned long lastRefresh;
    bool displayNeedsUpdate;
    
    // Buzzer state
    BuzzerPattern currentBeep;
    unsigned long beepStartTime;
    bool beepActive;
    
    // Static callbacks for ISR
    static PhysicalUI* instance;
    static void IRAM_ATTR handleEncoderA();
    static void IRAM_ATTR handleEncoderB();
    static void IRAM_ATTR handleButton();
    
    // Drawing functions
    void drawMainScreen();
    void drawPortSelect();
    void drawModeSelect();
    void drawBatterySelect();
    void drawCutoffAdjust();
    void drawConfirm();
    
    // Helper functions
    void drawHeader(const char* title);
    void drawPortStatus(int port, int y);
    void drawProgressBar(int x, int y, int width, int height, float percentage);
    void drawBattery(int x, int y, float voltage, float maxVoltage);
    
    // Menu navigation
    void handleEncoderChange();
    void handleButtonPress();
    void returnToMain();
    
    // Buzzer control
    void playBeep(BuzzerPattern pattern);
    void updateBuzzer();
    
public:
    PhysicalUI(PortData* data);
    
    bool begin();
    void update();
    
    // Public methods for external triggers
    void notifyComplete(int port);
    void notifyError(int port);
    void forceRedraw();
    
    // Encoder position access
    int getEncoderPosition() { return encoderPos; }
    void setEncoderPosition(int pos) { encoderPos = pos; }
};

#endif // UI_H

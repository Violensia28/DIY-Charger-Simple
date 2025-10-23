#include "UI.h"

// Static instance for ISR
PhysicalUI* PhysicalUI::instance = nullptr;

// ============================================
// CONSTRUCTOR
// ============================================

PhysicalUI::PhysicalUI(PortData* data) {
    portData = data;
    display = new Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
    
    currentMenu = MENU_MAIN;
    selectedPort = 0;
    menuIndex = 0;
    maxMenuIndex = 0;
    lastMenuActivity = 0;
    
    encoderPos = 0;
    lastEncoderPos = 0;
    buttonPressed = false;
    lastButtonPress = 0;
    
    lastRefresh = 0;
    displayNeedsUpdate = true;
    
    currentBeep = BEEP_NONE;
    beepStartTime = 0;
    beepActive = false;
    
    instance = this;
}

// ============================================
// INITIALIZATION
// ============================================

bool PhysicalUI::begin() {
    // Initialize OLED
    if (!display->begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        DEBUG_PRINTLN("ERROR: SSD1306 OLED not found");
        return false;
    }
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("DIY Charger v2.0");
    display->println("Initializing...");
    display->display();
    
    // Initialize Rotary Encoder
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleEncoderA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_DT), handleEncoderB, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handleButton, FALLING);
    
    // Initialize Buzzer
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    
    // Play startup beep
    playBeep(BEEP_SELECT);
    
    DEBUG_PRINTLN("Physical UI initialized");
    return true;
}

// ============================================
// ISR HANDLERS (✅ FIXED - Minimal processing in ISR)
// ============================================

void IRAM_ATTR PhysicalUI::handleEncoderA() {
    if (instance) {
        int clk = digitalRead(ENCODER_CLK);
        int dt = digitalRead(ENCODER_DT);
        
        if (clk == dt) {
            instance->encoderPos++;
        } else {
            instance->encoderPos--;
        }
    }
}

void IRAM_ATTR PhysicalUI::handleEncoderB() {
    // Handled by handleEncoderA
}

// ✅ FIX: Simplified ISR - only set flag, no millis() call
void IRAM_ATTR PhysicalUI::handleButton() {
    if (instance) {
        instance->buttonPressed = true;
    }
}

// ============================================
// UPDATE LOOP
// ============================================

void PhysicalUI::update() {
    unsigned long currentTime = millis();
    
    // Update buzzer
    updateBuzzer();
    
    // Check for encoder changes
    if (encoderPos != lastEncoderPos) {
        handleEncoderChange();
        lastEncoderPos = encoderPos;
        lastMenuActivity = currentTime;
        displayNeedsUpdate = true;
    }
    
    // ✅ FIX: Handle button press with debouncing in main loop
    if (buttonPressed) {
        // Debounce check
        if (currentTime - lastButtonPress > ENCODER_DEBOUNCE) {
            buttonPressed = false;
            handleButtonPress();
            lastButtonPress = currentTime;
            lastMenuActivity = currentTime;
            displayNeedsUpdate = true;
        } else {
            // Ignore bounce
            buttonPressed = false;
        }
    }
    
    // Menu timeout - return to main
    if (currentMenu != MENU_MAIN && 
        currentTime - lastMenuActivity > MENU_TIMEOUT) {
        returnToMain();
        displayNeedsUpdate = true;
    }
    
    // Refresh display
    if (displayNeedsUpdate || currentTime - lastRefresh > UI_REFRESH_INTERVAL) {
        switch (currentMenu) {
            case MENU_MAIN:
                drawMainScreen();
                break;
            case MENU_PORT_SELECT:
                drawPortSelect();
                break;
            case MENU_MODE_SELECT:
                drawModeSelect();
                break;
            case MENU_BATTERY_SELECT:
                drawBatterySelect();
                break;
            case MENU_CUTOFF_ADJUST:
                drawCutoffAdjust();
                break;
            case MENU_CONFIRM:
                drawConfirm();
                break;
        }
        
        display->display();
        lastRefresh = currentTime;
        displayNeedsUpdate = false;
    }
}

// ============================================
// MENU NAVIGATION
// ============================================

void PhysicalUI::handleEncoderChange() {
    int delta = encoderPos - lastEncoderPos;
    
    menuIndex += delta;
    
    // Wrap around
    if (menuIndex < 0) menuIndex = maxMenuIndex;
    if (menuIndex > maxMenuIndex) menuIndex = 0;
    
    playBeep(BEEP_MENU);
    
    #if DEBUG_UI
    DEBUG_PRINTF("Menu: %d, Index: %d/%d\n", currentMenu, menuIndex, maxMenuIndex);
    #endif
}

void PhysicalUI::handleButtonPress() {
    playBeep(BEEP_SELECT);
    
    switch (currentMenu) {
        case MENU_MAIN:
            // Enter port selection
            currentMenu = MENU_PORT_SELECT;
            menuIndex = 0;
            maxMenuIndex = NUM_PORTS - 1;
            break;
            
        case MENU_PORT_SELECT:
            // Select port and go to mode selection
            selectedPort = menuIndex;
            currentMenu = MENU_MODE_SELECT;
            menuIndex = portData[selectedPort].mode;
            maxMenuIndex = 2; // SAFETY, CHARGING, DISCHARGING
            break;
            
        case MENU_MODE_SELECT:
            // Set mode and go to battery selection
            portData[selectedPort].mode = (OperationMode)menuIndex;
            currentMenu = MENU_BATTERY_SELECT;
            menuIndex = portData[selectedPort].batteryType;
            maxMenuIndex = 2; // LIION, LIFEPO4, LIPO
            break;
            
        case MENU_BATTERY_SELECT:
            // Set battery type and go to cutoff adjust
            portData[selectedPort].batteryType = (BatteryType)menuIndex;
            currentMenu = MENU_CUTOFF_ADJUST;
            menuIndex = (int)(portData[selectedPort].customCutoff * 10); // 2.5V = 25
            maxMenuIndex = 35; // 2.0V to 3.5V
            break;
            
        case MENU_CUTOFF_ADJUST:
            // Set cutoff and confirm
            portData[selectedPort].customCutoff = menuIndex / 10.0;
            portData[selectedPort].useCustomCutoff = true;
            currentMenu = MENU_CONFIRM;
            menuIndex = 0;
            maxMenuIndex = 1; // Yes/No
            break;
            
        case MENU_CONFIRM:
            if (menuIndex == 0) {
                // Confirm - activate port
                portData[selectedPort].active = true;
                portData[selectedPort].reset();
                portData[selectedPort].startTime = millis();
                playBeep(BEEP_COMPLETE);
            }
            returnToMain();
            break;
    }
}

void PhysicalUI::returnToMain() {
    currentMenu = MENU_MAIN;
    menuIndex = 0;
    maxMenuIndex = NUM_PORTS - 1;
}

// ============================================
// DRAWING FUNCTIONS
// ============================================

void PhysicalUI::drawMainScreen() {
    display->clearDisplay();
    
    // Header
    drawHeader("DIY Charger v2.0");
    
    // Draw all ports (2x2 grid)
    for (int i = 0; i < NUM_PORTS; i++) {
        int x = (i % 2) * 64;
        int y = 16 + (i / 2) * 24;
        drawPortStatus(i, y);
    }
    
    // Footer
    display->setCursor(0, 56);
    display->setTextSize(1);
    display->print("Press to config");
}

void PhysicalUI::drawPortSelect() {
    display->clearDisplay();
    drawHeader("Select Port");
    
    for (int i = 0; i < NUM_PORTS; i++) {
        int y = 16 + i * 12;
        
        if (i == menuIndex) {
            display->fillRect(0, y, 128, 10, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        } else {
            display->setTextColor(SSD1306_WHITE);
        }
        
        display->setCursor(4, y + 1);
        display->print("Port ");
        display->print(i + 1);
        display->print(": ");
        display->print(portData[i].voltage, 2);
        display->print("V");
    }
}

void PhysicalUI::drawModeSelect() {
    display->clearDisplay();
    drawHeader("Select Mode");
    
    const char* modes[] = {"Safety", "Charging", "Discharging"};
    
    for (int i = 0; i <= 2; i++) {
        int y = 20 + i * 14;
        
        if (i == menuIndex) {
            display->fillRect(0, y, 128, 12, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        } else {
            display->setTextColor(SSD1306_WHITE);
        }
        
        display->setCursor(4, y + 2);
        display->print(modes[i]);
    }
    
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 56);
    display->print("Port ");
    display->print(selectedPort + 1);
}

void PhysicalUI::drawBatterySelect() {
    display->clearDisplay();
    drawHeader("Battery Type");
    
    const char* types[] = {"Li-ion (3.0V)", "LiFePO4 (2.5V)", "LiPo (3.0V)"};
    
    for (int i = 0; i <= 2; i++) {
        int y = 20 + i * 14;
        
        if (i == menuIndex) {
            display->fillRect(0, y, 128, 12, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        } else {
            display->setTextColor(SSD1306_WHITE);
        }
        
        display->setCursor(4, y + 2);
        display->print(types[i]);
    }
}

void PhysicalUI::drawCutoffAdjust() {
    display->clearDisplay();
    drawHeader("Cutoff Voltage");
    
    float voltage = menuIndex / 10.0;
    
    display->setTextSize(2);
    display->setCursor(30, 25);
    display->print(voltage, 1);
    display->print(" V");
    
    display->setTextSize(1);
    display->setCursor(20, 45);
    display->print("Range: 2.0-3.5V");
}

void PhysicalUI::drawConfirm() {
    display->clearDisplay();
    drawHeader("Confirm Settings");
    
    display->setTextSize(1);
    display->setCursor(0, 16);
    display->print("Port: ");
    display->println(selectedPort + 1);
    display->print("Mode: ");
    display->println(portData[selectedPort].getModeName());
    display->print("Battery: ");
    display->println(portData[selectedPort].getBatteryName());
    display->print("Cutoff: ");
    display->print(portData[selectedPort].customCutoff, 1);
    display->println("V");
    
    int y = 48;
    if (menuIndex == 0) {
        display->fillRect(0, y, 60, 12, SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK);
    }
    display->setCursor(15, y + 2);
    display->print("START");
    
    display->setTextColor(SSD1306_WHITE);
    if (menuIndex == 1) {
        display->fillRect(68, y, 60, 12, SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK);
    }
    display->setCursor(78, y + 2);
    display->print("CANCEL");
}

// ============================================
// HELPER DRAWING FUNCTIONS (✅ FIXED - Added bounds checking)
// ============================================

void PhysicalUI::drawHeader(const char* title) {
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display->setTextColor(SSD1306_BLACK);
    display->setCursor(2, 2);
    display->print(title);
    display->setTextColor(SSD1306_WHITE);
}

void PhysicalUI::drawPortStatus(int port, int y) {
    // ✅ FIX: Bounds checking
    if (port < 0 || port >= NUM_PORTS) return;
    if (y < 0 || y > 58) return;
    
    display->setTextSize(1);
    display->setCursor(0, y);
    display->print("P");
    display->print(port + 1);
    display->print(":");
    
    // Battery icon
    float maxV = BATTERY_CONFIGS[portData[port].batteryType].maxVoltage;
    drawBattery(18, y, portData[port].voltage, maxV);
    
    // Voltage
    display->setCursor(32, y);
    display->print(portData[port].voltage, 2);
    display->print("V");
    
    // Status indicator
    if (portData[port].active) {
        display->fillCircle(60, y + 4, 2, SSD1306_WHITE);
    }
}

void PhysicalUI::drawBattery(int x, int y, float voltage, float maxVoltage) {
    int width = 10;
    int height = 6;
    
    display->drawRect(x, y, width, height, SSD1306_WHITE);
    display->drawRect(x + width, y + 1, 2, 4, SSD1306_WHITE);
    
    float percentage = (voltage / maxVoltage) * 100;
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    
    int fillWidth = (width - 2) * percentage / 100;
    if (fillWidth > 0) {
        display->fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void PhysicalUI::drawProgressBar(int x, int y, int width, int height, float percentage) {
    display->drawRect(x, y, width, height, SSD1306_WHITE);
    
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    
    int fillWidth = (width - 2) * percentage / 100;
    if (fillWidth > 0) {
        display->fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

// ============================================
// BUZZER CONTROL
// ============================================

void PhysicalUI::playBeep(BuzzerPattern pattern) {
    currentBeep = pattern;
    beepStartTime = millis();
    beepActive = true;
    
    int freq = BUZZER_FREQ_MENU;
    
    switch (pattern) {
        case BEEP_MENU:
            freq = BUZZER_FREQ_MENU;
            break;
        case BEEP_SELECT:
            freq = BUZZER_FREQ_SELECT;
            break;
        case BEEP_COMPLETE:
            freq = BUZZER_FREQ_COMPLETE;
            break;
        case BEEP_ERROR:
            freq = BUZZER_FREQ_ERROR;
            break;
        default:
            break;
    }
    
    tone(BUZZER_PIN, freq);
}

void PhysicalUI::updateBuzzer() {
    if (!beepActive) return;
    
    unsigned long elapsed = millis() - beepStartTime;
    int duration = BUZZER_DURATION_SHORT;
    
    switch (currentBeep) {
        case BEEP_MENU:
            duration = BUZZER_DURATION_SHORT;
            break;
        case BEEP_SELECT:
            duration = BUZZER_DURATION_LONG;
            break;
        case BEEP_COMPLETE:
            duration = BUZZER_DURATION_COMPLETE;
            break;
        case BEEP_ERROR:
            duration = BUZZER_DURATION_LONG;
            break;
        default:
            break;
    }
    
    if (elapsed >= duration) {
        noTone(BUZZER_PIN);
        beepActive = false;
        currentBeep = BEEP_NONE;
    }
}

// ============================================
// PUBLIC NOTIFICATION METHODS
// ============================================

void PhysicalUI::notifyComplete(int port) {
    playBeep(BEEP_COMPLETE);
    displayNeedsUpdate = true;
}

void PhysicalUI::notifyError(int port) {
    playBeep(BEEP_ERROR);
    displayNeedsUpdate = true;
}

void PhysicalUI::forceRedraw() {
    displayNeedsUpdate = true;
}

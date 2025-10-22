# ⚡ Quick Start Guide - DIY Charger Simple v2.0

Get up and running in 15 minutes!

---

## 📦 What You Need

### Minimum Setup
- ✅ ESP32 DevKit V1
- ✅ 1x TP4056 module
- ✅ 1x INA226 module  
- ✅ 1x MOSFET IRLR7843
- ✅ 1x Resistor 7.5Ω 5W
- ✅ OLED SSD1306 128x64
- ✅ Rotary Encoder KY-040
- ✅ Active Buzzer 5V
- ✅ 12V 2A power supply
- ✅ Jumper wires

### Tools
- Soldering iron
- Multimeter
- USB cable (ESP32 programming)
- Computer with PlatformIO

---

## 🔌 Quick Wiring (Single Port Test)

### Step 1: I2C Bus Setup

```
ESP32          OLED + INA226
GPIO21 (SDA) ─── SDA (all devices)
GPIO22 (SCL) ─── SCL (all devices)
3.3V ────────── VCC
GND ─────────── GND
```

**Important:** Add 4.7kΩ pull-up resistors on SDA and SCL to 3.3V

### Step 2: MOSFET Circuit (Port 1)

```
ESP32 GPIO26 ─── 1kΩ ─── MOSFET Gate
                         MOSFET Drain ─── Load 7.5Ω ─── GND
                         MOSFET Source ─── GND
```

### Step 3: INA226 Connection

```
12V+ ─── INA226 IN+ ─── TP4056 IN+ ─── Battery+ ─── MOSFET Drain
GND ──── INA226 IN- ─── TP4056 GND ─── Battery- ─── MOSFET Source
```

### Step 4: Controls

```
ESP32          Encoder
GPIO32 ─────── CLK
GPIO33 ─────── DT
GPIO25 ─────── SW (Button)
3.3V ───────── VCC
GND ────────── GND

ESP32 GPIO27 ─── Buzzer+ (via transistor)
GND ──────────── Buzzer-
```

---

## 💻 Software Flash

### 1. Install PlatformIO (5 minutes)

```bash
# VSCode method (recommended)
code --install-extension platformio.platformio-ide

# Or CLI method
pip install platformio
```

### 2. Clone & Build (3 minutes)

```bash
git clone https://github.com/YOUR_USERNAME/DIY-Charger-Simple.git
cd DIY-Charger-Simple
pio run
```

### 3. Upload (2 minutes)

```bash
# Connect ESP32 via USB
pio run --target upload

# Monitor serial output
pio device monitor -b 115200
```

Expected output:
```
=====================================
  DIY Charger Simple v2.0
  ESP32 Battery Sorting System
  with OLED + Rotary Encoder UI
=====================================

Initializing hardware...
MOSFETs initialized
Physical UI initialized
Logger ready!
Web UI started successfully!
  SSID: DIY-Charger
  IP Address: http://192.168.4.1

System ready!
```

---

## 🎮 First Test

### Test 1: OLED Display

**Expected:** OLED shows main screen with 4 ports

```
┌──────────────────────────┐
│ DIY Charger v2.0         │
├──────────────────────────┤
│ P1: [▓▓▓] 4.15V ●        │
│ P2: [   ] 0.00V          │
│ P3: [▓▓ ] 3.98V ●        │
│ P4: [   ] 0.00V          │
│ [Press to config]        │
└──────────────────────────┘
```

**If blank:**
- Check I2C wiring (SDA/SCL)
- Run I2C scanner
- Verify OLED address (0x3C)

### Test 2: Rotary Encoder

**Action:** Rotate encoder
**Expected:** Menu beep sound

**Action:** Press button
**Expected:** Enter port selection menu

**If not working:**
- Check encoder wiring
- Verify pull-up resistors
- Test with multimeter

### Test 3: Web UI

**Steps:**
1. Connect WiFi to "DIY-Charger" (password: charger123)
2. Open http://192.168.4.1
3. See 4 port cards with live data

**If can't connect:**
- Check Serial Monitor for IP
- Restart ESP32
- Try different browser

### Test 4: INA226 Readings

**Connect battery to Port 1**
**Expected:** Voltage reading on OLED and Web UI

**If shows 0.00V:**
- Check INA226 I2C address (0x40)
- Verify wiring
- Run I2C scanner

### Test 5: Discharge Test

**Steps via OLED:**
1. Press encoder button → Port Select
2. Select Port 1
3. Select Mode: Discharging
4. Select Battery: Li-ion
5. Set Cutoff: 3.0V
6. Confirm → START

**Expected:**
- MOSFET turns ON
- Load resistor gets warm
- Voltage decreases
- mAh counter increases
- Stops at 3.0V cutoff

**Safety:** Monitor load resistor temperature!

---

## 🐛 Quick Troubleshooting

### Problem: OLED shows garbage

**Solution:**
```cpp
// In Config.h, try lower I2C speed
#define I2C_FREQ 100000  // Change from 400000
```

### Problem: Encoder too sensitive

**Solution:**
```cpp
// In Config.h, increase debounce
#define ENCODER_DEBOUNCE 100  // Change from 50
```

### Problem: Discharge won't start

**Check:**
- Mode set to "Discharging"
- Port marked as "Active"
- Battery voltage > cutoff
- MOSFET wiring correct

### Problem: Readings jumping

**Solution:**
- Check all ground connections
- Verify shunt resistor in INA226
- Increase filter samples in Config.h

---

## 📝 Next Steps

### Expand to 4 Ports

1. Add 3 more TP4056 modules
2. Add 3 more INA226 (addresses 0x41, 0x42, 0x43)
3. Add 3 more MOSFETs (GPIO 14, 12, 13)
4. Add 3 more load resistors

### Add Enclosure

1. Design or download 3D printable case
2. Add ventilation holes for resistors
3. Mount OLED on front panel
4. Label ports clearly

### Calibration

1. Compare INA226 with accurate multimeter
2. Adjust SHUNT_RESISTOR value if needed
3. Test with known capacity battery
4. Fine-tune cutoff voltages

---

## 📚 Cheat Sheet

### GPIO Quick Reference
```
GPIO21 = I2C SDA
GPIO22 = I2C SCL
GPIO26 = Port 1 MOSFET
GPIO14 = Port 2 MOSFET
GPIO12 = Port 3 MOSFET
GPIO13 = Port 4 MOSFET
GPIO32 = Encoder CLK
GPIO33 = Encoder DT
GPIO25 = Encoder Button
GPIO27 = Buzzer
```

### I2C Addresses
```
0x3C = OLED
0x40 = INA226 Port 1
0x41 = INA226 Port 2
0x42 = INA226 Port 3
0x43 = INA226 Port 4
```

### Mode Values (API)
```
0 = Safety
1 = Charging
2 = Discharging
```

### Battery Types (API)
```
0 = Li-ion (3.0V cutoff)
1 = LiFePO4 (2.5V cutoff)
2 = LiPo (3.0V cutoff)
```

### Useful Commands
```bash
# Build
pio run

# Upload
pio run -t upload

# Monitor
pio device monitor

# Clean build
pio run -t clean

# Update libraries
pio pkg update
```

---

## ⚡ Power Consumption

Typical consumption:
- ESP32: 80-240mA (WiFi active)
- OLED: 20-30mA
- INA226 x4: 4mA
- Buzzer: 30mA (when active)
- MOSFETs: <1mA (gate drive)

**Total idle:** ~150mA @ 5V = 0.75W  
**Total active:** ~300mA @ 5V = 1.5W

---

## 🎯 Success Checklist

- [ ] OLED displays main screen
- [ ] Encoder rotates and beeps
- [ ] Button press enters menu
- [ ] Web UI accessible at 192.168.4.1
- [ ] INA226 reads battery voltage
- [ ] MOSFET switches ON in discharge mode
- [ ] Load resistor gets warm during discharge
- [ ] Discharge stops at cutoff voltage
- [ ] Charging detection works
- [ ] Both UIs stay synchronized

**All checked?** Congratulations! Your DIY Charger is ready! 🎉

---

## 📞 Get Help

Stuck? Check these resources:

1. **Full documentation:** README.md
2. **GitHub Issues:** Report bugs
3. **Serial Monitor:** Debug output
4. **I2C Scanner:** Detect devices
5. **Multimeter:** Verify voltages

---

**Happy battery sorting!** 🔋⚡

*This guide assumes single port testing. Expand to 4 ports after successful single port operation.*
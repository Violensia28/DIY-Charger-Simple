- MOSFET: **OFF**
- INA226: Logging charge current
- Auto cutoff: 4.2V (Li-ion/LiPo) atau 3.65V (LiFePO4)
- **Use case**: Charging depleted batteries

### 3. Discharging Mode
- TP4056: **Can be OFF** (disconnect input)
- MOSFET: **ON** (controlled by ESP32)
- INA226: Logging discharge current
- **Auto cutoff**: Stops at configured voltage
- **Use case**: Capacity testing, battery sorting

---

## 🔧 Configuration

### Default Cutoff Voltages

Edit in `include/Config.h`:

```cpp
#define LIION_CUTOFF 3.0      // Li-ion cutoff
#define LIFEPO4_CUTOFF 2.5    // LiFePO4 cutoff
#define LIPO_CUTOFF 3.0       // LiPo cutoff
```

### Sampling Rate

```cpp
#define SAMPLE_INTERVAL_MS 500  // 2Hz sampling
#define FILTER_SAMPLES 5        // Median filter size
```

### WiFi Credentials

```cpp
#define AP_SSID "DIY-Charger"
#define AP_PASSWORD "charger123"
```

### UI Refresh Rates

```cpp
#define UI_REFRESH_INTERVAL 200      // OLED refresh (ms)
#define WS_UPDATE_INTERVAL 1000      // WebSocket update (ms)
```

### Buzzer Tones

```cpp
#define BUZZER_FREQ_MENU 2000        // Menu navigation (Hz)
#define BUZZER_FREQ_SELECT 2500      // Selection (Hz)
#define BUZZER_FREQ_COMPLETE 3000    // Task complete (Hz)
#define BUZZER_DURATION_SHORT 50     // Short beep (ms)
```

---

## 📈 CSV Data Format

Export format (via `/api/logs` endpoint):

```csv
Timestamp,Port,Voltage,Current,Power,mAh,Wh,Mode,Battery,Status
0,0,4.156,0.985,4.09,0.0,0.00,Discharging,Li-ion,Active
1,0,4.142,0.978,4.05,0.3,0.00,Discharging,Li-ion,Active
2,0,4.128,0.971,4.01,0.5,0.00,Discharging,Li-ion,Active
...
```

---

## 🧪 Testing Without Hardware

### Simulation Mode

Untuk testing tanpa hardware fisik:

1. Comment out hardware initialization di `main.cpp`
2. Use dummy data di `Logger.cpp`
3. Build dan test Web UI saja

```cpp
// In main.cpp setup()
// logger->begin();  // Comment this out
portData[0].voltage = 4.15;  // Dummy data
portData[0].current = 0.98;
```

### I2C Scanner

Untuk debug I2C connection:

```bash
# Upload I2C scanner sketch
pio run --target upload
pio device monitor
```

Expected output:
```
I2C Scanner
Scanning...
I2C device found at address 0x3C  (OLED)
I2C device found at address 0x40  (INA226 Port 1)
I2C device found at address 0x41  (INA226 Port 2)
I2C device found at address 0x42  (INA226 Port 3)
I2C device found at address 0x43  (INA226 Port 4)
Done.
```

---

## 🚀 CI/CD Pipeline

GitHub Actions automatically:

1. ✅ **Build firmware** on push/PR
2. ✅ **Run static analysis** (lint)
3. ✅ **Check firmware size**
4. ✅ **Upload artifacts** (firmware.bin)
5. ✅ **Create releases** (on tags)

### Creating a Release

```bash
git tag v2.0.0
git push origin v2.0.0
```

Firmware akan otomatis ter-upload ke GitHub Releases.

### Download Pre-built Firmware

```bash
# Download latest release
wget https://github.com/YOUR_USERNAME/DIY-Charger-Simple/releases/latest/download/firmware.bin

# Flash via esptool
esptool.py --port /dev/ttyUSB0 write_flash 0x10000 firmware.bin
```

---

## 🐛 Troubleshooting

### INA226 Not Found

```
Error: Port X: INA226 not found at 0xXX
```

**Solutions:**
- Check I2C wiring (SDA/SCL)
- Verify address jumpers on INA226
- Check 4.7kΩ pull-up resistors on I2C bus
- Run I2C scanner to detect devices
- Try lower I2C frequency (100kHz instead of 400kHz)

### OLED Not Displaying

```
Error: SSD1306 OLED not found
```

**Solutions:**
- Check OLED address (should be 0x3C or 0x3D)
- Verify I2C shared bus connection
- Test OLED with Arduino example sketch first
- Check VCC (5V) and GND connection
- Try different OLED module

### MOSFET Not Switching

**Check:**
- Gate resistor (1kΩ recommended)
- MOSFET orientation (Drain/Source/Gate pinout)
- ESP32 GPIO voltage (3.3V sufficient for IRLR7843)
- Verify GPIO mapping matches Config.h
- Measure gate voltage with multimeter (should be 3.3V when ON)

### Encoder Not Responding

**Check:**
- Pull-up resistors on CLK, DT, SW pins
- Encoder wiring (CLK=GPIO32, DT=GPIO33, SW=GPIO25)
- Try rotating slowly
- Check Serial Monitor for encoder debug messages
- Verify encoder VCC (3.3V not 5V)

### Buzzer Always On / Not Working

**Check:**
- Active vs Passive buzzer (use Active 5V buzzer)
- Transistor driver circuit if buzzer needs 5V
- GPIO27 connection
- Check buzzer polarity
- Reduce buzzer duration in Config.h if too annoying

### Web UI Not Loading

**Check:**
- WiFi connection to "DIY-Charger"
- IP address (should be 192.168.4.1)
- Browser cache (try incognito mode)
- Check Serial Monitor for WiFi status
- Try different browser (Chrome recommended)
- Restart ESP32

### Discharge Not Stopping at Cutoff

**Check:**
- Cutoff voltage setting (OLED menu or Web UI)
- INA226 voltage reading accuracy
- Battery type selection correct
- Mode set to "Discharging" not "Safety"
- Check Serial Monitor for status messages

### Measurements Inaccurate

**Solutions:**
- Calibrate INA226 shunt resistor value (0.1Ω default)
- Check all INA226 connections
- Verify median filter is working (5 samples)
- Compare with multimeter reading
- Ensure good contact at battery holder
- Check for voltage drop in wiring

---

## 📚 API Reference

### REST Endpoints

| Method | Endpoint | Parameters | Description |
|--------|----------|------------|-------------|
| GET | `/` | - | Main web interface (HTML) |
| GET | `/api/status` | - | Get all ports status (JSON) |
| POST | `/api/mode` | port, mode | Set port mode (0=Safety, 1=Charging, 2=Discharging) |
| POST | `/api/battery` | port, type | Set battery type (0=Li-ion, 1=LiFePO4, 2=LiPo) |
| POST | `/api/cutoff` | port, voltage | Set custom cutoff voltage (2.0-3.5V) |
| POST | `/api/reset` | port | Reset port accumulated data (mAh, Wh) |
| GET | `/api/logs` | - | Download CSV logs for all active ports |

### Example API Calls

```bash
# Get status
curl http://192.168.4.1/api/status

# Set Port 1 to Discharging mode
curl -X POST http://192.168.4.1/api/mode -d "port=0&mode=2"

# Set Port 2 battery type to LiFePO4
curl -X POST http://192.168.4.1/api/battery -d "port=1&type=1"

# Set Port 3 cutoff to 3.2V
curl -X POST http://192.168.4.1/api/cutoff -d "port=2&voltage=3.2"

# Reset Port 4 data
curl -X POST http://192.168.4.1/api/reset -d "port=3"

# Download logs
curl http://192.168.4.1/api/logs > battery_logs.csv
```

### WebSocket Protocol

Connect to `ws://192.168.4.1/ws` for real-time updates.

**Message Format (JSON):**
```json
{
  "ports": [
    {
      "voltage": 4.156,
      "current": 0.985,
      "power": 4.09,
      "mAh": 1250.5,
      "Wh": 4.85,
      "mode": 2,
      "batteryType": 0,
      "customCutoff": 3.0,
      "status": 1,
      "active": true
    },
    ...
  ]
}
```

**Status Codes:**
- `0` = Idle
- `1` = Active
- `2` = Complete
- `3` = Error

---

## 🎨 Customization

### Change WiFi Credentials

Edit `include/Config.h`:
```cpp
#define AP_SSID "MyCharger"
#define AP_PASSWORD "mypassword123"
```

### Adjust OLED Layout

Edit `src/UI.cpp` functions:
- `drawMainScreen()` - Main display layout
- `drawPortStatus()` - Per-port info display
- `drawBattery()` - Battery icon appearance

### Modify Web UI Theme

Edit `src/WebUI.cpp` in `handleRoot()` function:
```cpp
// Change colors
background: #1a1a1a;  // Dark theme
.port-card.active { border-color: #4CAF50; }  // Green

// Change to light theme
background: #f5f5f5;
color: #333;
```

### Add More Menu Options

1. Add new enum in `include/UI.h`
2. Implement drawing function in `src/UI.cpp`
3. Handle button press in `handleButtonPress()`
4. Update menu flow

### Custom Buzzer Patterns

Edit `include/Config.h`:
```cpp
#define BUZZER_FREQ_CUSTOM 2800
#define BUZZER_DURATION_CUSTOM 150
```

Add pattern in `src/UI.cpp`:
```cpp
case BEEP_CUSTOM:
    freq = BUZZER_FREQ_CUSTOM;
    duration = BUZZER_DURATION_CUSTOM;
    break;
```

---

## 📖 Code Structure

### Modular Design

```
Firmware Architecture
│
├── main.cpp
│   ├── Hardware initialization
│   ├── MOSFET control logic
│   ├── System synchronization
│   └── Main loop orchestration
│
├── Logger (INA226)
│   ├── Sensor communication
│   ├── Median filtering
│   ├── Accumulator updates (mAh/Wh)
│   └── CSV generation
│
├── WebUI (HTTP + WebSocket)
│   ├── WiFi AP management
│   ├── HTTP request handlers
│   ├── WebSocket broadcasts
│   └── JSON serialization
│
├── PhysicalUI (OLED + Encoder)
│   ├── Display rendering
│   ├── Menu state machine
│   ├── Encoder interrupt handling
│   └── Buzzer control
│
└── BatteryTypes
    ├── Type definitions
    ├── Configuration structs
    └── Helper methods
```

### Adding New Features

**Example: Add temperature monitoring**

1. Add sensor library to `platformio.ini`
2. Define pin in `include/Config.h`
3. Add temperature field to `PortData` struct
4. Update `Logger.cpp` to read temperature
5. Display in `UI.cpp` and `WebUI.cpp`

---

## 🤝 Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Follow existing code style
4. Test on hardware if possible
5. Update documentation
6. Commit changes (`git commit -m 'Add AmazingFeature'`)
7. Push to branch (`git push origin feature/AmazingFeature`)
8. Open Pull Request

### Code Style Guidelines

- Use descriptive variable names
- Comment complex logic
- Follow existing formatting
- Keep functions under 50 lines
- Use `DEBUG_PRINTLN()` for debug output

---

## 📋 TODO / Roadmap

### Version 2.1 (Planned)
- [ ] Data logging to SD card
- [ ] Battery temperature monitoring (DS18B20)
- [ ] Multi-language support (EN/ID)
- [ ] MQTT publish untuk Home Assistant
- [ ] OTA firmware update via Web UI

### Version 3.0 (Future)
- [ ] Bluetooth control via smartphone app
- [ ] Grafik discharge curve di Web UI
- [ ] AI-based battery health prediction
- [ ] Multi-cell support (2S, 3S packs)
- [ ] Internal resistance measurement

---

## 📝 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file.

```
MIT License

Copyright (c) 2025 DIY Charger Simple Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

---

## 🙏 Acknowledgments

- **Adafruit** - INA226 library and sensor modules
- **ESPAsyncWebServer** - Async web framework for ESP32
- **PlatformIO** - Modern development platform
- **SSD1306 Library** - OLED display driver
- **Community Contributors** - Bug reports and feature requests

---

## 📧 Support & Contact

- **GitHub Issues**: [Report bugs or request features](https://github.com/YOUR_USERNAME/DIY-Charger-Simple/issues)
- **Discussions**: [Ask questions and share projects](https://github.com/YOUR_USERNAME/DIY-Charger-Simple/discussions)
- **Email**: your.email@example.com

### Useful Links

- [ESP32 Pinout Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [INA226 Datasheet](https://www.ti.com/lit/ds/symlink/ina226.pdf)
- [TP4056 Datasheet](https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Prototyping/TP4056.pdf)
- [PlatformIO Documentation](https://docs.platformio.org/)

---

## ⚠️ Safety Warnings

**IMPORTANT - READ BEFORE USE:**

⚠️ **Lithium Battery Safety**
- Never leave charging/discharging batteries unattended
- Use fire-resistant container or bag
- Ensure proper ventilation
- Keep fire extinguisher nearby
- Monitor temperature during operation

⚠️ **Electrical Safety**
- Verify all wiring before powering on
- Use proper gauge wire for high current paths
- Check for shorts with multimeter
- Ensure proper heatsinking for MOSFETs and load resistors
- Do not exceed component ratings

⚠️ **Discharge Safety**
- 7.5Ω 5W resistor will get HOT (>100°C)
- Mount resistor on heatsink with thermal paste
- Keep resistor away from flammable materials
- Ensure adequate airflow
- Consider adding thermal shutdown

⚠️ **Battery Handling**
- Inspect batteries for damage before use
- Discard swollen, dented, or leaking batteries
- Never discharge below manufacturer's minimum voltage
- Respect battery chemistry cutoff voltages
- Use proper disposal methods for dead batteries

**By using this project, you accept all responsibility for safe operation.**

---

## 📸 Gallery

### Hardware Setup
```
[Photo: Completed device with OLED, encoder, and 4 battery ports]
[Photo: PCB layout/breadboard wiring]
[Photo: Enclosure design]
```

### OLED Screenshots
```
[Screenshot: Main screen showing 4 ports]
[Screenshot: Menu navigation]
[Screenshot: Discharge in progress]
```

### Web UI Screenshots
```
[Screenshot: Web dashboard on desktop]
[Screenshot: Mobile responsive view]
[Screenshot: Real-time monitoring]
```

---

**Built with ❤️ for the battery DIY community**

⭐ **Star this repo** if you find it useful!  
🐛 **Report issues** to help improve the project  
🔧 **Contribute** to make it even better

---

Last updated: 2025-01-XX  
Version: 2.0.0  
Status: **Production Ready** ✅# 🔋 DIY Charger Simple v2.0

**ESP32-based Battery Sorting System** untuk 18650, LiPo, dan LiFePO4  
**Dual Interface:** OLED + Rotary Encoder + Web UI

[![Build Status](https://github.com/YOUR_USERNAME/DIY-Charger-Simple/workflows/Build%20ESP32%20Firmware/badge.svg)](https://github.com/YOUR_USERNAME/DIY-Charger-Simple/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

---

## 📋 Fitur

✅ **4 Port Independen** - Monitoring dan kontrol per baterai  
✅ **Dual Interface** - OLED lokal + Web UI remote  
✅ **3 Mode Operasi** - Charging, Discharging, Safety  
✅ **3 Tipe Baterai** - Li-ion, LiFePO4, LiPo  
✅ **Logging Akurat** - INA226 dengan median filter (2Hz)  
✅ **Rotary Encoder** - Navigasi menu intuitif  
✅ **Buzzer Feedback** - Notifikasi audio untuk event  
✅ **Auto Cut-off** - Proteksi discharge berdasarkan tegangan  
✅ **CSV Export** - Data logging untuk analisis  
✅ **Sinkronisasi UI** - Perubahan di OLED langsung terlihat di Web UI

---

## 🛠️ Hardware Requirements

### Komponen Utama

| Komponen | Spesifikasi | Qty |
|----------|-------------|-----|
| ESP32 DevKit V1 | ESP32-WROOM-32 | 1 |
| TP4056 Module | 1A Li-ion Charger | 4 |
| INA226 Module | I2C Current/Voltage Sensor | 4 |
| MOSFET IRLR7843 | 30V 13A N-Channel | 4 |
| Resistor Kapur | 7.5Ω 5W | 4 |
| 18650 Battery Holder | Single cell | 4 |
| OLED Display | SSD1306 128x64 I2C | 1 |
| Rotary Encoder | KY-040 with button | 1 |
| Buzzer | 5V Active Buzzer | 1 |
| Step-down DC-DC | 12V → 5V, 2A | 1 |
| Power Supply | 12V 5A | 1 |

### Optional Components
- Enclosure (3D printable)
- LED indicator per port
- External antenna untuk WiFi

---

## 🔌 Wiring Diagram

### ESP32 Pinout (LOCKED - DO NOT CHANGE)

```
ESP32 DevKit V1
├── GPIO 21 ──── I2C SDA (INA226 x4 + OLED)
├── GPIO 22 ──── I2C SCL (INA226 x4 + OLED)
├── GPIO 26 ──── MOSFET Port 1 Gate
├── GPIO 14 ──── MOSFET Port 2 Gate
├── GPIO 12 ──── MOSFET Port 3 Gate
├── GPIO 13 ──── MOSFET Port 4 Gate
├── GPIO 32 ──── Encoder CLK
├── GPIO 33 ──── Encoder DT
├── GPIO 25 ──── Encoder SW (Button)
├── GPIO 27 ──── Buzzer
├── 5V ────────── Step-down output + OLED VCC
├── 3V3 ────────── Encoder VCC
└── GND ────────── Common ground
```

### I2C Bus Configuration

```
I2C Bus (GPIO21 SDA, GPIO22 SCL)
│
├── INA226 Port 1 (0x40) ─┐
├── INA226 Port 2 (0x41) ─┤
├── INA226 Port 3 (0x42) ─┼── 4.7kΩ Pull-up to 3.3V
├── INA226 Port 4 (0x43) ─┤
└── OLED SSD1306 (0x3C) ──┘
```

### Per Port Connection

```
Port 1 Example:
                                  
   12V ───┬─── TP4056 IN+         
          │                       
       INA226                     
       (0x40)                     
          │                       
          ├─── TP4056 BAT+        
          │                       
      [Battery]                   
      18650/LiPo                  
          │                       
          ├─── TP4056 BAT-        
          │                       
          ├─── MOSFET Drain       
          │    (IRLR7843)         
      [7.5Ω Load]                 
          │                       
   GND ───┴─── MOSFET Source      
                                  
  ESP32 GPIO26 ─── MOSFET Gate    
               ─── 1kΩ            
```

### INA226 I2C Addresses

Set jumper pads pada INA226 untuk address berbeda:

| Port | Address | A1 | A0 |
|------|---------|----|----|
| 1 | 0x40 | GND | GND |
| 2 | 0x41 | GND | VS+ |
| 3 | 0x42 | VS+ | GND |
| 4 | 0x43 | VS+ | VS+ |

### Rotary Encoder Wiring

```
KY-040 Rotary Encoder
├── CLK ──── GPIO32
├── DT  ──── GPIO33
├── SW  ──── GPIO25 (with 10kΩ pull-up)
├── VCC ──── 3.3V
└── GND ──── GND
```

### Buzzer Wiring

```
Active Buzzer (5V)
├── + ──── GPIO27 (via NPN transistor)
└── - ──── GND

Optional: Add transistor driver
GPIO27 ─── 1kΩ ─── Base (2N2222)
                   Collector ─── Buzzer+
                   Emitter ──── GND
```

---

## 💻 Software Setup

### 1. Install PlatformIO

#### Via VSCode
```bash
# Install VSCode extension
code --install-extension platformio.platformio-ide
```

#### Via CLI
```bash
pip install platformio
```

### 2. Clone Repository

```bash
git clone https://github.com/YOUR_USERNAME/DIY-Charger-Simple.git
cd DIY-Charger-Simple
```

### 3. Build Firmware

```bash
# Build
pio run

# Build and upload
pio run --target upload

# Monitor serial output
pio device monitor -b 115200
```

### 4. First Boot

Pada boot pertama, sistem akan:
1. Inisialisasi semua hardware
2. Tampil startup screen di OLED
3. Bunyi beep startup
4. Start WiFi AP "DIY-Charger"
5. Tampil main screen dengan status 4 port

---

## 🖥️ Physical UI (OLED + Encoder)

### Menu Navigation Flow

```
MAIN SCREEN (Default)
│   ├── Port 1: 4.15V ●
│   ├── Port 2: 0.00V
│   ├── Port 3: 3.98V ●
│   └── Port 4: 0.00V
│   [Press to config]
│
Press Button
│
PORT SELECT
│   ├── > Port 1: 4.15V
│   ├──   Port 2: 0.00V
│   ├──   Port 3: 3.98V
│   └──   Port 4: 0.00V
│
Press Button (Select Port 1)
│
MODE SELECT
│   ├── > Safety
│   ├──   Charging
│   └──   Discharging
│   [Port 1]
│
Press Button (Select Discharging)
│
BATTERY SELECT
│   ├── > Li-ion (3.0V)
│   ├──   LiFePO4 (2.5V)
│   └──   LiPo (3.0V)
│
Press Button (Select Li-ion)
│
CUTOFF ADJUST
│   [Cutoff Voltage]
│   
│       3.0 V
│   
│   [Range: 2.0-3.5V]
│
Press Button
│
CONFIRM
│   Port: 1
│   Mode: Discharging
│   Battery: Li-ion
│   Cutoff: 3.0V
│   
│   [START]  [CANCEL]
│
Press Button → Return to MAIN
```

### Encoder Controls

| Action | Function |
|--------|----------|
| **Rotate CW** | Scroll down / Increase value |
| **Rotate CCW** | Scroll up / Decrease value |
| **Press Button** | Select / Confirm |
| **Timeout (30s)** | Auto return to main screen |

### Buzzer Feedback

| Event | Pattern |
|-------|---------|
| Menu navigation | Short beep (50ms, 2000Hz) |
| Selection | Medium beep (200ms, 2500Hz) |
| Charge/Discharge complete | Long beep (500ms, 3000Hz) |
| Error | Long beep (200ms, 1500Hz) |

---

## 🌐 Web Interface

### Accessing Web UI

1. Power on the ESP32
2. Connect to WiFi AP: **DIY-Charger**
3. Password: **charger123**
4. Open browser: **http://192.168.4.1**

### Web UI Features

- **Real-time monitoring** - Auto-refresh setiap 1 detik
- **4 Port Cards** - Voltage, current, capacity, energy
- **Mode selection** - Dropdown per port
- **Battery type** - Li-ion / LiFePO4 / LiPo
- **Custom cutoff** - Input manual voltage
- **Data reset** - Clear accumulated mAh/Wh
- **Responsive design** - Mobile-friendly

### UI Synchronization

✅ **OLED → Web UI**: Perubahan di OLED langsung terlihat di browser  
✅ **Web UI → OLED**: Perubahan di browser langsung update di OLED  
✅ **Real-time status**: Status charging/discharging sinkron  
✅ **Error notification**: Error muncul di kedua interface

---

## 📊 Operation Modes

### 1. Safety Mode (Default)
- TP4056: **OFF**
- MOSFET: **OFF**
- INA226: Monitoring only
- **Use case**: Idle state, battery removal, standby

### 2. Charging Mode
- TP4056: **ON** (automatic CC/CV)
- MOSFET: **OFF**
- INA226: Logging charge current
- **Use case**: Charging depleted batteries

### 3. Discharging Mode
- TP4056: **Can be OFF** (disconnect input)
- MOSFET: **ON** (controlled by ESP32)
- INA226: Logging discharge current
- **Auto cutoff**: Stops at configured voltage
- **Use case**: Capacity testing, battery sorting

---

## 🔧 Configuration

### Default Cutoff Voltages

Edit in `include/Config.h`:

```cpp
#define LIION_CUTOFF 3.0      // Li-ion cutoff
#define LIFEPO4_CUTOFF 2.5    // LiFePO4 cutoff
#define LIPO_CUTOFF 3.0       // LiPo cutoff
```

### Sampling Rate

```cpp
#define SAMPLE_INTERVAL_MS 500  // 2Hz sampling
#define FILTER_SAMPLES 5        // Median filter size
```

### WiFi Credentials

```cpp
#define AP_SSID "DIY-Charger"
#define AP_PASSWORD "charger123"
```

---

## 📈 CSV Data Format

Export format (via `/api/logs` endpoint):

```csv
Timestamp,Port,Voltage,Current,Power,mAh,Wh,Mode,Battery,Status
0,0,4.156,0.985,4.09,0.0,0.00,Discharging,Li-ion,Active
1,0,4.142,0.978,4.05,0.3,0.00,Discharging,Li-ion,Active
2,0,4.128,0.971,4.01,0.5,0.00,Discharging,Li-ion,Active
...
```

---

## 🧪 Testing Without Hardware

### Simulation Mode (Coming Soon)

```bash
# Build for simulation
pio run -e simulation

# Run tests
pio test
```

---

## 🚀 CI/CD Pipeline

GitHub Actions automatically:

1. ✅ **Build firmware** on push/PR
2. ✅ **Run static analysis** (lint)
3. ✅ **Upload artifacts** (firmware.bin)
4. ✅ **Create releases** (on tags)

### Creating a Release

```bash
git tag v1.0.0
git push origin v1.0.0
```

Firmware akan otomatis ter-upload ke GitHub Releases.

---

## 🐛 Troubleshooting

### INA226 Not Found

```
Error: Port X: INA226 not found at 0xXX
```

**Solutions:**
- Check I2C wiring (SDA/SCL)
- Verify address jumpers
- Run I2C scanner: `pio run --target upload monitor`

### MOSFET Not Switching

**Check:**
- Gate resistor (1kΩ recommended)
- MOSFET orientation (Drain/Source/Gate)
- ESP32 GPIO voltage (3.3V sufficient for IRLR7843)

### Web UI Not Loading

**Check:**
- WiFi connection to "DIY-Charger"
- IP address (should be 192.168.4.1)
- Browser cache (try incognito mode)

---

## 📚 API Reference

### REST Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | Get all ports status (JSON) |
| POST | `/api/mode` | Set port mode (port, mode) |
| POST | `/api/battery` | Set battery type (port, type) |
| POST | `/api/cutoff` | Set cutoff voltage (port, voltage) |
| POST | `/api/reset` | Reset port data (port) |
| GET | `/api/logs` | Download CSV logs |

### WebSocket

Connect to `ws://192.168.4.1/ws` for real-time updates (1Hz).

---

## 🤝 Contributing

Contributions welcome! Please:

1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

---

## 📝 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file.

---

## 🙏 Acknowledgments

- **Adafruit** - INA226 library
- **ESPAsyncWebServer** - Async web framework
- **PlatformIO** - Development platform

---

## 📧 Contact

Project Link: [https://github.com/YOUR_USERNAME/DIY-Charger-Simple](https://github.com/YOUR_USERNAME/DIY-Charger-Simple)

---

**⚠️ Safety Warning**: Always supervise battery charging/discharging. Ensure proper ventilation and fire safety measures.
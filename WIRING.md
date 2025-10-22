# 🔌 Detailed Wiring Guide

Complete step-by-step wiring instructions for DIY Charger Simple v2.0

---

## 📐 Bill of Materials (BOM)

### Per Port Components

| Item | Specification | Qty/Port | Total (4 ports) |
|------|--------------|----------|-----------------|
| TP4056 Module | 1A Li-ion charger with protection | 1 | 4 |
| INA226 Module | I2C voltage/current sensor | 1 | 4 |
| IRLR7843 MOSFET | N-Channel 30V 13A | 1 | 4 |
| Load Resistor | 7.5Ω 5W Ceramic | 1 | 4 |
| Battery Holder | 18650 single cell | 1 | 4 |
| 1kΩ Resistor | 1/4W gate resistor | 1 | 4 |

### Shared Components

| Item | Specification | Qty |
|------|--------------|-----|
| ESP32 DevKit V1 | 30-pin development board | 1 |
| OLED Display | SSD1306 128x64 I2C | 1 |
| Rotary Encoder | KY-040 with button | 1 |
| Active Buzzer | 5V piezo buzzer | 1 |
| 2N2222 Transistor | NPN (for buzzer driver) | 1 |
| 4.7kΩ Resistor | 1/4W I2C pull-up | 2 |
| Step-down Module | 12V to 5V 2A | 1 |
| Power Supply | 12V 5A DC adapter | 1 |
| DC Jack | 5.5x2.1mm female | 1 |

### Wiring Materials

| Item | Specification | Length |
|------|--------------|--------|
| Wire 22AWG | Red (power) | 2m |
| Wire 22AWG | Black (ground) | 2m |
| Wire 24AWG | Various colors (signals) | 5m |
| Heat shrink tubing | Assorted sizes | 1m |
| Terminal blocks | 2-pin screw terminal | 8 |
| Perfboard/PCB | 10x15cm | 1 |

---

## 🔧 Assembly Steps

### Step 1: Prepare ESP32 Base

1. **Mount ESP32 on perfboard**
   - Use IC socket or solder directly
   - Leave space for other modules

2. **Add power distribution**
   ```
   12V Input ─── Step-down ─── 5V Rail ─── ESP32 VIN
                              └─── OLED VCC
   
   GND Rail ──────────────────┬─── ESP32 GND
                              ├─── OLED GND
                              ├─── All modules GND
                              └─── Power supply GND
   ```

3. **Add I2C bus**
   ```
   ESP32 GPIO26 ─── 1kΩ ─── MOSFET Gate (Pin 3)
```

**MOSFET Pinout (IRLR7843 TO-220):**
```
Looking at flat side with pins down:
Pin 1 (Left)   = Gate
Pin 2 (Center) = Drain  
Pin 3 (Right)  = Source
```

**Load Resistor Mounting:**
- Mount on heatsink with thermal paste
- Secure with M3 screw
- Keep away from plastic/wires
- Ensure airflow around resistor

### Step 6: Replicate for Ports 2-4

| Port | MOSFET GPIO | INA226 Address | A1 | A0 |
|------|------------|----------------|-----|-----|
| 1 | GPIO26 | 0x40 | GND | GND |
| 2 | GPIO14 | 0x41 | GND | VS+ |
| 3 | GPIO12 | 0x42 | VS+ | GND |
| 4 | GPIO13 | 0x43 | VS+ | VS+ |

**Each port identical to Port 1, just different addresses and GPIOs**

---

## 🔍 Detailed Connection Tables

### ESP32 GPIO Allocation (Complete)

| GPIO | Function | Connection | Notes |
|------|----------|------------|-------|
| 21 | I2C SDA | All I2C devices | 4.7kΩ pull-up |
| 22 | I2C SCL | All I2C devices | 4.7kΩ pull-up |
| 26 | MOSFET 1 | Port 1 Gate via 1kΩ | Active HIGH |
| 14 | MOSFET 2 | Port 2 Gate via 1kΩ | Active HIGH |
| 12 | MOSFET 3 | Port 3 Gate via 1kΩ | Active HIGH |
| 13 | MOSFET 4 | Port 4 Gate via 1kΩ | Active HIGH |
| 32 | Encoder CLK | Rotary encoder A | Interrupt pin |
| 33 | Encoder DT | Rotary encoder B | Interrupt pin |
| 25 | Encoder SW | Push button | Pull-up needed |
| 27 | Buzzer | Via transistor | PWM capable |
| 3V3 | Power | Encoder, INA226 | Max 600mA |
| 5V | Power | OLED, Buzzer | From step-down |
| GND | Ground | All components | Star ground |

### I2C Device Map

| Device | Address | SDA | SCL | VCC | Notes |
|--------|---------|-----|-----|-----|-------|
| OLED | 0x3C | GPIO21 | GPIO22 | 5V | Sometimes 0x3D |
| INA226 #1 | 0x40 | GPIO21 | GPIO22 | 3.3V | A1=GND, A0=GND |
| INA226 #2 | 0x41 | GPIO21 | GPIO22 | 3.3V | A1=GND, A0=VS+ |
| INA226 #3 | 0x42 | GPIO21 | GPIO22 | 3.3V | A1=VS+, A0=GND |
| INA226 #4 | 0x43 | GPIO21 | GPIO22 | 3.3V | A1=VS+, A0=VS+ |

### Power Distribution

| Rail | Voltage | Source | Consumers | Max Current |
|------|---------|--------|-----------|-------------|
| 12V | 12.0V | DC adapter | INA226 inputs, TP4056 | 4A |
| 5V | 5.0V | Step-down | ESP32, OLED, Buzzer | 2A |
| 3.3V | 3.3V | ESP32 reg | Encoder, INA226 logic | 600mA |
| GND | 0V | Common | All components | - |

---

## 🛠️ Assembly Tips

### Soldering Best Practices

1. **Use flux** - Rosin flux for clean joints
2. **Right temperature** - 350°C for leaded, 380°C for lead-free
3. **Tin first** - Pre-tin wire and pad
4. **Quick touch** - 2-3 seconds max per joint
5. **Inspect** - Use magnifier to check bridges

### Wire Management

1. **Color coding:**
   - Red = 12V power
   - Orange = 5V power
   - Yellow = 3.3V power
   - Black = Ground
   - Blue = I2C SDA
   - Green = I2C SCL
   - Other colors = GPIO signals

2. **Cable routing:**
   - Keep power and signal separate
   - Use cable ties every 5cm
   - Label both ends of wires
   - Leave service loops

3. **Strain relief:**
   - Hot glue wire bundles
   - Use P-clips for heavy wires
   - Don't pull on connectors

### Grounding Strategy

**Use star ground topology:**

```
                Power Supply GND
                       │
                  Main GND Bus
                       │
        ┌──────────────┼──────────────┐
        │              │              │
    ESP32 GND    Modules GND    Battery GND
```

**Avoid ground loops** - All grounds meet at ONE point

---

## ⚡ Power Circuit Detail

### 12V Input Stage

```
DC Adapter (12V 5A)
     │
     ├─── Reverse polarity protection diode (5A Schottky)
     │
     ├─── TVS diode (18V) to GND (overvoltage protection)
     │
     ├─── 100µF capacitor to GND (input filtering)
     │
     ├───┬─── To INA226 Port 1 VIN+
     │   ├─── To INA226 Port 2 VIN+
     │   ├─── To INA226 Port 3 VIN+
     │   └─── To INA226 Port 4 VIN+
     │
     └─── Step-down input
              │
              5V output ───┬─── ESP32 VIN
                           ├─── OLED VCC
                           └─── Buzzer VCC
```

### Step-down Selection

**Recommended:** LM2596 or MP1584EN modules

**Specifications:**
- Input: 12V
- Output: 5V adjustable
- Current: 2A minimum
- Efficiency: >90%
- Ripple: <50mV

**Adjustment:**
1. Connect input 12V
2. Adjust potentiometer with multimeter
3. Set output to exactly 5.0V
4. Add 100µF cap on output

---

## 🧪 Testing Procedure

### Pre-power Checks

**Use multimeter to verify:**

1. **No shorts:**
   - [ ] 12V to GND = Open circuit
   - [ ] 5V to GND = Open circuit
   - [ ] 3.3V to GND = Open circuit

2. **Correct resistance:**
   - [ ] Load resistors = 7.5Ω each
   - [ ] Gate resistors = 1kΩ each
   - [ ] I2C pull-ups = 4.7kΩ each

3. **Diode check:**
   - [ ] MOSFET Drain-Source = ~0.6V forward
   - [ ] Reverse polarity diode = ~0.3V forward

### Power-on Sequence

**Step 1:** Power without ESP32
```
1. Apply 12V input
2. Measure step-down output = 5.0V ±0.1V
3. Check for overheating (modules should be cool)
4. Verify no smoke/burning smell
```

**Step 2:** Add ESP32 (no load)
```
1. Connect ESP32 to USB (not 5V rail yet)
2. Program with test firmware
3. Verify serial output working
4. Disconnect USB
5. Connect ESP32 to 5V rail
6. ESP32 should boot from 5V supply
```

**Step 3:** Test I2C devices
```
1. Upload I2C scanner sketch
2. Verify all addresses detected:
   - 0x3C (OLED)
   - 0x40 (INA226 #1)
   - 0x41 (INA226 #2)
   - 0x42 (INA226 #3)
   - 0x43 (INA226 #4)
```

**Step 4:** Test outputs
```
1. Upload blink test for each MOSFET GPIO
2. Measure gate voltage:
   - OFF = 0V
   - ON = 3.3V
3. Verify buzzer beeps
4. Test encoder rotation and button
```

**Step 5:** Load test (Single port)
```
1. Connect dead battery to Port 1
2. Set mode to Discharging
3. MOSFET should turn ON
4. Load resistor should warm up
5. Current should flow (monitor INA226)
6. Temperature check: <80°C on resistor
```

---

## 🔥 Thermal Management

### Heat Sources

| Component | Idle Temp | Active Temp | Max Safe |
|-----------|-----------|-------------|----------|
| Load resistor 7.5Ω | 25°C | 100-120°C | 150°C |
| MOSFET IRLR7843 | 30°C | 40-50°C | 175°C |
| ESP32 chip | 40°C | 50-60°C | 125°C |
| TP4056 regulator | 35°C | 45-55°C | 125°C |

### Cooling Solutions

**Load Resistors (Critical!):**
```
Resistor mounting:
[Resistor] ─ Thermal paste ─ [Aluminum heatsink 30x30mm]
                              │
                         M3 screw + nut
                              │
                         [Base plate]
```

- Use thermal paste (not thermal pad)
- Minimum heatsink: 30x30x15mm
- Add fan if ambient >30°C
- Space resistors 5cm apart

**Enclosure Ventilation:**
```
Top: Exhaust holes (hot air rises)
Bottom: Intake holes (cool air)
Sides: Prevent airflow restriction

Hole pattern:
○ ○ ○ ○ ○  (5mm holes, 1cm spacing)
```

---

## 📦 Enclosure Design

### Recommended Layout

```
Top View:
┌─────────────────────────────┐
│  [OLED Display]             │
│  [Encoder Knob]             │
│                             │
│  [Port 1] [Port 2]          │
│  [Port 3] [Port 4]          │
│                             │
│  [Power Jack]               │
└─────────────────────────────┘

Side View:
┌────────┬────────┬──────────┐
│ OLED   │Battery │ ESP32    │
│ Encoder│Holders │ Modules  │
│        │        │          │
│        │ Load   │ Heatsink │
│        │Resistor│          │
└────────┴────────┴──────────┘
   Front    Middle    Back
```

### Dimensions

- Minimum internal: 200 x 150 x 80mm
- OLED cutout: 30 x 20mm
- Encoder hole: 7mm diameter
- Battery holder spacing: 50mm center-to-center
- Ventilation: 30% of top/bottom area

### Materials

**Recommended:**
- 3D printed PLA/PETG
- Laser-cut acrylic
- Sheet metal enclosure

**Avoid:**
- Fully sealed (overheating)
- Flammable materials near resistors
- Conductive materials without insulation

---

## 🔒 Safety Features

### Built-in Protection

1. **Reverse polarity** - Diode at 12V input
2. **Overvoltage** - TVS diode clamps spikes
3. **Undervoltage** - Software cutoff at MIN_VOLTAGE
4. **Overcurrent** - TP4056 has 1A limit
5. **Thermal** - Monitor resistor temperature
6. **Timeout** - Auto-disable after preset time

### Recommended Additions

**Fuse in 12V line:**
```
12V+ ─── 5A slow-blow fuse ─── System
```

**Emergency stop button:**
```
E-stop (NC) in series with 12V input
Press to cut all power immediately
```

**Temperature sensor on resistors:**
```
DS18B20 sensor mounted on heatsink
Software shutdown if >120°C
```

**Smoke detector:**
```
Mount near device
Connect alarm to family
```

---

## 📋 Post-Assembly Checklist

### Visual Inspection
- [ ] No loose wires
- [ ] All solder joints shiny and smooth
- [ ] No solder bridges
- [ ] Components properly oriented
- [ ] Heat shrink on all connections
- [ ] Strain relief applied
- [ ] Labels on all connectors

### Electrical Tests
- [ ] Continuity check all grounds
- [ ] No shorts between power rails
- [ ] Correct voltages (12V, 5V, 3.3V)
- [ ] I2C pull-ups present
- [ ] MOSFET gates isolated when off

### Functional Tests
- [ ] OLED displays correctly
- [ ] Encoder rotates smoothly
- [ ] Button press detected
- [ ] Buzzer sounds
- [ ] All INA226 detected
- [ ] WiFi AP starts
- [ ] Web UI accessible
- [ ] Voltage readings accurate
- [ ] MOSFETs switch correctly
- [ ] Discharge stops at cutoff

### Safety Tests
- [ ] Resistors don't overheat (use IR thermometer)
- [ ] No burning smell
- [ ] All exposed metal grounded
- [ ] Enclosure properly ventilated
- [ ] Emergency stop works
- [ ] Fire extinguisher nearby

---

## 🆘 Common Wiring Mistakes

### ❌ Wrong: Direct buzzer to GPIO
```
ESP32 GPIO27 ──────── Buzzer+ (5V, 30mA)
                      Buzzer- ──── GND
```
**Problem:** ESP32 GPIO max 40mA, buzzer draws too much

### ✅ Correct: Transistor driver
```
ESP32 GPIO27 ─── 1kΩ ─── 2N2222 Base
                         Collector ─── Buzzer+ ─── 5V
                         Emitter ──── GND
```

---

### ❌ Wrong: No I2C pull-ups
```
ESP32 GPIO21 ──────── SDA (floating)
ESP32 GPIO22 ──────── SCL (floating)
```
**Problem:** I2C communication unreliable or fails

### ✅ Correct: Pull-up resistors
```
ESP32 GPIO21 ──┬─── 4.7kΩ ─── 3.3V
               └─── SDA devices

ESP32 GPIO22 ──┬─── 4.7kΩ ─── 3.3V
               └─── SCL devices
```

---

### ❌ Wrong: MOSFET gate direct connection
```
ESP32 GPIO26 ──────── MOSFET Gate
```
**Problem:** Inrush current, possible GPIO damage

### ✅ Correct: Gate resistor
```
ESP32 GPIO26 ─── 1kΩ ─── MOSFET Gate
```

---

### ❌ Wrong: Ground loops
```
ESP32 GND ─── Wire ─── Module1 GND ─── Wire ─── Module2 GND
                                                      │
                                                  Power GND
```
**Problem:** Noise, measurement errors

### ✅ Correct: Star ground
```
           Power GND
                │
         Main GND Bus
                │
     ┌──────────┼──────────┐
     │          │          │
ESP32 GND  Module1 GND  Module2 GND
```

---

## 📞 Support

For wiring questions:
- Check continuity with multimeter
- Take clear photos of your build
- Post to GitHub Issues with photos
- Include I2C scanner output

**Remember:** When in doubt, double-check before powering on!

---

**Safety First!** ⚡🔥🛡️

*Never leave batteries charging/discharging unattended*21 (SDA) ─── 4.7kΩ to 3.3V ─── I2C SDA Rail
   ESP32 GPIO22 (SCL) ─── 4.7kΩ to 3.3V ─── I2C SCL Rail
   ```

### Step 2: Install OLED Display

```
OLED Module
├── VCC ──── 5V Rail
├── GND ──── GND Rail  
├── SDA ──── I2C SDA Rail
└── SCL ──── I2C SCL Rail
```

**Mounting:**
- Use standoffs or hot glue
- Position visible through enclosure window
- Keep away from heat sources (resistors)

### Step 3: Install Rotary Encoder

```
KY-040 Encoder
├── CLK ──── ESP32 GPIO32
├── DT  ──── ESP32 GPIO33
├── SW  ──── ESP32 GPIO25 + 10kΩ pull-up to 3.3V
├── VCC ──── 3.3V (NOT 5V!)
└── GND ──── GND Rail
```

**Mounting:**
- Panel-mount through enclosure
- Secure with nut
- Add knob for easy rotation

### Step 4: Install Buzzer with Driver

```
Buzzer Driver Circuit:

ESP32 GPIO27 ─── 1kΩ ─── Base (2N2222)
                         Collector ─── Buzzer+ ─── 5V
                         Emitter ──── GND
                         
Buzzer- ──── GND
```

**Why transistor?**
- ESP32 GPIO limited to 40mA
- 5V buzzer may draw 30-50mA
- Transistor provides current boost

### Step 5: Build Port 1 Circuit

**5a. Mount INA226**

```
INA226 Module (Address 0x40)
├── VCC ──── 3.3V
├── GND ──── GND Rail
├── SDA ──── I2C SDA Rail
├── SCL ──── I2C SCL Rail
├── VIN+ ─── 12V+ from power supply
└── VIN- ─── To TP4056 IN+

Address Configuration:
A1 = GND
A0 = GND
→ Address = 0x40
```

**5b. Mount TP4056**

```
TP4056 Module
├── IN+  ──── INA226 VIN-
├── IN-  ──── GND Rail
├── BAT+ ──── Battery holder +
└── BAT- ──── Battery holder -
```

**Note:** TP4056 BAT- also connects to MOSFET circuit

**5c. Add MOSFET and Load**

```
Discharge Circuit:

Battery holder - ────┬─── TP4056 BAT-
                     │
                     ├─── MOSFET Drain (Pin 2)
                     │
                  [7.5Ω Load]
                     │
                     └─── MOSFET Source (Pin 1) ──── GND Rail

ESP32 GPIO
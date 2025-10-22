#!/bin/bash

# Hardware Test Script for DIY Charger Simple
# This script automates hardware testing before deployment

set -e  # Exit on error

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
ESP32_PORT=${ESP32_PORT:-/dev/ttyUSB0}
BAUDRATE=115200
TIMEOUT=10

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  DIY Charger Simple - Hardware Test   ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo ""

# Function to print test status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
        exit 1
    fi
}

# Function to wait for user confirmation
confirm() {
    read -p "$1 (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${RED}Test cancelled by user${NC}"
        exit 1
    fi
}

# Check prerequisites
echo -e "${YELLOW}[1/8] Checking prerequisites...${NC}"

if ! command -v pio &> /dev/null; then
    echo -e "${RED}PlatformIO not found. Install with: pip install platformio${NC}"
    exit 1
fi
print_status 0 "PlatformIO installed"

if [ ! -c "$ESP32_PORT" ]; then
    echo -e "${RED}ESP32 not found at $ESP32_PORT${NC}"
    echo "Available ports:"
    ls /dev/ttyUSB* 2>/dev/null || ls /dev/cu.* 2>/dev/null || echo "None found"
    exit 1
fi
print_status 0 "ESP32 connected at $ESP32_PORT"

# Build firmware
echo -e "\n${YELLOW}[2/8] Building firmware...${NC}"
pio run > /dev/null 2>&1
print_status $? "Firmware compiled"

# Test 1: I2C Scanner
echo -e "\n${YELLOW}[3/8] Testing I2C devices...${NC}"
echo "Uploading I2C scanner..."

# Backup main.cpp
cp src/main.cpp src/main.cpp.bak

# Use I2C scanner
cp test/i2c_scanner.cpp src/main.cpp
pio run --target upload > /dev/null 2>&1
print_status $? "I2C scanner uploaded"

# Wait for ESP32 to boot
sleep 3

# Capture serial output
echo "Scanning I2C bus..."
timeout $TIMEOUT pio device monitor --baud $BAUDRATE | tee /tmp/i2c_scan.log &
MONITOR_PID=$!
sleep 5
kill $MONITOR_PID 2>/dev/null || true

# Check for expected devices
EXPECTED_DEVICES=(0x3C 0x40 0x41 0x42 0x43)
ALL_FOUND=true

for addr in "${EXPECTED_DEVICES[@]}"; do
    if grep -q "$addr" /tmp/i2c_scan.log; then
        echo -e "${GREEN}  ✓${NC} Device found at $addr"
    else
        echo -e "${RED}  ✗${NC} Device NOT found at $addr"
        ALL_FOUND=false
    fi
done

# Restore main.cpp
mv src/main.cpp.bak src/main.cpp

if [ "$ALL_FOUND" = false ]; then
    echo -e "${RED}Not all I2C devices detected!${NC}"
    echo "Check wiring and I2C addresses"
    exit 1
fi
print_status 0 "All I2C devices detected"

# Test 2: Upload main firmware
echo -e "\n${YELLOW}[4/8] Uploading main firmware...${NC}"
pio run --target upload > /dev/null 2>&1
print_status $? "Main firmware uploaded"
sleep 3

# Test 3: Check serial output
echo -e "\n${YELLOW}[5/8] Checking boot sequence...${NC}"
timeout $TIMEOUT pio device monitor --baud $BAUDRATE | tee /tmp/boot.log &
MONITOR_PID=$!
sleep 5
kill $MONITOR_PID 2>/dev/null || true

if grep -q "System ready!" /tmp/boot.log; then
    print_status 0 "Boot successful"
else
    print_status 1 "Boot failed"
fi

# Test 4: WiFi AP
echo -e "\n${YELLOW}[6/8] Checking WiFi Access Point...${NC}"
echo "Scanning for 'DIY-Charger' AP..."
sleep 2

if command -v nmcli &> /dev/null; then
    if nmcli dev wifi list | grep -q "DIY-Charger"; then
        print_status 0 "WiFi AP detected"
    else
        echo -e "${YELLOW}⚠${NC} WiFi AP not detected (may take up to 30s)"
    fi
elif command -v iwlist &> /dev/null; then
    if sudo iwlist wlan0 scan | grep -q "DIY-Charger"; then
        print_status 0 "WiFi AP detected"
    else
        echo -e "${YELLOW}⚠${NC} WiFi AP not detected"
    fi
else
    echo -e "${YELLOW}⚠${NC} Cannot scan WiFi (nmcli/iwlist not available)"
fi

# Test 5: OLED Display
echo -e "\n${YELLOW}[7/8] Testing OLED display...${NC}"
confirm "Is the OLED displaying the main screen?"
print_status 0 "OLED confirmed working"

# Test 6: Rotary Encoder
echo -e "\n${YELLOW}[8/8] Testing rotary encoder...${NC}"
echo "Please rotate the encoder..."
confirm "Did you hear a beep sound?"
print_status 0 "Encoder confirmed working"

# Optional: Voltage reading test
echo -e "\n${BLUE}Optional Test: Voltage Reading${NC}"
read -p "Connect a battery to Port 1 and press Enter..."

timeout 5 pio device monitor --baud $BAUDRATE | grep "Port 0" | head -1

echo ""
echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║     All Tests Passed Successfully!     ║${NC}"
echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
echo ""
echo "Hardware is ready for operation!"
echo ""
echo "Next steps:"
echo "  1. Connect batteries to test ports"
echo "  2. Access Web UI at http://192.168.4.1"
echo "  3. Start charging/discharging tests"
echo ""

# Cleanup
rm -f /tmp/i2c_scan.log /tmp/boot.log

exit 0

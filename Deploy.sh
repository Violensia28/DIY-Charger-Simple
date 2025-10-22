#!/bin/bash

# Deployment Script for DIY Charger Simple
# Automates firmware building, testing, and deployment

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Configuration
PROJECT_NAME="DIY Charger Simple"
VERSION=$(grep "version" platformio.ini | cut -d'=' -f2 | tr -d ' ' || echo "2.0.0")
BUILD_DIR=".pio/build/esp32dev"
BACKUP_DIR="backups"
RELEASE_DIR="releases"

# Functions
print_header() {
    echo -e "${CYAN}╔════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║  $1${NC}"
    echo -e "${CYAN}╚════════════════════════════════════════╝${NC}"
}

print_step() {
    echo -e "\n${BLUE}[$1] $2${NC}"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
    exit 1
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

confirm() {
    read -p "$(echo -e ${YELLOW}$1 [y/N]:${NC}) " -n 1 -r
    echo
    [[ $REPLY =~ ^[Yy]$ ]]
}

# Check prerequisites
check_prerequisites() {
    print_step "1/7" "Checking prerequisites"
    
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO not found. Install: pip install platformio"
    fi
    print_success "PlatformIO found"
    
    if ! command -v git &> /dev/null; then
        print_warning "Git not found. Version control disabled."
    else
        print_success "Git found"
    fi
    
    # Create directories
    mkdir -p "$BACKUP_DIR" "$RELEASE_DIR" logs reports
}

# Clean build
clean_build() {
    print_step "2/7" "Cleaning previous builds"
    
    if confirm "Clean all build artifacts?"; then
        pio run --target clean
        rm -rf .pio/build
        print_success "Build cleaned"
    else
        print_warning "Skipping clean"
    fi
}

# Build firmware
build_firmware() {
    print_step "3/7" "Building firmware v$VERSION"
    
    if pio run; then
        print_success "Firmware compiled successfully"
        
        # Show firmware size
        echo ""
        echo "Firmware size:"
        pio run --target size | grep "elf\|bin" || true
        
        # Check if firmware exists
        if [ ! -f "$BUILD_DIR/firmware.bin" ]; then
            print_error "Firmware binary not found!"
        fi
    else
        print_error "Build failed!"
    fi
}

# Backup current firmware
backup_firmware() {
    print_step "4/7" "Creating backup"
    
    TIMESTAMP=$(date +%Y%m%d_%H%M%S)
    BACKUP_FILE="$BACKUP_DIR/firmware_v${VERSION}_${TIMESTAMP}.bin"
    
    if [ -f "$BUILD_DIR/firmware.bin" ]; then
        cp "$BUILD_DIR/firmware.bin" "$BACKUP_FILE"
        print_success "Backup created: $BACKUP_FILE"
        
        # Keep only last 5 backups
        ls -t $BACKUP_DIR/firmware_*.bin | tail -n +6 | xargs rm -f 2>/dev/null || true
    else
        print_warning "No firmware to backup"
    fi
}

# Run tests
run_tests() {
    print_step "5/7" "Running tests"
    
    if [ -f "test/test_hardware.sh" ]; then
        if confirm "Run hardware tests?"; then
            bash test/test_hardware.sh
        else
            print_warning "Skipping hardware tests"
        fi
    else
        print_warning "No test script found"
    fi
}

# Upload firmware
upload_firmware() {
    print_step "6/7" "Uploading firmware"
    
    # Detect ESP32 port
    ESP32_PORT=""
    if [ -c /dev/ttyUSB0 ]; then
        ESP32_PORT=/dev/ttyUSB0
    elif ls /dev/cu.* &> /dev/null; then
        ESP32_PORT=$(ls /dev/cu.* | head -1)
    fi
    
    if [ -z "$ESP32_PORT" ]; then
        print_warning "ESP32 not detected. Available ports:"
        ls /dev/tty* 2>/dev/null || echo "None"
        read -p "Enter port manually (or 'skip'): " ESP32_PORT
        
        if [ "$ESP32_PORT" == "skip" ]; then
            print_warning "Skipping upload"
            return
        fi
    fi
    
    echo "Uploading to $ESP32_PORT..."
    
    if confirm "Proceed with upload?"; then
        if pio run --target upload --upload-port "$ESP32_PORT"; then
            print_success "Firmware uploaded successfully"
            
            # Wait for boot
            sleep 3
            
            if confirm "Open serial monitor?"; then
                echo "Press Ctrl+C to exit monitor"
                sleep 1
                pio device monitor --baud 115200
            fi
        else
            print_error "Upload failed!"
        fi
    else
        print_warning "Upload cancelled"
    fi
}

# Create release
create_release() {
    print_step "7/7" "Creating release package"
    
    if confirm "Create release package?"; then
        RELEASE_NAME="diy-charger-simple_v${VERSION}_$(date +%Y%m%d)"
        RELEASE_PATH="$RELEASE_DIR/$RELEASE_NAME"
        
        mkdir -p "$RELEASE_PATH"
        
        # Copy files
        cp "$BUILD_DIR/firmware.bin" "$RELEASE_PATH/"
        cp "$BUILD_DIR/firmware.elf" "$RELEASE_PATH/" 2>/dev/null || true
        cp README.md "$RELEASE_PATH/"
        cp QUICKSTART.md "$RELEASE_PATH/" 2>/dev/null || true
        cp LICENSE "$RELEASE_PATH/"
        
        # Create flash instructions
        cat > "$RELEASE_PATH/FLASH_INSTRUCTIONS.txt" << EOF
DIY Charger Simple v$VERSION
Flash Instructions
==================

Prerequisites:
- Python 3.x
- esptool (pip install esptool)

Flash via USB:
--------------
1. Connect ESP32 via USB
2. Identify port:
   - Linux: /dev/ttyUSB0 or /dev/ttyACM0
   - Mac: /dev/cu.usbserial-*
   - Windows: COM3, COM4, etc.

3. Flash firmware:
   esptool.py --port PORT write_flash 0x10000 firmware.bin

   Replace PORT with your ESP32 port.

4. Reset ESP32 and check serial output:
   screen PORT 115200

Flash via PlatformIO:
---------------------
If you have the source code:
   pio run --target upload --upload-port PORT

First Boot:
-----------
1. OLED should display startup screen
2. Buzzer beeps once
3. WiFi AP "DIY-Charger" starts (password: charger123)
4. Connect and open http://192.168.4.1

Troubleshooting:
----------------
- If upload fails, hold BOOT button during flash
- Check USB cable (data cable, not charge-only)
- Try different baud rate: --baud 460800
- Erase flash first: esptool.py --port PORT erase_flash

Support:
--------
GitHub: [YOUR_REPO_URL]
Documentation: See README.md
EOF
        
        # Create archive
        cd "$RELEASE_DIR"
        tar -czf "${RELEASE_NAME}.tar.gz" "$RELEASE_NAME"
        cd - > /dev/null
        
        print_success "Release created: $RELEASE_DIR/${RELEASE_NAME}.tar.gz"
        
        # Git tag
        if command -v git &> /dev/null && [ -d .git ]; then
            if confirm "Create git tag v$VERSION?"; then
                git tag -a "v$VERSION" -m "Release v$VERSION"
                print_success "Git tag created: v$VERSION"
                
                if confirm "Push tag to remote?"; then
                    git push origin "v$VERSION"
                    print_success "Tag pushed to remote"
                fi
            fi
        fi
    else
        print_warning "Release creation skipped"
    fi
}

# Show summary
show_summary() {
    echo ""
    print_header "Deployment Summary"
    echo ""
    echo "Project:  $PROJECT_NAME v$VERSION"
    echo "Date:     $(date '+%Y-%m-%d %H:%M:%S')"
    echo ""
    echo "Build artifacts:"
    echo "  - Firmware:  $BUILD_DIR/firmware.bin"
    echo "  - Backup:    $BACKUP_DIR/"
    echo "  - Release:   $RELEASE_DIR/"
    echo ""
    echo "Quick commands:"
    echo "  Monitor:     make monitor"
    echo "  Re-upload:   make upload"
    echo "  Clean:       make clean"
    echo ""
    print_success "Deployment completed successfully!"
    echo ""
}

# Main deployment flow
main() {
    clear
    print_header "$PROJECT_NAME - Deployment Script v$VERSION"
    echo ""
    
    check_prerequisites
    clean_build
    build_firmware
    backup_firmware
    run_tests
    upload_firmware
    create_release
    show_summary
}

# Run main function
main

exit 0
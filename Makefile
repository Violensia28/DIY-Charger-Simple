# Makefile for DIY Charger Simple
# Convenience commands for development and deployment

.PHONY: help build upload monitor clean test scan flash-test backup restore

# Default target
help:
	@echo "DIY Charger Simple - Development Commands"
	@echo ""
	@echo "Build & Deploy:"
	@echo "  make build        - Compile firmware"
	@echo "  make upload       - Upload firmware to ESP32"
	@echo "  make monitor      - Open serial monitor"
	@echo "  make flash        - Build + Upload + Monitor"
	@echo ""
	@echo "Testing:"
	@echo "  make scan         - Upload I2C scanner"
	@echo "  make test         - Run unit tests"
	@echo "  make flash-test   - Quick hardware test"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean        - Clean build files"
	@echo "  make backup       - Backup current firmware"
	@echo "  make restore      - Restore firmware backup"
	@echo "  make update-libs  - Update all libraries"
	@echo ""
	@echo "Analysis:"
	@echo "  make parse-logs   - Parse latest CSV logs"
	@echo "  make check-size   - Check firmware size"
	@echo ""

# Build firmware
build:
	@echo "🔨 Building firmware..."
	pio run

# Upload to ESP32
upload:
	@echo "📤 Uploading firmware..."
	pio run --target upload

# Open serial monitor
monitor:
	@echo "📡 Opening serial monitor..."
	pio device monitor -b 115200

# Full flash cycle
flash: build upload monitor

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build files..."
	pio run --target clean
	rm -rf .pio/build

# Upload I2C scanner for testing
scan:
	@echo "🔍 Uploading I2C scanner..."
	@if [ ! -f test/i2c_scanner.cpp ]; then \
		echo "Error: test/i2c_scanner.cpp not found"; \
		exit 1; \
	fi
	@# Temporarily rename main.cpp
	@mv src/main.cpp src/main.cpp.bak
	@cp test/i2c_scanner.cpp src/main.cpp
	pio run --target upload
	@# Restore original main.cpp
	@mv src/main.cpp.bak src/main.cpp
	@echo "✓ I2C scanner uploaded. Check serial monitor."

# Run unit tests
test:
	@echo "🧪 Running tests..."
	pio test

# Quick hardware test (blink + beep)
flash-test:
	@echo "⚡ Quick hardware test..."
	@echo "Testing GPIO outputs..."
	@# This would run a minimal test sketch
	@echo "Not implemented yet. Use 'make scan' for I2C test."

# Backup current firmware
backup:
	@echo "💾 Backing up firmware..."
	@mkdir -p backups
	@TIMESTAMP=$$(date +%Y%m%d_%H%M%S); \
	if [ -f .pio/build/esp32dev/firmware.bin ]; then \
		cp .pio/build/esp32dev/firmware.bin backups/firmware_$$TIMESTAMP.bin; \
		echo "✓ Backup saved: backups/firmware_$$TIMESTAMP.bin"; \
	else \
		echo "❌ No firmware found. Run 'make build' first."; \
	fi

# Restore from backup
restore:
	@echo "📂 Available backups:"
	@ls -1 backups/*.bin 2>/dev/null || echo "No backups found"
	@read -p "Enter backup filename to restore: " BACKUP; \
	if [ -f "backups/$$BACKUP" ]; then \
		esptool.py --port /dev/ttyUSB0 write_flash 0x10000 "backups/$$BACKUP"; \
		echo "✓ Firmware restored from $$BACKUP"; \
	else \
		echo "❌ Backup not found"; \
	fi

# Update all libraries
update-libs:
	@echo "📦 Updating libraries..."
	pio pkg update

# Check firmware size
check-size:
	@echo "📊 Checking firmware size..."
	pio run --target size

# Parse logs (requires Python script)
parse-logs:
	@echo "📈 Parsing battery logs..."
	@if [ -f tools/parse_logs.py ]; then \
		python3 tools/parse_logs.py logs/*.csv 2>/dev/null || echo "No log files found in logs/"; \
	else \
		echo "❌ tools/parse_logs.py not found"; \
	fi

# OTA upload (when WiFi is enabled)
ota:
	@echo "📡 OTA Upload..."
	@read -p "Enter ESP32 IP address: " IP; \
	pio run --target upload --upload-port $$IP

# Generate documentation
docs:
	@echo "📚 Generating documentation..."
	@echo "Opening README.md..."
	@if command -v mdless > /dev/null; then \
		mdless README.md; \
	else \
		cat README.md; \
	fi

# Format code (requires clang-format)
format:
	@echo "🎨 Formatting code..."
	@find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i
	@echo "✓ Code formatted"

# Lint code
lint:
	@echo "🔍 Running linter..."
	pio check

# Install dependencies
install-deps:
	@echo "📦 Installing dependencies..."
	@command -v pio > /dev/null || (echo "Installing PlatformIO..." && pip install platformio)
	@command -v python3 > /dev/null || (echo "Python 3 required" && exit 1)
	@pip install matplotlib numpy
	@echo "✓ Dependencies installed"

# Quick development cycle
dev: build upload
	@echo "✓ Development flash complete"
	@echo "Opening monitor in 2 seconds..."
	@sleep 2
	@make monitor

# Release build
release:
	@echo "🚀 Building release..."
	@# Ensure clean build
	@make clean
	@make build
	@make backup
	@echo "✓ Release build complete"
	@echo "Firmware ready at: .pio/build/esp32dev/firmware.bin"

# Show ESP32 info
info:
	@echo "ℹ️  ESP32 Information:"
	@pio device list
	@echo ""
	@echo "Port auto-detection:"
	@ls /dev/ttyUSB* 2>/dev/null || ls /dev/cu.* 2>/dev/null || echo "No ESP32 found"

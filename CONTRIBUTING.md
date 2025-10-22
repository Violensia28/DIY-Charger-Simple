# 🤝 Contributing to DIY Charger Simple

Thank you for your interest in contributing! This document provides guidelines and instructions for contributing to the project.

---

## 📋 Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [Development Setup](#development-setup)
4. [Making Changes](#making-changes)
5. [Testing](#testing)
6. [Submitting Changes](#submitting-changes)
7. [Code Style](#code-style)
8. [Documentation](#documentation)

---

## 📜 Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inclusive environment. Be respectful, professional, and constructive in all interactions.

### Expected Behavior

- Use welcoming and inclusive language
- Be respectful of differing viewpoints
- Accept constructive criticism gracefully
- Focus on what's best for the community
- Show empathy towards others

### Unacceptable Behavior

- Harassment, discrimination, or trolling
- Publishing others' private information
- Other unprofessional conduct

---

## 🚀 Getting Started

### Prerequisites

- Git installed
- PlatformIO IDE or CLI
- ESP32 DevKit V1 (for hardware testing)
- Basic knowledge of C++, Arduino, and ESP32

### Areas for Contribution

We welcome contributions in:

- **🐛 Bug Fixes** - Fix reported issues
- **✨ New Features** - Add functionality
- **📚 Documentation** - Improve guides and comments
- **🧪 Testing** - Add unit tests, hardware tests
- **🎨 UI/UX** - Improve Web UI or OLED interface
- **🔧 Hardware** - PCB designs, enclosure designs
- **📊 Analysis** - Data visualization, algorithms

---

## 💻 Development Setup

### 1. Fork and Clone

```bash
# Fork the repository on GitHub first, then:
git clone https://github.com/YOUR_USERNAME/DIY-Charger-Simple.git
cd DIY-Charger-Simple

# Add upstream remote
git remote add upstream https://github.com/ORIGINAL_OWNER/DIY-Charger-Simple.git
```

### 2. Install Dependencies

```bash
# Install PlatformIO
pip install platformio

# Install Python tools (optional)
pip install matplotlib numpy pandas

# Install pre-commit hooks (optional)
pip install pre-commit
pre-commit install
```

### 3. Build and Test

```bash
# Build firmware
pio run

# Run tests
pio test

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### 4. Create Branch

```bash
# Always create a new branch for your work
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/issue-number-description
```

---

## 🔧 Making Changes

### Branch Naming Convention

- Feature: `feature/add-temperature-sensor`
- Bug fix: `fix/issue-123-oled-crash`
- Documentation: `docs/update-api-guide`
- Refactor: `refactor/logger-class`
- Test: `test/add-unit-tests`

### Commit Messages

Follow the [Conventional Commits](https://www.conventionalcommits.org/) format:

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `test`: Adding tests
- `chore`: Maintenance tasks

**Examples:**
```bash
git commit -m "feat(logger): add temperature monitoring support"
git commit -m "fix(webui): resolve WebSocket reconnection issue"
git commit -m "docs(readme): update wiring diagram"
```

### Code Organization

```
src/
├── main.cpp           # Main application logic
├── Logger.cpp         # INA226 sensor handling
├── WebUI.cpp          # Web interface
└── UI.cpp             # OLED + encoder interface

include/
├── Config.h           # Global configuration
├── BatteryTypes.h     # Type definitions
├── Logger.h           # Logger interface
├── WebUI.h            # Web UI interface
└── UI.h               # Physical UI interface

test/
├── i2c_scanner.cpp    # Hardware test utilities
└── test_hardware.sh   # Automated testing script

tools/
└── parse_logs.py      # Log analysis scripts
```

---

## 🧪 Testing

### Unit Tests

```bash
# Run all tests
pio test

# Run specific test
pio test -f test_logger

# Run with verbose output
pio test -v
```

### Hardware Tests

```bash
# Automated hardware test
bash test/test_hardware.sh

# Manual I2C scan
pio run --target upload
# (Upload i2c_scanner.cpp first)
```

### Testing Checklist

Before submitting, verify:

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] I2C devices detected correctly
- [ ] OLED displays properly
- [ ] Web UI loads and updates
- [ ] No memory leaks (check serial output)
- [ ] Encoder responds correctly
- [ ] Buzzer functions
- [ ] Discharge stops at cutoff
- [ ] Charging detection works

---

## 📤 Submitting Changes

### Pull Request Process

1. **Update your fork:**
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Push your changes:**
   ```bash
   git push origin feature/your-feature-name
   ```

3. **Create Pull Request on GitHub:**
   - Clear title describing the change
   - Reference any related issues (#123)
   - Describe what changed and why
   - Include screenshots/videos if UI changes
   - List testing performed

4. **PR Template:**
   ```markdown
   ## Description
   Brief description of changes
   
   ## Type of Change
   - [ ] Bug fix
   - [ ] New feature
   - [ ] Documentation update
   - [ ] Breaking change
   
   ## Testing
   - [ ] Compiled successfully
   - [ ] Tested on hardware
   - [ ] Added/updated tests
   
   ## Screenshots
   (if applicable)
   
   ## Checklist
   - [ ] Code follows style guidelines
   - [ ] Documentation updated
   - [ ] No new warnings
   - [ ] Tested on ESP32
   ```

5. **Code Review:**
   - Address review comments
   - Make requested changes
   - Push updates to same branch

6. **Merge:**
   - Maintainer will merge when approved
   - Delete your branch after merge

---

## 🎨 Code Style

### C++ Style Guide

**Naming Conventions:**
```cpp
// Classes: PascalCase
class BatteryLogger {};

// Functions: camelCase
void updateMOSFETs() {}

// Variables: camelCase
int portNumber = 0;

// Constants: UPPER_CASE
#define MAX_VOLTAGE 4.5

// Private members: camelCase with underscore
class Example {
private:
    int _privateVar;
};
```

**Formatting:**
```cpp
// Indentation: 4 spaces (no tabs)
void example() {
    if (condition) {
        doSomething();
    }
}

// Braces: Same line for functions, next line for if/for/while
void function() {
    // ...
}

if (condition) 
{
    // ...
}

// Line length: Max 100 characters
// Comments: Use // for single line, /* */ for blocks
```

**Best Practices:**
```cpp
// Always use braces, even for single statements
if (condition) {
    statement();
}

// Declare variables close to usage
void function() {
    int x = 10;  // Use x here
    // ...
}

// Use const where possible
const int MAX_PORTS = 4;

// Prefer early returns
if (error) {
    return false;
}
// Continue with main logic

// Add debug output
#if DEBUG_LOGGER
DEBUG_PRINTLN("Message");
#endif
```

### JavaScript Style (Web UI)

```javascript
// Use const/let, not var
const API_URL = 'http://192.168.4.1/api';
let currentPort = 0;

// Use arrow functions
const updateUI = (data) => {
    // ...
};

// Use template literals
console.log(`Port ${port}: ${voltage}V`);

// Handle errors
fetch(url)
    .then(response => response.json())
    .catch(error => console.error('Error:', error));
```

### Python Style (Tools)

```python
# Follow PEP 8
# Use snake_case for functions and variables
def parse_log_file(filename):
    battery_data = []
    # ...

# Type hints (Python 3.6+)
def calculate_capacity(data: list) -> float:
    return sum(data)

# Docstrings
def function(param):
    """
    Brief description.
    
    Args:
        param: Description
        
    Returns:
        Description
    """
    pass
```

---

## 📚 Documentation

### Code Comments

```cpp
/**
 * @brief Initialize INA226 sensor
 * @param port Port number (0-3)
 * @return true if successful, false otherwise
 */
bool initPort(int port);

// Inline comments explain WHY, not WHAT
voltage = readVoltage();  // Apply median filter to reduce noise
```

### README Updates

When adding features, update:
- Main README.md (feature list)
- API.md (if API changes)
- WIRING.md (if hardware changes)
- QUICKSTART.md (if setup changes)

### Commit Documentation

```bash
# Good commit message
git commit -m "feat(logger): add oversampling for better accuracy

Implements 10x oversampling on INA226 readings to reduce noise.
Average measurement time increased by 5ms but accuracy improved
by 15% in testing.

Closes #42"

# Bad commit message
git commit -m "fix stuff"
```

---

## 🐛 Reporting Bugs

### Before Reporting

1. Check existing issues
2. Test with latest version
3. Verify it's not a wiring/hardware problem
4. Reproduce consistently

### Bug Report Template

```markdown
**Describe the bug**
Clear description of the problem

**To Reproduce**
Steps to reproduce:
1. Go to '...'
2. Click on '...'
3. See error

**Expected behavior**
What should happen

**Actual behavior**
What actually happens

**Screenshots**
If applicable

**Environment:**
- Firmware version: [e.g. v2.0.0]
- ESP32 board: [e.g. DevKit V1]
- Hardware: [e.g. 4x INA226, OLED SSD1306]
- Power supply: [e.g. 12V 5A]

**Serial Output:**
```
Paste relevant serial monitor output
```

**Additional context**
Any other information
```

---

## ✨ Feature Requests

### Proposal Template

```markdown
**Feature Description**
Clear description of the feature

**Use Case**
Why is this needed? What problem does it solve?

**Proposed Solution**
How should it work?

**Alternatives Considered**
What other approaches were considered?

**Additional Context**
Mockups, diagrams, examples
```

---

## 🏆 Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md
- Credited in release notes
- Mentioned in commit messages

---

## 📞 Questions?

- **GitHub Discussions:** For general questions
- **GitHub Issues:** For bugs and features
- **Email:** (if provided)

---

## 📄 License

By contributing, you agree that your contributions will be licensed under the same MIT License that covers this project.

---

**Thank you for contributing!** 🎉

Every contribution, no matter how small, helps make this project better for everyone.
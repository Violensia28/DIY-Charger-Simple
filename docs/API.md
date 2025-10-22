# 🌐 API Documentation - DIY Charger Simple v2.0

Complete REST API and WebSocket protocol documentation.

---

## 📡 Base URL

```
http://192.168.4.1
```

Default WiFi credentials:
- SSID: `DIY-Charger`
- Password: `charger123`

---

## 🔐 Authentication

Currently, no authentication is required. All endpoints are publicly accessible within the WiFi network.

**Security Note:** Change WiFi password in production by editing `Config.h`:
```cpp
#define AP_PASSWORD "your_secure_password"
```

---

## 📋 REST API Endpoints

### GET /

**Description:** Main web interface (HTML page)

**Response:** HTML
```html
<!DOCTYPE html>
<html>
...
</html>
```

**Status Codes:**
- `200 OK` - Success

---

### GET /api/status

**Description:** Get real-time status of all 4 battery ports

**Response:** JSON
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
    ...3 more ports
  ]
}
```

**Response Fields:**

| Field | Type | Description | Range/Values |
|-------|------|-------------|--------------|
| voltage | float | Battery voltage in Volts | 0.0 - 4.5 |
| current | float | Current in Amperes | -3.0 to +3.0 |
| power | float | Power in Watts | calculated |
| mAh | float | Accumulated capacity | 0+ |
| Wh | float | Accumulated energy | 0+ |
| mode | int | Operation mode | 0=Safety, 1=Charging, 2=Discharging |
| batteryType | int | Battery chemistry | 0=Li-ion, 1=LiFePO4, 2=LiPo |
| customCutoff | float | Custom cutoff voltage | 2.0 - 3.5 |
| status | int | Port status | 0=Idle, 1=Active, 2=Complete, 3=Error |
| active | bool | Port active flag | true/false |

**Example:**
```bash
curl http://192.168.4.1/api/status
```

**Status Codes:**
- `200 OK` - Success
- `500 Internal Server Error` - Server error

---

### POST /api/mode

**Description:** Set operation mode for a specific port

**Content-Type:** `application/x-www-form-urlencoded`

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| port | int | Yes | Port number (0-3) |
| mode | int | Yes | Mode: 0=Safety, 1=Charging, 2=Discharging |

**Request:**
```bash
curl -X POST http://192.168.4.1/api/mode \
     -d "port=0&mode=2"
```

**Response:** `200 OK` with body `OK`

**Status Codes:**
- `200 OK` - Mode changed successfully
- `400 Bad Request` - Invalid parameters

**Notes:**
- Setting mode to Safety (0) will deactivate the port
- Setting mode to Charging/Discharging will activate the port
- Changes are immediately reflected in both Web UI and OLED

---

### POST /api/battery

**Description:** Set battery type for a specific port

**Content-Type:** `application/x-www-form-urlencoded`

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| port | int | Yes | Port number (0-3) |
| type | int | Yes | Type: 0=Li-ion, 1=LiFePO4, 2=LiPo |

**Request:**
```bash
curl -X POST http://192.168.4.1/api/battery \
     -d "port=1&type=1"
```

**Response:** `200 OK` with body `OK`

**Status Codes:**
- `200 OK` - Battery type changed
- `400 Bad Request` - Invalid parameters

**Battery Type Details:**

| Type | Value | Nominal | Max | Default Cutoff |
|------|-------|---------|-----|----------------|
| Li-ion | 0 | 3.7V | 4.2V | 3.0V |
| LiFePO4 | 1 | 3.2V | 3.65V | 2.5V |
| LiPo | 2 | 3.7V | 4.2V | 3.0V |

---

### POST /api/cutoff

**Description:** Set custom cutoff voltage for a specific port

**Content-Type:** `application/x-www-form-urlencoded`

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| port | int | Yes | Port number (0-3) |
| voltage | float | Yes | Cutoff voltage (2.0 - 3.5V) |

**Request:**
```bash
curl -X POST http://192.168.4.1/api/cutoff \
     -d "port=2&voltage=3.2"
```

**Response:** `200 OK` with body `OK`

**Status Codes:**
- `200 OK` - Cutoff voltage updated
- `400 Bad Request` - Invalid voltage range

**Notes:**
- Cutoff voltage must be between 2.0V and 3.5V
- Setting custom cutoff enables `useCustomCutoff` flag
- Custom cutoff overrides default battery type cutoff
- Recommended ranges by battery type:
  - Li-ion: 2.8V - 3.0V
  - LiFePO4: 2.3V - 2.8V
  - LiPo: 3.0V - 3.3V

---

### POST /api/reset

**Description:** Reset accumulated data (mAh, Wh) for a specific port

**Content-Type:** `application/x-www-form-urlencoded`

**Parameters:**

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| port | int | Yes | Port number (0-3) |

**Request:**
```bash
curl -X POST http://192.168.4.1/api/reset \
     -d "port=0"
```

**Response:** `200 OK` with body `OK`

**Status Codes:**
- `200 OK` - Port data reset
- `400 Bad Request` - Invalid port number

**What gets reset:**
- mAh counter → 0
- Wh counter → 0
- Start time → current time
- Error count → 0
- Error message → cleared
- Status → Idle

**What is preserved:**
- Mode setting
- Battery type
- Custom cutoff voltage
- Current voltage/current readings

---

### GET /api/logs

**Description:** Download CSV logs for all active ports

**Response:** CSV file
```csv
Timestamp,Port,Voltage(V),Current(A),Power(W),mAh,Wh,Mode,Battery,Status
0,0,4.156,0.985,4.09,0.0,0.00,Discharging,Li-ion,Active
1,0,4.142,0.978,4.05,0.3,0.00,Discharging,Li-ion,Active
2,0,4.128,0.971,4.01,0.5,0.00,Discharging,Li-ion,Active
...
```

**Content-Type:** `text/csv`

**Example:**
```bash
curl http://192.168.4.1/api/logs > battery_log.csv
```

**Status Codes:**
- `200 OK` - CSV data returned

**CSV Fields:**

| Field | Description | Unit |
|-------|-------------|------|
| Timestamp | Seconds since test start | seconds |
| Port | Port number | 0-3 |
| Voltage(V) | Battery voltage | Volts |
| Current(A) | Current flow | Amperes |
| Power(W) | Instantaneous power | Watts |
| mAh | Accumulated capacity | milliamp-hours |
| Wh | Accumulated energy | Watt-hours |
| Mode | Operation mode | text |
| Battery | Battery type | text |
| Status | Port status | text |

**Usage with Python:**
```python
import pandas as pd
df = pd.read_csv('http://192.168.4.1/api/logs')
print(df.head())
```

---

## 🔌 WebSocket API

### Connection

**URL:** `ws://192.168.4.1/ws`

**Protocol:** WebSocket (RFC 6455)

**Example (JavaScript):**
```javascript
const ws = new WebSocket('ws://192.168.4.1/ws');

ws.onopen = () => {
    console.log('Connected to DIY Charger');
};

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    console.log('Received:', data);
};

ws.onerror = (error) => {
    console.error('WebSocket error:', error);
};

ws.onclose = () => {
    console.log('Disconnected');
};
```

### Message Format

**Server → Client (Broadcast every 1 second):**

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
    // ... 3 more ports
  ]
}
```

**Initial Connection:**
Upon connection, the server immediately sends the current status.

**Update Frequency:**
- Normal: 1 Hz (every 1000ms)
- Can be changed in Config.h: `WS_UPDATE_INTERVAL`

**Client → Server:**
Currently, WebSocket is receive-only. Use REST API for control commands.

### Example: Real-time Monitoring

```javascript
const ws = new WebSocket('ws://192.168.4.1/ws');

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    
    // Update UI for each port
    data.ports.forEach((port, index) => {
        document.getElementById(`voltage-${index}`).innerText = 
            port.voltage.toFixed(3) + ' V';
        document.getElementById(`current-${index}`).innerText = 
            port.current.toFixed(3) + ' A';
        document.getElementById(`capacity-${index}`).innerText = 
            port.mAh.toFixed(1) + ' mAh';
    });
};
```

### Error Handling

```javascript
let reconnectInterval;

function connect() {
    const ws = new WebSocket('ws://192.168.4.1/ws');
    
    ws.onclose = () => {
        console.log('Connection lost. Reconnecting in 3s...');
        reconnectInterval = setTimeout(connect, 3000);
    };
    
    ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        ws.close();
    };
}

connect();
```

---

## 📊 Status Codes Reference

### Operation Modes

| Code | Name | Description |
|------|------|-------------|
| 0 | Safety | All outputs OFF, monitoring only |
| 1 | Charging | TP4056 active, charging battery |
| 2 | Discharging | MOSFET ON, discharging through load |

### Battery Types

| Code | Name | Nominal | Max | Default Cutoff |
|------|------|---------|-----|----------------|
| 0 | Li-ion | 3.7V | 4.2V | 3.0V |
| 1 | LiFePO4 | 3.2V | 3.65V | 2.5V |
| 2 | LiPo | 3.7V | 4.2V | 3.0V |

### Port Status

| Code | Name | Description |
|------|------|-------------|
| 0 | Idle | Port inactive, no operation |
| 1 | Active | Charging or discharging in progress |
| 2 | Complete | Operation finished (reached cutoff/full) |
| 3 | Error | Error detected (check error message) |

---

## 🔧 Configuration Examples

### Example 1: Start Discharge Test

```bash
# Set battery type to Li-ion
curl -X POST http://192.168.4.1/api/battery -d "port=0&type=0"

# Set cutoff to 3.0V
curl -X POST http://192.168.4.1/api/cutoff -d "port=0&voltage=3.0"

# Start discharging
curl -X POST http://192.168.4.1/api/mode -d "port=0&mode=2"

# Monitor progress
curl http://192.168.4.1/api/status | jq '.ports[0]'
```

### Example 2: Monitor Multiple Ports

```python
import requests
import time

BASE_URL = "http://192.168.4.1"

while True:
    response = requests.get(f"{BASE_URL}/api/status")
    data = response.json()
    
    for i, port in enumerate(data['ports']):
        if port['active']:
            print(f"Port {i+1}: {port['voltage']:.3f}V "
                  f"{port['current']:.3f}A "
                  f"{port['mAh']:.1f}mAh")
    
    time.sleep(2)
```

### Example 3: Batch Configuration

```bash
#!/bin/bash
# Configure all 4 ports for Li-ion discharge test

BASE_URL="http://192.168.4.1/api"

for port in {0..3}; do
    # Set Li-ion type
    curl -X POST "$BASE_URL/battery" -d "port=$port&type=0"
    
    # Set 3.0V cutoff
    curl -X POST "$BASE_URL/cutoff" -d "port=$port&voltage=3.0"
    
    # Start discharging
    curl -X POST "$BASE_URL/mode" -d "port=$port&mode=2"
    
    echo "Port $((port+1)) configured"
done

echo "All ports configured for discharge"
```

### Example 4: Auto-stop on Complete

```python
import requests
import time

BASE_URL = "http://192.168.4.1"

def wait_for_completion(port_num):
    while True:
        response = requests.get(f"{BASE_URL}/api/status")
        port = response.json()['ports'][port_num]
        
        if port['status'] == 2:  # Complete
            print(f"Port {port_num+1} complete!")
            print(f"  Capacity: {port['mAh']:.1f} mAh")
            print(f"  Energy: {port['Wh']:.2f} Wh")
            break
        elif port['status'] == 3:  # Error
            print(f"Port {port_num+1} error!")
            break
        
        time.sleep(5)

# Wait for port 1 to complete
wait_for_completion(0)
```

---

## 🛡️ Error Handling

### HTTP Error Responses

**400 Bad Request:**
```json
{
  "error": "Invalid parameters",
  "details": "Port number must be 0-3"
}
```

**500 Internal Server Error:**
```json
{
  "error": "Internal server error",
  "details": "Failed to read sensor data"
}
```

### Common Error Scenarios

1. **Invalid Port Number:**
   - Request: `port=5`
   - Response: `400 Bad Request`

2. **Invalid Voltage Range:**
   - Request: `voltage=5.0`
   - Response: `400 Bad Request`

3. **Sensor Communication Error:**
   - Port status will be `3` (Error)
   - Error message available in port data

---

## 🔒 Security Considerations

### Current Implementation

- ✅ WiFi AP with password protection
- ✅ Local network only (no internet exposure)
- ⚠️ No API authentication
- ⚠️ No HTTPS encryption
- ⚠️ No rate limiting

### Recommendations for Production

1. **Change Default Password:**
   ```cpp
   #define AP_PASSWORD "your_strong_password_here"
   ```

2. **Add Basic Auth (Optional):**
   ```cpp
   server->on("/api/*", HTTP_ANY, [](AsyncWebServerRequest *request){
       if(!request->authenticate("admin", "password"))
           return request->requestAuthentication();
       // ... handle request
   });
   ```

3. **Enable HTTPS (Advanced):**
   - Requires certificate generation
   - ESP32 supports HTTPS but increases complexity

4. **Network Isolation:**
   - Keep charger on separate WiFi network
   - Use firewall rules if necessary

---

## 📈 Rate Limits

### Current Limits

- **REST API:** No limit (use responsibly)
- **WebSocket:** 1 message/second from server
- **Concurrent connections:** 4 WebSocket clients max

### Recommendations

- Poll `/api/status` no faster than 1Hz
- Use WebSocket for real-time updates instead of polling
- Avoid rapid mode changes (<1 second intervals)

---

## 🧪 Testing API

### Using cURL

```bash
# Get status
curl http://192.168.4.1/api/status

# Set mode with verbose output
curl -v -X POST http://192.168.4.1/api/mode \
     -d "port=0&mode=2"

# Download logs
curl -o battery_log.csv http://192.168.4.1/api/logs
```

### Using Postman

1. Import collection (create manually):
   - GET `http://192.168.4.1/api/status`
   - POST `http://192.168.4.1/api/mode`
   - POST `http://192.168.4.1/api/battery`
   - POST `http://192.168.4.1/api/cutoff`
   - POST `http://192.168.4.1/api/reset`
   - GET `http://192.168.4.1/api/logs`

2. Set Content-Type to `application/x-www-form-urlencoded`

3. Test each endpoint

### Using Python Requests

```python
import requests

BASE_URL = "http://192.168.4.1/api"

# Get status
response = requests.get(f"{BASE_URL}/status")
print(response.json())

# Set mode
response = requests.post(f"{BASE_URL}/mode", 
                        data={"port": 0, "mode": 2})
print(response.text)

# Download logs
response = requests.get(f"{BASE_URL}/logs")
with open("battery_log.csv", "w") as f:
    f.write(response.text)
```

---

## 📝 Changelog

### v2.0.0 (Current)
- ✅ Initial REST API implementation
- ✅ WebSocket real-time updates
- ✅ CSV log export
- ✅ Multi-port control

### Planned (v2.1)
- [ ] Basic authentication
- [ ] HTTPS support
- [ ] MQTT integration
- [ ] Data logging to SD card
- [ ] Historical data API

---

## 🆘 Support

### Documentation
- Main README: `README.md`
- Quick Start: `QUICKSTART.md`
- Wiring Guide: `WIRING.md`

### Community
- GitHub Issues: [Report bugs](https://github.com/YOUR_USERNAME/DIY-Charger-Simple/issues)
- Discussions: [Ask questions](https://github.com/YOUR_USERNAME/DIY-Charger-Simple/discussions)

---

**Last Updated:** 2025-01-XX  
**API Version:** 2.0.0  
**Status:** Production Ready ✅
#include "WebUI.h"

// ============================================
// CONSTRUCTOR
// ============================================

WebUI::WebUI(PortData* data) {
    portData = data;
    server = new AsyncWebServer(WEB_PORT);
    ws = new AsyncWebSocket("/ws");
    lastUpdate = 0;
}

// ============================================
// INITIALIZATION
// ============================================

bool WebUI::begin() {
    // Start WiFi Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CLIENTS);
    
    IPAddress IP = WiFi.softAPIP();
    DEBUG_PRINT("AP IP address: ");
    DEBUG_PRINTLN(IP);
    
    // Setup WebSocket
    ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
        this->onWsEvent(server, client, type, arg, data, len);
    });
    server->addHandler(ws);
    
    // Setup HTTP routes
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleRoot(request);
    });
    
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetStatus(request);
    });
    
    server->on("/api/mode", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleSetMode(request);
    });
    
    server->on("/api/battery", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleSetBattery(request);
    });
    
    server->on("/api/cutoff", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleSetCutoff(request);
    });
    
    server->on("/api/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleReset(request);
    });
    
    server->on("/api/logs", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetLogs(request);
    });
    
    // Start server
    server->begin();
    DEBUG_PRINTLN("Web server started");
    
    return true;
}

// ============================================
// UPDATE LOOP
// ============================================

void WebUI::update() {
    ws->cleanupClients();
    
    unsigned long currentTime = millis();
    if (currentTime - lastUpdate >= WS_UPDATE_INTERVAL) {
        broadcastStatus();
        lastUpdate = currentTime;
    }
}

// ============================================
// HTTP HANDLERS
// ============================================

void WebUI::handleRoot(AsyncWebServerRequest *request) {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>DIY Charger Simple</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: Arial, sans-serif; background: #1a1a1a; color: #fff; padding: 20px; }
        h1 { text-align: center; margin-bottom: 30px; color: #4CAF50; }
        .container { max-width: 1200px; margin: 0 auto; }
        .port-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; }
        .port-card { background: #2a2a2a; border-radius: 10px; padding: 20px; border: 2px solid #333; }
        .port-card.active { border-color: #4CAF50; }
        .port-card.error { border-color: #f44336; }
        .port-header { display: flex; justify-content: space-between; margin-bottom: 15px; }
        .port-title { font-size: 1.2em; font-weight: bold; }
        .status-badge { padding: 5px 10px; border-radius: 5px; font-size: 0.8em; }
        .status-idle { background: #666; }
        .status-active { background: #4CAF50; }
        .status-complete { background: #2196F3; }
        .status-error { background: #f44336; }
        .metrics { margin: 15px 0; }
        .metric { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid #333; }
        .metric-label { color: #999; }
        .metric-value { font-weight: bold; color: #4CAF50; }
        .controls { margin-top: 15px; }
        .control-group { margin-bottom: 10px; }
        label { display: block; margin-bottom: 5px; color: #999; font-size: 0.9em; }
        select, input { width: 100%; padding: 8px; background: #1a1a1a; border: 1px solid #444; color: #fff; border-radius: 5px; }
        button { width: 100%; padding: 10px; margin-top: 10px; border: none; border-radius: 5px; cursor: pointer; font-weight: bold; }
        .btn-start { background: #4CAF50; color: white; }
        .btn-stop { background: #f44336; color: white; }
        .btn-reset { background: #FF9800; color: white; }
        button:hover { opacity: 0.8; }
        .footer { text-align: center; margin-top: 30px; color: #666; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔋 DIY Charger Simple</h1>
        <div class="port-grid" id="portGrid"></div>
        <div class="footer">
            <p>Connected to: <span id="apName">DIY-Charger</span></p>
        </div>
    </div>
    
    <script>
        let ws;
        
        function connectWebSocket() {
            ws = new WebSocket('ws://' + location.hostname + '/ws');
            
            ws.onopen = () => { console.log('WebSocket connected'); };
            ws.onclose = () => { setTimeout(connectWebSocket, 3000); };
            ws.onmessage = (e) => { updateUI(JSON.parse(e.data)); };
        }
        
        function updateUI(data) {
            const grid = document.getElementById('portGrid');
            grid.innerHTML = '';
            
            data.ports.forEach((port, idx) => {
                const card = createPortCard(idx, port);
                grid.appendChild(card);
            });
        }
        
        function createPortCard(idx, port) {
            const div = document.createElement('div');
            div.className = 'port-card ' + (port.active ? 'active' : '');
            if (port.status === 3) div.className += ' error';
            
            div.innerHTML = `
                <div class="port-header">
                    <div class="port-title">Port ${idx + 1}</div>
                    <div class="status-badge status-${getStatusClass(port.status)}">${getStatusText(port.status)}</div>
                </div>
                <div class="metrics">
                    <div class="metric"><span class="metric-label">Voltage:</span><span class="metric-value">${port.voltage.toFixed(3)} V</span></div>
                    <div class="metric"><span class="metric-label">Current:</span><span class="metric-value">${port.current.toFixed(3)} A</span></div>
                    <div class="metric"><span class="metric-label">Power:</span><span class="metric-value">${port.power.toFixed(2)} W</span></div>
                    <div class="metric"><span class="metric-label">Capacity:</span><span class="metric-value">${port.mAh.toFixed(0)} mAh</span></div>
                    <div class="metric"><span class="metric-label">Energy:</span><span class="metric-value">${port.Wh.toFixed(2)} Wh</span></div>
                </div>
                <div class="controls">
                    <div class="control-group">
                        <label>Mode:</label>
                        <select id="mode${idx}" onchange="setMode(${idx}, this.value)">
                            <option value="0" ${port.mode === 0 ? 'selected' : ''}>Safety</option>
                            <option value="1" ${port.mode === 1 ? 'selected' : ''}>Charging</option>
                            <option value="2" ${port.mode === 2 ? 'selected' : ''}>Discharging</option>
                        </select>
                    </div>
                    <div class="control-group">
                        <label>Battery Type:</label>
                        <select id="battery${idx}" onchange="setBattery(${idx}, this.value)">
                            <option value="0" ${port.batteryType === 0 ? 'selected' : ''}>Li-ion (3.0V)</option>
                            <option value="1" ${port.batteryType === 1 ? 'selected' : ''}>LiFePO4 (2.5V)</option>
                            <option value="2" ${port.batteryType === 2 ? 'selected' : ''}>LiPo (3.0V)</option>
                        </select>
                    </div>
                    <div class="control-group">
                        <label>Custom Cutoff (V):</label>
                        <input type="number" id="cutoff${idx}" step="0.1" min="2.0" max="3.5" value="${port.customCutoff.toFixed(1)}" onchange="setCutoff(${idx}, this.value)">
                    </div>
                    <button class="btn-reset" onclick="resetPort(${idx})">Reset Data</button>
                </div>
            `;
            
            return div;
        }
        
        function getStatusClass(status) {
            const classes = ['idle', 'active', 'complete', 'error'];
            return classes[status] || 'idle';
        }
        
        function getStatusText(status) {
            const texts = ['Idle', 'Active', 'Complete', 'Error'];
            return texts[status] || 'Unknown';
        }
        
        function setMode(port, mode) {
            fetch('/api/mode', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `port=${port}&mode=${mode}`
            });
        }
        
        function setBattery(port, type) {
            fetch('/api/battery', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `port=${port}&type=${type}`
            });
        }
        
        function setCutoff(port, voltage) {
            fetch('/api/cutoff', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `port=${port}&voltage=${voltage}`
            });
        }
        
        function resetPort(port) {
            if (confirm('Reset all data for Port ' + (port + 1) + '?')) {
                fetch('/api/reset', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: `port=${port}`
                });
            }
        }
        
        connectWebSocket();
    </script>
</body>
</html>
)rawliteral";
    
    request->send(200, "text/html", html);
}

void WebUI::handleGetStatus(AsyncWebServerRequest *request) {
    request->send(200, "application/json", getStatusJSON());
}

void WebUI::handleSetMode(AsyncWebServerRequest *request) {
    if (request->hasParam("port", true) && request->hasParam("mode", true)) {
        int port = request->getParam("port", true)->value().toInt();
        int mode = request->getParam("mode", true)->value().toInt();
        
        if (port >= 0 && port < NUM_PORTS && mode >= 0 && mode <= 2) {
            portData[port].mode = (OperationMode)mode;
            
            if (mode == SAFETY) {
                portData[port].active = false;
            } else {
                portData[port].active = true;
                if (portData[port].startTime == 0) {
                    portData[port].startTime = millis();
                }
            }
            
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(400, "text/plain", "Invalid parameters");
}

void WebUI::handleSetBattery(AsyncWebServerRequest *request) {
    if (request->hasParam("port", true) && request->hasParam("type", true)) {
        int port = request->getParam("port", true)->value().toInt();
        int type = request->getParam("type", true)->value().toInt();
        
        if (port >= 0 && port < NUM_PORTS && type >= 0 && type <= 2) {
            portData[port].batteryType = (BatteryType)type;
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(400, "text/plain", "Invalid parameters");
}

void WebUI::handleSetCutoff(AsyncWebServerRequest *request) {
    if (request->hasParam("port", true) && request->hasParam("voltage", true)) {
        int port = request->getParam("port", true)->value().toInt();
        float voltage = request->getParam("voltage", true)->value().toFloat();
        
        if (port >= 0 && port < NUM_PORTS && voltage >= 2.0 && voltage <= 3.5) {
            portData[port].customCutoff = voltage;
            portData[port].useCustomCutoff = true;
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(400, "text/plain", "Invalid parameters");
}

void WebUI::handleReset(AsyncWebServerRequest *request) {
    if (request->hasParam("port", true)) {
        int port = request->getParam("port", true)->value().toInt();
        
        if (port >= 0 && port < NUM_PORTS) {
            portData[port].reset();
            request->send(200, "text/plain", "OK");
            return;
        }
    }
    request->send(400, "text/plain", "Invalid parameters");
}

void WebUI::handleGetLogs(AsyncWebServerRequest *request) {
    String csv = "Timestamp,Port,Voltage,Current,Power,mAh,Wh,Mode,Battery,Status\n";
    
    // This is a placeholder - in full implementation, read from storage
    for (int i = 0; i < NUM_PORTS; i++) {
        if (portData[i].active) {
            unsigned long ts = (millis() - portData[i].startTime) / 1000;
            csv += String(ts) + "," + String(i) + ",";
            csv += String(portData[i].voltage, 3) + ",";
            csv += String(portData[i].current, 3) + ",";
            csv += String(portData[i].power, 2) + ",";
            csv += String(portData[i].mAh, 1) + ",";
            csv += String(portData[i].Wh, 2) + ",";
            csv += portData[i].getModeName();
csv += ",";
csv += portData[i].getBatteryName();
csv += ",";
csv += portData[i].getStatusName();
csv += "\n";
        }
    }
    
    request->send(200, "text/csv", csv);
}

// ============================================
// WEBSOCKET HANDLERS
// ============================================

void WebUI::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        DEBUG_PRINTF("WebSocket client #%u connected\n", client->id());
        client->text(getStatusJSON());
    } else if (type == WS_EVT_DISCONNECT) {
        DEBUG_PRINTF("WebSocket client #%u disconnected\n", client->id());
    }
}

void WebUI::broadcastStatus() {
    if (ws->count() > 0) {
        ws->textAll(getStatusJSON());
    }
}

// ============================================
// JSON GENERATION
// ============================================

String WebUI::getStatusJSON() {
    StaticJsonDocument<2048> doc;
    JsonArray ports = doc.createNestedArray("ports");
    
    for (int i = 0; i < NUM_PORTS; i++) {
        JsonObject port = ports.createNestedObject();
        port["voltage"] = portData[i].voltage;
        port["current"] = portData[i].current;
        port["power"] = portData[i].power;
        port["mAh"] = portData[i].mAh;
        port["Wh"] = portData[i].Wh;
        port["mode"] = portData[i].mode;
        port["batteryType"] = portData[i].batteryType;
        port["customCutoff"] = portData[i].customCutoff;
        port["status"] = portData[i].status;
        port["active"] = portData[i].active;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

void WebUI::notifyClients(const String& message) {
    if (ws->count() > 0) {
        ws->textAll(message);
    }
}

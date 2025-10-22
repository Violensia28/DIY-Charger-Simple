#ifndef WEBUI_H
#define WEBUI_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "BatteryTypes.h"

// ============================================
// WEB UI CLASS
// ============================================

class WebUI {
private:
    AsyncWebServer* server;
    AsyncWebSocket* ws;
    PortData* portData;
    
    unsigned long lastUpdate;
    
    // Request handlers
    void handleRoot(AsyncWebServerRequest *request);
    void handleGetStatus(AsyncWebServerRequest *request);
    void handleSetMode(AsyncWebServerRequest *request);
    void handleSetBattery(AsyncWebServerRequest *request);
    void handleSetCutoff(AsyncWebServerRequest *request);
    void handleReset(AsyncWebServerRequest *request);
    void handleGetLogs(AsyncWebServerRequest *request);
    
    // WebSocket handlers
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                   AwsEventType type, void *arg, uint8_t *data, size_t len);
    
    // Helper functions
    String getStatusJSON();
    String getPortJSON(int port);
    void broadcastStatus();
    
public:
    WebUI(PortData* data);
    
    bool begin();
    void update();
    
    void notifyClients(const String& message);
};

#endif // WEBUI_H

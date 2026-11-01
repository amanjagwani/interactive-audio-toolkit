#ifndef WEB_H
#define WEB_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include "controlHandler.h"

namespace iat
{

inline AsyncWebServer webServer(80);
inline AsyncWebSocket webSocket("/ws");
inline ControlRing webInbox;
inline bool webRestartPending = false;

inline const char *wifiSetupPage = R"(<!DOCTYPE html><html><body><h3>WiFi setup</h3>
<input id="s" placeholder="SSID"><br><input id="p" type="password" placeholder="Password"><br>
<button onclick="const w=new WebSocket('ws://'+location.host+'/ws');w.onopen=()=>{w.send(JSON.stringify({controlName:'wifiCredentials',value:{ssid:s.value,password:p.value}}));document.body.innerHTML='Saved, rebooting'}">Save</button>
</body></html>)";

inline void onWebSocketEvent(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type != WS_EVT_DATA)
    {
        return;
    }
    AwsFrameInfo *info = static_cast<AwsFrameInfo *>(arg);
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        sendControlMessage(webInbox, reinterpret_cast<const char *>(data), len);
    }
}

inline void webSetup()
{
    Preferences prefs;
    prefs.begin("iat-wifi", false);
    String ssid = prefs.getString("ssid", "");
    String password = prefs.getString("password", "");
    prefs.end();

    if (ssid.length() > 0)
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++)
        {
            delay(500);
        }
    }
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP(deviceName.c_str());
    }
    deviceMac = WiFi.macAddress();
    MDNS.begin(deviceName.c_str());
    LittleFS.begin(true);

    controlHandlers["wifiCredentials"] = [](JsonVariant value) {
        Preferences prefs;
        prefs.begin("iat-wifi", false);
        prefs.putString("ssid", value["ssid"].as<String>());
        prefs.putString("password", value["password"].as<String>());
        prefs.end();
        webRestartPending = true;
    };

    webSocket.onEvent(onWebSocketEvent);
    webServer.addHandler(&webSocket);
    webServer.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", wifiSetupPage);
    });
    webServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    webServer.onNotFound([](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    webServer.begin();

    addTransport([](const char *json, size_t len) {
        webSocket.textAll(json, len);
    });
}

inline void webLoop()
{
    drainControlRing(webInbox);
    webSocket.cleanupClients();
    if (webRestartPending)
    {
        delay(500);
        ESP.restart();
    }
}

}

#endif

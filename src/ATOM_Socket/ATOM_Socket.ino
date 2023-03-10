

#include "M5Atom.h"
#include "AtomSocket.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "index.h"

const char* ssid     = "aaaaa";
const char* password = "bbbbb";

int Voltage, ActivePower = 0;
float Current = 0;

WebServer server(80);

void handleRoot() {
    server.send(200, "text/html", html);
}

#define RXD   22
#define RELAY 23

ATOMSOCKET ATOM;

HardwareSerial AtomSerial(2);

bool RelayFlag = false;

String DataCreate() {
    String RelayState = (RelayFlag) ? "on" : "off";
    String Data = "vol:<mark>" + String(Voltage) + "</mark>V#current:<mark>" +
                  String(Current) + "</mark>A#power:<mark>" +
                  String(ActivePower) + "</mark>W#state:<mark>" + RelayState +
                  "</mark>";
    return Data;
}

void setup() {
    M5.begin(true, false, true);
    M5.dis.drawpix(0, 0xfff000);
    ATOM.Init(AtomSerial, RELAY, RXD);
    WiFi.softAP(ssid, password);
    Serial.print("AP SSID: ");
    Serial.println(ssid);
    Serial.print("AP PASSWORD: ");
    Serial.println(password);
    Serial.print("IP address: ");
    Serial.println(
        WiFi.softAPIP());  // IP address assigned to your ESP  获取ip地址
    server.on("/", handleRoot);

    server.on("/on", []() {
        RelayFlag = true;
        server.send(200, "text/plain", DataCreate());
    });

    server.on("/off", []() {
        RelayFlag = false;
        server.send(200, "text/plain", DataCreate());
    });

    server.on("/data", []() { server.send(200, "text/plain", DataCreate()); });

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    ATOM.SerialReadLoop();
    if (ATOM.SerialRead == 1) {
        Voltage     = ATOM.GetVol();
        Current     = ATOM.GetCurrent();
        ActivePower = ATOM.GetActivePower();
        Serial.print("Voltage: ");
        Serial.print(Voltage);
        Serial.println(" V");
        Serial.print("Current: ");
        Serial.print(Current);
        Serial.println(" A");
        Serial.print("ActivePower: ");
        Serial.print(ActivePower);
        Serial.println(" W");
    }
    if (M5.Btn.wasPressed()) {
        RelayFlag = !RelayFlag;
    }
    if (RelayFlag) {
        M5.dis.drawpix(0, 0xff0000);
        ATOM.SetPowerOn();
    } else {
        ATOM.SetPowerOff();
        M5.dis.drawpix(0, 0xfff000);
    }
    M5.update();
    server.handleClient();
}

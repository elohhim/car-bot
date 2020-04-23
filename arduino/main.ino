#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET D5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* WIFI_MANAGER_SSID = "CARBOT";
const char* WIFI_MANAGER_PASSWORD = "12345678";

ESP8266WebServer server(80);

uint8_t LEDpin = D6;
bool LEDState = false;

void setup() {
    Serial.begin(115200);
    pinMode(LEDpin, OUTPUT);
    delay(1000);
    setupDisplay();
    setupWifiManager();
    setupServer();
}

void setupDisplay() {
    Serial.println("Setting up display");
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
    }
    display.display();
    delay(2000);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
}

void setupWifiManager() {
    Serial.println("# Setting up WiFi connection");
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.autoConnect(WIFI_MANAGER_SSID, WIFI_MANAGER_PASSWORD);

    Serial.println("# Finished seting up WiFi connection");
}

void setupServer() {
    Serial.println("# Setting up server");
    setupRoutes();
    server.begin();
    Serial.println("# Finished setting up server");
}

void setupRoutes() {
    setupRoute("/", onRoot);
    setupRoute("/led", onLED);
    server.on("/resetWifi", onResetWifi);
    server.onNotFound(onNotFound);
}

void setupRoute(const String &uri, std::function<void ()> handler) {
    setupRoute(uri, HTTP_GET, handler);
}

void setupRoute(const String &uri, HTTPMethod httpMethod, std::function<void ()> handler) {
    // Some interceptor would be momre handy here
    server.on(uri, [=]() {
        Serial.print("# ");
        switch(httpMethod) {
            case HTTP_GET:
                Serial.print("GET");
                break;
            case HTTP_POST:
                Serial.print("POST");
                break;
        }
        Serial.print(" ");
        Serial.print(uri);
        handler();
    });
}

void onRoot() {
    server.send(200, "text/html", "Hello NodeMCU World!");
}

void onResetWifi() {
    server.send(200, "text/plain", "Resetting WiFi connection");
    Serial.println("Resetting WiFi connection settings");
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    delay(1000);
    Serial.println("Restarting hardware...");
    ESP.restart();
}

void onLED() {
    LEDState = !LEDState;
}

void onNotFound() {
    server.send(404, "text/plain", "Not found");
} 

void loop() {
    server.handleClient();
    display.setCursor(10, 10);
    display.println(WiFi.localIP().toString().c_str());
    display.display();
    digitalWrite(LEDpin, LEDState ? HIGH : LOW);
}
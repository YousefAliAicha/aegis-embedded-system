/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║  AEGIS — Networked Rotating Ultrasonic Radar System  v1.0        ║
 * ║  Board   : Deneyap Kart 1A (ESP32-S3)                            ║
 * ║  LCD     : 1602A, HD44780-compatible, 4-bit parallel mode        ║
 * ║  Author  : [Yousef Ali Aicha]  |  Portfolio Project              ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 *  Pin Assignments
 *  ───────────────────────────────────────────────────────────────────
 *  D0  ── SG90 servo PWM signal
 *  D1  ── HC-SR04 TRIG (10 µs output pulse)
 *  D11 ── HC-SR04 ECHO (input, isolated from flash bus)
 *
 *  D10 ── LCD RS  (Register Select)
 *  D4  ── LCD E   (Enable / clock)
 *  D5  ── LCD D4  (data bit 4)
 *  D6  ── LCD D5  (data bit 5)
 *  D7  ── LCD D6  (data bit 6)
 *  D12 ── LCD D7  (data bit 7, isolated from flash bus)
 *
 *  Power Notes
 *  ───────────────────────────────────────────────────────────────────
 *  SG90 VCC    → MB102 5 V rail (never power servo from board 3.3 V)
 *  HC-SR04 VCC → MB102 5 V rail
 *  LCD VDD     → MB102 5 V rail
 *  LCD VO      → GND  (10k Potentiometer Wiper)
 *  LCD RW      → GND  (write-only mode)
 *  MB102 GND   → Deneyap GND (mandatory common-ground reference)
 *
 *  Communication Protocol (WebSocket JSON)
 *  ───────────────────────────────────────────────────────────────────
 *  Outbound (ESP32 → browser), every sweep step:
 *    { "a": <angle 0-180>, "d": <dist cm | -1>, "det": <bool> }
 *
 *  Inbound commands (browser → ESP32):
 *    { "cmd": "SET_SWEEP_MIN",   "val": <0-180> }
 *    { "cmd": "SET_SWEEP_MAX",   "val": <0-180> }
 *    { "cmd": "SET_STEP_DELAY",  "val": <ms>    }
 *    { "cmd": "SET_THRESHOLD",   "val": <cm>    }
 *    { "cmd": "SET_STEP_DEG",    "val": <1-10>  }
 *    { "cmd": "MANUAL_STEER",    "val": <0-180> }
 *    { "cmd": "AUTO_RESUME"                     }
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>   // Install: "WebSockets" by Markus Sattler
#include <ESP32Servo.h>         // Install: "ESP32Servo" by Kevin Harrington
#include <LiquidCrystal.h>      // Built-in Arduino library
#include <ArduinoJson.h>        // Install: "ArduinoJson" by Benoit Blanchon

#include "secrets.h"
#include "dashboard.h"          // self-contained HTML dashboard, served over HTTP

// ── Pin definitions ──────────────────────────────────────────────────
#define SERVO_PIN   D0
#define TRIG_PIN    D1
#define ECHO_PIN    D11

#define LCD_RS      D10
#define LCD_EN      D4
#define LCD_D4      D5
#define LCD_D5      D6
#define LCD_D6      D7
#define LCD_D7      D12

// ── Radar defaults (all runtime-adjustable via WebSocket commands) ───
#define DEFAULT_SWEEP_MIN_DEG      0
#define DEFAULT_SWEEP_MAX_DEG      180
#define DEFAULT_SWEEP_STEP_DEG     2
#define DEFAULT_STEP_DELAY_MS      25
#define DEFAULT_DETECT_THRESHOLD_CM 40
#define MAX_RANGE_CM               300
#define SONAR_TIMEOUT_US           30000   // ~514 cm, safely beyond MAX_RANGE_CM

// ── Objects ──────────────────────────────────────────────────────────
Servo            radarServo;
LiquidCrystal    lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
WebServer        httpServer(80);
WebSocketsServer wsServer(81);

// ── Runtime-tunable parameters ───────────────────────────────────────
int   sweepMinDeg      = DEFAULT_SWEEP_MIN_DEG;
int   sweepMaxDeg      = DEFAULT_SWEEP_MAX_DEG;
int   sweepStepDeg     = DEFAULT_SWEEP_STEP_DEG;
int   stepDelayMs      = DEFAULT_STEP_DELAY_MS;
int   detectThreshCm   = DEFAULT_DETECT_THRESHOLD_CM;

// ── Sweep state ──────────────────────────────────────────────────────
int           currentAngle   = DEFAULT_SWEEP_MIN_DEG;
int           sweepDir       = 1;          // +1 → towards max, -1 → towards min
float         currentDistCm  = -1.0f;
bool          detected       = false;
bool          manualMode     = false;      // pauses auto-sweep when true
unsigned long lastStepTime   = 0;

// ─────────────────────────────────────────────────────────────────────
//  SONAR — fires trigger pulse, measures echo, returns distance in cm.
//  Returns -1.0 on timeout (out of range) or readings beyond MAX_RANGE_CM.
// ─────────────────────────────────────────────────────────────────────
float measureDistanceCm() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(4);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, SONAR_TIMEOUT_US);
    if (duration == 0) return -1.0f;

    float cm = (duration * 0.0343f) / 2.0f;
    return (cm > MAX_RANGE_CM) ? -1.0f : cm;
}

// ─────────────────────────────────────────────────────────────────────
//  LCD — overwrites in place to prevent flicker (no lcd.clear() in loop)
// ─────────────────────────────────────────────────────────────────────
void updateLCD(int angle, float dist, bool det) {
    char row0[17];
    char row1[17];

    if (dist < 0.0f) {
        snprintf(row0, sizeof(row0), "A:%3d\xDF D: --- cm", angle);
    } else {
        snprintf(row0, sizeof(row0), "A:%3d\xDF D:%4dcm ", angle, (int)dist);
    }

    snprintf(row1, sizeof(row1), det ? "** DETECTED **  " : "  Scanning...   ");

    lcd.setCursor(0, 0); lcd.print(row0);
    lcd.setCursor(0, 1); lcd.print(row1);
}

// ─────────────────────────────────────────────────────────────────────
//  WebSocket — broadcast current telemetry frame to all connected clients
// ─────────────────────────────────────────────────────────────────────
void broadcastTelemetry() {
    // Short keys to minimise over-the-air JSON payload size
    StaticJsonDocument<128> doc;
    doc["a"]   = currentAngle;
    doc["d"]   = (currentDistCm < 0.0f) ? -1.0f : currentDistCm;
    doc["det"] = detected;

    char buf[128];
    serializeJson(doc, buf);
    wsServer.broadcastTXT(buf);
}

// ─────────────────────────────────────────────────────────────────────
//  WebSocket — parse and apply inbound command from browser
// ─────────────────────────────────────────────────────────────────────
void handleCommand(const String& raw) {
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, raw) != DeserializationError::Ok) return;

    const char* cmd = doc["cmd"];
    if (!cmd) return;

    if (strcmp(cmd, "SET_SWEEP_MIN") == 0) {
        sweepMinDeg = constrain((int)doc["val"], 0, sweepMaxDeg - 1);

    } else if (strcmp(cmd, "SET_SWEEP_MAX") == 0) {
        sweepMaxDeg = constrain((int)doc["val"], sweepMinDeg + 1, 180);

    } else if (strcmp(cmd, "SET_STEP_DELAY") == 0) {
        stepDelayMs = constrain((int)doc["val"], 5, 200);

    } else if (strcmp(cmd, "SET_THRESHOLD") == 0) {
        detectThreshCm = constrain((int)doc["val"], 1, MAX_RANGE_CM);

    } else if (strcmp(cmd, "SET_STEP_DEG") == 0) {
        sweepStepDeg = constrain((int)doc["val"], 1, 10);

    } else if (strcmp(cmd, "MANUAL_STEER") == 0) {
        manualMode   = true;
        currentAngle = constrain((int)doc["val"], 0, 180);
        radarServo.write(currentAngle);

    } else if (strcmp(cmd, "AUTO_RESUME") == 0) {
        manualMode = false;
    }
}

// ─────────────────────────────────────────────────────────────────────
//  WebSocket event handler
// ─────────────────────────────────────────────────────────────────────
void onWebSocketEvent(uint8_t clientId,
                      WStype_t type,
                      uint8_t* payload,
                      size_t   length)
{
    switch (type) {
        case WStype_CONNECTED:
            Serial.printf("[WS] Client #%u connected\n", clientId);
            break;

        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u disconnected\n", clientId);
            break;

        case WStype_TEXT:
            handleCommand(String((char*)payload));
            break;

        default:
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────
//  HTTP — serve the self-contained dashboard (stored in dashboard.h)
// ─────────────────────────────────────────────────────────────────────
void handleRoot() {
    httpServer.send_P(200, "text/html", DASHBOARD_HTML);
}

// ─────────────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    // LCD
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("AEGIS  v1.0     ");
    lcd.setCursor(0, 1); lcd.print("Initialising... ");

    // GPIO
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);

    // Servo — park at minimum angle
    radarServo.attach(SERVO_PIN, 500, 2400);
    radarServo.write(sweepMinDeg);
    delay(500);

    // Wi-Fi
    lcd.setCursor(0, 1); lcd.print("Connecting WiFi ");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(400);
        Serial.print('.');
        attempts++;
    }

    lcd.clear();

    if (WiFi.status() == WL_CONNECTED) {
        // HTTP server — serves dashboard page
        httpServer.on("/", handleRoot);
        httpServer.begin();

        // WebSocket server on port 81
        wsServer.begin();
        wsServer.onEvent(onWebSocketEvent);

        lcd.setCursor(0, 0); lcd.print("ONLINE          ");
        lcd.setCursor(0, 1); lcd.print(WiFi.localIP());

        Serial.printf("\n[AEGIS] Dashboard → http://%s\n",
                      WiFi.localIP().toString().c_str());
        Serial.printf("[AEGIS] WebSocket → ws://%s:81\n",
                      WiFi.localIP().toString().c_str());
        delay(3000);
    } else {
        lcd.setCursor(0, 0); lcd.print("WIFI TIMEOUT    ");
        lcd.setCursor(0, 1); lcd.print("OFFLINE MODE    ");
        Serial.println("\n[AEGIS] Wi-Fi failed. Running offline radar loop.");
        delay(2000);
    }

    lcd.clear();
    lastStepTime = millis();
}

// ─────────────────────────────────────────────────────────────────────
//  MAIN LOOP
// ─────────────────────────────────────────────────────────────────────
void loop() {
    // Service WebSocket and HTTP stacks
    if (WiFi.status() == WL_CONNECTED) {
        wsServer.loop();
        httpServer.handleClient();
    }

    unsigned long now = millis();

    // Non-blocking sweep tick
    if (!manualMode && (now - lastStepTime >= (unsigned long)stepDelayMs)) {
        lastStepTime = now;

        radarServo.write(currentAngle);

        float dist = measureDistanceCm();
        bool  det  = (dist > 0.0f && dist < detectThreshCm);

        currentDistCm = dist;
        detected      = det;

        updateLCD(currentAngle, dist, det);
        broadcastTelemetry();

        Serial.printf("[AEGIS] Pos: %3d° | Dist: ", currentAngle);
        if (dist < 0.0f) Serial.print("  OOR     ");
        else             Serial.printf("%5.1f cm  ", dist);
        Serial.println(det ? "[INTERCEPTED]" : "[CLEAR]");

        // Advance angle and reverse at boundaries
        currentAngle = constrain(
            currentAngle + (sweepDir * sweepStepDeg),
            sweepMinDeg,
            sweepMaxDeg
        );

        if (currentAngle >= sweepMaxDeg) sweepDir = -1;
        else if (currentAngle <= sweepMinDeg) sweepDir = 1;
    }
}

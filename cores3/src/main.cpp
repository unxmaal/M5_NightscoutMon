#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <SD.h>

#include "config.h"
#include "nightscout.h"
#include "display.h"
#include "alerts.h"
#include "ns_config_parse.h"

/* ── Globals ───────────────────────────────────────────────────── */

static Config     cfg;
static NSinfo     ns;
static ErrorLog   errLog;
static AlarmState alarmState;

static WiFiMulti  wifiMulti;

static int  currentPage       = PAGE_GLUCOSE;
static int  brightnessLevel   = 0;
static int  brightnessValues[3];
static unsigned long lastNsCheck = 0;
static int  pollCount         = 0;

static const char *ntpServer = "pool.ntp.org";

/* ── Config loading from SD card INI ──────────────────────────── */

static bool loadConfigFromSD() {
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD card mount failed");
        return false;
    }

    File f = SD.open("/M5NS.INI", FILE_READ);
    if (!f) {
        Serial.println("M5NS.INI not found on SD");
        SD.end();
        return false;
    }

    size_t fileSize = f.size();
    if (fileSize == 0 || fileSize > 8192) {
        Serial.printf("M5NS.INI bad size: %u\n", (unsigned)fileSize);
        f.close();
        SD.end();
        return false;
    }

    char *buf = new char[fileSize + 1];
    size_t bytesRead = f.read((uint8_t *)buf, fileSize);
    f.close();
    SD.end();

    buf[bytesRead] = '\0';

    int parsed = parseConfigBuffer(buf, bytesRead, &cfg);
    delete[] buf;

    Serial.printf("Parsed %d config values from M5NS.INI\n", parsed);
    return parsed > 0;
}

/* ── WiFi ──────────────────────────────────────────────────────── */

static void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    for (int i = 0; i < CFG_MAX_WLAN; i++) {
        if (cfg.wlanssid[i][0] != '\0')
            wifiMulti.addAP(cfg.wlanssid[i], cfg.wlanpass[i]);
    }

    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setFont(&FreeSans9pt7b);
    M5.Display.setTextDatum(TL_DATUM);
    M5.Display.drawString("Connecting WiFi...", 10, 100);

    int attempts = 0;
    while (wifiMulti.run() != WL_CONNECTED && attempts < 40) {
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.drawString("WiFi connected", 10, 100);
        M5.Display.drawString(WiFi.localIP().toString().c_str(), 10, 120);
        delay(1000);
    } else {
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_RED, TFT_BLACK);
        M5.Display.drawString("WiFi FAILED", 10, 100);
        delay(2000);
    }

    // NTP time sync
    configTime(cfg.timeZone, cfg.dst, ntpServer, "time.nist.gov", "time.google.com");
    struct tm timeinfo;
    for (int i = 0; i < 30; i++) {
        if (getLocalTime(&timeinfo))
            break;
        delay(1000);
    }
}

/* ── Brightness ────────────────────────────────────────────────── */

static void cycleBrightness() {
    brightnessLevel = (brightnessLevel + 1) % 3;
    M5.Display.setBrightness(brightnessValues[brightnessLevel]);
}

/* ── Nightscout polling ────────────────────────────────────────── */

static void pollNightscout() {
    if (millis() - lastNsCheck < 15000)
        return;
    lastNsCheck = millis();

    // Only poll if data is stale (>5 min) and we've waited a few cycles
    struct tm now;
    int sensorAgeSec = 86400;
    if (getLocalTime(&now))
        sensorAgeSec = (int)difftime(mktime(&now), ns.sensTime);

    if (sensorAgeSec > 305 && pollCount > 3) {
        pollCount = 0;
        readNightscout(cfg, ns, errLog);
    }
    pollCount++;

    drawPage(currentPage, cfg, ns, errLog);
}

/* ── Setup ─────────────────────────────────────────────────────── */

void setup() {
    auto m5cfg = M5.config();
    M5.begin(m5cfg);

    // Speaker
    M5.Speaker.begin();
    M5.Speaker.setVolume(64);

    // Load config: defaults first, then overlay from SD INI
    configDefaults(&cfg);
    loadConfigFromSD();

    brightnessValues[0] = cfg.brightness1;
    brightnessValues[1] = cfg.brightness2;
    brightnessValues[2] = cfg.brightness3;
    M5.Display.setBrightness(brightnessValues[brightnessLevel]);

    currentPage = cfg.default_page;

    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.setFont(&FreeSansBold18pt7b);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString(cfg.deviceName, 160, 60);
    M5.Display.setFont(&FreeSans9pt7b);
    M5.Display.drawString("CoreS3", 160, 100);

    connectWiFi();

    // Initial fetch
    readNightscout(cfg, ns, errLog);
    drawPage(currentPage, cfg, ns, errLog);
}

/* ── Loop ──────────────────────────────────────────────────────── */

void loop() {
    M5.update();

    // Button A (left touch zone): cycle brightness
    if (M5.BtnA.wasPressed()) {
        cycleBrightness();
    }

    // Button B (middle): snooze
    if (M5.BtnB.wasPressed()) {
        alarmState.snooze(cfg.snooze_timeout);
        drawPage(currentPage, cfg, ns, errLog);
    }

    // Button C (right): toggle page
    if (M5.BtnC.wasPressed()) {
        currentPage = (currentPage + 1) % NUM_PAGES;
        drawPage(currentPage, cfg, ns, errLog);
    }

    // Poll Nightscout + redraw
    pollNightscout();

    // Check alarms
    checkAlarms(cfg, ns, alarmState);

    // Auto-restart on too many errors
    if (cfg.restart_at_logged_errors > 0 &&
        errLog.count >= cfg.restart_at_logged_errors) {
        ESP.restart();
    }

    delay(100);
}

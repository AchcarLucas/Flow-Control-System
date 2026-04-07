#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>

#include <config.h>

#include <data_monitor.h>

#include <webserver.h>

#include <index_request.h>
#include <analysis_request.h>
#include <raw_request.h>
#include <simulate_request.h>
#include <cleanup_request.h>
#include <reset_request.h>
#include <download_request.h>
#include <delete_request.h>

#include <sample_api.h>

AsyncWebServer *webServer;
DataMonitor *dataMonitor;

// WebServer PAGE
WebServer *indexRequest;
WebServer *analysisRequest;
WebServer *rawRequest;
WebServer *cleanupRequest;

// WebServer GET
WebServer *simulateRequest;
WebServer *resetRequest;
WebServer *downloadRequest;
WebServer *deleteRequest;

// WebServer API
WebServer *sampleAPI;

volatile uint16_t inFlow = 0;
volatile uint16_t outFlow = 0;

void IRAM_ATTR interruptionThrow() {
    if (digitalRead(IN_FLOW)) {
        inFlow++;
    }

    if (digitalRead(OUT_FLOW)) {
        outFlow++;
    }

    Serial.printf("[Interruption]: InFlow {%u} OutFlow {%u}\n", inFlow, outFlow);
}

void routineThrow(struct tm timeinfo) {
    uint16_t _inFlow = inFlow;
    uint16_t _outFlow = outFlow;

    inFlow = outFlow = 0;

    Serial.printf("[Routine]: (%02d/%02d/%d - %02d:%02d:%02d) InFlow {%u} OutFlow {%u}\n",
        timeinfo.tm_mday,
        timeinfo.tm_mon + 1,
        timeinfo.tm_year + 1900,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec,
        _inFlow,
        _outFlow
    );
}

void settingHardware() {
    Serial.begin(115200);

    // Configuração das portas In|Out do ESP32
    pinMode(LED_PIN, OUTPUT);

    pinMode(IN_FLOW, INPUT);
    pinMode(OUT_FLOW, INPUT);
    pinMode(INTERRUPTION, INPUT);

    attachInterrupt(digitalPinToInterrupt(INTERRUPTION), interruptionThrow, FALLING);

    esp_task_wdt_init(60, true);
}

void initLittleFS() {
    Serial.println("Attempting to mounting LittleFS, please wait...");

    // Iniciar LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("\nAn error occurred while trying to mount LittleFS.");
        abort();
    }

    Serial.println("LittleFS successfully mounted.\n");
}

void initWifi() {
    Serial.printf("SSID: %s\n", WIFI_SSID);
    Serial.print("Attempting to connect to WiFi, please wait...");

    // Conectar Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) { 
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nWiFi successfully connected");
    Serial.printf("IP: %s\n\n", WiFi.localIP().toString().c_str());
}

void initNTP() {
    Serial.print("Waiting for NTP synchronization...");

    // Sincroniza com os servidores NTP
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com");

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nNTP successfully synchronized.");
    Serial.println(&timeinfo, "Current Time: %A, %B %d %Y %H:%M:%S\n");
}

void initServer() {
    Serial.println("Server configuration and initialization");

    webServer = new AsyncWebServer(SERVER_PORT);
    dataMonitor = new DataMonitor(DATABASE, CLEANUP);

    // WebServer PAGE
    indexRequest = new IndexRequest(webServer, dataMonitor);
    indexRequest->onServer();

    analysisRequest = new AnalysisRequest(webServer, dataMonitor);
    analysisRequest->onServer();

    rawRequest = new RawRequest(webServer, dataMonitor);
    rawRequest->onServer();

    // WebServer GET
    simulateRequest = new SimulateRequest(webServer, dataMonitor);
    simulateRequest->onServer();

    cleanupRequest = new CleanupRequest(webServer, dataMonitor);
    cleanupRequest->onServer();

    resetRequest = new ResetRequest(webServer, dataMonitor);
    resetRequest->onServer();

    downloadRequest = new DownloadRequest(webServer, dataMonitor);
    downloadRequest->onServer();

    deleteRequest = new DeleteRequest(webServer, dataMonitor);
    deleteRequest->onServer();

    // WebServer API
    sampleAPI = new SampleAPI(webServer, dataMonitor);
    sampleAPI->onServer();

    webServer->begin();

    Serial.println("Server successfully initialized.\n");
}

void setup() {
    settingHardware();

    // Exibe Modo Debug
    Serial.printf("DEBUG: %s\n\n", DEBUG == 1 ? "True" : "False");

    VISUAL_INDICATOR_ON();

    // Aviso visual para indicar que o sistema esta sendo iniciado
    for(uint8_t i = 0; i < 8; ++i) {
        VISUAL_INDICATOR_ON();
        VISUAL_INDICATOR_OFF();
    }

    VISUAL_INDICATOR_ON();

    initLittleFS();
    initWifi();
    initNTP();
    initServer();

    VISUAL_INDICATOR_OFF();
}

void loop() {
    struct tm timeinfo;

    static int lastMinutesProcessed = -1;

    // 1. Lógica do Relógio (NTP)
    if (getLocalTime(&timeinfo)) {
        int currentMinutes = timeinfo.tm_min;

        // Chama a routine a cada 10 minutos
        if (currentMinutes % 1 == 0) {
            if (currentMinutes != lastMinutesProcessed) {
                routineThrow(timeinfo);
                lastMinutesProcessed = currentMinutes;
            }
        } else {
            lastMinutesProcessed = -1;
        }
    }

    delay(250);
}
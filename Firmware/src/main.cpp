#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>

#include <config.h>

#include <data_monitor.h>
#include <routine_monitor.h>

#include <webserver.h>

#include <index_request.h>
#include <analysis_request.h>
#include <raw_request.h>
#include <stats_request.h>

#include <simulate_request.h>
#include <cleanup_request.h>
#include <reset_request.h>
#include <download_request.h>
#include <delete_request.h>

#include <sample_api.h>
#include <stats_api.h>

AsyncWebServer *webServer;

DataMonitor *dataMonitor;
RoutineMonitor *routineMonitor;

// WebServer PAGE
WebServer *indexRequest;
WebServer *analysisRequest;
WebServer *rawRequest;
WebServer *statsRequest;

// WebServer GET
WebServer *simulateRequest;
WebServer *cleanupRequest;
WebServer *resetRequest;
WebServer *downloadRequest;
WebServer *deleteRequest;

// WebServer API
WebServer *sampleAPI;
WebServer *statsAPI;

void settingHardware() {
    Serial.begin(115200);

    // Configuração das portas In|Out do ESP32
    pinMode(LED_PIN, OUTPUT);

    pinMode(IN_FLOW, INPUT);
    pinMode(OUT_FLOW, INPUT);
    pinMode(INTERRUPTION, INPUT);

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

    statsRequest = new StatsRequest(webServer, dataMonitor);
    statsRequest->onServer();

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

    statsAPI = new StatsAPI(webServer, dataMonitor);
    statsAPI->onServer();

    webServer->begin();

    Serial.println("Server successfully initialized.\n");
}

void initRoutine() {
    Serial.println("Routine configuration and initialization");
    routineMonitor = new RoutineMonitor(dataMonitor, IN_FLOW, OUT_FLOW, INTERRUPTION, SAMPLE_STEP);
    Serial.println("Routine successfully initialized.\n");
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
    initRoutine();

    VISUAL_INDICATOR_OFF();
}

void loop() {
    routineMonitor->running();
    delay(250);
}
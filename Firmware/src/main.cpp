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

AsyncWebServer *webserver;
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

    webserver = new AsyncWebServer(SERVER_PORT);
    dataMonitor = new DataMonitor(DATABASE, CLEANUP);

    // WebServer PAGE
    indexRequest = new IndexRequest(webserver, dataMonitor);
    indexRequest->onServer();

    analysisRequest = new AnalysisRequest(webserver, dataMonitor);
    analysisRequest->onServer();

    rawRequest = new RawRequest(webserver, dataMonitor);
    rawRequest->onServer();

    // WebServer GET
    simulateRequest = new SimulateRequest(webserver, dataMonitor);
    simulateRequest->onServer();

    cleanupRequest = new CleanupRequest(webserver, dataMonitor);
    cleanupRequest->onServer();

    resetRequest = new ResetRequest(webserver, dataMonitor);
    resetRequest->onServer();

    downloadRequest = new DownloadRequest(webserver, dataMonitor);
    downloadRequest->onServer();

    deleteRequest = new DeleteRequest(webserver, dataMonitor);
    deleteRequest->onServer();

    // WebServer API
    sampleAPI = new SampleAPI(webserver, dataMonitor);
    sampleAPI->onServer();

    webserver->begin();

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
    // O AsyncWebServer não precisa de nada no loop!
    // Realiza a leitura digital de cada pino
    int statusD15 = digitalRead(D15);
    int statusD4  = digitalRead(D4);
    int statusD14 = digitalRead(D14);

    // Exibe os valores no Monitor Serial
    /*
    Serial.print("D15: "); Serial.print(statusD15);
    Serial.print(" | D4: "); Serial.print(statusD4);
    Serial.print(" | D14: "); Serial.println(statusD14);
    */

    // Pequeno atraso para facilitar a leitura humana no monitor
    delay(250);
}
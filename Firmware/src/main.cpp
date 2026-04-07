#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <sqlite3.h>
#include <LittleFS.h>
#include <esp_task_wdt.h>

#include <config.h>

#include <data_monitor.h>

#include <page.h>
#include <analysis_page.h>
#include <raw_page.h>
#include <waiting_page.h>

#include <webserver.h>
#include <index_request.h>
#include <analysis_request.h>
#include <raw_request.h>
#include <simulate_request.h>
#include <cleanup_request.h>
#include <reset_request.h>

#include <json.h>
#include <sample_json.h>

AsyncWebServer *webserver;
DataMonitor *monitor;

// Page Request
WebServer *indexRequest;
WebServer *analysisRequest;
WebServer *rawRequest;
WebServer *cleanupRequest;

// Get Request
WebServer *simulateRequest;
WebServer *resetRequest;

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

void downloadRequest() {
    // Rota para baixar o banco de dados: http://[IP-DO-ESP]/download
    webserver->on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();
        
        if (!LittleFS.exists("/" DATABASE)) {
            request->send(404, "text/plain", "An error occurred while trying to download the database " DATABASE " file not found");
            FINISH_SERVER_PROCESSING();
            return;
        }

        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/" DATABASE, "application/octet-stream");

        String attachment = "attachment; filename=\"" + String(DATABASE) + "\"";
        response->addHeader("Content-Disposition", attachment.c_str());
        request->send(response);

        FINISH_SERVER_PROCESSING();
    });
}

void deleteRequest() {
    // Deleta um dado especifico com base no seu ID: http://[IP-DO-ESP]/delete?id={id}
    webserver->on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        if (!request->hasParam("id")) {
            request->send(404, "text/plain", "An error occurred while trying to delete; the id parameter is not present.");
            FINISH_SERVER_PROCESSING();
            return;
        }

        uint32_t id = request->getParam("id")->value().toInt();

        bool result = monitor->removeSamplesByID(id);

        if(!result) {
            request->send(404, "text/plain", "An error occurred while trying to delete the id " + String(id) + ".");
            FINISH_SERVER_PROCESSING();
            return;
        }

        request->redirect("/raw");

        FINISH_SERVER_PROCESSING();
    });
}

void sampleAPIRequest() {
    // t_start ('YYYY-MM-DD HH:MM:SS') and t_end ('YYYY-MM-DD HH:MM:SS')
    webserver->on("/api/samples", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        if (!(request->hasParam("t_start") && request->hasParam("t_end"))) {
            request->send(404, "application/json", "{ \"status\": \"failed\" }");
            FINISH_SERVER_PROCESSING();
            return;
        }

        String tStart = request->getParam("t_start")->value();
        String tEnd = request->getParam("t_end")->value();
        
        Serial.printf("t_start %s - t_end %s\n", tStart.c_str(), tEnd.c_str());

        std::list<Sample> samples = monitor->selectSamples(tStart, tEnd);

        JSON *json = new SampleJson(samples);
        
        request->send(200, "application/json", json->serialize());

        delete json;

        FINISH_SERVER_PROCESSING();
    });
}

void initServer() {
    Serial.println("Server configuration and initialization");

    webserver = new AsyncWebServer(SERVER_PORT);
    monitor = new DataMonitor(DATABASE, CLEANUP);

    indexRequest        = new IndexRequest(webserver, monitor);
    analysisRequest     = new AnalysisRequest(webserver, monitor);
    rawRequest          = new RawRequest(webserver, monitor);
    simulateRequest     = new SimulateRequest(webserver, monitor);
    cleanupRequest      = new CleanupRequest(webserver, monitor);
    resetRequest        = new ResetRequest(webserver, monitor);

    // PAGE Request
    indexRequest->onServer();
    analysisRequest->onServer();
    rawRequest->onServer();

    // GET Request
    simulateRequest->onServer();
    cleanupRequest->onServer();
    resetRequest->onServer();

    downloadRequest();
    deleteRequest();

    // API Request
    sampleAPIRequest();

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
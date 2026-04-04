#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <sqlite3.h>
#include <LittleFS.h>

#include <data_monitor.h>

#include <page.h>

#include <index_page.h>
#include <analysis_page.h>
#include <raw_page.h>

#include <sample_json.h>

#define D15 15
#define D14 14
#define D4 4
#define D2 2

// Pino do LED (Indicador visual)
const int LED_PIN = D2;

// Pinos de estado do controle de fluxo
const int IN_FLOW = D15;
const int OUT_FLOW = D4;

// Pino de identificação de interrupção
const int INTERRUPTION = D14;

// Configuração de fuso: -3 horas * 3600 segundos
const long gmtOffset_sec = TZS * 3600;
// Configuração do horário de verão: 3600 se houver horário de verão
const int daylightOffset_sec = 0;

DataMonitor *monitor;

AsyncWebServer server(SERVER_PORT);

#define CHECK_DEBUG() if (DEBUG != 1) return

#define STARTING_SERVER_PROCESSING() digitalWrite(LED_PIN, HIGH)
#define FINISH_SERVER_PROCESSING() digitalWrite(LED_PIN, LOW)

#define VISUAL_INDICATOR_ON() digitalWrite(LED_PIN, HIGH); delay(200)
#define VISUAL_INDICATOR_OFF() digitalWrite(LED_PIN, LOW); delay(100)

void settingHardware() {
    Serial.begin(115200);

    // Configuração das portas In|Out do ESP32
    pinMode(LED_PIN, OUTPUT);

    pinMode(IN_FLOW, INPUT);
    pinMode(OUT_FLOW, INPUT);
    pinMode(INTERRUPTION, INPUT);
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

void index_request() {
    // PAGE 'index' http://[IP-DO-ESP]/index
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        Page *indexPage = new IndexPage();

        request->send(200, "text/html", indexPage->page());

        delete indexPage;

        FINISH_SERVER_PROCESSING();
    });
}

void analysis_request() {
    // PAGE 'analysis data' http://[IP-DO-ESP]/analysis?start={datetime}&end={datetime}
    server.on("/analysis", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        Page *analysisPage = new AnalysisPage(DATABASE);

        request->send(200, "text/html", analysisPage->page());

        delete analysisPage;

        FINISH_SERVER_PROCESSING();
    });
}

void raw_request() {
    // PAGE 'raw data' http://[IP-DO-ESP]/raw?page={page}
    server.on("/raw", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        uint16_t limit = 10;
        uint16_t currentPage = 1;

        if (request->hasParam("page")) {
            currentPage = request->getParam("page")->value().toInt();
            if (currentPage < 1) {
                currentPage = 1;
            }
        }

        uint32_t totalPages = monitor->getTotalPages(limit);
        std::list<Sample> samples = monitor->selectSamples(currentPage, limit);

        Serial.println("Current Page: " + String(currentPage) + " Total Pages: " + String(totalPages));

        Page *rawPage = new RawPage(
            DATABASE,
            currentPage,
            totalPages,
            limit,
            samples
        );

        request->send(200, "text/html", rawPage->page());

        delete rawPage;

        FINISH_SERVER_PROCESSING();
    });
}

void simulate_request() {
    CHECK_DEBUG();

    // Rota para simulação de inserção de dados: http://[IP-DO-ESP]/simulate
    server.on("/simulate", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        bool result = monitor->insertSamples({ 
            Sample(10, esp_random() % 2000, esp_random() % 2000)
        });

        if(!result) {
            request->send(404, "text/plain", "An error occurred while inserting a simulation into the database.");
            FINISH_SERVER_PROCESSING();
            return;
        }

        request->redirect("/raw");

        FINISH_SERVER_PROCESSING();
    });
}

void cleanup_request() {
    // Limpeza de otimização do banco de dados: http://[IP-DO-ESP]/cleanup
    server.on("/cleanup", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        bool result = monitor->cleanup();

        if(!result) {
            request->send(404, "text/plain", "An error occurred while attempting to perform an database optimized cleanup on the system.");
            FINISH_SERVER_PROCESSING();
            return;
        }

        request->redirect("/raw");

        FINISH_SERVER_PROCESSING();
    });
}

void reset_request() {
    // Reset do banco de dados: http://[IP-DO-ESP]/reset
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        bool result = monitor->reset();

        if(!result) {
            request->send(404, "text/plain", "An error occurred while attempting to perform an database reset on the system.");
            FINISH_SERVER_PROCESSING();
            return;
        }

        request->redirect("/raw");

        FINISH_SERVER_PROCESSING();
    });
}

void download_request() {
    // Rota para baixar o banco de dados: http://[IP-DO-ESP]/download
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
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

void delete_request() {
    // Deleta um dado especifico com base no seu ID: http://[IP-DO-ESP]/delete?id={id}
    server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
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

void sample_api_request() {
    // t_start ('YYYY-MM-DD HH:MM:SS') and t_end ('YYYY-MM-DD HH:MM:SS')
    server.on("/api/samples", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        if (!(request->hasParam("t_start") && request->hasParam("t_end"))) {
            request->send(404, "application/json", "{ \"status\": \"failed\" }");
            FINISH_SERVER_PROCESSING();
            return;
        }

        String tStart = request->getParam("t_start")->value();
        String tEnd = request->getParam("t_end")->value();
        
        Serial.printf("t_start %s - t_end %s", tStart.c_str(), tEnd.c_str());

        std::list<Sample> samples = monitor->selectSamples(tStart, tEnd);

        SampleJson json = SampleJson(samples);
        
        request->send(200, "application/json", json.serialize());

        FINISH_SERVER_PROCESSING();
    });
}

void initServer() {
    Serial.println("Server configuration and initialization");
    monitor = new DataMonitor(DATABASE, CLEANUP);

    // PAGE Request
    index_request();
    analysis_request();
    raw_request();

    // GET Request
    simulate_request();
    cleanup_request();
    reset_request();
    download_request();
    delete_request();

    // API Request
    sample_api_request();

    server.begin();

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
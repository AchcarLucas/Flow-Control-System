#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <sqlite3.h>
#include <LittleFS.h>

#include <dao_sqlite.h>
#include <data_monitor.h>

#include <page.h>
#include <test_page.h>

#define D15 15
#define D14 14
#define D4 4
#define D2 2

// Configurações do Wi-Fi (Configuração no arquivo /env/.env)
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

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

SQLiteDAO *dao;
DataMonitor *monitor;

AsyncWebServer server(80);

#define CHECK_DEBUG() if (DEBUG != 1) return

#define STARTING_SERVER_PROCESSING() digitalWrite(LED_PIN, HIGH)
#define FINISH_SERVER_PROCESSING() digitalWrite(LED_PIN, LOW); return

#define VISUAL_INDICATOR_ON() digitalWrite(LED_PIN, HIGH); delay(200)
#define VISUAL_INDICATOR_OFF() digitalWrite(LED_PIN, LOW); delay(100)

void initLittleFS() {
    Serial.print("Attempting to mounting LittleFS, please wait...");

    // Iniciar LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("An error occurred while trying to mount LittleFS.");
        abort();
    }
}

void initWifi() {
    Serial.print("Attempting to connect to WiFi, please wait...");

    // Conectar Wi-Fi
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) { 
        Serial.print(".");
        delay(500);
    }

    Serial.println("\nIP: " + WiFi.localIP().toString());
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

    Serial.println(&timeinfo, "Current Time: %A, %B %d %Y %H:%M:%S");
}

void hardwareSetting() {
    Serial.begin(115200);

    // Configuração das portas In|Out do ESP32
    pinMode(LED_PIN, OUTPUT);
    pinMode(IN_FLOW, INPUT);
    pinMode(OUT_FLOW, INPUT);
    pinMode(INTERRUPTION, INPUT);

    VISUAL_INDICATOR_ON();

    // Exibe Modo Debug
    Serial.printf("DEBUG: %d\n", DEBUG);
    // Exibe o SSID do Wifi
    Serial.println("SSID: " + String(ssid));

    // Aviso visual para indicar que o sistema esta sendo iniciado
    for(uint8_t i = 0; i < 8; ++i) {
        VISUAL_INDICATOR_ON();
        VISUAL_INDICATOR_OFF();
    }

    VISUAL_INDICATOR_ON();

    initLittleFS();
    initWifi();
    initNTP();

    VISUAL_INDICATOR_OFF();
}

void setup() {
    hardwareSetting();

    monitor = new DataMonitor(DATABASE, CLEANUP);

    // Limpeza de otimização do banco de dados
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();

        request->redirect("/test");

        FINISH_SERVER_PROCESSING();
    });

    // 4. Configurar Rotas do WebServer
    server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request) {
        CHECK_DEBUG();

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

        Page *testPage = new TestPage(
            DATABASE,
            currentPage,
            totalPages,
            limit,
            samples
        );

        request->send(200, "text/html", testPage->page());

        delete testPage;

        FINISH_SERVER_PROCESSING();
    });

    // Rota para simulação de inserção de dados: http://[IP-DO-ESP]/simulate
    server.on("/simulate", HTTP_GET, [](AsyncWebServerRequest *request) {
        CHECK_DEBUG();

        STARTING_SERVER_PROCESSING();

        bool result = monitor->insertSamples({ 
            Sample(10, esp_random() % 2000, esp_random() % 2000)
        });

        if(!result) {
            request->send(404, "text/plain", "An error occurred while inserting a simulation into the database.");
            FINISH_SERVER_PROCESSING();
        }

        request->redirect("/");

        FINISH_SERVER_PROCESSING();
    });

    // Limpeza de otimização do banco de dados: http://[IP-DO-ESP]/cleanup
    server.on("/cleanup", HTTP_GET, [](AsyncWebServerRequest *request) {
        CHECK_DEBUG();

        STARTING_SERVER_PROCESSING();

        bool result = monitor->cleanup();

        if(!result) {
            request->send(404, "text/plain", "An error occurred while attempting to perform an database optimized cleanup on the system.");
            FINISH_SERVER_PROCESSING();
        }

        request->redirect("/");

        FINISH_SERVER_PROCESSING();
    });

    // Reset do banco de dados: http://[IP-DO-ESP]/reset
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        CHECK_DEBUG();

        STARTING_SERVER_PROCESSING();

        bool result = monitor->reset();

        if(!result) {
            request->send(404, "text/plain", "An error occurred while attempting to perform an database reset on the system.");
            FINISH_SERVER_PROCESSING();
        }

        request->redirect("/");

        FINISH_SERVER_PROCESSING();
    });
    
    // Rota para baixar o banco de dados: http://[IP-DO-ESP]/download
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        STARTING_SERVER_PROCESSING();
        
        if (!LittleFS.exists("/" DATABASE)) {
            request->send(404, "text/plain", "An error occurred while trying to download the database " DATABASE " file not found");
            FINISH_SERVER_PROCESSING();
        }

        AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/" DATABASE, "application/octet-stream");

        String attachment = "attachment; filename=\"" + String(DATABASE) + "\"";
        response->addHeader("Content-Disposition", attachment.c_str());
        request->send(response);

        FINISH_SERVER_PROCESSING();
    });

    server.begin();

    digitalWrite(LED_PIN, LOW);
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
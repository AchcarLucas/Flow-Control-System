#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <sqlite3.h>
#include <LittleFS.h>

#include <dao_sqlite.h>
#include <data_monitor.h>

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

void hardwareSetting() {
    Serial.begin(115200);

    // Configuração das portas In|Out do ESP32
    pinMode(LED_PIN, OUTPUT);

    pinMode(IN_FLOW, INPUT);
    pinMode(OUT_FLOW, INPUT);
    pinMode(INTERRUPTION, INPUT);

    // Inicia o aviso visual em HIGH
    digitalWrite(LED_PIN, HIGH);

    Serial.printf("DEBUG: %d\n", DEBUG);

    // Exibe o SSID do Wifi
    Serial.print("SSID: ");
    Serial.println(ssid);

    // Aviso visual para indicar que o sistema esta sendo iniciado
    for(uint8_t i = 0; i < 8; ++i) {
        digitalWrite(LED_PIN, LOW);
        delay(500);
        digitalWrite(LED_PIN, HIGH);
        delay(500);
    }

    // Iniciar LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("Erro ao montar LittleFS");
        return;
    }

    // Conectar Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Attempting to connect to WiFi, please wait...");

    while (WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nIP: " + WiFi.localIP().toString());

    // Sincroniza com os servidores do Google ou NTP.br
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com");

    Serial.print("Waiting for NTP synchronization...");

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
        delay(500);
        Serial.print(".");
    }

    Serial.println(&timeinfo, "Current Time: %A, %B %d %Y %H:%M:%S");
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

        String html = 
        "<!DOCTYPE html>"
        "<html lang=\"pt-br\">"
        "<head>"
            "<meta charset=\"UTF-8\">"
            "<title>Monitor de Fluxo - Univesp</title>"
        "</head>";
    
        html += "<h1>[Test Mode] - Monitor de Fluxo - Univesp</h1>";
        html += "<h2>\t\tDATABASE:" DATABASE "</h1>";

        ////////////////////////////////////////////////////////////////

        html += "<table border='1' style='border-collapse: collapse; margin: auto; font-family: sans-serif; min-width: 500px;'>"
                "<tr>"
                    "<th>ID</th>"
                    "<th>Data/Hora</th>"
                    "<th>Tempo Amostragem (Minutos)</th>"
                    "<th>Entrada</th>"
                    "<th>Saída</th>"
                "</tr>";

        uint32_t totalPages = monitor->getTotalPages(limit);
        std::list<Sample> samples = monitor->selectSamples(currentPage, limit);

        Serial.println("Total Pages: " + String(totalPages) + " Current Page: " + currentPage);

        for (const auto& sample : samples) {
            html += "<tr>";
            html += "<td>" + String(sample.id) + "</td>";
            html += "<td>" + String(sample.timestamp.c_str()) + "</td>";
            html += "<td>" + String(sample.sampling_time) + "</td>";
            html += "<td>" + String(sample.in) + "</td>";
            html += "<td>" + String(sample.out) + "</td>";
            html += "</tr>";
        }

        html += "</table><br>";

        ////////////////////////////////////////////////////////////////

        html += "<style>"
            ".pagination { display: flex; list-style: none; padding: 0; gap: 5px; justify-content: center; font-family: sans-serif; }"
            ".pagination a { "
            "    text-decoration: none; "
            "    padding: 8px 12px; "
            "    border: 1px solid #ccc; "
            "    color: #333; "
            "    border-radius: 4px; "
            "}"
            ".pagination a.active { background-color: #007bff; color: white; border-color: #007bff; font-weight: bold; }"
            ".pagination a:hover:not(.active) { background-color: #f0f0f0; }"
            ".disabled { color: #ccc !important; pointer-events: none; border-color: #eee !important; }"
        "</style>";

        // Sistema de Paginação
        html += "<ul class='pagination'>";

        // Botão anterior
        if (currentPage > 1) {
            html += "<li><a href='?page=" + String(currentPage - 1) + "'>Anterior</a></li>";
        } else {
            html += "<li><a class='disabled'>Anterior</a></li>";
        }

        // Status
        html += "<li><a class='active'>" + String(currentPage) + " / " + String(totalPages) + "</a></li>";

        // Botão próximo
        if (currentPage < totalPages) {
            html += "<li><a href='?page=" + String(currentPage + 1) + "'>Próxima</a></li>";
        } else {
            html += "<li><a class='disabled'>Próxima</a></li>";
        }

        html += "</ul><br>";

        ////////////////////////////////////////////////////////////////

        // Estilo de botão real: cinza, com borda e sombra suave
        String style = "color: black; background-color: #e7e7e7; border: 1px solid #ccc; "
                    "padding: 10px 20px; text-align: center; text-decoration: none; "
                    "display: inline-block; border-radius: 4px; font-family: sans-serif; "
                    "margin-right: 10px; font-weight: bold; box-shadow: 1px 1px 2px #888888;";

        html += "<div style='text-align: center; margin: 20px 0; width: 100%;'>";

        String simulateButton = "<a href='/simulate' style='%s'>Simulate Flow</a>";
        simulateButton.replace("%s", style);
        html += simulateButton;

        String cleanupButton = "<a href='/cleanup' style='%s'>Cleanup Optimization Database</a>";
        cleanupButton.replace("%s", style);
        html += cleanupButton;

        String resetButton = "<a href='/reset' style='%s'>Reset Database</a>";
        resetButton.replace("%s", style);
        html += resetButton;

        String downloadButton = "<a href='/download' style='%s'>Download Database</a>";
        downloadButton.replace("%s", style);
        html += downloadButton;

        html += "</div>";

        ////////////////////////////////////////////////////////////////

        request->send(200, "text/html", html);

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
    Serial.print("D15: "); Serial.print(statusD15);
    Serial.print(" | D4: "); Serial.print(statusD4);
    Serial.print(" | D14: "); Serial.println(statusD14);

    // Pequeno atraso para facilitar a leitura humana no monitor
    delay(250);
}
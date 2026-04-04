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

// Configurações do Wi-Fi
const char* ssid = "SSID_REMOVED";
const char* password = "PASSWORD_REMOVED";

// Pino do LED (O padrão da maioria das placas é o 2)
const int LED_PIN = D2;

// Pino de controle de fluxo
const int IN_FLOW = D15;
const int OUT_FLOW = D4;
const int INTERRUPTION = D14;

AsyncWebServer server(80);
sqlite3 *db;

SQLiteDAO *dao;
DataMonitor *monitor;

// Configuração de fuso: -3 horas * 3600 segundos
const long gmtOffset_sec = -3 * 3600; 
const int daylightOffset_sec = 0; // 3600 se houver horário de verão

// Função auxiliar para executar comandos SQL
int db_exec(const char *sql) {
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        Serial.printf("Erro SQL: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    pinMode(IN_FLOW, INPUT);
    pinMode(OUT_FLOW, INPUT);
    pinMode(INTERRUPTION, INPUT);

    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);

    digitalWrite(LED_PIN, HIGH);

    // 1. Iniciar LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("Erro ao montar LittleFS");
        return;
    }

    // 2. Conectar Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Attempting to connect to WiFi, please wait...");
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nIP: " + WiFi.localIP().toString());

    // Sincroniza com os servidores do Google ou NTP.br
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com");

    Serial.print("Waiting for NTP synchronization...");

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) { delay(500); Serial.print("."); }

    Serial.println(&timeinfo, "Current Time: %A, %B %d %Y %H:%M:%S");

    monitor = new DataMonitor("monitor.db", "-3 months");

    // 4. Configurar Rotas do WebServer
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<h1>Monitor de Fluxo - Univesp - Test</h1>";
        html += "<table border='1'>"
                "<tr>"
                    "<th>ID</th>"
                    "<th>Data/Hora</th>"
                    "<th>Tempo Amostragem</th>"
                    "<th>Entrada</th>"
                    "<th>Saída</th>"
                "</tr>";

        // Pisca o LED rapidamente
        digitalWrite(LED_PIN, HIGH);
        
        std::list<Sample> samples = monitor->selectSamples(10); // Exibe os 10 registros mais recentes

        for (const auto& sample : samples) {
            html += "<tr>";
            html += "<td>" + String(sample.id) + "</td>";
            html += "<td>" + String(sample.timestamp.c_str()) + "</td>";
            html += "<td>" + String(sample.sampling_time) + "</td>";
            html += "<td>" + String(sample.in) + "</td>";
            html += "<td>" + String(sample.out) + "</td>";
            html += "</tr>";
        }

        html += "</table><br><a href='/add'>Simular Detecção</a>";
        request->send(200, "text/html", html);

        digitalWrite(LED_PIN, LOW);
    });

    // Rota para simular inserção de dados
    server.on("/add", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(LED_PIN, HIGH);

        monitor->insertSamples({Sample(10, 1, 1)});

        request->redirect("/");

        digitalWrite(LED_PIN, LOW);
    });
    
    // Rota para baixar o banco de dados: http://[IP-DO-ESP]/download
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        digitalWrite(LED_PIN, HIGH);

        if (LittleFS.exists("/monitor.db")) {
            // Criamos a resposta apontando para o arquivo no LittleFS
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/monitor.db", "application/octet-stream");
            
            // Este é o "pulo do gato": força o navegador a reconhecer o nome do arquivo
            response->addHeader("Content-Disposition", "attachment; filename=\"monitor.db\"");
            
            request->send(response);
        } else {
            request->send(404, "text/plain", "Arquivo nao encontrado");
        }

        digitalWrite(LED_PIN, LOW);
    });

    server.begin();

    digitalWrite(LED_PIN, LOW);
}

/*
void limparRegistrosAntigos() {
    if (WiFi.status() != WL_CONNECTED) return; // Precisa de internet para a hora estar certa

    char *zErrMsg = 0;
    // Deleta registros onde a coluna 'momento' é anterior a 2 meses atrás
    const char* sql = "DELETE FROM fluxo WHERE momento < datetime('now', '-2 month');";
    
    int rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if (rc != SQLITE_OK) {
        Serial.printf("Erro na limpeza: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        Serial.println("Limpeza de 2 meses concluída com sucesso.");
        // Opcional: Libera o espaço vazio no arquivo .db
        sqlite3_exec(db, "VACUUM;", NULL, NULL, NULL);
    }
}
*/

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
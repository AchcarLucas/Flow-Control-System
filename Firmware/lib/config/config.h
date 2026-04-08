#ifndef CONFIG_H
#define CONFIG_H

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

#if DEBUG != 0
    #define SerialPrint(x) Serial.print(x)
    #define SerialPrintln(x) Serial.println(x)
    #define SerialPrintf(...) Serial.printf(__VA_ARGS__)
#else
    #define SerialPrint(x)
    #define SerialPrintln(x)
    #define SerialPrintf(...)
#endif

#define CHECK_DEBUG() if (DEBUG != 1) return

#define STARTING_SERVER_PROCESSING() digitalWrite(LED_PIN, HIGH)
#define FINISH_SERVER_PROCESSING() digitalWrite(LED_PIN, LOW)

#define VISUAL_INDICATOR_ON() digitalWrite(LED_PIN, HIGH); delay(200)
#define VISUAL_INDICATOR_OFF() digitalWrite(LED_PIN, LOW); delay(100)

#endif
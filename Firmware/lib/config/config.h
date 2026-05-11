#ifndef CONFIG_H
#define CONFIG_H

// Interruption
#define D25 25

// In Flow
#define D18 18
// Out Flow
#define D17 17

#define D2 2

// Pino do LED (Indicador visual)
const int LED_PIN = D2;

// Pinos de estado do controle de fluxo
const int IN_FLOW = D18;
const int OUT_FLOW = D17;

// Pino de identificação de interrupção
const int INTERRUPTION = D25;

// Configuração de fuso: -3 horas * 3600 segundos
const long gmtOffset_sec = TZS * 3600;
// Configuração do horário de verão: 3600 se houver horário de verão
const int daylightOffset_sec = 0;

#define PRINT_FREE_HEAP_SIZE() Serial.printf("  - FREE HEAP: %d bytes\n", esp_get_free_heap_size())

#define CHECK_DEBUG() if (DEBUG != 1) return

#define STARTING_SERVER_PROCESSING() digitalWrite(LED_PIN, HIGH)
#define FINISH_SERVER_PROCESSING() digitalWrite(LED_PIN, LOW)

#define VISUAL_INDICATOR_ON() digitalWrite(LED_PIN, HIGH); delay(200)
#define VISUAL_INDICATOR_OFF() digitalWrite(LED_PIN, LOW); delay(100)

#endif
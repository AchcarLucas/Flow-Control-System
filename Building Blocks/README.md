# Sistema de Monitoramento de Fluxo de Pessoas (Projeto Integrador V)

Este projeto consiste em um sistema de monitoramento de fluxo capaz de identificar a direção do movimento (entrada e saída) através de sensores ópticos e lógica digital, integrando os dados a um servidor web hospedado em um **ESP32**.

---

## 🛠️ Ferramentas e Tecnologias

### Software
* **[SimulIDE 1.0.0](https://simulide.com/p/)**: Software para a simulação dos circuitos.
* **[Visual Studio Code](https://code.visualstudio.com/)**: IDE para Programação do Firmware.
* **[PlatformIO](https://platformio.org/)**: Embedded Software Development para ESP32.
* **[ESP-IDF / Arduino](https://www.arduino.cc/en/software/)**: Ambiente de compilação do Firmware ESP32.

### Hardware
* **Microcontrolador**: ESP32 (Servidor e Processamento).
* **Arquitetura**: Circuitos integrados CMOS/TTL para processamento de sinal em tempo real.

## 📑 Documentação Técnica

### 1. Módulo de Aquisição de Sinal

<img src="./Signal Acquisition Circuit/signal-acquisition-circuit.png" width="500">

O objetivo deste estágio é converter a interrupção física de um feixe óptico em um sinal digital limpo, imune a interferências externas.

#### 1.1. Modulação e Portadora (Osciladores 555)

<div style="display: flex; gap: 10px;">
  <img src="./Signal%20Acquisition%20Circuit/freq-osc-1khz.png" width="400">
  <img src="./Signal%20Acquisition%20Circuit/freq-osc-58khz.png" width="400">
</div>

* **High-Freq-Osc (58 kHz):** Gera a frequência de modulação para os LEDs. Isso garante que o sistema ignore a luz ambiente (contínua) ou lâmpadas fluorescentes (60 Hz).
* **Low-Freq-Osc(1KHz):** Provê a base de tempo para a lógica de multiplexação.

<div style="display: flex; gap: 10px;">
  <img src="./Signal%20Acquisition%20Circuit/demux-circuit.png" width="400">
  <img src="./Signal%20Acquisition%20Circuit/demux-signal.png" width="400">
</div>



* **Demultiplexação (74HC238):** Gerencia a ativação sequencial dos sensores para evitar interferência mútua (*cross-talk*).

#### 1.2. Condicionamento (Filtro RC)
<img src="./Signal%20Acquisition%20Circuit/filters.png" width="400">

O sinal recebido passa por um filtro passivo para integração dos pulsos de 58 kHz:
$$fc = \frac{1}{2 \cdot \pi \cdot R \cdot C}$$
Entrega um nível lógico estável (**HIGH** para feixe livre, **LOW** para feixe interrompido).

### 2. Lógica de Identificação de Fluxo

<img src="./Signal Conditioning and Flow Identification Circuit/signal-conditioning-and-flow-identification.png" width="400">

Processa a sequência de ativação dos sensores para determinar o sentido do movimento através da detecção de borda.

#### 2.1. Detecção de Sequência (Flip-Flops D)
* **Entrada (In-Flow):** Ativado quando o sinal do **Sensor A** precede o **Sensor B**.
* **Saída (Out-Flow):** Ativado quando o sinal do **Sensor B** precede o **Sensor A**.
Utiliza os *clocks* dos Flip-Flops para travar o estado no momento da interrupção sequencial.

#### 2.2. Temporização de Reset (Monoestável)
Utiliza um Timer 555 para evitar múltiplas contagens (repique) e garantir o processamento correto:
* **Fórmula:** $$T = 1.1 \cdot R \cdot C$$
* **Tempo de Travamento:** Aproximadamente **0.297 segundos** (utilizando $R=1k\Omega$ e $C=270\mu F$).
O sistema ignora novas entradas durante este intervalo e reseta a lógica para o próximo evento de fluxo.

### 3. Definição das Saídas (Interface com ESP32)

<img src="./Processing e Server/processing-server-esp32-circuit.png" width="400">



| Saída | Tipo | Descrição Operacional |
| :--- | :--- | :--- |
| **In-Flow** | Digital | Pulso HIGH indicando entrada de pessoa. |
| **Out-Flow** | Digital | Pulso HIGH indicando saída de pessoa. |
| **Interruption** | Digital | Saída da porta OR. Ativa em qualquer evento de fluxo. |

---

## 🔗 Referências Consultadas

* [O Circuito Integrado 555 - Newton C. Braga](https://www.newtoncbraga.com.br/index.php/como-funciona/592-o-circuito-integrado-555-art011)
* [Condicionamento de Sinais - Newton C. Braga](https://www.newtoncbraga.com.br/como-funciona/2416-art370.html)
* [74HC139/74LS139 Logic Guide](https://www.build-electronic-circuits.com/7400-series-integrated-circuits/74hc139-74ls139/)
* [ESP32 Intro and Pinout](https://esp32.implrust.com/esp32-intro/pinout.html)
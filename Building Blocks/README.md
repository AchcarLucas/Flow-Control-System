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

---

## 📦 Lista de Componentes Detalhada

### 1. Módulo de Aquisição de Sinal
| Categoria | Componente / Valor | Detalhes Técnicos | Qtd |
| :--- | :--- | :--- | :--- |
| **CIs** | NE555 | Temporizador / Oscilador | 02 |
| **CIs** | 74HC08 | Quad 2-Input AND Gate | 01 |
| **CIs** | 74HC14 | Hex Inverting Schmitt Trigger (Buffer) | 01 |
| **CIs** | 74HC238 | Demultiplexador 3-para-8 | 01 |
| **Sensores** | E39-R1 / TD-08 | Espelho Prismático | 02 |
| **Sensores** | KY-008 | Emissor Laser | 02 |
| **Sensores** | TIL78 | Fototransistor NPN | 02 |
| **Semicond.** | BC547 | Transistor NPN | 02 |
| **Semicond.** | BC557 | Transistor PNP | 02 |
| **Semicond.** | 1N4148 | Diodo de sinal rápido | 02 |
| **Indicador** | LED Azul | Sinalização de feixe | 02 |
| **Resistor** | 12k $\Omega$ | Resistor de filme de carbono | 01 |
| **Resistor** | 10k $\Omega$ | Resistor de filme de carbono | 02 |
| **Resistor** | 5k $\Omega$ | Resistor de filme de carbono | 02 |
| **Resistor** | 2k2 $\Omega$ | Resistor de filme de carbono | 01 |
| **Resistor** | 1k $\Omega$ | Resistor de filme de carbono | 04 |
| **Resistor** | 200 $\Omega$ | Resistor de filme de carbono | 02 |
| **Capacitor** | 100uF | Eletrolítico (10-16V) | 02 |
| **Capacitor** | 100nF | Cerâmico (Desacoplamento) | 05 |
| **Capacitor** | 33nF | Cerâmico / Poliéster | 02 |
| **Capacitor** | 1nF | Cerâmico / Poliéster | 01 |
| **Extras** | Cabo Blindado | 22 AWG - 2 Vias | 02 |

### 2. Condicionamento e Identificação de Fluxo
| Categoria | Componente / Valor | Detalhes Técnicos | Qtd |
| :--- | :--- | :--- | :--- |
| **CIs** | NE555 | Temporizador (Monoestável) | 01 |
| **CIs** | CD4093 | Quad 2-Input NAND Schmitt Trigger | 02 |
| **CIs** | 74HC74 | Dual D-Type Flip-Flop | 01 |
| **Indicador** | LED Vermelho | Indicador In-Flow | 01 |
| **Indicador** | LED Verde | Indicador de Status | 01 |
| **Resistor** | 10k $\Omega$ | Resistor de filme de carbono | 01 |
| **Resistor** | 1k $\Omega$ | Resistor de filme de carbono | 04 |
| **Resistor** | 200 $\Omega$ | Resistor de filme de carbono | 01 |
| **Resistor** | 100 $\Omega$ | Resistor de filme de carbono | 02 |
| **Capacitor** | 270uF | Eletrolítico (25V) - Reset Timer | 01 |
| **Capacitor** | 100nF | Cerâmico (Desacoplamento) | 04 |
| **Capacitor** | 10uF | Eletrolítico (10-16V) | 02 |
| **Capacitor** | 10nF | Cerâmico | 01 |

### 3. Servidor e Processamento
| Categoria | Componente / Valor | Detalhes Técnicos | Qtd |
| :--- | :--- | :--- | :--- |
| **MCU** | ESP32 | Microcontrolador com Wi-Fi/BT | 01 |
| **Capacitor** | 100nF | Cerâmico (Desacoplamento ESP32) | 01 |

---

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
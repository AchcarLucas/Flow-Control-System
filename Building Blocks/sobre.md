# Documentação Técnica: Sistema de Monitoramento de Fluxo (Projeto Integrador)

Este documento descreve o funcionamento do circuito de aquisição e identificação de direção para monitoramento de fluxo de pessoas, projetado para integração com microcontroladores (ESP32).

---

## 1. Módulo de Aquisição de Sinal
O objetivo deste estágio é converter a interrupção física de um feixe óptico em um sinal digital limpo, imune a interferências externas.

### 1.1. Modulação e Portadora (Osciladores 555)
* **High-Freq-Osc (58 kHz):** Gera a frequência de modulação para os LEDs IR. Isso garante que o sistema ignore a luz ambiente (contínua) ou lâmpadas fluorescentes (60 Hz).
* **Low-Freq-Osc:** Provê a base de tempo para a lógica de multiplexação.
* **Demultiplexação (74HC238):** Gerencia a ativação sequencial dos sensores para evitar interferência mútua (cross-talk).

### 1.2. Condicionamento (Filtro RC)
* O sinal recebido passa por um filtro passivo onde:
    $$fc = \frac{1}{2 \cdot \pi \cdot R \cdot C}$$
* Este filtro integra os pulsos de 58 kHz, entregando um nível lógico estável (`HIGH` para feixe livre, `LOW` para feixe interrompido).

---

## 2. Lógica de Identificação de Fluxo
Este módulo processa a sequência de ativação dos sensores para determinar o sentido do movimento.

### 2.1. Detecção de Sequência (Flip-Flops D)
* **Entrada (In-Flow):** Ativado quando o sinal do **Sensor A** precede o **Sensor B**.
* **Saída (Out-Flow):** Ativado quando o sinal do **Sensor B** precede o **Sensor A**.
* A configuração utiliza os Clocks dos Flip-Flops para "travar" o estado assim que o primeiro sensor é cortado.

### 2.2. Temporização de Reset (Monoestável)
Para evitar múltiplas contagens por uma única pessoa (efeito de "repique" ou balanço de braços), utiliza-se um Timer 555 em modo monoestável:
* **Tempo de Travamento (T):** $$0.297 \text{ segundos}$$
* **Fórmula:** $$T = 1.1 \cdot R \cdot C$$ (Utilizando $R=1k\Omega$ e $C=270\mu F$).
* **Ação:** Após qualquer detecção válida, o sistema ignora novas entradas por ~300ms e então reseta os Flip-Flops para o estado inicial.

---

## 3. Definição das Saídas (Interface com ESP32)

| Saída | Tipo | Descrição Operacional |
| :--- | :--- | :--- |
| **In-Flow** | Digital | Pulso em nível alto indicando entrada. Conectado ao LED Vermelho. |
| **Out-Flow** | Digital | Pulso em nível alto indicando saída. Conectado ao LED Azul. |
| **Interruption** | Digital | Saída da porta OR. Sobe para nível alto em qualquer evento de fluxo. |

---
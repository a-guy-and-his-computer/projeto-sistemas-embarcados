# TerraGuard 🌧️⛰️

Sistema IoT de monitoramento preventivo de risco de deslizamentos utilizando ESP32, MQTT, Flask e React.

---

# 📌 Sobre o Projeto

O **TerraGuard** é um sistema embarcado e IoT desenvolvido para monitorar condições ambientais associadas ao risco de deslizamentos de terra em áreas vulneráveis.

O sistema coleta dados de sensores arduino e envia essas informações em tempo real para um dashboard web através do protocolo MQTT.

O objetivo do projeto é fornecer uma solução de baixo custo para monitoramento preventivo de encostas e áreas de risco.

---

# 🎯 Objetivos

* Monitorar condições ambientais em tempo real;
* Detectar fatores associados a deslizamentos;
* Exibir informações em um dashboard web;
* Gerar alertas preventivos;
* Demonstrar uma arquitetura IoT funcional utilizando ESP32.

---

# 🧠 Tecnologias Utilizadas

## Hardware

* ESP32
* MPU6050
* Sensor de Umidade de Solo
* Sensor de Chuva
* LED RGB
* Protoboard
* Jumpers

---

## Software

* PlatformIO
* Flask
* React
* Mosquitto
* MQTT
* Python
* JavaScript
* HTML/CSS

---

# 🏗️ Arquitetura do Sistema

```text
Sensores
   ↓
ESP32
   ↓ MQTT
Mosquitto Broker
   ↓
Backend Flask
   ↓
Dashboard React
```

---

# 📡 Sensores Utilizados

## 🌧️ Sensor de Chuva

Responsável por detectar presença e intensidade de chuva.

---

## 🌱 Sensor de Umidade do Solo

Responsável por medir a saturação do solo.

---

## 📐 MPU6050

Responsável por detectar:

* inclinação;
* vibração;
* possíveis movimentações do terreno.

---

# 🚨 Classificação de Risco

| Chuva | Umidade | Inclinação | Risco   |
| ----- | ------- | ---------- | ------- |
| Baixa | Baixa   | Baixa      | Seguro  |
| Média | Média   | Média      | Atenção |
| Alta  | Alta    | Média      | Alto    |
| Alta  | Alta    | Alta       | Crítico |

---

# 📂 Estrutura do Projeto

```text
terraguard/
│
├── README.md
│
├── backend/
│   ├── app.py
│   ├── mqtt_client.py
│   └── requirements.txt
│
├── frontend/
│   ├── src/
│   ├── package.json
│   └── vite.config.js
│
├── esp32/
│   ├── platformio.ini
│   └── src/
│       └── main.cpp
│
├── docs/
│
└── schematics/
```

---

# 🔌 MQTT

## Tópicos MQTT

```text
terraguard/chuva
terraguard/umidade
terraguard/inclinacao
terraguard/risco
```

---

# ⚙️ Como Executar

## 0. Mudar configurações da sua máquina

### Código da ESP32

Em esp32/src/main.cpp, mude:
```bash
const char* ssid = "NOME-SEU-WIFI"; // MUDAR
const char* password = "SENHA-DO-SEU-WIFI"; // MUDAR

const char* mqtt_server = "SEU-IPV4"; // MUDAR
```

e em esp32/platform.ini, mude:
```bash
upload_port = COM99 # MUDAR BASEADO NO PORT DO SEU DISPOSITIVO
```

## 1. Instalar Dependências

### Backend

```bash
cd backend
pip install -r requirements.txt
```

---

### Frontend

```bash
cd frontend
npm install
```

---

# 🛰️ Executar Broker MQTT

## Mosquitto

```bash
mosquitto -v
```

ou

```bash
docker compose up
```

---

# ▶️ Rodar Backend

```bash
cd backend
python app.py
```

API disponível em:

```text
http://localhost:5000/api/data
```

---

# 💻 Rodar Frontend

```bash
cd frontend
npm run dev
```

Dashboard disponível em:

```text
http://localhost:5173
```

---

# 📲 Configuração do ESP32

No arquivo:

```text
esp32/src/main.cpp
```

Configurar:

```cpp
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

const char* mqtt_server = "IP_DO_COMPUTADOR";
```

---

# 🧪 Testes

Durante o desenvolvimento inicial, dados simulados foram utilizados utilizando:

```cpp
random()
```

permitindo validar:

* comunicação MQTT;
* atualização em tempo real;
* integração backend/frontend;
* dashboard.

---

# 📊 Análise de Algoritmos — Histórico de Telemetria
 
Como extensão para a disciplina de Análise de Algoritmos, o TerraGuard implementa e compara duas abordagens para o gerenciamento do histórico de leituras dos sensores no ESP32.
 
## Vertente 1 — Array com Deslocamento · O(n)
 
A inserção de um novo dado exige o deslocamento de todos os elementos existentes no array. Para cada nova leitura, o microcontrolador executa **N − 1 operações de cópia**, resultando em complexidade de tempo linear:
 
> **O(n)**
 
Em cenários de gargalo de rede ou falha momentânea de conexão MQTT, o acúmulo de leituras pendentes faz com que o tempo de deslocamento de memória exceda a janela de amostragem, causando **jitter severo** e podendo acionar o **Watchdog Timer** (reinicialização da placa).
 
## Vertente 2 — Buffer Circular · O(1)
 
O Buffer Circular utiliza aritmética modular para calcular os índices de Head e Tail. A inserção de um novo elemento requer apenas a sobrescrita do valor no índice atual e o incremento circular do ponteiro:
 
```
index = (index + 1) mod N
```
 
Essa operação **não depende do tamanho do buffer**, resultando em complexidade de tempo constante:
 
> **O(1)**
 
Em situações de latência de rede, a implementação O(1) atua como amortecedor no padrão **Produtor-Consumidor**, absorvendo variações sem comprometer a estabilidade do sistema.
 
## Comparativo
 
| Critério              | Array com Deslocamento | Buffer Circular    |
| --------------------- | ---------------------- | ------------------ |
| Complexidade          | O(n)                   | O(1)               |
| Uso de memória        | Alocação dinâmica      | Tamanho fixo       |
| Estabilidade (N=20k)  | Latência crescente     | ~2 µs estável      |
| Risco de WDT Reset    | Alto                   | Nenhum             |
| Adequação ao ESP32    | Inadequada             | Recomendada        |
 
Testes de estresse com **N = 20.000 amostras** confirmaram empiricamente que a latência da Vertente 1 escala linearmente, enquanto a Vertente 2 mantém performance estável próxima a **2 µs**, com heap livre estável.

---

# 🚀 Se o projeto avançar: Funcionalidades Futuras

* gráficos históricos;
* alertas por Telegram;
* alertas por email;
* banco de dados;
* mapa de risco;
* autenticação;
* suporte IPv6;
* armazenamento em nuvem.

---

# 👥 Equipe

- [@a-guy-and-his-computer](https://www.github.com/a-guy-and-his-computer)
- [@lovepxdro](https://www.github.com/lovepxdro)
- [@kaiquegb](https://www.github.com/kaiquegb)


---

# 📚 Disciplina

Projeto desenvolvido para a disciplina de Sistemas Embarcados e Análise de Algorítimos.

CESAR School — 2026.

---

# 📄 Licença

Projeto acadêmico desenvolvido para fins educacionais.

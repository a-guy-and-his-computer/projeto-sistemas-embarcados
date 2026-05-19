#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "uaifai-tiradentes";
const char* password = "bemvindoaocesar";
const char* mqtt_server = "192.168.0.10"; // O IP local da sua máquina rodando o Docker

WiFiClient espClient;
PubSubClient client(espClient);

// Definição dos pinos simulados para os sensores do TerraGuard
#define PINO_CHUVA 34
#define PINO_UMIDADE 35
#define PINO_INCLINACAO 32

// Estrutura para os dados dos sensores
struct SensorData {
    int chuva;
    int umidade;
    int inclinacao;
};

// Declaração da fila do FreeRTOS
QueueHandle_t sensorQueue;

void setup_wifi() {
    Serial.print("Conectando ao WiFi ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado!");
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentando conexão MQTT...");
        if (client.connect("ESP32_TerraGuard")) {
            Serial.println("Conectado ao broker MQTT!");
        } else {
            Serial.print("Falhou, rc=");
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos.");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

// Task 1: Leitura dos Sensores
void TaskReadSensors(void *pvParameters) {
    SensorData data;
    for (;;) {
        // Leitura analógica dos sensores
        data.chuva = analogRead(PINO_CHUVA);
        data.umidade = analogRead(PINO_UMIDADE);
        data.inclinacao = analogRead(PINO_INCLINACAO);

        // Envia os dados para a fila aguardando no máximo o portMAX_DELAY
        if (xQueueSend(sensorQueue, &data, portMAX_DELAY) != pdPASS) {
            Serial.println("Falha ao enviar para a fila");
        }

        // Delay de 2 segundos para a próxima leitura
        vTaskDelay(2000 / portTICK_PERIOD_MS); 
    }
}

// Task 2: Comunicação MQTT
void TaskMQTT(void *pvParameters) {
    SensorData data;
    for (;;) {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        // Aguarda os dados na fila e publica assim que disponíveis
        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdPASS) {
            client.publish("terraguard/chuva", String(data.chuva).c_str());
            client.publish("terraguard/umidade", String(data.umidade).c_str());
            client.publish("terraguard/inclinacao", String(data.inclinacao).c_str());
            
            Serial.println("Dados publicados via MQTT!");
        }
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);

    // Cria a fila para armazenar até 5 leituras simultâneas
    sensorQueue = xQueueCreate(5, sizeof(SensorData));

    if (sensorQueue != NULL) {
        // Criação das Tasks atribuindo diferentes prioridades e núcleos
        xTaskCreatePinnedToCore(TaskReadSensors, "Leitura", 2048, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(TaskMQTT, "Comunicacao", 4096, NULL, 2, NULL, 0);
    }
}

void loop() {
    // O loop fica vazio pois as rotinas estão isoladas nas tasks do FreeRTOS
}
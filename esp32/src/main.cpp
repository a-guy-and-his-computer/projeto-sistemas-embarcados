#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "OI FIBRA_LOST"; 
const char* password = "10251718"; 

const char* mqtt_server = "192.168.1.5"; 

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_MPU6050 mpu;

// Usando pinos garantidos do ADC1 (Funcionam 100% com Wi-Fi ligado)
#define PINO_CHUVA 34    // ADC1_CH6
#define PINO_UMIDADE 32  // ADC1_CH4 (MUDE O FIO DA UMIDADE PARA O GPIO 32)

// LED RGB
#define LED_R 25
#define LED_G 26
#define LED_B 27

struct SensorData {
    int chuva;
    int umidade;
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    float temperatura;
    String risco;
    unsigned long timestamp;
};

QueueHandle_t sensorQueue;
QueueHandle_t mqttQueue;

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Conectando ao WiFi ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi conectado!");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentando conexão MQTT...");
        if (client.connect("ESP32_TerraGuard")) {
            Serial.println("Conectado!");
        } else {
            Serial.print("Falhou rc=");
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos");
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}

String calcularRisco(SensorData data) {
    // --- MODIFICADO AQUI: VALIDAÇÃO POR PORCENTAGEM DE CHUVA (0 a 100%) ---
    // Considera "muita chuva" se passar de 60% da capacidade do sensor
    bool muitaChuva = data.chuva > 60; 
    // ---------------------------------------------------------------------
    
    bool soloMolhado = data.umidade > 2000; 

    bool vibracao = abs(data.gyroX) > 1.5 || abs(data.gyroY) > 1.5 || abs(data.gyroZ) > 1.5;

    if (muitaChuva && soloMolhado && vibracao) return "CRITICO";
    if (muitaChuva && soloMolhado) return "ALTO";
    if (muitaChuva || soloMolhado) return "MEDIO";

    return "BAIXO";
}

void atualizarLED(String risco) {
    if (risco == "BAIXO") {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_B, LOW);
    } else if (risco == "MEDIO") {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_B, LOW);
    } else if (risco == "ALTO") {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);
    } else if (risco == "CRITICO") {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS); 
        digitalWrite(LED_R, LOW);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void TaskReadSensors(void *pvParameters) {
    SensorData data;
    sensors_event_t acel, giro, temp;

    for (;;) {
        // MPU6050
        mpu.getEvent(&acel, &giro, &temp);

        data.accelX = acel.acceleration.x;
        data.accelY = acel.acceleration.y;
        data.accelZ = acel.acceleration.z;
        data.gyroX = giro.gyro.x;
        data.gyroY = giro.gyro.y;
        data.gyroZ = giro.gyro.z;
        data.temperatura = temp.temperature;

        // Sensores analógicos com amostragem para evitar ruído
        int leituraChuva = 0;
        int lecturaUmidade = 0;
        for(int i=0; i<10; i++) {
            leituraChuva += analogRead(PINO_CHUVA);
            lecturaUmidade += analogRead(PINO_UMIDADE);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        
        // --- MODIFICADO AQUI: CALIBRAÇÃO DA CHUVA VIA MAP ---
        int leituraBrutaChuva = (leituraChuva / 10);
        Serial.printf("DEBUG CHUVA REAL: %d\n", leituraBrutaChuva);

        // ATUALIZADO COM O SEU VALOR SECO DE 1172:
        int valorSeco = 1400;     // Valor lido com o sensor seco
        int valorMolhado = 200;   // TESTE: Coloque água e mude para o valor exato que aparecer!

        // Converte a variação analógica bruta para uma escala de 0 a 100%
        data.chuva = map(leituraBrutaChuva, valorSeco, valorMolhado, 0, 100);
        data.chuva = constrain(data.chuva, 0, 100);
        // ----------------------------------------------------

        // Mantida a inversão padrão para a umidade por enquanto
        data.umidade = 4095 - (lecturaUmidade / 10);
        data.timestamp = millis();

        // Debug no terminal (Agora mostra a chuva em %)
        Serial.println("-------------------------");
        Serial.printf("Chuva: %d%% | Umidade: %d\n", data.chuva, data.umidade);
        Serial.printf("Accel X: %.2f | Gyro X: %.2f\n", data.accelX, data.gyroX);

        // Envia para fila sem travar infinitamente se o MQTT atrasar
        xQueueSend(sensorQueue, &data, pdMS_TO_TICKS(50));

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void TaskProcessData(void *pvParameters) {
    SensorData data;
    for (;;) {
        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdPASS) {
            data.risco = calcularRisco(data);
            atualizarLED(data.risco);
            xQueueSend(mqttQueue, &data, pdMS_TO_TICKS(50));
        }
    }
}

void TaskMQTT(void *pvParameters) {
    SensorData data;
    for (;;) {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();

        if (xQueueReceive(mqttQueue, &data, portMAX_DELAY) == pdPASS) {
            // Publicações otimizadas
            client.publish("terraguard/chuva", String(data.chuva).c_str());
            client.publish("terraguard/umidade", String(data.umidade).c_str());
            client.publish("terraguard/accel/x", String(data.accelX).c_str());
            client.publish("terraguard/accel/y", String(data.accelY).c_str());
            client.publish("terraguard/accel/z", String(data.accelZ).c_str());
            client.publish("terraguard/gyro/x", String(data.gyroX).c_str());
            client.publish("terraguard/gyro/y", String(data.gyroY).c_str());
            client.publish("terraguard/gyro/z", String(data.gyroZ).c_str());
            client.publish("terraguard/temperatura", String(data.temperatura).c_str());
            client.publish("terraguard/risco", data.risco.c_str());

            Serial.println("MQTT publicado!");
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Libera o núcleo momentaneamente
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);

    // Configuração explícita dos pinos analógicos para 3.3V estáveis
    analogSetAttenuation(ADC_11db); 
    pinMode(PINO_CHUVA, INPUT);
    pinMode(PINO_UMIDADE, INPUT);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setBufferSize(1024);

    Wire.begin(21, 22);
    if (!mpu.begin()) {
        Serial.println("MPU6050 não encontrado! Verifique a fiação I2C.");
        while (1) { delay(10); }
    }
    Serial.println("MPU6050 iniciado!");

    sensorQueue = xQueueCreate(5, sizeof(SensorData));
    mqttQueue = xQueueCreate(5, sizeof(SensorData));

    xTaskCreatePinnedToCore(TaskReadSensors, "Leitura", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(TaskProcessData, "Processamento", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(TaskMQTT, "MQTT", 8192, NULL, 3, NULL, 0);
}

void loop() {}
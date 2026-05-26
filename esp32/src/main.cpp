#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

const char* ssid = "NOME-DO-SEU-WIFI"; // MUDAR
const char* password = "SUA-SENHA"; // MUDAR

const char* mqtt_server = "SEU-IPV4"; // MUDAR

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_MPU6050 mpu;

#define PINO_CHUVA 34
#define PINO_UMIDADE 35

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

    Serial.println("");
    Serial.println("WiFi conectado!");
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

    bool muitaChuva = data.chuva > 2500;

    bool soloMolhado = data.umidade > 2500;

    bool vibracao =
        abs(data.gyroX) > 1.5 ||
        abs(data.gyroY) > 1.5 ||
        abs(data.gyroZ) > 1.5;

    if (muitaChuva && soloMolhado && vibracao) {
        return "CRITICO";
    }

    if (muitaChuva && soloMolhado) {
        return "ALTO";
    }

    if (muitaChuva || soloMolhado) {
        return "MEDIO";
    }

    return "BAIXO";
}


void atualizarLED(String risco) {

    if (risco == "BAIXO") {

        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_B, LOW);
    }

    else if (risco == "MEDIO") {

        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_B, LOW);
    }

    else if (risco == "ALTO") {

        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);
    }

    else if (risco == "CRITICO") {

        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);

        delay(100);

        digitalWrite(LED_R, LOW);

        delay(100);
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

        // Sensores analógicos
        data.chuva = 4095 - analogRead(PINO_CHUVA);

        data.umidade = 4095 - analogRead(PINO_UMIDADE);

        // Timestamp
        data.timestamp = millis();

        // Envia para fila
        xQueueSend(sensorQueue, &data, portMAX_DELAY);

        // Debug
        Serial.println("-------------------------");

        Serial.print("Chuva: ");
        Serial.println(data.chuva);

        Serial.print("Umidade: ");
        Serial.println(data.umidade);

        Serial.print("Accel X: ");
        Serial.println(data.accelX);

        Serial.print("Gyro X: ");
        Serial.println(data.gyroX);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}


void TaskProcessData(void *pvParameters) {

    SensorData data;

    for (;;) {

        if (xQueueReceive(sensorQueue, &data, portMAX_DELAY) == pdPASS) {
        
            // Calcula risco
            data.risco = calcularRisco(data);

            // Atualiza LED
            atualizarLED(data.risco);

            // Envia para MQTT
            xQueueSend(mqttQueue, &data, portMAX_DELAY);
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

            // Publica sensores
            client.publish(
                "terraguard/chuva",
                String(data.chuva).c_str()
            );

            client.publish(
                "terraguard/umidade",
                String(data.umidade).c_str()
            );

            // MPU6050
            client.publish(
                "terraguard/accel/x",
                String(data.accelX).c_str()
            );

            client.publish(
                "terraguard/accel/y",
                String(data.accelY).c_str()
            );

            client.publish(
                "terraguard/accel/z",
                String(data.accelZ).c_str()
            );

            client.publish(
                "terraguard/gyro/x",
                String(data.gyroX).c_str()
            );

            client.publish(
                "terraguard/gyro/y",
                String(data.gyroY).c_str()
            );

            client.publish(
                "terraguard/gyro/z",
                String(data.gyroZ).c_str()
            );

            // Temperatura
            client.publish(
                "terraguard/temperatura",
                String(data.temperatura).c_str()
            );

            // Risco
            client.publish(
                "terraguard/risco",
                data.risco.c_str()
            );

            Serial.println("MQTT publicado!");
        }
    }
}


void setup() {
    Serial.begin(115200);

    // LED RGB
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);

    // WIFI
    setup_wifi();

    // MQTT
    client.setServer(mqtt_server, 1883);

    client.setBufferSize(1024);

    // I2C (Comunicação com MPU6050)
    Wire.begin(21, 22);

    // MPU6050
    if (!mpu.begin()) {

        Serial.println("MPU6050 não encontrado");

        while (1) {
            delay(10);
        }
    }

    Serial.println("MPU6050 iniciado!");

    // Filas
    sensorQueue = xQueueCreate(5, sizeof(SensorData));

    mqttQueue = xQueueCreate(5, sizeof(SensorData));

    // Tasks
    xTaskCreatePinnedToCore(
        TaskReadSensors,
        "Leitura",
        4096,
        NULL,
        1,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        TaskProcessData,
        "Processamento",
        4096,
        NULL,
        2,
        NULL,
        1
    );

    xTaskCreatePinnedToCore(
        TaskMQTT,
        "MQTT",
        8192,
        NULL,
        3,
        NULL,
        0
    );
}


void loop() {
}
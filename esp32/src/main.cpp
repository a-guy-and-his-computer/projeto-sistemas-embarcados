#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "uaifai-tiradentes";
const char* password = "bemvindoaocesar";

const char* mqtt_server = "192.168.0.10";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

void setup_wifi() {
    delay(10);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void reconnect() {
    while (!client.connected()) {
        client.connect("ESP32Client");
    }
}

void setup() {
    Serial.begin(115200);

    setup_wifi();

    client.setServer(mqtt_server, 1883);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }

    client.loop();

    long now = millis();

    if (now - lastMsg > 2000) {
        lastMsg = now;

        int chuva = random(0, 100);
        int umidade = random(0, 100);
        int inclinacao = random(0, 30);

        String risco = "Seguro";

        if (chuva > 70 || umidade > 70 || inclinacao > 20) {
            risco = "Alto";
        }

        client.publish(
            "terraguard/chuva",
            String(chuva).c_str()
        );

        client.publish(
            "terraguard/umidade",
            String(umidade).c_str()
        );

        client.publish(
            "terraguard/inclinacao",
            String(inclinacao).c_str()
        );

        client.publish(
            "terraguard/risco",
            risco.c_str()
        );
    }
}
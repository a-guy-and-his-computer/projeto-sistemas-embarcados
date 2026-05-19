import paho.mqtt.client as mqtt
import json
import time

# Dicionário consumido pelo app.py para retornar no endpoint /api/data
sensor_data = {
    "chuva": 0,
    "umidade": 0,
    "inclinacao": 0,
    "risco": "Seguro"
}

# Em ambiente Docker, aponte para o nome do serviço do broker
BROKER = 'localhost'  # Use 'mqtt-broker' se estiver usando Docker Compose
PORT = 1883

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Conectado ao broker MQTT com sucesso")
        client.subscribe("terraguard/chuva")
        client.subscribe("terraguard/umidade")
        client.subscribe("terraguard/inclinacao")
    else:
        print(f"Falha ao conectar. Código: {rc}")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode('utf-8')
    
    try:
        value = int(payload)
        
        # Atualiza a métrica correta com base no tópico recebido
        if topic == "terraguard/chuva":
            sensor_data["chuva"] = value
        elif topic == "terraguard/umidade":
            sensor_data["umidade"] = value
        elif topic == "terraguard/inclinacao":
            sensor_data["inclinacao"] = value
            
        # Avaliação simples de risco baseada em limites hipotéticos
        if sensor_data["chuva"] > 3000 or sensor_data["inclinacao"] > 2000:
            sensor_data["risco"] = "Crítico 🚨"
        else:
            sensor_data["risco"] = "Seguro ✅"
            
        print(f"[{topic}] recebido. Dados atuais: {sensor_data}")
        
    except ValueError:
        print("Erro ao processar o payload.")

def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    try:
        client.connect(BROKER, PORT, 60)
        client.loop_start()
        return client
    except Exception as e:
        print(f"Erro ao conectar: {e}")
        return None

def stop_mqtt(client):
    if client:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    client = start_mqtt()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nEncerrando...")
        stop_mqtt(client)
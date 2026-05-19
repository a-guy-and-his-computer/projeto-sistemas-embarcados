import paho.mqtt.client as mqtt
import json

# Dicionário consumido pelo app.py para retornar no endpoint /api/data
sensor_data = {
    "chuva": 0,
    "umidade": 0,
    "inclinacao": 0,
    "risco": "Seguro"
}

# Em ambiente Docker, aponte para o nome do serviço do broker
BROKER = 'mosquitto' 
PORT = 1883

def on_connect(client, userdata, flags, rc):
    print(f"Conectado ao broker MQTT com código: {rc}")
    client.subscribe("terraguard/chuva")
    client.subscribe("terraguard/umidade")
    client.subscribe("terraguard/inclinacao")

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

    # Conecta e inicia o loop em background de forma não bloqueante
    client.connect(BROKER, PORT, 60)
    client.loop_start()

if __name__ == "__main__":
    start_mqtt()
    # Loop dummy apenas para manter o script ativo se executado avulso
    import time
    while True:
        time.sleep(1)
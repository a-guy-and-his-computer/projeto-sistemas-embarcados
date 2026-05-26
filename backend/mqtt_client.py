import paho.mqtt.client as mqtt
import time
import os

# Dicionário atualizado para conter todos os dados enviados pela ESP32
sensor_data = {
    "chuva": 0,
    "umidade": 0,
    "accelX": 0.0, "accelY": 0.0, "accelZ": 0.0,
    "gyroX": 0.0, "gyroY": 0.0, "gyroZ": 0.0,
    "temperatura": 0.0,
    "risco": "BAIXO"
}

BROKER = os.environ.get('MQTT_BROKER', 'localhost')
PORT = 1883

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"Conectado ao broker MQTT com sucesso")
        # Subscreve no wildcard '#' para pegar TUDO dentro de terraguard/
        client.subscribe("terraguard/#")
    else:
        print(f"Falha ao conectar. Código: {rc}")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode('utf-8')
    
    try:
        # Lógica para tratar os diferentes tipos de dados
        if topic == "terraguard/risco":
            sensor_data["risco"] = payload
        else:
            # Tenta converter para float (abrange int e float)
            value = float(payload)
            
            # Mapeamento dos tópicos para as chaves do dicionário
            if topic == "terraguard/chuva": sensor_data["chuva"] = int(value)
            elif topic == "terraguard/umidade": sensor_data["umidade"] = int(value)
            elif topic == "terraguard/accel/x": sensor_data["accelX"] = value
            elif topic == "terraguard/accel/y": sensor_data["accelY"] = value
            elif topic == "terraguard/accel/z": sensor_data["accelZ"] = value
            elif topic == "terraguard/gyro/x": sensor_data["gyroX"] = value
            elif topic == "terraguard/gyro/y": sensor_data["gyroY"] = value
            elif topic == "terraguard/gyro/z": sensor_data["gyroZ"] = value
            elif topic == "terraguard/temperatura": sensor_data["temperatura"] = value
            
        print(f"[{topic}] recebido: {payload}")
        
    except ValueError:
        print(f"Erro ao processar o payload '{payload}' no tópico '{topic}'")

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
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
    "risco": "BAIXO",
    "latencia_on": 0,
    "latencia_o1": 0,
    "heap_livre": 0
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
        if topic == "terraguard/risco":
            sensor_data["risco"] = payload
        else:
            value = float(payload)
            
            if topic == "terraguard/chuva": 
                # A ESP32 já envia em porcentagem, basta salvar o valor
                sensor_data["chuva"] = int(value)
                
            elif topic == "terraguard/umidade": 
                # Converte para 0-100% para o dashboard mostrar certinho
                sensor_data["umidade"] = int((value / 4095.0) * 100)
                
            elif topic == "terraguard/accel/x": sensor_data["accelX"] = round(value, 2)
            elif topic == "terraguard/accel/y": sensor_data["accelY"] = round(value, 2)
            elif topic == "terraguard/accel/z": sensor_data["accelZ"] = round(value, 2)
            elif topic == "terraguard/gyro/x": sensor_data["gyroX"] = round(value, 2)
            elif topic == "terraguard/gyro/y": sensor_data["gyroY"] = round(value, 2)
            elif topic == "terraguard/gyro/z": sensor_data["gyroZ"] = round(value, 2)
            elif topic == "terraguard/perf/latencia_on": sensor_data["latencia_on"] = int(value) # Armazena a latência do método ineficiente (em ms)
            elif topic == "terraguard/perf/latencia_o1": sensor_data["latencia_o1"] = int(value) #Armazena a latência do método O(1) (em ms)
            elif topic == "terraguard/perf/heap": sensor_data["heap_livre"] = int(value) # Armazena a memória heap livre (em bytes)
            
            elif topic == "terraguard/temperatura": 
                # Arredonda a temperatura para 1 casa decimal (ex: 26.5°C)
                sensor_data["temperatura"] = round(value, 1)
            
        print(f"[{topic}] recebido e tratado: {sensor_data}")
        
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
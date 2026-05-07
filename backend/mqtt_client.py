import json
import paho.mqtt.client as mqtt

sensor_data = {
    "chuva": 0,
    "umidade": 0,
    "inclinacao": 0,
    "risco": "Seguro"
}

BROKER = 'localhost'
PORT = 1883

TOPICS = [
    ('terraguard/chuva', 0),
    ('terraguard/umidade', 0),
    ('terraguard/inclinacao', 0),
    ('terraguard/risco', 0)
]


def on_connect(client, userdata, flags, rc):
    print('Conectado ao broker MQTT')

    for topic in TOPICS:
        client.subscribe(topic)


def on_message(client, userdata, msg):
    payload = msg.payload.decode()

    if msg.topic == 'terraguard/chuva':
        sensor_data['chuva'] = payload

    elif msg.topic == 'terraguard/umidade':
        sensor_data['umidade'] = payload

    elif msg.topic == 'terraguard/inclinacao':
        sensor_data['inclinacao'] = payload

    elif msg.topic == 'terraguard/risco':
        sensor_data['risco'] = payload

    print(sensor_data)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
client.loop_start()
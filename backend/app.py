from flask import Flask, jsonify
from flask_cors import CORS
# Importamos o sensor_data e a função que inicia tudo
from mqtt_client import sensor_data, start_mqtt 

app = Flask(__name__)
CORS(app)

# Inicia o cliente MQTT ao subir o servidor
mqtt_client = start_mqtt()

@app.route('/api/data')
def get_data():
    return jsonify(sensor_data)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
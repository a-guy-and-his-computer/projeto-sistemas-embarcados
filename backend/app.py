from flask import Flask, jsonify
from flask_cors import CORS
from mqtt_client import sensor_data

app = Flask(__name__)
CORS(app)

@app.route('/api/data')
def get_data():
    return jsonify(sensor_data)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
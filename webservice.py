from flask import Flask
from flask import jsonify
from flask import request
from flask_pymongo import PyMongo
from flask_cors import CORS
import json

app = Flask(__name__)
CORS(app)

app.config['MONGO_DBNAME']='projek'
app.config['MONGO_URI']='mongodb://0.0.0.0:27017/projek'

mongo = PyMongo(app)

@app.route('/get', methods=['GET'])
def getData():
	sensor=mongo.db.sensor
	output=[]
	for s in sensor.find().limit(10).sort("_id",-1):
		output.append({'co2':s['co2'],'temp':s['temperature'], 'hum':s['humidity']})
	return json.dumps(output)

@app.route('/add', methods=['POST'])

def addData():
	#category=request.json['kategori']
	#city=request.json['kota']
	data=json.loads(request.get_json())
	print(data)
	mongo.db.sensor.insert(data)
	
	return "OK"

if __name__=='__main__':
	app.run(host='0.0.0.0', port=8000)

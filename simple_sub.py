import paho.mqtt.client as mqtt_client
import http.client, json
from pymongo import MongoClient
#mqtt client
sub=mqtt_client.Client()
sub.connect("10.34.231.22",1883)

ip_server="192.168.56.3"
port_server=5000

#mongoclient
client=MongoClient('mongodb://0.0.0.0:27017/')
db=client['mytrip']

conn=http.client.HTTPConnection(ip_server, port=port_server)

def handle_message(mqttc,obj,msg):
	topic=msg.topic
	payload=msg.payload
	payload=payload.decode('ascii')
	print("topic: " +topic+"\npayload: "+payload)
	#tambah_API(payload)
	addData(payload)
'''
def tambah_API(data):
	header={"Content-Type": "application/json"}
	conn.request('POST','/add', body=json.dumps(data),headers=header)
	resp=conn.getresponse().read()
	print(resp.decode('ascii'))
'''
def addData(data):
	db.trip.insert(json.loads(data))

sub.on_message=handle_message

sub.subscribe("/sensor/#")

sub.loop_forever()

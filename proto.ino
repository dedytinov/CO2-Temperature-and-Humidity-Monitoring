#include "DHT.h"
#define DHTPIN11 4
#define DHTTYPE11 DHT11
#include "MQ135.h"
#define ANALOGPIN A0    //  Define Analog PIN on Arduino Board
#define RZERO 206.85    //  Define RZERO Calibration Value
MQ135 gasSensor = MQ135(ANALOGPIN);

#include <WiFiEspClient.h>
#include <WiFiEsp.h>
#include <WiFiEspUdp.h>
#include <PubSubClient.h>
#include "SoftwareSerial.h"

#define WIFI_AP "SKT"
#define WIFI_PASSWORD "1234567890"
const char* mqttServer = "192.168.1.254";
const int mqttPort = 1883;
const char* mqttUser = "";
const char* mqttPassword = "";

WiFiEspClient espClient;
PubSubClient client(espClient);
SoftwareSerial soft(2, 3); // RX, TX
DHT dht11(DHTPIN11, DHTTYPE11);
int status = WL_IDLE_STATUS;
unsigned long lastSend;

void setup() {

  Serial.begin(9600);
  dht11.begin();
  float rzero = gasSensor.getRZero();
  delay(3000);
  Serial.print("MQ135 RZERO Calibration Value : ");
  Serial.println(rzero);
  InitWiFi();
  client.setServer( mqttServer, mqttPort );
  lastSend = 0;
}

void loop() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(WIFI_AP);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      delay(500);
    }
    Serial.println("Connected to AP");
  }

  if ( !client.connected() ) {
    reconnect();
  }
  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendCO2Data();
    lastSend = millis();
  }
  client.loop();
}

void getAndSendCO2Data()
{
  float ppm = gasSensor.getPPM();
  delay(1000);
  digitalWrite(13,HIGH);

  // Check if any reads failed and exit early (to try again).
  if (isnan(ppm)) {
    Serial.println("Failed to read from MQ135 sensor!");
    return;
  }

  Serial.print("CO2 ppm value : ");
  Serial.println(ppm);
  String co2 = String(ppm);
  float h = dht11.readHumidity();
  float t = dht11.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  String temperature = String(t);
  String humidity = String(h);

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"co2\":"; payload += co2; payload += ",";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"humidity\":"; payload += humidity;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "/sensor/data", attributes );
  Serial.println( attributes );
}

void InitWiFi()
{
  // initialize serial for ESP module
  soft.begin(9600);
  // initialize ESP module
  WiFi.init(&soft);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_AP);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    delay(500);
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to Broker ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("Arduino Uno Device", mqttUser, mqttPassword) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

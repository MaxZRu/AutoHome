#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoModbus.h>
#include <WiFiNINA.h>
#include <MQTT.h>

const char ssid[] = "your_ssid";
const char pass[] = "your_password";

const char mqtt_server[] = "your_mqtt_server";
int mqtt_port = 1883;

const int modbus_server_id = 1;

EthernetClient ethClient;
ModbusTCPClient modbusClient(ethClient);

WiFiClient wifiClient;
MQTTClient mqttClient(256);

void connectWifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected!");
}

void connectMqtt() {
  Serial.print("Connecting to MQTT...");
  mqttClient.begin(mqtt_server, mqtt_port, wifiClient);

  while (!mqttClient.connect("arduino", "user", "password")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("Connected!");
}

void mqttCallback(char *topic, byte *payload, unsigned int len) {
  // Handle MQTT messages here
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  connectWifi();

  if (!modbusClient.begin(modbus_server_id)) {
    Serial.println("Failed to connect to Modbus server");
    while (1);
  }

  mqttClient.onMessage(mqttCallback);
  connectMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  if (!mqttClient.connected()) {
    connectMqtt();
  }

  // Read data from Modbus server
  int value = modbusClient.readHoldingRegisters(0, 1);
  if (modbusClient.lastError() != 0) {
    Serial.println("Error reading Modbus: " + String(modbusClient.lastError()));
  } else {
    // Publish the value to MQTT
    mqttClient.publish("modbus/data", String(value));
  }

  mqttClient.loop();
  delay(1000);
}
#include <Ethernet.h>
#include <ModbusIP_ESP8266.h>
#include <PubSubClient.h>

// Configure your Ethernet and MQTT settings
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
IPAddress server(192, 168, 1, 2);
const char* mqtt_server = "your_mqtt_server";
const int mqtt_port = 1883;
const char* mqtt_user = "your_mqtt_user";
const char* mqtt_pass = "your_mqtt_pass";
const char* mqtt_topic = "modbus_data";

// Initiate Modbus and Ethernet clients
EthernetClient ethClient;
ModbusIP mb;
PubSubClient mqttClient(ethClient);

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  delay(1500);

  mb.config(&ethClient, ip);

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  if (!mqttClient.connected()) {
    reconnect();
  }
}

void loop() {
  mb.task();
  mqttClient.loop();

  if (!mqttClient.connected()) {
    reconnect();
  }

  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    readAndPublishModbusData();
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages, if needed
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ArduinoClient", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void readAndPublishModbusData() {
  // Read and publish Modbus data over MQTT
  uint16_t data = 0;
  mb.readHreg(1, 100, &data, 1);
  char msg[50];
  snprintf(msg, 50, "Modbus device 1, register 100: %d", data);
  mqttClient.publish(mqtt_topic, msg);
}
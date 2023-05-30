#include <Ethernet.h>
#include <ModbusIP_ESP8266.h>
#include <PubSubClient.h>

// Modbus IP settings
IPAddress server(192, 168, 1, 100);
ModbusIP mb;

// MQTT settings
const char* mqtt_server = "your_mqtt_server_here";
const char* mqtt_user = "your_mqtt_user_here";
const char* mqtt_password = "your_mqtt_password_here";
const char* mqtt_topic = "your_mqtt_topic_here";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip);
  mb.server(server);

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  mb.task();  // Modbus IP communication

  // MQTT connection
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  // Read data via Modbus IP
  uint16_t data;
  mb.readHreg(server, 0, &data, 1);

  // Publish data via MQTT
  char msg[10];
  snprintf(msg, sizeof(msg), "%d", data);
  mqttClient.publish(mqtt_topic, msg);
  delay(1000);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming MQTT messages
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ArduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
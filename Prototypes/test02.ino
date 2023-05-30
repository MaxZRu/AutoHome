#include <Wire.h>
#include <Ethernet.h>
#include <Modbus.h>
#include <ModbusTCP.h>
#include <PubSubClient.h>

// Modbus Ethernet settings
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
IPAddress server(192, 168, 1, 2);

// MQTT settings
const char* mqtt_server = "example.com";
const char* mqtt_topic = "modbus_data";
const char* mqtt_client_name = "Modbus_MQTT";

EthernetClient ethClient;
ModbusTCPClient modbusTCPClient(ethClient);
PubSubClient mqttClient(ethClient);

// Function to read Modbus registers and publish to MQTT
void readAndPublish() {
  uint16_t data[10];
  int result = modbusTCPClient.readHoldingRegisters(1, 0, 10, data);

  if (result == 0) {
    char payload[100];
    snprintf(payload, sizeof(payload), "R1:%d,R2:%d,R3:%d,R4:%d,R5:%d,R6:%d,R7:%d,R8:%d,R9:%d,R10:%d", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
    mqttClient.publish(mqtt_topic, payload);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Ethernet.begin(mac, ip);
  mqttClient.setServer(mqtt_server, 1883);
  modbusTCPClient.begin(server, 502);
}

void loop() {
  if (!mqttClient.connected()) {
    while (!mqttClient.connect(mqtt_client_name)) {
      // Wait for MQTT connection
      delay(5000);
    }
  }
  mqttClient.loop();
  readAndPublish();
  delay(1000);
}
#include <Ethernet.h>
#include <ModbusIP_ESP8266.h>
#include <PubSubClient.h>
#include <MQTT.h>

// Configuration
#define MODBUS_SERVER_IP IPAddress(192, 168, 1, 100)
#define MQTT_SERVER_IP IPAddress(192, 168, 1, 101)
#define MQTT_CLIENT_ID "ArduinoModbusClient"
#define MQTT_TOPIC "modbus/data"

// Modbus Registers
const int HR_START_ADDRESS = 0;
const int HR_TOTAL_REGISTERS = 6;

// Ethernet settings
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 102);

// Ethernet client
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

// Modbus IP object
ModbusIP mb;

// Callback function for MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // Process incoming MQTT messages here
}

// Initialize Ethernet
void setupEthernet() {
  Ethernet.begin(mac, ip);
  delay(1500);
}

// Initialize Modbus
void setupModbus() {
  mb.begin();
}

// Initialize MQTT
void setupMQTT() {
  mqttClient.setServer(MQTT_SERVER_IP, 1883);
  mqttClient.setCallback(callback);
}

// Connect to MQTT
void connectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      // Connected
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  setupEthernet();
  setupModbus();
  setupMQTT();
}

void loop() {
  // Reconnect to MQTT if needed
  if (!mqttClient.connected()) {
    connectMQTT();
  }

  // Process MQTT messages
  mqttClient.loop();

  // Read Modbus registers
  uint16_t data[HR_TOTAL_REGISTERS];
  mb.readHreg(MODBUS_SERVER_IP, HR_START_ADDRESS, data, HR_TOTAL_REGISTERS);

  // Publish Modbus data to MQTT
  char mqttPayload[64];
  for (int i = 0; i < HR_TOTAL_REGISTERS; i++) {
    snprintf(mqttPayload, sizeof(mqttPayload), "Register %d: %d", HR_START_ADDRESS + i, data[i]);
    mqttClient.publish(MQTT_TOPIC, mqttPayload);
  }

  // Wait for a while
  delay(5000);
}

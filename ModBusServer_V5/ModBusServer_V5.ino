//Регистры  MODBUS
/*
HREG:
#401
#402
#403
#404 Лампа входа в машины (0..255)
#405 Лампа кухни (0..255)
#406
#407
#408
#409
#410

IREG 3x Рагистр данные Из PLС в Экран (только в одну сторону)
1 Уровень электричества 0..1023
2 Уровнеь воды в баке (литры) 0..1023
3 Зажигание, ключ зажигания 0..1023
4 Задняя передача 0..1023
5 Габаритные огни 0..1023
6
7
8
9
10            // 0x100 Battery capacity 0...100  (100)
11            // 0x101 Battery Voltage *0.1 or /10  (140)
12            // 0x102 Chargin Ток зарядки *0.1 or /10 (398)
13            // 0x103 Температура Мл и Ст байты   (7449)
         // Нагрузка   
14            // 0x104 Напряжение нагрузки *0.1 /10 (140)
15            // 0x105 Ток нагрузки *0.1  /10   (356)
16            // 0x106 Мощность нагрузки *0.1 /10  (49) Вт
       // Солнечные панели 
17            // 0x107 Солнечная панель *0.1 Напряжение (160)
18            // 0x108 Солнечная панель *0.01 ТОК (362)
19            // 0x109 Мощность с панели Вт (58)
20            // 0x10A 0/1 - Включена нагрузка (0)
21            // 0x10B Минимальное напряжение аккумулятора за день (138)
22            // 0x10C Максимальное напряжение аккумулятора за день (144)
23            // 0x10D Максимальный ток зарядки за день (449)
24            // 0x10E Максимальный ток Разрядки за день (364)
25            // 0x10F Максимальная мощность зарядки за день (62)
26            // 0x110 Максимальная мощность Разрядки за день (51)
27            // 0x111 Зарядка Ампер*Часы за день (11)
28            // 0x112 Разрядка Ампер*Часы за день (8)
29            // 0x113 Мощность выработанная за день (155)
30            // 0x114 Мощность потребляемая за день (0)


Coil От Экрана в PLC 
1 Реле 1 CONTROLLINO_R1
2 Реле 2 CONTROLLINO_R2
3 Реле 3 CONTROLLINO_R3
4 Реле 4 CONTROLLINO_R4
5
6
7
8
9
10

Ists
1
2
3
4
5
6
7
8
9
10

*/


//#define MODBUSIP_DEBUG
//#define MODBUSRTU_DEBUG


#include <SPI.h>
#include <Controllino.h>  //3.0.7
#include "ModbusRtu.h"
#include "auto_home_pass.h"
/*
#define MQTT_CLIENT_ID         "MEGA_005"
#define MQTT_CLIENT_USER       "XXXXXXX"
#define MQTT_CLIENT_PASS       "XXXXXXX"
#define MQTT_CLIENT_HOST       "XXXXXXX"
#define MQTT_CLIENT_PORT       7498
#define WORK_AREA               1  //1 - автодом, 2 -отладка (тест)
*/


#include <Ethernet.h>           // Ethernet library v2 is required
#include <ModbusEthernet.h>     //ArduinoModbus 1.0.8 and modbus-esp8266 4.1.0  (https://github.com/emelianov/modbus-esp8266)
#include <Adafruit_NeoPixel.h>  //1.11.0
#include <MQTT.h>               // MQTT 2.5.1. https://github.com/256dpi/arduino-mqtt (MQTT Joel Gaehwiler)  //PubSubClient 2.8 //EspMQTTClient 1.13.3

#if (WORK_AREA == 1)  //  автодом
#elif (WORK_AREA == 2)  // TEST 0
#endif  // конец условия

ModbusEthernet mb;  // Declare ModbusTCP instance
MQTTClient clientMQTT;
EthernetClient net;

EthernetUDP udp;

#define MasterModbusAdd 0
#define SlaveModbusAdd 1
#define RS485Serial 3
Modbus2 ControllinoModbusMaster(MasterModbusAdd, RS485Serial, 0);

uint16_t ModbusSlaveRegisters_MPPT_Solar[50];
modbus_t ModbusQuery[1];

char receivingBuffer[100];

Adafruit_NeoPixel pixels(64, CONTROLLINO_D3, NEO_GRB + NEO_KHZ800);

void setup_pin() {

  pinMode(CONTROLLINO_R0, OUTPUT);
  pinMode(CONTROLLINO_R1, OUTPUT);
  pinMode(CONTROLLINO_R2, OUTPUT);
  pinMode(CONTROLLINO_R3, OUTPUT);
  pinMode(CONTROLLINO_R4, OUTPUT);
  pinMode(CONTROLLINO_R5, OUTPUT);
  pinMode(CONTROLLINO_R6, OUTPUT);
  pinMode(CONTROLLINO_R7, OUTPUT);
  pinMode(CONTROLLINO_R8, OUTPUT);
  pinMode(CONTROLLINO_R9, OUTPUT);
  pinMode(CONTROLLINO_R10, OUTPUT);
  pinMode(CONTROLLINO_R11, OUTPUT);
  pinMode(CONTROLLINO_R12, OUTPUT);
  pinMode(CONTROLLINO_R13, OUTPUT);
  pinMode(CONTROLLINO_R14, OUTPUT);
  pinMode(CONTROLLINO_R15, OUTPUT); //Инвертор 220 Вольт через РЕЛЕ


  pinMode(CONTROLLINO_D0, OUTPUT);
  pinMode(CONTROLLINO_D1, OUTPUT);
  pinMode(CONTROLLINO_D2, OUTPUT);
  pinMode(CONTROLLINO_D3, OUTPUT);
  pinMode(CONTROLLINO_D4, OUTPUT);  // Свет вход
  pinMode(CONTROLLINO_D5, OUTPUT);  // Свет кухня
  pinMode(CONTROLLINO_D6, OUTPUT);  // Свет салон
  pinMode(CONTROLLINO_D7, OUTPUT);
  pinMode(CONTROLLINO_D8, OUTPUT);
  pinMode(CONTROLLINO_D9, OUTPUT);
  pinMode(CONTROLLINO_D10, OUTPUT);
  pinMode(CONTROLLINO_D11, OUTPUT);
  pinMode(CONTROLLINO_D12, OUTPUT);  // Сигнал для зарядки (аналог машина заведена)
  pinMode(CONTROLLINO_D13, OUTPUT);
  pinMode(CONTROLLINO_D14, OUTPUT);
  pinMode(CONTROLLINO_D15, OUTPUT);
  pinMode(CONTROLLINO_D16, OUTPUT);
  pinMode(CONTROLLINO_D17, OUTPUT);
  pinMode(CONTROLLINO_D18, OUTPUT);
  pinMode(CONTROLLINO_D18, OUTPUT);
  pinMode(CONTROLLINO_D19, OUTPUT);
  pinMode(CONTROLLINO_D20, OUTPUT);
  pinMode(CONTROLLINO_D21, OUTPUT);
  pinMode(CONTROLLINO_D22, OUTPUT);
  pinMode(CONTROLLINO_D23, OUTPUT);

  pinMode(CONTROLLINO_A0, INPUT);  // Напряжение бортовой сети
  pinMode(CONTROLLINO_A1, INPUT);  // Уровень воды в баке
  pinMode(CONTROLLINO_A2, INPUT);  // Зажигание, ключ зажигания, подключено от магнитолы
  pinMode(CONTROLLINO_A3, INPUT);  // Задняя передача
  pinMode(CONTROLLINO_A4, INPUT);  // Габаритные огни
  pinMode(CONTROLLINO_A5, INPUT);
  pinMode(CONTROLLINO_A6, INPUT);
  pinMode(CONTROLLINO_A7, INPUT);
  pinMode(CONTROLLINO_A8, INPUT);
  pinMode(CONTROLLINO_A9, INPUT);
  pinMode(CONTROLLINO_A10, INPUT);
  pinMode(CONTROLLINO_A11, INPUT);
  pinMode(CONTROLLINO_A12, INPUT);
  pinMode(CONTROLLINO_A13, INPUT);
  pinMode(CONTROLLINO_A14, INPUT);
  pinMode(CONTROLLINO_A15, INPUT);

  //  pinMode(CONTROLLINO_A0, INPUT);
  //  pinMode(CONTROLLINO_D0, OUTPUT);
  //  pinMode(CONTROLLINO_D2, OUTPUT);
  //  pinMode(CONTROLLINO_D4, OUTPUT);
  //  pinMode(CONTROLLINO_D11, OUTPUT);

  //starting ...
  digitalWrite(CONTROLLINO_D2, false);
  digitalWrite(CONTROLLINO_D11, false);
}

void setup_neopixel() {
  //pixels.setPin(CONTROLLINO_D16);
  //pixels.updateLength(16);
  //pixels.Color(0, 150, 0)
  pixels.begin();
  pixels.clear();
  pixels.fill(pixels.Color(0, 0, 0), 0, 64);
  pixels.show();
  //pixels.fill(start,)
  //pixels.setPin();
  //
  // void begin(void);
  // void show(void);
  // void setPin(int16_t p);
  // void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
  // void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
  // void setPixelColor(uint16_t n, uint32_t c);
  // void fill(uint32_t c = 0, uint16_t first = 0, uint16_t count = 0);
  // void setBrightness(uint8_t);
  // void clear(void);
  //  void updateLength(uint16_t n); - длина ...
}
/*
  LWMQTT_SUCCESS = 0,
  LWMQTT_BUFFER_TOO_SHORT = -1,
  LWMQTT_VARNUM_OVERFLOW = -2,
  LWMQTT_NETWORK_FAILED_CONNECT = -3,
  LWMQTT_NETWORK_TIMEOUT = -4,
  LWMQTT_NETWORK_FAILED_READ = -5,
  LWMQTT_NETWORK_FAILED_WRITE = -6,
  LWMQTT_REMAINING_LENGTH_OVERFLOW = -7,
  LWMQTT_REMAINING_LENGTH_MISMATCH = -8,
  LWMQTT_MISSING_OR_WRONG_PACKET = -9,
  LWMQTT_CONNECTION_DENIED = -10,
  LWMQTT_FAILED_SUBSCRIPTION = -11,
  LWMQTT_SUBACK_ARRAY_OVERFLOW = -12,
  LWMQTT_PONG_TIMEOUT = -13,
*/

void connect_MQTT() {

  static unsigned int MQTT_CONNECT_COUNT = 0;
  Serial.print("connecting MQTT...");
  Serial.print(" CodeError:");
  Serial.print(clientMQTT.lastError());
  //u_907SA2 q7KL2bTE
  clientMQTT.setTimeout(200);


  if (!clientMQTT.connect(MQTT_CLIENT_ID, MQTT_CLIENT_USER, MQTT_CLIENT_PASS)) {
    Serial.print(" ReConnect CodeError:");
    Serial.println(clientMQTT.lastError());
    // -3 LWMQTT_NETWORK_FAILED_CONNECT
    // -9 LWMQTT_MISSING_OR_WRONG_PACKET
    //
    //clientMQTT.disconnect();
    //clientMQTT.begin("m4.wqtt.ru", 7498, net);

    MQTT_CONNECT_COUNT = MQTT_CONNECT_COUNT + 1;
    Serial.print("Connect MQTT(count):");
    Serial.print(MQTT_CONNECT_COUNT);
    Serial.println(".");
    delay(100);
    if (MQTT_CONNECT_COUNT > 30) {
      softwareReset();
      MQTT_CONNECT_COUNT = 0;
    }
  } else {
    Serial.println("\nconnected!");
    clientMQTT.subscribe("IVECO/LAMP_KITCHEN_ON", 1);
    clientMQTT.subscribe("IVECO/LAMP_KITCHEN_LIGHT", 1);
    clientMQTT.subscribe("IVECO/LAMP_KITCHEN_RGB", 1);

    clientMQTT.subscribe("IVECO/LAMP_ENTER_ON", 1);
    clientMQTT.subscribe("IVECO/LAMP_ENTER_LIGHT", 1);

    clientMQTT.subscribe("IVECO/LAMP_SALON_ON", 1);
    clientMQTT.subscribe("IVECO/LAMP_SALON_LIGHT", 1);
    MQTT_CONNECT_COUNT = 0;

    //clientMQTT.subscribe("/R7");
  }
  // clientMQTT.unsubscribe("/hello");
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void messageReceived(String &topic, String &payload) {
  static unsigned int LAMP_KITCHEN_ON = 0;
  static unsigned int LAMP_ENTER_ON = 0;

  static unsigned int LAMP_KITCHEN_LIGHT = 0;
  static unsigned int LAMP_ENTER_LIGHT = 0;

  static unsigned int LAMP_SALON_ON = 0;
  static unsigned int LAMP_SALON_LIGHT = 0;


  Serial.println("incoming: " + topic + " - " + payload);
  /*
  if (topic=="/R7") {
      digitalWrite(CONTROLLINO_R7, payload.toInt()); // 0x.. #1
  }
*/
  if (topic == "IVECO/LAMP_KITCHEN_ON") {  //404
    LAMP_KITCHEN_ON = payload.toInt();
    mb.Hreg(5 - 1, round(LAMP_KITCHEN_ON * LAMP_KITCHEN_LIGHT * 0.5));
    if (payload.toInt() > 90) {
      mb.Hreg(5 - 1, LAMP_KITCHEN_ON * 255);
    }
    if (LAMP_KITCHEN_ON == 0) {
      //RGB
      mb.Hreg(6 - 1, 0);
      mb.Hreg(7 - 1, 0);
      mb.Hreg(8 - 1, 0);
    }
    Serial.println(mb.Hreg(5 - 1));
  }

  if (topic == "IVECO/LAMP_KITCHEN_LIGHT") {  //404
    LAMP_KITCHEN_LIGHT = payload.toInt();
    mb.Hreg(5 - 1, round(LAMP_KITCHEN_ON * LAMP_KITCHEN_LIGHT * 0.5));
    if (payload.toInt() > 90) {
      mb.Hreg(5 - 1, LAMP_KITCHEN_ON * 255);
      analogWrite(CONTROLLINO_D5, mb.Hreg(5 - 1));  //#Hreg (405) => D5 PWM (4000гц)
    }
    Serial.println(mb.Hreg(5 - 1));
  }

  if (topic == "IVECO/LAMP_KITCHEN_RGB") {  //404

    String Color_R = getValue(payload, ',', 0);
    String Color_G = getValue(payload, ',', 1);
    String Color_B = getValue(payload, ',', 2);

    uint16_t Color_R_int = Color_R.toInt() * LAMP_KITCHEN_ON;
    uint16_t Color_G_int = Color_G.toInt() * LAMP_KITCHEN_ON;
    uint16_t Color_B_int = Color_B.toInt() * LAMP_KITCHEN_ON;

    mb.Hreg(6 - 1, Color_R_int);
    mb.Hreg(7 - 1, Color_G_int);
    mb.Hreg(8 - 1, Color_B_int);

    uint16_t color_r = min(mb.Hreg(6 - 1), 255);
    uint16_t color_g = min(mb.Hreg(7 - 1), 255);
    uint16_t color_b = min(mb.Hreg(8 - 1), 255);
    pixels.fill(pixels.Color(color_r, color_b, color_g), 0, 64);
    pixels.show();

    /*
      LAMP_KITCHEN_LIGHT = payload.toInt();
      mb.Hreg(5-1,round(LAMP_KITCHEN_ON*LAMP_KITCHEN_LIGHT*0.5));
      if (payload.toInt()>90) 
      {
        mb.Hreg(5-1,LAMP_KITCHEN_ON*255);
      }
*/
  }

  //IVECO/LAMP_KITCHEN_RGB 10,255,0


  if (topic == "IVECO/LAMP_ENTER_ON") {  //405
    LAMP_ENTER_ON = payload.toInt();

    mb.Hreg(4 - 1, round(LAMP_ENTER_ON * LAMP_ENTER_LIGHT * 0.5));
    if (payload.toInt() > 90) {
      mb.Hreg(4 - 1, LAMP_ENTER_ON * 255);
      analogWrite(CONTROLLINO_D4, mb.Hreg(4 - 1));  //#Hreg (404) => D4 PWM (4000гц)
    }
    Serial.println(mb.Hreg(4 - 1));
  }

  if (topic == "IVECO/LAMP_ENTER_LIGHT") {  //404
    LAMP_ENTER_LIGHT = payload.toInt();
    mb.Hreg(4 - 1, round(LAMP_ENTER_ON * LAMP_ENTER_LIGHT * 0.5));
    if (payload.toInt() > 90) {
      mb.Hreg(4 - 1, LAMP_ENTER_ON * 255);
      analogWrite(CONTROLLINO_D4, mb.Hreg(4 - 1));  //#Hreg (404) => D4 PWM (4000гц)
    }
    Serial.println(mb.Hreg(4 - 1));
  }


  if (topic == "IVECO/LAMP_SALON_ON") {  //405
    LAMP_SALON_ON = payload.toInt();

    mb.Hreg(6 - 1, round(LAMP_SALON_ON * LAMP_SALON_LIGHT * 0.5));
    if (LAMP_SALON_ON > 90) {
      mb.Hreg(6 - 1, LAMP_SALON_ON * 255);
      analogWrite(CONTROLLINO_D6, mb.Hreg(6 - 1));  //#Hreg (404) => D4 PWM (4000гц)
    }
    Serial.println(mb.Hreg(6 - 1));
  }

  if (topic == "IVECO/LAMP_SALON_LIGHT") {  //404
    LAMP_SALON_LIGHT = payload.toInt();
    mb.Hreg(6 - 1, round(LAMP_SALON_ON * LAMP_SALON_LIGHT * 0.5));
    if (payload.toInt() > 90) {
      mb.Hreg(6 - 1, LAMP_SALON_ON * 255);
      analogWrite(CONTROLLINO_D6, mb.Hreg(6 - 1));  //#Hreg (404) => D4 PWM (4000гц)
    }
    Serial.println(mb.Hreg(6 - 1));
  }

  //    clientMQTT.subscribe("IVECO/LAMP_KITCHEN_LIGHT");
  //    clientMQTT.subscribe("IVECO/LAMP_ENTER_LIGHT");


  // Note: Do not use the clientMQTT in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `clientMQTT.loop()`.
}


void setup_Modbus_server() {

  mb.server();  // Act as Modbus TCP server

  //Выключатели на экране ЭКРАН->PLC
  // Регистр 0х - адрес 2  равен Coil (2-1)
  // Регистр 4х_Bit - адрес 4.00 = равен Hreg(4-1) - режим побитовый 4.00 = 0/1  4.02 = 0/4


  //uint16_t offset, uint16_t value = 0, uint16_t numregs = 1
  //Смещение, Значение по умолчанию, Количество Регистров..
  //mb.addIreg(1 - 1, 40);     // 3x.. .... Исходящее... из ПЛК в ЭКРАН

  // c 1 регистра, создать 10 штук  -401 - это смещение...
  //Регистр в обе стороны 400
  //401,402,403,404,405,406,407,408,409,410
  mb.addHreg(401 - 401, 0, 10);  // 3x.. .... Исходящее... из ПЛК в ЭКРАН

  //Добавляем регистр 405 в Экране, в Arduino это 405-401 = 4 регистр
  //mb.addHreg(405 - 401, 30);  // 405   4x Входящий из Исходящий Экрана в ПЛК

  //Регистр в одну сторону 300 от ПЛК в ЭКРАН
  mb.addIreg(1 - 1, 0, 40);  // 3x

  //Регистр битный в обе стороны 000
  mb.addCoil(1 - 1, false, 10);  //

  //Регистр ввода данных в одну сторону... 100 от ПЛК в ЭКРАН
  mb.addIsts(1 - 1, false, 10);


  //mb.addHreg(402 - 401, 30);  // 405   4x Входящий из Исходящий Экрана в ПЛК
  /*
  mb.addHreg(5 - 1, 50);  // 3x.. Регулятор яркости
  mb.addCoil(5 - 1, false);  // 3x.. Регулятор яркости
  mb.addIsts(5 - 1, false);  // 3x.. Регулятор яркости
  */

  //
  // void addCoil (offset word, bool value) из Экрана в ПЛК
  // void addHreg (offset word, word value)
  // void addIsts (offset word, bool value) из ПЛК в Экран
  // void addIreg (offset word, word value)
  // SET
  // bool Coil (offset word, bool value) // 0....1 (приемка РЕЛЕ)
  // bool Hreg (offset word, word value) // ЧИСЛО  (приемка ДАННЫЕ)
  // bool Ists (offset word, bool value) // 0....1 (передача A0...)
  // bool IREG (offset word, word value) // ЧИСЛО 3x.. (передача A0...)
}

void setup() {
  Serial.begin(115200);  // Open serial communications and wait for port to open

  Serial.println("Start...");

  uint8_t mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x21 };



#if (WORK_AREA == 1)   //  автодом

    uint8_t ip[] = { 192, 168, 10, 10 };
    uint8_t gateway[] = { 192, 168, 10, 1 };


#elif (WORK_AREA == 2) // TEST 0

  uint8_t ip[] = { 192, 168, 11, 122 };
  uint8_t gateway[] = { 192, 168, 11, 1 };

#endif   






  uint8_t dns[] = { 8, 8, 8, 8 };
  uint8_t subnet[] = { 255, 255, 255, 0 };

  Serial.print("Ethernet.begin...");

  Ethernet.begin(mac, ip, dns, gateway, subnet);
  //Ethernet.begin(mac, ip, gateway, subnet);
  Serial.println("OK IP 192.168.10.10 ");
  Serial.print("Setup PIN...");
  setup_pin();
  Serial.println("OK");

  delay(1000);  // give the Ethernet shield a second to initialize

  Serial.print("Setup ModBus_SERVER...");
  setup_Modbus_server();
  Serial.println("OK");

  Serial.print("Setup setup_neopixel...");
  setup_neopixel();
  Serial.println("OK");

  udp.begin(8080);  //udp 8080 порт

  // TCCR1B = TCCR1B & 0b11111000 | 0x02;
  // TCCR2B = TCCR2B & 0b11111000 | 0x02;
  // TCCR3B = TCCR3B & 0b11111000 | 0x02;
  TCCR4B = TCCR4B & 0b11111000 | 0x01;  //Таймер 4 для пинов D4, D5, D6 //31000 гц

  clientMQTT.setTimeout(200);
  clientMQTT.begin(MQTT_CLIENT_HOST, MQTT_CLIENT_PORT, net);  //94.250.251.152 u_907SA2 q7KL2bTE
                                              //  uint8_t mqtt[] = { 94, 250, 251, 152 };
                                              //  clientMQTT.begin(mqtt, 7498, net); //94.250.251.152 u_907SA2 q7KL2bTE

  clientMQTT.onMessage(messageReceived);


  ModbusQuery[0].u8id = SlaveModbusAdd;                      // slave address
  ModbusQuery[0].u8fct = 3;                                  // function code (this one is registers read)
  ModbusQuery[0].u16RegAdd = 0x100;                          // start address in slave
  ModbusQuery[0].u16CoilsNo = 20;                            // number of elements (coils or registers) to read
  ModbusQuery[0].au16reg = ModbusSlaveRegisters_MPPT_Solar;  // pointer to a memory array in the CONTROLLINO

  ControllinoModbusMaster.begin(9600);       // baud-rate at 19200
  ControllinoModbusMaster.setTimeOut(5000);  // if there is no answer in 5000 ms, roll over


  connect_MQTT();

  //  ArduinoOTA.begin(Ethernet.localIP(), "AvtoMEGA", "123", InternalStorage);
}



void timer_1(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    {
      digitalWrite(CONTROLLINO_D1, !digitalRead(CONTROLLINO_D1));
    }
  }
}

//Программный сброс
void softwareReset() {

  Serial.println("REBOOT ............ REBOOT ....................");
  delay(100);
  asm volatile("jmp 0");
}

void timer_2(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    {
      //digitalWrite(CONTROLLINO_D2, !digitalRead(CONTROLLINO_D2));
    }
  }
}


void timer_modus_pin(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    {
      //digitalWrite(CONTROLLINO_D2, !digitalRead(CONTROLLINO_D2));
    }
  }
}


void timer_rgb(long delay_ms) {
  static unsigned long timer_long = 0;
  static uint16_t color_r_last = 0;
  static uint16_t color_g_last = 0;
  static uint16_t color_b_last = 0;

  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    {
      uint16_t color_r = min(mb.Hreg(6 - 1), 255);
      uint16_t color_g = min(mb.Hreg(7 - 1), 255);
      uint16_t color_b = min(mb.Hreg(8 - 1), 255);

      if ((color_r != color_r_last) || (color_g != color_g_last) || (color_b != color_b_last)) {
        pixels.fill(pixels.Color(color_r, color_b, color_g), 0, 64);
        pixels.show();
        color_r_last = color_r;
        color_g_last = color_g;
        color_b_last = color_b;
      }
    }
  }
}


void timer_worker(long delay_ms) {
  static unsigned long timer_long = 0;
  static unsigned long work_time = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    {
      Serial.print("Work .. ");
      Serial.print(work_time);
      Serial.println();
      work_time++;

      if (clientMQTT.connected()) {
        clientMQTT.publish("IVECO/TIME_WORK", String(work_time), true, 1);
      }
    }
  }
}


void timer_modbus_list(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    {
      Serial.println("MODBUS REG:");

      for (int i = 1; i < 11; i++) {
        Serial.print("[");
        Serial.print(i);
        Serial.print("] = Hreg:");
        Serial.print(mb.Hreg(i - 1));
        Serial.print(" Coil:");
        Serial.print(mb.Coil(i - 1));
        Serial.print(" Ists:");
        Serial.print(mb.Ists(i - 1));
        Serial.print(" Ireg:");
        Serial.println(mb.Ireg(i - 1));
      }
    }
  }
}



void timer_Coil_RELE(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();

    digitalWrite(CONTROLLINO_R1, mb.Coil(1 - 1));  // 0x.. #1
    digitalWrite(CONTROLLINO_R2, mb.Coil(2 - 1));  // 0x.. #2
    digitalWrite(CONTROLLINO_R3, mb.Coil(3 - 1));  // 0x.. #3
    digitalWrite(CONTROLLINO_R4, mb.Coil(4 - 1));  // 0x.. #4
  }
}

void timer_PWM(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();

    analogWrite(CONTROLLINO_D5, mb.Hreg(5 - 1));  //#Hreg (405) => D5 PWM (4000гц)
    analogWrite(CONTROLLINO_D4, mb.Hreg(4 - 1));  //#Hreg (404) => D4 PWM (4000гц)
    analogWrite(CONTROLLINO_D6, mb.Hreg(6 - 1));  //#Hreg (404) => D4 PWM (4000гц)
  }
}

void timer_input_analog(long delay_ms) {
  static unsigned long timer_long = 0;
  static unsigned long count = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    count++;
    if (count > 100) {
      count = 0;
    }
    // Бортовое напряжение сети
    mb.Ireg(1 - 1, round(15.1 / 1024 * 10 * analogRead(CONTROLLINO_A0)));  //15Вольт=1024
    // Уроверь воды в баке коэф 3 + a1/3.2
    mb.Ireg(2 - 1, 3 + round(analogRead(CONTROLLINO_A1) / 3.2));

    mb.Ireg(3 - 1, analogRead(CONTROLLINO_A2));  //Зажигание, ключ зажигания
    mb.Ireg(4 - 1, analogRead(CONTROLLINO_A3));  //Задняя передача
    mb.Ireg(5 - 1, analogRead(CONTROLLINO_A4));  //Габаритные огни

    //Активация автоматической зарядки от двигателя:
    if (analogRead(CONTROLLINO_A2) > 500) {
      digitalWrite(CONTROLLINO_D12, 1);
    } else {
      digitalWrite(CONTROLLINO_D12, 0);
    }
  }
}


void timer_sendMQTT(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();
    if (clientMQTT.connected()) {
      clientMQTT.publish("IVECO/WATER_LEVEL", String(round(analogRead(CONTROLLINO_A1) / 3.2)), true, 1);           //Уровень воды
      clientMQTT.publish("IVECO/VOLTAGE", String((15.1 / 1024  * analogRead(CONTROLLINO_A0)),2), true, 1);  //Напряжение бортовой сети 12.0
      clientMQTT.publish("IVECO/PLC/A1", String(analogRead(CONTROLLINO_A1)), true, 1);                             //Уровень Воды
      clientMQTT.publish("IVECO/PLC/A2", String(analogRead(CONTROLLINO_A2)), true, 1);                             //Зажигание
      clientMQTT.publish("IVECO/PLC/A3", String(analogRead(CONTROLLINO_A3)), true, 1);                             //Задняя передача
      clientMQTT.publish("IVECO/PLC/A4", String(analogRead(CONTROLLINO_A4)), true, 1);                             //Габариты
    }
  }
}


void timer_sendMQTT_SOLAR(long delay_ms) {
  static unsigned long timer_long = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();

    /*
    if (clientMQTT.connected()) {
      for (int i = 0; i < 21; i++) {
        clientMQTT.publish("IVECO/SOLAR/" + String(i), String(ModbusSlaveRegisters_MPPT_Solar[i]), true, 1);  //Габариты
      }
    }
    */

    /* Публикуем только то что читаем ... */
    if (clientMQTT.connected()) {
         clientMQTT.publish("IVECO/SOLAR/0" , String(ModbusSlaveRegisters_MPPT_Solar[0] ), true, 1);  // Процент заряда Аккумулятора
         clientMQTT.publish("IVECO/SOLAR/1" , String(float(ModbusSlaveRegisters_MPPT_Solar[1])/10,1), true, 1);  // Напряжение батареи 12.0
         clientMQTT.publish("IVECO/SOLAR/5" , String(float(ModbusSlaveRegisters_MPPT_Solar[5])/100,2), true, 1);  // Ток нагрузки /100
         clientMQTT.publish("IVECO/SOLAR/6" , String(ModbusSlaveRegisters_MPPT_Solar[6] ), true, 1);  // Мощность нагрузки Вт
         clientMQTT.publish("IVECO/SOLAR/9" , String(ModbusSlaveRegisters_MPPT_Solar[9] ), true, 1);  // Мощность солнечных панелей  Вт

    }
    


  }
}


void timer_Modbus_COM3(long delay_ms) {
  static unsigned long timer_long = 0;
  static unsigned long myState = 0;
  static unsigned long currentQuery = 0;
  if (millis() - timer_long > delay_ms) {
    timer_long = millis();

    switch (myState) {
      case 0:
        myState++;
        break;
      case 1:
        Serial.println("--- ControllinoModbusMaster sender-begin ---");
        ControllinoModbusMaster.query(ModbusQuery[0]);
        Serial.println("--- ControllinoModbusMaster sender-end ---");
        myState++;
        break;
      case 2:
        ControllinoModbusMaster.poll();  // check incoming messages
        if (ControllinoModbusMaster.getState() == COM_IDLE) {
          // response from the slave was received
          myState = 0;
          Serial.println("---------- READ RESPONSE RECEIVED ----");

          for (int i = 0; i < 21; i++) {
            mb.Ireg(10 - 1 + i, ModbusSlaveRegisters_MPPT_Solar[i]);
          }


          /*    3xx        
10            // 0x100 Battery capacity 0...100  (100)
11            // 0x101 Battery Voltage *0.1 or /10  (140)
12            // 0x102 Charg in Ток зарядки *0.1 or /10 (398)
13            // 0x103 Температура Мл и Ст байты   (7449)
         // Нагрузка   
14            // 0x104 Напряжение нагрузки *0.1 /10 (140)
15            // 0x105 Ток нагрузки *0.01  /100   (356)
16            // 0x106 Мощность нагрузки *0.1 /10  (49)
       // Солнечные панели 
17            // 0x107 Солнечная панель *0.1 Напряжение (160)
18            // 0x108 Солнечная панель *0.01 ТОК (362)
19            // 0x109 Мощность с панели Вт (58)
20            // 0x10A 0/1 - Включена нагрузка (0)
21            // 0x10B Минимальное напряжение аккумулятора за день (138)
22            // 0x10C Максимальное напряжение аккумулятора за день (144)
23            // 0x10D Максимальный ток зарядки за день (449)
24            // 0x10E Максимальный ток Разрядки за день (364)
25            // 0x10F Максимальная мощность зарядки за день (62)
26            // 0x110 Максимальная мощность Разрядки за день (51)
27            // 0x111 Зарядка Ампер*Часы за день (11)
28            // 0x112 Разрядка Ампер*Часы за день (8)
39            // 0x113 Мощность выработанная за день (155)
30            // 0x114 Мощность потребляемая за день (0)
*/

          for (int i = 0; i < 21; i++) {
            Serial.print(i);
            Serial.print(":=");
            //Serial.println(ModbusSlaveRegisters[i], HEX);
            Serial.println(ModbusSlaveRegisters_MPPT_Solar[i], DEC);
          }
          Serial.println("");
        }
        break;
    }
  }
}

String UDP_command(String &data) {
  String Result = "";

  //Result = Result + data;

  if (data == "Reboot") {
    softwareReset();
  }

  if (data == "Test") {
    Result = "Связь ОК";
  }

  if (data == "LK=1") {
    mb.Hreg(5 - 1, 255);
    Result = "Кухня ВКЛ";
  }

  if (data == "LK=0") {
    mb.Hreg(5 - 1, 0);
    Result = "Кухня ВЫКЛ";
  }

  if (data == "LE=1") {
    mb.Hreg(4 - 1, 255);
    Result = "Выход ВКЛ";
  }
  if (data == "LE=0") {
    mb.Hreg(4 - 1, 0);
    Result = "Выход ВЫКЛ";
  }

  if (data == "220=0") {
    digitalWrite(CONTROLLINO_R15, 0); //Выключаем 220 Вольт
    Result = "220 ВЫКЛ";
  }

  if (data == "220=1") {
    digitalWrite(CONTROLLINO_R15, 1); //Включаем 220 Вольт
    Result = "220 ВКЛ";
  }


  return Result;
}


void UDP_loop() {

  int size = udp.parsePacket();  // считываем размер принятого пакета
  if (size) {
    // есть пакет Udp, выводим информацию о пакете
    Serial.print("Received packet from ");
    IPAddress ipFrom = udp.remoteIP();
    Serial.println(ipFrom);
    Serial.print("Size ");
    Serial.print(size);
    Serial.print(", port ");
    Serial.println(udp.remotePort());

    // чтение Udp-пакета и передача в последовательный порт
    udp.read(receivingBuffer, size);
    receivingBuffer[size] = 0;
    Serial.println("-------------------");
    Serial.println(receivingBuffer);
    Serial.println();

    String reciving = String(receivingBuffer);
    String data2 = UDP_command(reciving);
    Serial.println(data2);

    //uint8_t dataArray[dataString.length()];
    data2.toCharArray(receivingBuffer, data2.length() + 1);
    // ответ клиенту
    udp.beginPacket(udp.remoteIP(), 8080);  //udp.beginPacket(udp.remoteIP(), udp.remotePort());
    //udp.write(receivingBuffer,data2.length()+1);
    udp.write(receivingBuffer);
    udp.endPacket();
  }
}

void tasks() {
  timer_1(100);  //D1 моргалка...
  timer_rgb(200);
  timer_worker(1000);
  timer_Coil_RELE(100);
  timer_PWM(100);  //Управляемые PWM порты с экрана
  timer_input_analog(100);
  timer_sendMQTT(10000);        //передача данных mqtt
  timer_sendMQTT_SOLAR(60000);  //передача данных mqtt
  timer_Modbus_COM3(5000);
}

/*
  udp send!
  udp.beginPacket(remoteIp, remotePort);
  serializeJson(doc, udp);
  udp.println();
  udp.endPacket();
*/
void loop() {
  clientMQTT.loop();

  mb.task();  // Server Modbus TCP queries

  if (!clientMQTT.connected()) {
    connect_MQTT();
  }

  //ArduinoOTA.poll();

  // digitalWrite(CONTROLLINO_R0, mb.Coil(1-1)); // 3x.. #1
  // digitalWrite(CONTROLLINO_R5, mb.Coil(5-1)); // 3x.. #5
  //mb.Hreg(1 - 1, analogRead(CONTROLLINO_A0));
  //analogWrite(CONTROLLINO_D4, mb.Hreg(5 - 1));

  tasks();  //Все прочие задачи в loop
  delay(1);

  UDP_loop();
}

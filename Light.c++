#include <WiFi.h>
#include <WiFiManager.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>  // เพิ่มส่วนขยาย WiFiClientSecure

const char mqttClientID[] = "Light-Test";
const char mqttServer[] = "";
const int mqttPort = 8883;
const char mqttUsername[] = "";
const char mqttPassword[] = "";
const int led1Pin = 2;
const int swPin = 15;

bool LightAction = false;
bool swAction = false;
bool setupCompleted = false;

bool ledAction = false;
String ON = "on";    // ประกาศตัวแปร ON ไว้นอก setup()
String OFF = "off";  // ประกาศตัวแปร ON ไว้นอก setup()

DynamicJsonDocument doc(1024);
DynamicJsonDocument docIncome(1024);
WiFiClientSecure net;
MQTTClient client(1024);
unsigned long lastMillis = 0;
String jsonString;

String topicStatus = "/status/Light/" + String(mqttClientID);
String topicLed1 = String(mqttClientID);

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP32AP");  // "ESP32AP" เป็นชื่อของอุปกรณ์ WiFi ที่จะใช้ในโหมด Access Point
  Serial.println("WiFi connected!");   // แสดงข้อความเมื่อเชื่อมต่อ WiFi สำเร็จ
}

void checkingWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nconnecting wifi ...");
    delay(1000);
  }
  net.setInsecure();
}

void subscribeMessage(String topic) {
  client.subscribe(topic);
}

void unSubscribeMessage(String &topic) {
  client.unsubscribe(topic);
}

void connectMqttServer() {
  Serial.print("\nconnecting mqtt server...");
  while (!client.connect(mqttClientID, mqttUsername, mqttPassword)) {
    Serial.print(".");
    delay(1000);
  }
  client.setKeepAlive(10000);
  client.setCleanSession(true);
  client.setTimeout(10000);
  Serial.println("\nmqtt connected!");
}
void action(String topic, String message) {
  DeserializationError error = deserializeJson(docIncome, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  String status = docIncome["data"];
  if (status == ON) {
    LightAction = true;
    checkStatusAndAction(led1Pin, LightAction);
    doc["Light"] = status;
  } else if (status == OFF) {
    LightAction = false;
    checkStatusAndAction(led1Pin, LightAction);
    doc["Light"] = status;
  }
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  action(topic, payload);

  serializeJson(doc, jsonString);
  publishMessage(topicStatus, jsonString, false, 0);
}

void publishMessage(String topic, String message, bool retain, int qos) {
  client.publish(topic, message, retain, qos);
  Serial.println("publish topic :" + topic);
}

void checkStatusAndAction(int pin, bool status) {
  status = !status;  // สลับค่า status ด้วย NOT operator (!)
  Serial.println(String(status) + "checkStatusAndAction");
  digitalWrite(pin, status);
  Serial.println("Digital Pin: " + String(pin) + (status ? " turned ON" : " turned OFF"));
}

void setupMqttServer() {
  client.begin(mqttServer, mqttPort, net);
}

void setupOutput() {
  pinMode(led1Pin, OUTPUT);
}
void setupinput() {
  pinMode(swPin, INPUT);
}
void connect() {
  checkingWifi();
  connectMqttServer();
  subscribeMessage(topicLed1);
}

void setup() {
  Serial.begin(115200);
  setupOutput();
  setupinput();

  connectToWiFi();
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");

  doc["Light"] = "off";
  doc["Client"] = String(mqttClientID);

  setupMqttServer();
  Serial.println(WiFi.localIP());
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  delay(100);
  client.loop();  // ตรวจสอบการเชื่อมต่อ MQTT และรับข้อความ

  if (WiFi.status() != WL_CONNECTED) {
    readinput();  // ทำงานเมื่อไม่ได้เชื่อมต่อ WiFi
  }

  // publish a message roughly
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    serializeJson(doc, jsonString);
    publishMessage(topicStatus, jsonString, false, 0);
  }
  readinput();
}
void readinput() {
  if (WiFi.status() == WL_CONNECTED) {
    if (digitalRead(swPin) == HIGH) {
      swAction = !swAction;         // INVERT VALUE
      Serial.println("sw to led");  // DEBUG

      checkStatusAndAction(led1Pin, swAction);   // DIGITAL WRITE
      doc["Light"] = (swAction) ? "on" : "off";  // SET PUBLISH VALUE
    }
  } else if (WiFi.status() != WL_CONNECTED) {
      if (digitalRead(swPin) == HIGH) {
        swAction = !swAction;                     // INVERT VALUE
        Serial.println("sw to led");              // DEBUG
        checkStatusAndAction(led1Pin, swAction);  // DIGITAL WRITE
      }
    }
}

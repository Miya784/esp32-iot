#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiManager.h>  // เรียกใช้ไลบรารี WiFiManager



const char mqttClientID[] = "Gate-Test";
const char mqttServer[] = "emqx.burhan.cloud";
const int mqttPort = 8883;
const char mqttUsername[] = "han";
const char mqttPassword[] = "han";

const int motorPin1 = 26;  // ขา 26 ของ ESP32
const int motorPin2 = 27;  // ขา 27 ของ ESP32

const int sensorPin1 = 12;  // ขา 12 ของ ESP32
const int sensorPin2 = 13;  // ขา 13 ของ ESP32
const int sensorPin3 = 14;  // ขา 14 ของ ESP32

const int t0 = 32;
const int t1 = 33;

String Open = "Open";
String Close = "Close";

bool OpenClese = false;

DynamicJsonDocument doc(1024);
DynamicJsonDocument docIncome(1024);
WiFiClientSecure net;
MQTTClient client(1024);
unsigned long lastMillis = 0;
String jsonString;

String topicStatus = "/status/Gate/" + String(mqttClientID);
String topicGate = String(mqttClientID);

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

void action(String topic, String message) {
  DeserializationError error = deserializeJson(docIncome, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }
  String status = docIncome["data"];

  if (status == Open) {
    checkStatusAndAction(status);
    doc["Gate"] = status;
  } else if (status == Close) {
    checkStatusAndAction(status);
    doc["Gate"] = status;
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

void checkStatusAndAction(String status) {
  if (status = Open || t0 == HIGH) {
    while (sensorPin1 == HIGH) {
      forward();
      delay(100);
      if (digitalRead(t0) == HIGH || digitalRead(sensorPin1) == LOW) {
        break;  // หากเงื่อนไขไม่ถูกต้องให้ออกจากลูป
      }
    }
  } else if (status = Close || t1 == HIGH) {
    while (sensorPin2 == HIGH) {
      backward();
      Serial.println("Moving backward...");
      delay(100);
      bool sensor3 = digitalRead(sensorPin3);
      while (sensor3 == LOW) {
        stopMotor();
        Serial.println("Stopped");
        if (digitalRead(sensorPin3) == HIGH) {
          delay(100);
          break;  // หากเงื่อนไขไม่ถูกต้องให้ออกจากลูป
        }
      }
      if (digitalRead(t1) == HIGH || digitalRead(sensorPin2) == LOW) {
        delay(100);
        break;  // หากเงื่อนไขไม่ถูกต้องให้ออกจากลูป
      }
    }
  }
}

void setupMqttServer() {
  client.begin(mqttServer, mqttPort, net);
}


void setupOutput() {
  // Set the relay pins as outputs
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
}

void setupInput() {
  // Set the relay pins as outputs
  pinMode(t0, INPUT);
  pinMode(t1, INPUT);
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);
}

void connect() {
  checkingWifi();

  connectMqttServer();
  subscribeMessage(topicGate);
}

void setup() {
  Serial.begin(115200);
  setupOutput();
  setupInput();

  connectToWiFi();

  doc["Gate"] = "Close";
  doc["Client"] = String(mqttClientID);

  setupMqttServer();
  client.onMessage(messageReceived);
  connect();
}

void loop() {
  delay(2000);

  if (!client.connected()) {
    connect();
  }
  client.loop();

  // publish a message roughly
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();

    serializeJson(doc, jsonString);
    publishMessage(topicStatus, jsonString, false, 0);
  }

  readinput();
}

void readinput() {
  if (digitalRead(t1) == HIGH) {
    String status = "Close";
    checkStatusAndAction(status);
    doc["Gate"] = "Close";
    Serial.println("Close");  // DEBUG
  } else if (digitalRead(t0) == HIGH) {
    String status = "Open";
    checkStatusAndAction(status);
    Serial.println("Open");  // DEBUG
    doc["Gate"] = "Open";
  }
}

void forward() {
  digitalWrite(motorPin1, HIGH);  // ส่งสัญญาณ PWM เพื่อควบคุมความเร็ว
  digitalWrite(motorPin2, LOW);
}

// ฟังก์ชันสำหรับหมุนมอเตอร์ย้อนกลับ
void backward() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, HIGH);  // ส่งสัญญาณ PWM เพื่อควบคุมความเร็ว
}

// ฟังก์ชันสำหรับหยุดมอเตอร์
void stopMotor() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
}
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiManager.h> 



const char mqttClientID[] = "Fan-Test";
const char mqttServer[] = "";
const int mqttPort = 8883;
const char mqttUsername[] = "";
const char mqttPassword[] = "";
bool ONOFF = false ;
const int led1Pin = 15;
const int led2Pin = 2;
const int led3Pin = 4;

const int t0 = 13;
const int t1 = 12;
const int t2 = 14;
const int t3 = 27;


String ON1 = "ON1";
String ON2 = "ON2";
String ON3 = "ON3";
String Off = "Off";


DynamicJsonDocument doc(1024);
DynamicJsonDocument docIncome(1024);
WiFiClientSecure net;
MQTTClient client(1024);
unsigned long lastMillis = 0;
String jsonString;

String topicStatus = "/status/Fan/" + String(mqttClientID);
String topicfan = String(mqttClientID);

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
  bool FanAction1 = false;
  bool FanAction2 = false;
  bool FanAction3 = false;

  if (status == ON1) {
    FanAction1 = true;
    FanAction2 = false;
    FanAction3 = false;
    checkStatusAndAction(led1Pin, FanAction1);
    doc["Fan"] = status;
  } else if (status == ON2) {
    FanAction1 = false;
    FanAction2 = true;
    FanAction3 = false;
    checkStatusAndAction(led2Pin, FanAction2);
    doc["Fan"] = status;
  } else if (status == ON3) {
    FanAction1 = false;
    FanAction2 = false;
    FanAction3 = true;
    checkStatusAndAction(led3Pin, FanAction3);
    doc["Fan"] = status;
  } else if (status == Off) {
    FanAction1 = false;
    FanAction2 = false;
    FanAction3 = false;
    checkStatusAndAction(led1Pin, FanAction1);
    checkStatusAndAction(led2Pin, FanAction2);
    checkStatusAndAction(led3Pin, FanAction3);
    doc["Fan"] = status;
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
  if (status) {
    digitalOn(pin);
  } else {
    digitalOff(pin);
  }
}

// Flexible function to control any relay
void digitalOn(int pin) {
  digitalWrite(pin, HIGH);
  // Serial.println("Digital Pin: " + String(pin) + " turned ON");
}

void digitalOff(int pin) {
  digitalWrite(pin, LOW);
  // Serial.println("Digital Pin: " + String(pin) + " turned OFF");
}


void setupMqttServer() {
  client.begin(mqttServer, mqttPort, net);
}


void setupOutput() {
  // Set the relay pins as outputs
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
}

void setupInput() {
  // Set the relay pins as outputs
  pinMode(t0, INPUT);
  pinMode(t1, INPUT);
  pinMode(t2, INPUT);
  pinMode(t3, INPUT);
}

void connect() {
  checkingWifi();

  connectMqttServer();
  subscribeMessage(topicfan);
  // subscribeMessage(topicLed2);
}

void setup() {
  Serial.begin(115200);
  setupOutput();
  setupInput();

  connectToWiFi();

  doc["Fan"] = "off";
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
  if (digitalRead(t0) == HIGH) {
    ONOFF = ONOFF ? false : true;
    if (!ONOFF) {
      checkStatusAndAction(led1Pin, false);
      checkStatusAndAction(led2Pin, false);
      checkStatusAndAction(led3Pin, false);
      doc["Fan"] = "Off";
    }
    Serial.println("sw to touch On/Off");  // DEBUG
  } else if (digitalRead(t1) == HIGH) {
    checkStatusAndAction(led1Pin, true);
    checkStatusAndAction(led2Pin, false);
    checkStatusAndAction(led3Pin, false);
    Serial.println("sw to touch On1");  // DEBUG
    doc["Fan"] = "ON1";
  } else if (digitalRead(t2) == HIGH) {
    checkStatusAndAction(led1Pin, false);
    checkStatusAndAction(led2Pin, true);
    checkStatusAndAction(led3Pin, false);
    Serial.println("sw to touch On2");  // DEBUG
    doc["Fan"] = "ON2";
  } else if (digitalRead(t3) == HIGH) {
    checkStatusAndAction(led1Pin, false);
    checkStatusAndAction(led2Pin, false);
    checkStatusAndAction(led3Pin, true);
    doc["Fan"] = "ON3";
    Serial.println("sw to touch On3");  // DEBUG
  }
}

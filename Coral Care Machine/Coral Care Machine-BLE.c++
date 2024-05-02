#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ThreeWire.h>  // Time
#include <RtcDS1302.h>  // Time
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC1_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // send data auto follow time
#define CHARACTERISTIC2_UUID "beb5483e-36e1-4688-b7f5-ea07361b26ad"  // receive data mode 2
#define CHARACTERISTIC3_UUID "aeb5483e-36e1-4688-b7f5-ea07361b26ac"  // receive data mode 2
#define CHARACTERISTIC4_UUID "72c44656-5537-4a94-aeef-04111ec01238"  // setting time

#define LedRoyalBlue 15
#define LedGreen 2
#define LedDeepred 4
#define LedViolet 16
#define LedUV 17
#define LedBlue 5
#define LedCoolwhite10 18

// Define the PWM channels
#define CHANNEL_RoyalBlue 0
#define CHANNEL_Green 1
#define CHANNEL_Deepred 2
#define CHANNEL_Violet 3
#define CHANNEL_UV 4
#define CHANNEL_Blue 5
#define CHANNEL_Coolwhite10 6

#define switchMode 12

#define inputCLK 27
#define inputDT 26
#define inputSettingTime 25
#define DIRECTION_CW 0    // clockwise direction
#define DIRECTION_CCW 1   // counter-clockwise direction
#define DIRECTION_CW1 0   // clockwise direction
#define DIRECTION_CCW1 1  // counter-clockwise direction

#define ONE_WIRE_BUS 13
#define TDS_SENSOR_PIN 34

#define TWO_PI 6.28318530718  // 2 times Pi
#define SPEED_FACTOR 0.001    // A factor to adjust the speed of the effect

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

const int CLK_PIN = 23;
const int DAT_PIN = 32;
const int RST_PIN = 33;


const int freq = 1000;    // Frequency = 30KHz
const int resolution = 8;  // Using 8 bits for resolution

unsigned long startTime;
int counter = 0;
int currentStateCLK;
int previousStateCLK;
int direction = DIRECTION_CW;
int direction1 = DIRECTION_CW1;



ThreeWire myWire(DAT_PIN, CLK_PIN, RST_PIN);  // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

struct PWMValues {
  int RoyalBlue;
  int Green;
  int Deepred;
  int Violet;
  int UV;
  int Blue;
  int Coolwhite10;
};

struct SettingTime {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int sec;
};

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}
float readTDS() {
  int tdsValue = analogRead(TDS_SENSOR_PIN);
  float voltage = tdsValue * (3.3 / 4095.0);                                                            // Convert the analog reading (which goes from 0 - 4095) to a voltage (0-3.3V)
  return (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * 0.5;  // Conversion formula from voltage to TDS
}

PWMValues getPWMByHour() {
  RtcDateTime now = Rtc.GetDateTime();
  uint8_t hour = now.Hour();
  uint8_t minute = now.Minute();
  PWMValues values;
  if (hour >= 5 && hour < 7) {
    int elapsedMinutes = (hour - 5) * 60 + minute;
    float proportion = elapsedMinutes / 120.0;  // 120 minutes = 2 hours
    values.RoyalBlue = 1 + proportion * (255 - 1);
    values.Green = 1 + proportion * (90 - 1);
    values.Deepred = 1 + proportion * (10 - 1);
    values.Violet = 60 + proportion * (153 - 60);
    values.UV = 70 + proportion * (127 - 70);
    values.Blue = 70 + proportion * (255 - 70);
    values.Coolwhite10 = 1 + proportion * (255 - 127);
  } else if (hour >= 17 && hour < 19) {
    int elapsedMinutes = (hour - 17) * 60 + minute;
    float proportion = elapsedMinutes / 120.0;  // 120 minutes = 2 hours
    values.RoyalBlue = 255 - proportion * (255 - 1);
    values.Green = 90 - proportion * (90 - 1);
    values.Deepred = 10 - proportion * (10 - 1);
    values.Violet = 153 - proportion * (153 - 60);
    values.UV = 127 - proportion * (127 - 70);
    values.Blue = 255 - proportion * (255 - 70);
    values.Coolwhite10 = 255 - proportion * (255 - 1);
  } else if (hour >= 19 && hour < 5) {
    values.RoyalBlue = 0;
    values.Green = 0;
    values.Deepred = 0;
    values.Violet = 60;
    values.UV = 70;
    values.Blue = 70;
    values.Coolwhite10 = 0;
  } else {
    values.RoyalBlue = 255;
    values.Green = 90;
    values.Deepred = 10;
    values.Violet = 153;
    values.UV = 127;
    values.Blue = 255;
    values.Coolwhite10 = 255;
  }
  return values;
}

PWMValues updatePwmValues() {
  PWMValues values;
  values.RoyalBlue;
  values.Green;
    values.Deepred;
      values.Violet;
        values.UV;
          values.Blue;
            values.Coolwhite10; 
            return values;
}

PWMValues northernLightsEffect() {
  float phase = (millis() - startTime) * SPEED_FACTOR;
  PWMValues values;
  values.RoyalBlue = 128 + 127 * sin(phase);
  values.Green = 128 + 127 * sin(phase + TWO_PI / 8);
  values.Deepred = 128 + 127 * sin(phase + TWO_PI / 4);
  values.Violet = 128 + 127 * sin(phase + 3 * TWO_PI / 8);
  values.UV = 128 + 127 * sin(phase + 5 * TWO_PI / 8);
  values.Blue = 128 + 127 * sin(phase + 3 * TWO_PI / 4);
  values.Coolwhite10 = 128 + 127 * sin(phase + 7 * TWO_PI / 8);

  return values;
}

PWMValues dayTime() {
  PWMValues values;
  values.RoyalBlue = 255;
  values.Green = 90;
  values.Deepred = 10;
  values.Violet = 153;
  values.UV = 127;
  values.Blue = 255;
  values.Coolwhite10 = 255;

  return values;
}

PWMValues nighttime() {
  PWMValues values;
  values.RoyalBlue = 0;
  values.Green = 0;
  values.Deepred = 0;
  values.Violet = 60;
  values.UV = 70;
  values.Blue = 70;
  values.Coolwhite10 = 0;

  return values;
}

PWMValues readPWMValues() {
  PWMValues values;
  values.RoyalBlue = ledcRead(CHANNEL_RoyalBlue);
  values.Green = ledcRead(CHANNEL_Green);
  values.Deepred = ledcRead(CHANNEL_Deepred);
  values.Violet = ledcRead(CHANNEL_Violet);
  values.UV = ledcRead(CHANNEL_UV);
  values.Blue = ledcRead(CHANNEL_Blue);
  values.Coolwhite10 = ledcRead(CHANNEL_Coolwhite10);
  return values;
}
// Create OLED object. For a 128x64 pixel display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic1 = NULL;  // for sending data
BLECharacteristic* pCharacteristic2 = NULL;  // for sending data
BLECharacteristic* pCharacteristic3 = NULL;  // for sending data
BLECharacteristic* pCharacteristic4 = NULL;  // for sending data


bool deviceConnected = false;
bool prevDeviceConnected = false;


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};
// send data auto follow time
class MyCallback1 : public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) {
    PWMValues values = readPWMValues();
    char buffer[50];
    snprintf(buffer, sizeof(buffer), " %d %d %d %d %d %d %d ", values.RoyalBlue, values.Green, values.Deepred, values.Violet, values.UV, values.Blue, values.Coolwhite10);
    pCharacteristic->setValue(buffer);
  }
};

// receive data mode 2
String currentMode = "";  // Global variable to store the current mode
class MyCallback2 : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string stdStr = pCharacteristic->getValue();
    const char* cStr = stdStr.c_str();
    String valueString = String(cStr);
    if (valueString.length() > 0) {
      const char* value = valueString.c_str();  // Corrected variable name
      if (strcmp(value, "mode1") == 0) {
        currentMode = "mode1";
        Serial.println(currentMode);
      } else if (strcmp(value, "mode2") == 0) {  // Use strcmp here as well
        currentMode = "mode2";
        Serial.println(currentMode);
      } else if (strcmp(value, "mode3") == 0) {
        currentMode = "mode3";
        Serial.println(currentMode);
      } else if (strcmp(value, "mode4") == 0) {
        currentMode = "mode4";
        Serial.println(currentMode);
      } else if (strcmp(value, "mode5") == 0) {
        currentMode = "mode5";
        Serial.println(currentMode);
      }
    }
  }
};
// receive data mode 2
PWMValues pwmValues;
class MyCallback3 : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.write(reinterpret_cast<const uint8_t*>(value.c_str()), value.length());
    Serial.println();  // Just to add a newline after the binary data
    if (value.length() >= sizeof(double) * 7) {
      double percentages[7];
      memcpy(percentages, value.c_str(), sizeof(double) * 7);

      for (int i = 0; i < 7; i++) {
        Serial.println(percentages[i]);
      }
      pwmValues.RoyalBlue = static_cast<int>(percentages[0]);
      pwmValues.Green = static_cast<int>(percentages[1]);
      pwmValues.Deepred = static_cast<int>(percentages[2]);
      pwmValues.Violet = static_cast<int>(percentages[3]);
      pwmValues.UV = static_cast<int>(percentages[4]);
      pwmValues.Blue = static_cast<int>(percentages[5]);
      pwmValues.Coolwhite10 = static_cast<int>(percentages[6]);
    }
  }
};

SettingTime SettingTime;
class MyCallback4 : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.println(value.c_str());
    // Check for a minimum expected length (e.g., "2023 10 01 12 30 59" = 19 chars)
    if (value.length() == 19) {
      sscanf(value.c_str(), "%04d %02d %02d %02d %02d %02d", &SettingTime.year, &SettingTime.month, &SettingTime.day, &SettingTime.hour, &SettingTime.minute, &SettingTime.sec);
      Serial.print(SettingTime.year);
      Serial.print("  ");
      Serial.print(SettingTime.month);
      Serial.print("  ");
      Serial.print(SettingTime.day);
      Serial.print("  ");
      Serial.print(SettingTime.hour);
      Serial.print("  ");
      Serial.print(SettingTime.minute);
      Serial.print("  ");
      Serial.print(SettingTime.sec);
      Serial.println("  ");
      setDS1302Time(SettingTime.year, SettingTime.month, SettingTime.day, SettingTime.hour, SettingTime.minute, SettingTime.sec);
    } else {
      Serial.println("Received unexpected data size or format.");
    }
  }
};


void setup() {
  Serial.begin(115200);
  sensors.begin();  // Start up the library for DS18B20
  // Create the BLE Device
  BLEDevice::init("Coral of Ocean");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);
  // Create BLE Characteristic for sending data
  pCharacteristic1 = pService->createCharacteristic(
    CHARACTERISTIC1_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic1->setCallbacks(new MyCallback1());

  // Create BLE Characteristic for receive data
  pCharacteristic2 = pService->createCharacteristic(
    CHARACTERISTIC2_UUID,
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic2->setCallbacks(new MyCallback2());
  // Create BLE Characteristic for receive data
  pCharacteristic3 = pService->createCharacteristic(
    CHARACTERISTIC3_UUID,
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic3->setCallbacks(new MyCallback3());
  // Create BLE Characteristic for receive data
  pCharacteristic4 = pService->createCharacteristic(
    CHARACTERISTIC4_UUID,
    BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic4->setCallbacks(new MyCallback4());
  // Start the service
  pService->start();
  // Set up PWM channels
  ledcSetup(CHANNEL_RoyalBlue, freq, resolution);
  ledcSetup(CHANNEL_Green, freq, resolution);
  ledcSetup(CHANNEL_Deepred, freq, resolution);
  ledcSetup(CHANNEL_Violet, freq, resolution);
  ledcSetup(CHANNEL_UV, freq, resolution);
  ledcSetup(CHANNEL_Blue, freq, resolution);
  ledcSetup(CHANNEL_Coolwhite10, freq, resolution);

  // Attach pins to channels
  ledcAttachPin(LedRoyalBlue, CHANNEL_RoyalBlue);
  ledcAttachPin(LedGreen, CHANNEL_Green);
  ledcAttachPin(LedDeepred, CHANNEL_Deepred);
  ledcAttachPin(LedViolet, CHANNEL_Violet);
  ledcAttachPin(LedUV, CHANNEL_UV);
  ledcAttachPin(LedBlue, CHANNEL_Blue);
  ledcAttachPin(LedCoolwhite10, CHANNEL_Coolwhite10);
  // Set encoder pins as inputs
  pinMode(switchMode, INPUT);

  pinMode(inputCLK, INPUT);
  pinMode(inputDT, INPUT);
  pinMode(inputSettingTime, INPUT);

  // setDS1302Time(2023, 7, 6, 18, 55, 0); // Set time to August 25th, 2023, at 5:00:00
  previousStateCLK = digitalRead(inputCLK);


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.display();
  display.println("Hello, world!");
  delay(2000);
  display.clearDisplay();
}
void loop() {
  if (digitalRead(inputSettingTime) == HIGH) {
    if (counter >= 60) {
      counter = 0;
    }
    updateEncoder();
  } else {
    // showCurrentTime();
    if (deviceConnected) {
      pCharacteristic1->setCallbacks(new MyCallback1());
      if (currentMode == "mode1") {
        PWMValues values = getPWMByHour();
        led(values);
        showCurrentTime();
      } else if (currentMode == "mode2") {
        PWMValues values = northernLightsEffect();
        led(values);
      } else if (currentMode == "mode3") {
        PWMValues values = dayTime();
        led(values);
      } else if (currentMode == "mode4") {
        PWMValues values = nighttime();
        led(values);
      } else if (currentMode == "mode5") {
        led(pwmValues);
      }

    } else if ((!deviceConnected)) {
      Serial.println("disconnect");
      BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
      pAdvertising->addServiceUUID(SERVICE_UUID);
      pAdvertising->start();
      Serial.println("BLE server started");
      // int buttonState = digitalRead(switchMode);
      if (digitalRead(switchMode) == HIGH) {
        getPwmRotary();
        Serial.println("switchMode high");
      } else {
        showCurrentTime();
        PWMValues values = getPWMByHour();
        led(values);
        Serial.println("switchMode LOW");
      }
    }
    float tds = readTDS();
    float temperatureC = readTemperature();
    display.clearDisplay();  // Clear previous contents
    display.setCursor(0, 0);
    display.print(F("TDS :"));
    display.println(tds);
    display.print(F("Temp:"));
    display.println(temperatureC);
    display.display();  // Actually display all of the above
  }

  // delay(100);
}
void getPwmRotary() {
  // Read the current state of inputCLK
      PWMValues values;
    values.RoyalBlue = counter;
    values.Green = counter;
    values.Deepred = counter;
    values.Violet = counter;
    values.UV = counter;
    values.Blue = counter;
    values.Coolwhite10 = counter;
  currentStateCLK = digitalRead(inputCLK);

  if (currentStateCLK != previousStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(inputDT) == HIGH) {
      counter = max(0, counter - 10);
      direction1 = DIRECTION_CCW1;
    } else {
      counter = min(255, counter + 10);
      direction1 = DIRECTION_CW1;
    }
    PWMValues values;
    led(values);
  }
  previousStateCLK = currentStateCLK;
}
void updateEncoder() {
  SettingTime.hour;
  SettingTime.minute = 0;

  currentStateCLK = digitalRead(inputCLK);

  if (currentStateCLK != previousStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(inputDT) == HIGH) {
      counter = max(-1, counter - 1);
      direction = DIRECTION_CCW;
    } else {
      counter = min(60, counter + 1);
      direction = DIRECTION_CW;
    }
    if (counter == 60) {
      counter = 0;
      SettingTime.hour = (SettingTime.hour + 1) % 24;
    }
    if (counter == -1) {
      counter = 59;
      SettingTime.hour = (SettingTime.hour - 1 + 24) % 24;
    }
    SettingTime.minute = counter;
    Serial.print(" hour : ");
    Serial.print(SettingTime.hour);
    Serial.print("  minute : ");
    Serial.println(SettingTime.minute);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(F("Time:"));
    display.print(SettingTime.hour);
    display.print(F(":"));
    display.println(SettingTime.minute);
    display.display();
    setDS1302Time(SettingTime.year, SettingTime.month, SettingTime.day, SettingTime.hour, SettingTime.minute, SettingTime.sec);
    PWMValues values = getPWMByHour();
    led(values);
  }

  previousStateCLK = currentStateCLK;
}

void showCurrentTime() {
  RtcDateTime now = Rtc.GetDateTime();
  char str[20];
  snprintf(str, sizeof(str), "%04u-%02u-%02u %02u:%02u:%02u", now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), now.Second());
  Serial.println(str);
}

void setDS1302Time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  // Ensure the RTC is initialized
  if (!Rtc.IsDateTimeValid()) {
    Rtc.Begin();
  }

  RtcDateTime dt(year, month, day, hour, minute, second);
  Rtc.SetDateTime(dt);

  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }
}

void led(const PWMValues& values) {
  ledcWrite(CHANNEL_RoyalBlue, values.RoyalBlue);
  ledcWrite(CHANNEL_Green, values.Green);
  ledcWrite(CHANNEL_Deepred, values.Deepred);
  ledcWrite(CHANNEL_Violet, values.Violet);
  ledcWrite(CHANNEL_UV, values.UV);
  ledcWrite(CHANNEL_Blue, values.Blue);
  ledcWrite(CHANNEL_Coolwhite10, values.Coolwhite10);

  Serial.print(values.RoyalBlue);
  Serial.print("  ");
  Serial.print(values.Green);
  Serial.print("  ");
  Serial.print(values.Deepred);
  Serial.print("  ");
  Serial.print(values.Violet);
  Serial.print("  ");
  Serial.print(values.UV);
  Serial.print("  ");
  Serial.print(values.Blue);
  Serial.print("  ");
  Serial.println(values.Coolwhite10);
}
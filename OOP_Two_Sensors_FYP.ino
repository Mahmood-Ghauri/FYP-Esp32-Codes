// Libraries for SD Card Module
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// SPI Port PINS for SD Card
#define REASSIGN_PINS
const int sck = 14, miso = 12, mosi = 13, cs = 27;

// Libraries for RTC DS3231
#include <Wire.h>
#include <RTClib.h>
RTC_DS3231 rtc;

// WiFi & Firebase Setup
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#define WIFI_SSID "MAUG"
#define WIFI_PASSWORD "d24e95dc"
#define API_KEY "AIzaSyDBcxIo-G9o1Q_sEJx2Ly1NYT52YibvUFk"
#define DATABASE_URL "https://fyp-aiot-smart-meter-dataset-default-rtdb.asia-southeast1.firebasedatabase.app/"
FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

// Data Structure for Appliances
struct Appliance {
    float voltage = 1, current = 1, app_power = 1, react_power = 1, act_power = 1, power_fact = 1;
    bool cycle = false;
};

Appliance iron, vacuum, fridge, oven, load;
String I_cycle = "1", V_cycle = "1", F_cycle = "1";

String simp_time = "NULL", date = "NULL", unixtime = "NULL";
int i = 1;
unsigned long lastTime = 0, timerDelay = 3000;

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600, SERIAL_8N1, 4, 2);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Wire.begin();
    if (!rtc.begin()) Serial.println("Couldn't find RTC");

    SPI.begin(sck, miso, mosi, cs);
    if (!SD.begin(cs)) Serial.println("Card Mount Failed");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(1000);
    Serial.println("Connected to Wi-Fi");

    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = "ghauribrother12@gmail.com";
    auth.user.password = "MAhm12##";
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

void loop() {
    readSerialData(Serial, oven, "Oven");
    readSerialData(Serial1, load, "Load");
    readSerialData(Serial2, iron, "Iron");
    readSerialData(Serial2, vacuum, "Vacuum");
    readSerialData(Serial2, fridge, "Fridge");

    updateTime();
    saveToSD();
    if ((millis() - lastTime) > timerDelay) sendDataToFirebase();
    delay(3000);
}

void readSerialData(HardwareSerial &serialPort, Appliance &appliance, const char* name) {
    if (serialPort.available() > 0) {
        String data = serialPort.readStringUntil('\n');
        extractValue(data, "Voltage:", "V", appliance.voltage);
        extractValue(data, "Current:", "A", appliance.current);
        extractValue(data, "Apparent Power:", "VA", appliance.app_power);
        extractValue(data, "Power Factor:", "\n", appliance.power_fact);
        appliance.act_power = appliance.app_power * appliance.power_fact;
        appliance.react_power = sqrt(pow(appliance.app_power, 2) - pow(appliance.act_power, 2));
        appliance.cycle = appliance.current >= 0.5;
    }
}

void extractValue(const String &data, const String &key, const String &unit, float &value) {
    int index = data.indexOf(key);
    if (index != -1) value = data.substring(index + key.length(), data.indexOf(unit, index)).toFloat();
}

void updateTime() {
    DateTime now = rtc.now();
    unixtime = String(now.unixtime());
    date = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());
    simp_time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
}

void saveToSD() {
    File file = SD.open("/hello.txt", FILE_APPEND);
    if (!file) return;
    file.printf("%s,%s,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%d\n",
                unixtime.c_str(), date.c_str(), simp_time.c_str(),
                load.voltage, load.current, iron.current, vacuum.current, fridge.current, oven.current,
                load.act_power, iron.act_power, vacuum.act_power, fridge.act_power, oven.act_power,
                load.react_power, iron.react_power, vacuum.react_power, fridge.react_power, oven.react_power,
                load.app_power, iron.app_power, vacuum.app_power, fridge.app_power, oven.app_power,
                iron.cycle, vacuum.cycle, fridge.cycle, oven.cycle);
    file.close();
}

void sendDataToFirebase() {
    lastTime = millis();
    FirebaseJson json;
    json.set("/UnixTime", unixtime);
    json.set("/Date", date);
    json.set("/Time", simp_time);
    json.set("/Voltage", load.voltage);
    json.set("/Total_Current", load.current);
    json.set("/Iron_Current", iron.current);
    json.set("/Vacuum_Current", vacuum.current);
    json.set("/Fridge_Current", fridge.current);
    json.set("/Oven_Current", oven.current);
    json.set("/Total_Act_Power", load.act_power);
    json.set("/Total_ReAct_Power", load.react_power);
    json.set("/Total_App_Power", load.app_power);
    json.set("/Iron_Cycle", iron.cycle);
    json.set("/Vacuum_Cycle", vacuum.cycle);
    json.set("/Fridge_Cycle", fridge.cycle);
    json.set("/Oven_Cycle", oven.cycle);

    String path = "/sensorData/" + String(i++);
    if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
        Serial.println("Data sent successfully");
    } else {
        Serial.println(fbdo.errorReason());
    }
}
// Libraries for SD Card
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Libraries for RTC DS3231
#include <Wire.h>
#include <RTClib.h>

// Libraries for Firebase
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// SPI Pins for SD Card
#define REASSIGN_PINS
int sck = 14, miso = 12, mosi = 13, cs = 27;

// RTC Module
RTC_DS3231 rtc;

// Appliance Data Structure
struct ApplianceData {
  String cycle = "1";
  float voltage = 1, current = 1, app_power = 1, react_power = 1, act_power = 1;
  float power_fact = 1;  // Only used for oven and load
};

// Appliance Instances
ApplianceData iron, vacuum, fridge, oven, load;

// Global Variables
String simp_time = "NULL", date = "NULL", unixtime = "NULL";
String receivedData, receivedData_1, receivedData_2;
int dataIndex = 1;

// Firebase Configuration
#define WIFI_SSID "MAUG"
#define WIFI_PASSWORD "d24e95dc"
#define API_KEY "AIzaSyDBcxIo-G9o1Q_sEJx2Ly1NYT52YibvUFk"
#define DATABASE_URL "https://fyp-aiot-smart-meter-dataset-default-rtdb.asia-southeast1.firebasedatabase.app/" 

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

unsigned long lastTime = 0;
const unsigned long timerDelay = 3000;  // 3-second update interval

// Core 1 Task: Handles Serial Communication & Data Processing
void core1Task(void *pvParameters) {
  for (;;) {
    if (Serial.available() > 0) {
      receivedData = Serial.readStringUntil('\n');
      ovenData(receivedData);
    }
    if (Serial2.available() > 0) {
      receivedData_1 = Serial2.readStringUntil('\n');
      loadData(receivedData_1);
    }
    if (Serial1.available() > 0) {
      receivedData_2 = Serial1.readStringUntil('\n');
      otherData(receivedData_2);
    }
    delay(100);
  }
}

// Core 0 Task: Handles Firebase & SD Card Logging
void core0Task(void *pvParameters) {
  for (;;) {
    // Get RTC Time
    DateTime now = rtc.now();
    unixtime = String(now.unixtime());
    date = String(now.year()) + "-" + String(now.month()) + "-" + String(now.day());
    simp_time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

    // Store Data in SD Card
    File file = SD.open("/data.txt", FILE_APPEND);
    if (file) {
      file.print(unixtime + "," + date + "," + simp_time + ",");

      file.print(load.voltage); file.print(",");

      file.print(load.current); file.print(",");
      file.print(iron.current); file.print(",");
      file.print(vacuum.current); file.print(",");
      file.print(fridge.current); file.print(",");
      file.print(oven.current); file.print(",");

      file.print(load.app_power); file.print(",");
      file.print(iron.app_power); file.print(",");
      file.print(vacuum.app_power); file.print(",");
      file.print(fridge.app_power); file.print(",");
      file.print(oven.app_power); file.print(",");

      file.print(load.act_power); file.print(",");
      file.print(iron.act_power); file.print(",");
      file.print(vacuum.act_power); file.print(",");
      file.print(fridge.act_power); file.print(",");
      file.print(oven.act_power); file.print(",");

      file.print(load.react_power); file.print(",");
      file.print(iron.react_power); file.print(",");
      file.print(vacuum.react_power); file.print(",");
      file.print(fridge.react_power); file.print(",");
      file.print(oven.react_power); file.print(",");

      file.print(iron.cycle); file.print(",");
      file.print(vacuum.cycle); file.print(",");
      file.print(fridge.cycle); file.print(",");
      file.print(oven.cycle);

      file.println();
      file.close();
    }

    // Send Data to Firebase
    if ((millis() - lastTime) > timerDelay) {
      lastTime = millis();
      String path = "/sensorData/" + String(date);
      FirebaseJson json;

      json.add("/Data_Index", dataIndex);

      json.add("/UnixTime", unixtime);
      json.add("/Date", date);
      json.add("/Time", simp_time);

      json.add("/Voltage", load.voltage);

      json.add("/Total_Current", load.current);
      json.add("/Iron_Current", iron.current);
      json.add("/Vacuum_Current", vacuum.current);
      json.add("/Fridge_Current", fridge.current);
      json.add("/Oven_Current", oven.current);

      json.add("/Total_App_Power", load.app_power);
      json.add("/Iron_App_Power", iron.app_power);
      json.add("/Vacuum_App_Power", vacuum.app_power);
      json.add("/Fridge_App_Power", fridge.app_power);
      json.add("/Oven_App_Power", oven.app_power);

      json.add("/Total_Act_Power", load.act_power);
      json.add("/Iron_Act_Power", iron.act_power);
      json.add("/Vacuum_Act_Power", vacuum.act_power);
      json.add("/Fridge_Act_Power", fridge.act_power);
      json.add("/Oven_Act_Power", oven.act_power);

      json.add("/Total_ReAct_Power", load.react_power);
      json.add("/Iron_ReAct_Power", iron.react_power);
      json.add("/Vacuum_ReAct_Power", vacuum.react_power);
      json.add("/Fridge_ReAct_Power", fridge.react_power);
      json.add("/Oven_ReAct_Power", oven.react_power);

      json.add("/Iron_Cycle", iron.cycle);
      json.add("/Vacuum_Cycle", vacuum.cycle);
      json.add("/Fridge_Cycle", fridge.cycle);
      json.add("/Oven_Cycle", oven.cycle);

      if (Firebase.RTDB.pushJSON(&fbdo, path.c_str(), &json)) {
        Serial.println("Data sent successfully");
      } else {
        Serial.print("Failed to send data: ");
        Serial.println(fbdo.errorReason());
      }
      dataIndex++;
    }
    delay(3000);
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, 4, 2);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for data...");

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Initialize SD Card
  SPI.begin(sck, miso, mosi, cs);
  if (!SD.begin(cs)) {
    Serial.println("Card Mount Failed");
    while (1);
  }

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  // Firebase Setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = "ghauribrother12@gmail.com";
  auth.user.password = "MAhm12##";
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Start Core Tasks
  xTaskCreatePinnedToCore(core1Task, "Core1Task", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(core0Task, "Core0Task", 10000, NULL, 1, NULL, 0);
}

void loop() {
  vTaskDelay(portMAX_DELAY); // Prevents unnecessary execution
}

// Data Processing Functions
void ovenData(String data) {
  int ovoltageIndex = data.indexOf("Voltage:");
  int ocurrentIndex = data.indexOf("Current:");
  int oapp_powerIndex = data.indexOf("Apparent Power:");
  int opower_factIndex = data.indexOf("Power Factor:");

  if (ovoltageIndex != -1)
    oven.voltage = data.substring(ovoltageIndex + 9, data.indexOf("V", ovoltageIndex)).toFloat();
  if (ocurrentIndex != -1)
    oven.current = data.substring(ocurrentIndex + 8, data.indexOf("A", ocurrentIndex)).toFloat();
  if (opower_factIndex != -1)
    oven.power_fact = data.substring(opower_factIndex + 13, data.indexOf("\n", opower_factIndex)).toFloat();
  if (oapp_powerIndex != -1) {
    oven.app_power = data.substring(oapp_powerIndex + 15, data.indexOf("VA", oapp_powerIndex)).toFloat();
    oven.act_power = oven.app_power * oven.power_fact;
    oven.react_power = sqrt(pow(oven.app_power, 2) - pow(oven.act_power, 2));
  }
  if (oven.current >= 0.5)
    oven.cycle = "1";
  if (oven.current < 0.5)
    oven.cycle = "0";
}

void loadData(String data) {
  int lvoltageIndex = data.indexOf("Voltage:");
  int lcurrentIndex = data.indexOf("Current:");
  int lapp_powerIndex = data.indexOf("Apparent Power:");
  int lpower_factIndex = data.indexOf("Power Factor:");

  if (lvoltageIndex != -1)
    load.voltage = data.substring(lvoltageIndex + 9, data.indexOf("V", lvoltageIndex)).toFloat();
  if (lcurrentIndex != -1)
    load.current = data.substring(lcurrentIndex + 8, data.indexOf("A", lcurrentIndex)).toFloat();
  if (lpower_factIndex != -1)
    load.power_fact = data.substring(lpower_factIndex + 13, data.indexOf("\n", lpower_factIndex)).toFloat();
  if (lapp_powerIndex != -1) {
    load.app_power = data.substring(lapp_powerIndex + 15, data.indexOf("VA", lapp_powerIndex)).toFloat();
    load.act_power = load.app_power * load.power_fact;
    load.react_power = sqrt(pow(load.app_power, 2) - pow(load.act_power, 2));
  }
  if (load.current >= 0.5)
    load.cycle = "1";
  if (load.current < 0.5)
    load.cycle = "0";
}

void otherData(String data) {
  int ivoltageIndex = data.indexOf("Iron Voltage: ");
  int icurrentIndex = data.indexOf("Iron Current: ");
  int iact_powerIndex = data.indexOf("Iron Active Power: ");
  int ireact_powerIndex = data.indexOf("Iron Reactive Power: ");
  int iapp_powerIndex = data.indexOf("Iron Apparent Power: ");
  int icycleIndex = data.indexOf("Iron Cycle: ");

  if (ivoltageIndex != -1)
    iron.voltage = data.substring(ivoltageIndex + 14, data.indexOf(" V", ivoltageIndex)).toFloat();
  if (icurrentIndex != -1)
    iron.current = data.substring(icurrentIndex + 14, data.indexOf(" A", icurrentIndex)).toFloat();
  if (iact_powerIndex != -1)
    iron.act_power = data.substring(iact_powerIndex + 19, data.indexOf(" W", iact_powerIndex)).toFloat();
  if (ireact_powerIndex != -1)
    iron.react_power = data.substring(ireact_powerIndex + 21, data.indexOf(" VAR", ireact_powerIndex)).toFloat();
  if (iapp_powerIndex != -1)
    iron.app_power = data.substring(iapp_powerIndex + 21, data.indexOf(" VA", iapp_powerIndex)).toFloat();
  if (icycleIndex != -1)
    iron.cycle = data.substring(icycleIndex + 12, data.indexOf("\n", icycleIndex));
  

  int vvoltageIndex = data.indexOf("Vacuum Voltage: ");
  int vcurrentIndex = data.indexOf("Vacuum Current: ");
  int vact_powerIndex = data.indexOf("Vacuum Active Power: ");
  int vreact_powerIndex = data.indexOf("Vacuum Reactive Power: ");
  int vapp_powerIndex = data.indexOf("Vacuum Apparent Power: ");
  int vcycleIndex = data.indexOf("Vacuum Cycle: ");

  if (vvoltageIndex != -1)
    vacuum.voltage = data.substring(vvoltageIndex + 16, data.indexOf(" V", vvoltageIndex)).toFloat();
  if (vcurrentIndex != -1)
    vacuum.current = data.substring(vcurrentIndex + 16, data.indexOf(" A", vcurrentIndex)).toFloat();
  if (vact_powerIndex != -1)
    vacuum.act_power = data.substring(vact_powerIndex + 21, data.indexOf(" W", vact_powerIndex)).toFloat();
  if (vreact_powerIndex != -1)
    vacuum.react_power = data.substring(vreact_powerIndex + 23, data.indexOf(" VAR", vreact_powerIndex)).toFloat();
  if (vapp_powerIndex != -1)
    vacuum.app_power = data.substring(vapp_powerIndex + 23, data.indexOf(" VA", vapp_powerIndex)).toFloat();
  if (vcycleIndex != -1)
    vacuum.cycle = data.substring(vcycleIndex + 14, data.indexOf("\n", vcycleIndex));
  

  int fvoltageIndex = data.indexOf("Fridge Voltage: ");
  int fcurrentIndex = data.indexOf("Fridge Current: ");
  int fact_powerIndex = data.indexOf("Fridge Active Power: ");
  int freact_powerIndex = data.indexOf("Fridge Reactive Power: ");
  int fapp_powerIndex = data.indexOf("Fridge Apparent Power: ");
  int fcycleIndex = data.indexOf("Fridge Cycle: ");

  if (fvoltageIndex != -1)
    fridge.voltage = data.substring(fvoltageIndex + 16, data.indexOf(" V", fvoltageIndex)).toFloat();
  if (fcurrentIndex != -1)
    fridge.current = data.substring(fcurrentIndex + 16, data.indexOf(" A", fcurrentIndex)).toFloat();
  if (fact_powerIndex != -1)
    fridge.act_power = data.substring(fact_powerIndex + 21, data.indexOf(" W", fact_powerIndex)).toFloat();
  if (freact_powerIndex != -1)
    fridge.react_power = data.substring(freact_powerIndex + 23, data.indexOf(" VAR", freact_powerIndex)).toFloat();
  if (fapp_powerIndex != -1)
    fridge.app_power = data.substring(fapp_powerIndex + 23, data.indexOf(" VA", fapp_powerIndex)).toFloat();
  if (fcycleIndex != -1)
    fridge.cycle = data.substring(fcycleIndex + 14, data.indexOf("\n", fcycleIndex));
}
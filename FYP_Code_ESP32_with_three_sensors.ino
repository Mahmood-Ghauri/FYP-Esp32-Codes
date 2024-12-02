// Libraries for SD Card Module
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// SPI Port PINS for SD Card
#define REASSIGN_PINS
int sck = 14;
int miso = 12;
int mosi = 13;
int cs = 27;

// Libraries for RTC DS3231
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc; // Create an instance of the RTC_DS3231 class

// Variables to store received data of iron
bool I_cycle = 0;
float I_voltage = 0.0;
float I_current = 0.0;
float I_app_power = 0.0;
float I_react_power = 0.0;
float I_act_power = 0.0;
float I_power_fact = 0.0;

// Variables to store received data of vacuum cleaner
bool V_cycle = 0;
float V_voltage = 0.0;
float V_current = 0.0;
float V_app_power = 0.0;
float V_react_power = 0.0;
float V_act_power = 0.0;
float V_power_fact = 0.0;

// Variables to store received data of fridge
bool F_cycle = 0;
float F_voltage = 0.0;
float F_current = 0.0;
float F_app_power = 0.0;
float F_react_power = 0.0;
float F_act_power = 0.0;
float F_power_fact = 0.0;

float voltage = 0.0;
float total_power = 0.0;
String dateTimeStr = "NULL";

void setup() {
  // Initialize Serial ports
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, 16, 17);
  Serial2.begin(9600, SERIAL_8N1, 4, 2);
  Serial.println("Waiting for data...");

  // Initialize RTC Date & Time
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  // Uncomment this if you want to reset the time and date
  // Set RTC to the current Unix time
  //rtc.adjust(DateTime(UnixTimeToDate(1700000000))); 

  // Initialize SD Card
  #ifdef REASSIGN_PINS
    SPI.begin(sck, miso, mosi, cs);
    if (!SD.begin(cs)) 
    {
  #else
 
  if (!SD.begin()) 
  {
    #endif
    Serial.println("Card Mount Failed");
    delay(100);
  }
  uint8_t cardType = SD.cardType();
  while (cardType == CARD_NONE) 
  {
    Serial.println("No SD card attached");
    delay(100);
  }
}

void loop() {

  // Check if data is available on Serial (IRON)
  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n'); // Read the data line

    // Calling Function to calculate values of iron
    ironData(receivedData);
  }

  // Check if data is available on Serial1 (VACUUM CLEANER)
  if (Serial1.available() > 0) {
    String receivedData_1 = Serial1.readStringUntil('\n'); // Read the data line

    // Calling Function to calculate values of vacuum cleaner
    vacuumData(receivedData_1);
  }

  // Check if data is available on Serial2 (FRIDGE)
  if (Serial2.available() > 0) {
    String receivedData_2 = Serial2.readStringUntil('\n'); // Read the data line

    // Calling Function to calculate values of fridge
    fridgeData(receivedData_2);
  }

  // Format the date and time as a string
  DateTime now = rtc.now();
  uint32_t unixTime = now.unixtime();
  dateTimeStr = String(now.year()) + String(now.month()) + String(now.day()) + " " + String(unixTime);

  // Storing Data in SD Card
  File file = SD.open("/hello.txt", FILE_APPEND);
  while (!file) 
  {
    Serial.println("Failed to open file for writing");
    delay(100);
  }
  file.print(dateTimeStr);
  file.print(",");

  voltage = (I_voltage+V_voltage+F_voltage)/3;
  file.print(voltage);
  file.print(",");

  file.print(I_current);
  file.print(",");
  file.print(V_current);
  file.print(",");
  file.print(F_current);
  file.print(",");

  total_power = I_act_power+V_act_power+F_act_power;
  file.print(total_power);
  file.print(",");

  file.print(I_act_power);
  file.print(",");
  file.print(V_act_power);
  file.print(",");
  file.print(F_act_power);
  file.print(",");

  file.print(I_react_power);
  file.print(",");
  file.print(V_react_power);
  file.print(",");
  file.print(F_react_power);
  file.print(",");

  file.print(I_app_power);
  file.print(",");
  file.print(V_app_power);
  file.print(",");
  file.print(F_app_power);
  file.print(",");

  file.print(I_cycle);
  file.print(",");
  file.print(V_cycle);
  file.print(",");
  file.print(F_cycle);
  file.println();

  file.close();

  // Send Value to the other ESP32 using UART Communication
  Serial.println("-------------------------------");
  Serial.println("Iron Voltage: " + String(I_voltage) + " V");
  Serial.println("Iron Current: " + String(I_current) + " A");
  Serial.println("Iron Active Power: " + String(I_act_power) + " W");
  Serial.println("Iron Reactive Power: " + String(I_react_power) + " VAR");
  Serial.println("Iron Apparent Power: " + String(I_app_power) + " VA");
  Serial.println("Iron Cycle: " + String(I_cycle));

  Serial.println("-------------------------------");
  Serial.println("Vacuum Voltage: " + String(V_voltage) + " V");
  Serial.println("Vacuum Current: " + String(V_current) + " A");
  Serial.println("Vacuum Active Power: " + String(V_act_power) + " W");
  Serial.println("Vacuum Reactive Power: " + String(V_react_power) + " VAR");
  Serial.println("Vacuum Apparent Power: " + String(V_app_power) + " VA");
  Serial.println("Vacuum Cycle: " + String(V_cycle));

  Serial.println("-------------------------------");
  Serial.println("Fridge Voltage: " + String(F_voltage) + " V");
  Serial.println("Fridge Current: " + String(F_current) + " A");
  Serial.println("Fridge Active Power: " + String(F_act_power) + " W");
  Serial.println("Fridge Reactive Power: " + String(F_react_power) + " VAR");
  Serial.println("Fridge Apparent Power: " + String(F_app_power) + " VA");
  Serial.println("Fridge Cycle: " + String(F_cycle));

  delay(2000); // Delay to avoid flooding with data
}

// Function to Iron data
void ironData(String data) {
  int ivoltageIndex = data.indexOf("Voltage:");
  int icurrentIndex = data.indexOf("Current:");
  int iapp_powerIndex = data.indexOf("Apparent Power:");
  int ipower_factIndex = data.indexOf("Power Factor:");
 
  if (ivoltageIndex != -1 && icurrentIndex != -1 && iapp_powerIndex != -1 && ipower_factIndex != -1) {
    // Extract the voltage value
    I_voltage = data.substring(ivoltageIndex + 8, data.indexOf("V", ivoltageIndex)).toFloat();

    // Extract the current value
    I_current = data.substring(icurrentIndex + 8, data.indexOf("A", icurrentIndex)).toFloat();

    // Extract the apparent power value
    I_app_power = data.substring(iapp_powerIndex + 15, data.indexOf("VA", iapp_powerIndex)).toFloat();

    // Extract the power factor value
    I_power_fact = data.substring(ipower_factIndex + 13, data.indexOf("\n", ipower_factIndex)).toFloat();

    // Calculation of State of Appliance
    if (I_current >= 0.5){
      I_cycle = 1;
    }

    // Calculation of active power
    I_act_power = I_app_power * I_power_fact;

    // Calculation of reactive power
    I_react_power = sqrt(pow(I_app_power,2) - pow(I_act_power,2));
  }
}

// Function to Vacuum data
void vacuumData(String data) {
  int vvoltageIndex = data.indexOf("Voltage:");
  int vcurrentIndex = data.indexOf("Current:");
  int vapp_powerIndex = data.indexOf("Apparent Power:");
  int vpower_factIndex = data.indexOf("Power Factor:");
 
  if (vvoltageIndex != -1 && vcurrentIndex != -1 && vapp_powerIndex != -1 && vpower_factIndex != -1) {
    // Extract the voltage value
    V_voltage = data.substring(vvoltageIndex + 8, data.indexOf("V", vvoltageIndex)).toFloat();

    // Extract the current value
    V_current = data.substring(vcurrentIndex + 8, data.indexOf("A", vcurrentIndex)).toFloat();

    // Extract the apparent power value
    V_app_power = data.substring(vapp_powerIndex + 15, data.indexOf("VA", vapp_powerIndex)).toFloat();

    // Extract the power factor value
    V_power_fact = data.substring(vpower_factIndex + 13, data.indexOf("\n", vpower_factIndex)).toFloat();

    // Calculation of State of Appliance
    if (V_current >= 0.5){
      V_cycle = 1;
    }

    // Calculation of active power
    V_act_power = V_app_power * V_power_fact;

    // Calculation of reactive power
    V_react_power = sqrt(pow(V_app_power,2) - pow(V_act_power,2));
  }
}

// Function to Fridge data
void fridgeData(String data) {
  // Example: "Voltage: 230.0 V Current: 10.00 A Power: 2300.00 W"
  int fvoltageIndex = data.indexOf("Voltage:");
  int fcurrentIndex = data.indexOf("Current:");
  int fapp_powerIndex = data.indexOf("Apparent Power:");
  int fpower_factIndex = data.indexOf("Power Factor:");
 
  if (fvoltageIndex != -1 && fcurrentIndex != -1 && fapp_powerIndex != -1 && fpower_factIndex != -1) {
    // Extract the voltage value
    F_voltage = data.substring(fvoltageIndex + 8, data.indexOf("V", fvoltageIndex)).toFloat();

    // Extract the current value
    F_current = data.substring(fcurrentIndex + 8, data.indexOf("A", fcurrentIndex)).toFloat();

    // Extract the apparent power value
    F_app_power = data.substring(fapp_powerIndex + 15, data.indexOf("VA", fapp_powerIndex)).toFloat();

    // Extract the power factor value
    F_power_fact = data.substring(fpower_factIndex + 13, data.indexOf("\n", fpower_factIndex)).toFloat();

    // Calculation of State of Appliance
    if (F_current >= 0.5){
      F_cycle = 1;
    }

    // Calculation of active power
    F_act_power = F_app_power * F_power_fact;

    // Calculation of reactive power
    F_react_power = sqrt(pow(F_app_power,2) - pow(F_act_power,2));
  }
}
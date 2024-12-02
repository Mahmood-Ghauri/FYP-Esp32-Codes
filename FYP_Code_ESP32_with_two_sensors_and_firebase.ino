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
String I_cycle = "0";
float I_voltage = 0.0;
float I_current = 0.0;
float I_app_power = 0.0;
float I_react_power = 0.0;
float I_act_power = 0.0;

// Variables to store received data of vacuum cleaner
String V_cycle = "0";
float V_voltage = 0.0;
float V_current = 0.0;
float V_app_power = 0.0;
float V_react_power = 0.0;
float V_act_power = 0.0;

// Variables to store received data of fridge
String F_cycle = "0";
float F_voltage = 0.0;
float F_current = 0.0;
float F_app_power = 0.0;
float F_react_power = 0.0;
float F_act_power = 0.0;

// Variables to store received data of Oven
bool O_cycle = 0;
float O_voltage = 0.0;
float O_current = 0.0;
float O_app_power = 0.0;
float O_react_power = 0.0;
float O_act_power = 0.0;
float O_power_fact = 0.0;

// Variables to store received data of Overall Load
bool L_cycle = 0;
float L_voltage = 0.0;
float L_current = 0.0;
float L_app_power = 0.0;
float L_react_power = 0.0;
float L_act_power = 0.0;
float L_power_fact = 0.0;

String timedate = "NULL";
float voltage = 0.0;
float total_power = 0.0;

//Firebase Setup
#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "MAUG"
#define WIFI_PASSWORD "d24e95dc"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyDBcxIo-G9o1Q_sEJx2Ly1NYT52YibvUFk"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://fyp-aiot-smart-meter-dataset-default-rtdb.asia-southeast1.firebasedatabase.app/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "ghauribrother12@gmail.com"
#define USER_PASSWORD "MAhm12##"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  // Initialize Serial ports
  Serial.begin(9600);                             // Oven (Appliance 04)
  Serial1.begin(9600, SERIAL_8N1, 16, 17);        // Total (Overall Power Consumption)
  Serial2.begin(9600, SERIAL_8N1, 4, 2);          // For serial Communication with other ESP32 for other Appliances Values
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

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);
}

void loop() {

  // Check if data is available on Serial (IRON)
  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n'); // Read the data line

    // Calling Function to calculate values of iron
    ovenData(receivedData);
  }

  // Check if data is available on Serial1 (VACUUM CLEANER)
  if (Serial1.available() > 0) {
    String receivedData_1 = Serial1.readStringUntil('\n'); // Read the data line

    // Calling Function to calculate values of vacuum cleaner
    loadData(receivedData_1);
  }

  // Check if data is available on Serial2 (FRIDGE)
  if (Serial2.available() > 0) {
    String receivedData_2 = Serial2.readStringUntil('\n'); // Read the data line

    // Calling Function to calculate values of fridge
    otherData(receivedData_2);
  }

  // Format the date and time as a string
  DateTime now = rtc.now();
  uint32_t unixTime = now.unixtime();
  timedate = String(now.year()) + String(now.month()) + String(now.day()) + " " + String(unixTime);

  // Storing Data in SD Card
  File file = SD.open("/hello.txt", FILE_APPEND);
  while (!file) 
  {
    Serial.println("Failed to open file for writing");
    delay(100);
  }

  file.print(timedate);
  file.print(",");

  voltage = (I_voltage+V_voltage+F_voltage+O_voltage)/4;
  file.print(voltage);
  file.print(",");

  file.print(I_current);
  file.print(",");
  file.print(V_current);
  file.print(",");
  file.print(F_current);
  file.print(",");
  file.print(O_current);
  file.print(",");

  total_power = I_act_power+V_act_power+F_act_power+O_act_power;
  file.print(total_power);
  file.print(",");

  file.print(I_act_power);
  file.print(",");
  file.print(V_act_power);
  file.print(",");
  file.print(F_act_power);
  file.print(",");
  file.print(O_act_power);
  file.print(",");

  file.print(I_react_power);
  file.print(",");
  file.print(V_react_power);
  file.print(",");
  file.print(F_react_power);
  file.print(",");
  file.print(O_react_power);
  file.print(",");

  file.print(I_app_power);
  file.print(",");
  file.print(V_app_power);
  file.print(",");
  file.print(F_app_power);
  file.print(",");
  file.print(O_app_power);
  file.print(",");

  file.print(I_cycle);
  file.print(",");
  file.print(V_cycle);
  file.print(",");
  file.print(F_cycle);
  file.print(",");
  file.print(O_cycle);
  file.println();

  file.close();

  overallData();

  FirebaseJson json;

  // Populate JSON object with the data
  json.add("TimeDate",timedate);

  json.add("Voltage",voltage);

  json.add("Iron_Current",I_current);
  json.add("Vacuum_Current",V_current);
  json.add("Fridge_Current",F_current);
  json.add("Oven_Current",O_current);

  json.add("Total_Power",total_power);

  json.add("Iron_Act_Power",I_act_power);
  json.add("Vacuum_Act_Power",V_act_power);
  json.add("Fridge_Act_Power",F_act_power);
  json.add("Oven_Act_Power",O_act_power);

  json.add("Iron_ReAct_Power",I_react_power);
  json.add("Vacuum_ReAct_Power",V_react_power);
  json.add("Fridge_ReAct_Power",F_react_power);
  json.add("Oven_ReAct_Power",O_react_power);

  json.add("Iron_App_Power",I_app_power);
  json.add("Vacuum_App_Power",V_app_power);
  json.add("Fridge_App_Power",F_app_power);
  json.add("Oven_App_Power",O_app_power);

  json.add("Iron_Cycle",I_cycle);
  json.add("Vacuum_Cycle",V_cycle);
  json.add("Fridge_Cycle",F_cycle);
  json.add("Oven_Cycle",O_cycle);

  delay(2000); // Delay to avoid flooding with data
}

void overallData() {
  File file = SD.open("alldata.txt", FILE_APPEND);
  while (!file) 
  {
    Serial.println("Failed to open file for writing");
    delay(100);
  }

  file.print(timedate);
  file.print(",");
  file.print(L_voltage);
  file.print(",");
  file.print(L_current);
  file.print(",");
  file.print(L_act_power);
  file.print(",");
  file.print(L_react_power);
  file.print(",");
  file.print(L_app_power);
  file.print(",");
  file.print(L_power_fact);
  file.print(",");
  file.print(I_cycle);
  file.print(",");
  file.print(V_cycle);
  file.print(",");
  file.print(F_cycle);
  file.print(",");
  file.print(O_cycle);
  file.println(); // Add a newline at the end

  file.close();
}

// Function to Oven data
void ovenData(String data) {
  int ovoltageIndex = data.indexOf("Voltage:");
  int ocurrentIndex = data.indexOf("Current:");
  int oapp_powerIndex = data.indexOf("Apparent Power:");
  int opower_factIndex = data.indexOf("Power Factor:");
 
  if (ovoltageIndex != -1 && ocurrentIndex != -1 && oapp_powerIndex != -1 && opower_factIndex != -1) {
    // Extract the voltage value
    O_voltage = data.substring(ovoltageIndex + 8, data.indexOf("V", ovoltageIndex)).toFloat();

    // Extract the current value
    O_current = data.substring(ocurrentIndex + 8, data.indexOf("A", ocurrentIndex)).toFloat();

    // Extract the apparent power value
    O_app_power = data.substring(oapp_powerIndex + 15, data.indexOf("VA", oapp_powerIndex)).toFloat();

    // Extract the power factor value
    O_power_fact = data.substring(opower_factIndex + 13, data.indexOf("\n", opower_factIndex)).toFloat();

    // Calculation of State of Appliance
    if (O_current >= 0.5){
      O_cycle = 1;
    }

    // Calculation of active power
    O_act_power = O_app_power * O_power_fact;

    // Calculation of reactive power
    O_react_power = sqrt(pow(O_app_power,2) - pow(O_act_power,2));
  }
}

// Function to Total Load data
void loadData(String data) {
  int lvoltageIndex = data.indexOf("Voltage:");
  int lcurrentIndex = data.indexOf("Current:");
  int lapp_powerIndex = data.indexOf("Apparent Power:");
  int lpower_factIndex = data.indexOf("Power Factor:");
 
  if (lvoltageIndex != -1 && lcurrentIndex != -1 && lapp_powerIndex != -1 && lpower_factIndex != -1) {
    // Extract the voltage value
    L_voltage = data.substring(lvoltageIndex + 8, data.indexOf("V", lvoltageIndex)).toFloat();

    // Extract the current value
    L_current = data.substring(lcurrentIndex + 8, data.indexOf("A", lcurrentIndex)).toFloat();

    // Extract the apparent power value
    L_app_power = data.substring(lapp_powerIndex + 15, data.indexOf("VA", lapp_powerIndex)).toFloat();

    // Extract the power factor value
    L_power_fact = data.substring(lpower_factIndex + 13, data.indexOf("\n", lpower_factIndex)).toFloat();

    // Calculation of State of Appliance
    if (L_current >= 0.5){
      L_cycle = 1;
    }

    // Calculation of active power
    L_act_power = L_app_power * L_power_fact;

    // Calculation of reactive power
    L_react_power = sqrt(pow(L_app_power,2) - pow(L_act_power,2));
  }
}

// Function to Iron, Vacuum, Fridge data
void otherData(String data) {
  int ivoltageIndex = data.indexOf("Iron Voltage: ");
  int icurrentIndex = data.indexOf("Iron Current: ");
  int iact_powerIndex = data.indexOf("Iron Active Power: ");
  int ireact_powerIndex = data.indexOf("Iron Reactive Power: ");
  int iapp_powerIndex = data.indexOf("Iron Apparent Power: ");
  int icycleIndex = data.indexOf("Iron Cycle: ");
 
  if (ivoltageIndex != -1 && icurrentIndex != -1 && iact_powerIndex != -1 && ireact_powerIndex != -1 && iapp_powerIndex != -1 && icycleIndex != -1) {

    // Extract the voltage value
    I_voltage = data.substring(ivoltageIndex + 14, data.indexOf(" V", ivoltageIndex)).toFloat();
    
    // Extract the current value
    I_current = data.substring(icurrentIndex + 14, data.indexOf(" A", icurrentIndex)).toFloat();
    
    // Extract the active power value
    I_act_power = data.substring(iact_powerIndex + 19, data.indexOf(" W", iact_powerIndex)).toFloat();

    // Extract the reactive power value
    I_react_power = data.substring(ireact_powerIndex + 21, data.indexOf(" VAR", ireact_powerIndex)).toFloat();

    // Extract the apparent power value
    I_app_power = data.substring(iapp_powerIndex + 21, data.indexOf(" VA", iapp_powerIndex)).toFloat();

    // Extract the cycle value
    I_cycle = data.substring(icycleIndex + 12, data.indexOf("\n", icycleIndex));
  }


  int vvoltageIndex = data.indexOf("Vacuum Voltage: ");
  int vcurrentIndex = data.indexOf("Vacuum Current: ");
  int vact_powerIndex = data.indexOf("Vacuum Active Power: ");
  int vreact_powerIndex = data.indexOf("Vacuum Reactive Power: ");
  int vapp_powerIndex = data.indexOf("Vacuum Apparent Power: ");
  int vcycleIndex = data.indexOf("Vacuum Cycle: ");
 
  if (vvoltageIndex != -1 && vcurrentIndex != -1 && vact_powerIndex != -1 && vreact_powerIndex != -1 && vapp_powerIndex != -1 && vcycleIndex != -1) {

    // Extract the voltage value
    V_voltage = data.substring(vvoltageIndex + 16, data.indexOf(" V", vvoltageIndex)).toFloat();
    
    // Extract the current value
    V_current = data.substring(vcurrentIndex + 16, data.indexOf(" A", vcurrentIndex)).toFloat();
    
    // Extract the active power value
    V_act_power = data.substring(vact_powerIndex + 21, data.indexOf(" W", vact_powerIndex)).toFloat();

    // Extract the reactive power value
    V_react_power = data.substring(vreact_powerIndex + 23, data.indexOf(" VAR", vreact_powerIndex)).toFloat();

    // Extract the apparent power value
    V_app_power = data.substring(vapp_powerIndex + 23, data.indexOf(" VA", vapp_powerIndex)).toFloat();

    // Extract the cycle value
    V_cycle = data.substring(vcycleIndex + 14, data.indexOf("\n", vcycleIndex));
  }


  int fvoltageIndex = data.indexOf("Fridge Voltage: ");
  int fcurrentIndex = data.indexOf("Fridge Current: ");
  int fact_powerIndex = data.indexOf("Fridge Active Power: ");
  int freact_powerIndex = data.indexOf("Fridge Reactive Power: ");
  int fapp_powerIndex = data.indexOf("Fridge Apparent Power: ");
  int fcycleIndex = data.indexOf("Fridge Cycle: ");
 
  if (fvoltageIndex != -1 && fcurrentIndex != -1 && fact_powerIndex != -1 && freact_powerIndex != -1 && fapp_powerIndex != -1 && fcycleIndex != -1) {

    // Extract the voltage value
    F_voltage = data.substring(fvoltageIndex + 16, data.indexOf(" V", fvoltageIndex)).toFloat();
    
    // Extract the current value
    F_current = data.substring(fcurrentIndex + 16, data.indexOf(" A", fcurrentIndex)).toFloat();
    
    // Extract the active power value
    F_act_power = data.substring(fact_powerIndex + 21, data.indexOf(" W", fact_powerIndex)).toFloat();

    // Extract the reactive power value
    F_react_power = data.substring(freact_powerIndex + 23, data.indexOf(" VAR", freact_powerIndex)).toFloat();

    // Extract the apparent power value
    F_app_power = data.substring(fapp_powerIndex + 23, data.indexOf(" VA", fapp_powerIndex)).toFloat();

    // Extract the cycle value
    F_cycle = data.substring(fcycleIndex + 14, data.indexOf("\n", fcycleIndex));
  }
}
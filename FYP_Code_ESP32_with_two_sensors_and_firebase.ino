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
String I_cycle = "1";
float I_voltage = 1;
float I_current = 1;
float I_app_power = 1;
float I_react_power = 1;
float I_act_power = 1;

// Variables to store received data of vacuum cleaner
String V_cycle = "1";
float V_voltage = 1;
float V_current = 1;
float V_app_power = 1;
float V_react_power = 1;
float V_act_power = 1;

// Variables to store received data of fridge
String F_cycle = "1";
float F_voltage = 1;
float F_current = 1;
float F_app_power = 1;
float F_react_power = 1;
float F_act_power = 1;

// Variables to store received data of Oven
bool O_cycle = 1;
float O_voltage = 1;
float O_current = 1;
float O_app_power = 1;
float O_react_power = 1;
float O_act_power = 1;
float O_power_fact = 1;

// Variables to store received data of Overall Load
bool L_cycle = 1;
float L_voltage = 1;
float L_current = 1;
float L_app_power = 1;
float L_react_power = 1;
float L_act_power = 1;
float L_power_fact = 1;

String timedate = "NULL";
float total_power = 0.0;

String receivedData;
String receivedData_1;
String receivedData_2;

//Firebase Setup
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Your WiFi credentials
#define WIFI_SSID "MAUG"
#define WIFI_PASSWORD "d24e95dc"

// Firebase project credentials
#define API_KEY "AIzaSyDBcxIo-G9o1Q_sEJx2Ly1NYT52YibvUFk"
#define DATABASE_URL "https://fyp-aiot-smart-meter-dataset-default-rtdb.asia-southeast1.firebasedatabase.app/" // Database URL

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  // Send data every 5 second

void setup() {
  // Initialize Serial ports
  Serial.begin(9600);                             // Oven (Appliance 04)
  Serial1.begin(9600, SERIAL_8N1, 4, 2 );      //RX,TX  // Total (Overall Power Consumption)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);          // For serial Communication with other ESP32 for other Appliances Values
  Serial.println("Waiting for data...");

  // Initialize RTC Date & Time
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  // Uncomment this if you want to reset the time and date
  // Set RTC to the current Unix time
  // unsigned long unixTime = 1733249000;  // Example Unix timestamp
  // rtc.adjust(DateTime(unixTime));

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

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Assign the user authentication (optional)
  auth.user.email = "ghauribrother12@gmail.com";       // Optional for authentication
  auth.user.password = "MAhm12##"; // Optional for authentication

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  // Check if data is available on Serial (IRON)
  while (Serial.available() > 0) {
    Serial.print("Oven");
    receivedData = Serial.readStringUntil('\n');
    ovenData(receivedData);
  }

  // Check if data is available on Serial1 (VACUUM CLEANER)
  while (Serial2.available() > 0) {
    Serial.print("Overall Load");
    receivedData_1 = Serial2.readStringUntil('\n');
    loadData(receivedData_1);
  }

  // Check if data is available on Serial2 (FRIDGE)
  while (Serial1.available() > 0) {
    Serial.print("Fridge, Vacuum, Iron");
    receivedData_2 = Serial1.readStringUntil('\n');
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

  file.print(L_voltage);
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

  // Send data every `timerDelay` milliseconds
  if ((millis() - lastTime) > timerDelay) {
    lastTime = millis();

    // JSON object to store data
    String path = "/sensorData/" + String(millis()); // Timestamp as key
    FirebaseJson json;
    // Populate JSON object with the data
    json.set("/TimeDate",timedate);

    json.set("/Voltage",L_voltage);

    json.set("/Iron_Current",I_current);
    json.set("/Vacuum_Current",V_current);
    json.set("/Fridge_Current",F_current);
    json.set("/Oven_Current",O_current);

    json.set("/Total_Power",total_power);

    json.set("/Iron_Act_Power",I_act_power);
    json.set("/Vacuum_Act_Power",V_act_power);
    json.set("/Fridge_Act_Power",F_act_power);
    json.set("/Oven_Act_Power",O_act_power);

    json.set("/Iron_ReAct_Power",I_react_power);
    json.set("/Vacuum_ReAct_Power",V_react_power);
    json.set("/Fridge_ReAct_Power",F_react_power);
    json.set("/Oven_ReAct_Power",O_react_power);

    json.set("/Iron_App_Power",I_app_power);
    json.set("/Vacuum_App_Power",V_app_power);
    json.set("/Fridge_App_Power",F_app_power);
    json.set("/Oven_App_Power",O_app_power);

    json.set("/Iron_Cycle",I_cycle);
    json.set("/Vacuum_Cycle",V_cycle);
    json.set("/Fridge_Cycle",F_cycle);
    json.set("/Oven_Cycle",O_cycle);

    // Send data to Firebase
    if (Firebase.RTDB.setJSON(&fbdo, path.c_str(), &json)) {
      Serial.println("Data sent successfully:");
    } else {
      Serial.print("Failed to send data: ");
      Serial.println(fbdo.errorReason());
    }
  }
  delay(2000); // Delay to avoid flooding with data
}

void overallData() {
  File file = SD.open("/alldata.txt", FILE_APPEND);
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
 
  if (ovoltageIndex != -1)
  {
    O_voltage = data.substring(ovoltageIndex + 8, data.indexOf("V", ovoltageIndex)).toFloat();
  }

  if (ocurrentIndex != -1)
  {
    O_current = data.substring(ocurrentIndex + 8, data.indexOf("A", ocurrentIndex)).toFloat();
  }

  if (opower_factIndex != -1)
  {
    O_power_fact = data.substring(opower_factIndex + 13, data.indexOf("\n", opower_factIndex)).toFloat();
  }

  if (oapp_powerIndex != -1)
  {
    O_app_power = data.substring(oapp_powerIndex + 15, data.indexOf("VA", oapp_powerIndex)).toFloat();
    O_act_power = O_app_power * O_power_fact;
    O_react_power = sqrt(pow(O_app_power,2) - pow(O_act_power,2));
  }
  if (O_current >= 0.5)
  {
    O_cycle = 1;
  }
  if (O_current <= 0.5)
  {
    O_cycle = 0;
  }
}

// Function to Total Load data
void loadData(String data) {
  int lvoltageIndex = data.indexOf("Voltage:");
  int lcurrentIndex = data.indexOf("Current:");
  int lapp_powerIndex = data.indexOf("Apparent Power:");
  int lpower_factIndex = data.indexOf("Power Factor:");
 
  if (lvoltageIndex != -1)
  {
    L_voltage = data.substring(lvoltageIndex + 8, data.indexOf("V", lvoltageIndex)).toFloat();
  }

  if (lcurrentIndex != -1)
  {
    L_current = data.substring(lcurrentIndex + 8, data.indexOf("A", lcurrentIndex)).toFloat();
  }

  if (lpower_factIndex != -1)
  {
    L_power_fact = data.substring(lpower_factIndex + 13, data.indexOf("\n", lpower_factIndex)).toFloat();
  }

  if (lapp_powerIndex != -1)
  {
    L_app_power = data.substring(lapp_powerIndex + 15, data.indexOf("VA", lapp_powerIndex)).toFloat();
    L_act_power = L_app_power * L_power_fact;
    L_react_power = sqrt(pow(L_app_power,2) - pow(L_act_power,2));
  }
  if (L_current >= 0.5)
  {
    L_cycle = 1;
  }
  if (L_current <= 0.5)
  {
    L_cycle = 0;
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
 
  if (ivoltageIndex != -1) {
    I_voltage = data.substring(ivoltageIndex + 14, data.indexOf(" V", ivoltageIndex)).toFloat();
  }
  if (icurrentIndex != -1) {
    I_current = data.substring(icurrentIndex + 14, data.indexOf(" A", icurrentIndex)).toFloat();
  }
  if (iact_powerIndex != -1) {
    I_act_power = data.substring(iact_powerIndex + 19, data.indexOf(" W", iact_powerIndex)).toFloat();
  }
  if (ireact_powerIndex != -1) {
    I_react_power = data.substring(ireact_powerIndex + 21, data.indexOf(" VAR", ireact_powerIndex)).toFloat();
  }
  if (iapp_powerIndex != -1) {
    I_app_power = data.substring(iapp_powerIndex + 21, data.indexOf(" VA", iapp_powerIndex)).toFloat();
  }
  if (icycleIndex != -1) {
    I_cycle = data.substring(icycleIndex + 12, data.indexOf("\n", icycleIndex));
  }


  int vvoltageIndex = data.indexOf("Vacuum Voltage: ");
  int vcurrentIndex = data.indexOf("Vacuum Current: ");
  int vact_powerIndex = data.indexOf("Vacuum Active Power: ");
  int vreact_powerIndex = data.indexOf("Vacuum Reactive Power: ");
  int vapp_powerIndex = data.indexOf("Vacuum Apparent Power: ");
  int vcycleIndex = data.indexOf("Vacuum Cycle: ");
 
  if (vvoltageIndex != -1) {
    V_voltage = data.substring(vvoltageIndex + 16, data.indexOf(" V", vvoltageIndex)).toFloat();
  }
  if (vcurrentIndex != -1) {
    V_current = data.substring(vcurrentIndex + 16, data.indexOf(" A", vcurrentIndex)).toFloat();
  }
  if (vact_powerIndex != -1) {
    V_act_power = data.substring(vact_powerIndex + 21, data.indexOf(" W", vact_powerIndex)).toFloat();
  }
  if (vreact_powerIndex != -1) {
    V_react_power = data.substring(vreact_powerIndex + 23, data.indexOf(" VAR", vreact_powerIndex)).toFloat();
  }
  if (vapp_powerIndex != -1) {
    V_app_power = data.substring(vapp_powerIndex + 23, data.indexOf(" VA", vapp_powerIndex)).toFloat();
  }
  if (vcycleIndex != -1) {
    V_cycle = data.substring(vcycleIndex + 14, data.indexOf("\n", vcycleIndex));
  }


  int fvoltageIndex = data.indexOf("Fridge Voltage: ");
  int fcurrentIndex = data.indexOf("Fridge Current: ");
  int fact_powerIndex = data.indexOf("Fridge Active Power: ");
  int freact_powerIndex = data.indexOf("Fridge Reactive Power: ");
  int fapp_powerIndex = data.indexOf("Fridge Apparent Power: ");
  int fcycleIndex = data.indexOf("Fridge Cycle: ");
 
  if (fvoltageIndex != -1) {
    F_voltage = data.substring(fvoltageIndex + 16, data.indexOf(" V", fvoltageIndex)).toFloat();
  }
  if (fcurrentIndex != -1) {
    F_current = data.substring(fcurrentIndex + 16, data.indexOf(" A", fcurrentIndex)).toFloat();
  }
  if (fact_powerIndex != -1) {
    F_act_power = data.substring(fact_powerIndex + 21, data.indexOf(" W", fact_powerIndex)).toFloat();
  }
  if (freact_powerIndex != -1) {
    F_react_power = data.substring(freact_powerIndex + 23, data.indexOf(" VAR", freact_powerIndex)).toFloat();
  }
  if (fapp_powerIndex != -1) {
    F_app_power = data.substring(fapp_powerIndex + 23, data.indexOf(" VA", fapp_powerIndex)).toFloat();
  }
  if (fcycleIndex != -1) {
    F_cycle = data.substring(fcycleIndex + 14, data.indexOf("\n", fcycleIndex));
  }
}
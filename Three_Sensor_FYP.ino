// Variables to store received data of iron
bool I_cycle = 1;
float I_voltage = 1;
float I_current = 1;
float I_app_power = 1;
float I_react_power = 1;
float I_act_power = 1;
float I_power_fact = 1;

// Variables to store received data of vacuum cleaner
bool V_cycle = 1;
float V_voltage = 1;
float V_current = 1;
float V_app_power = 1;
float V_react_power = 1;
float V_act_power = 1;
float V_power_fact = 1;

// Variables to store received data of fridge
bool F_cycle = 1;
float F_voltage = 1;
float F_current = 1;
float F_app_power = 1;
float F_react_power = 1;
float F_act_power = 1;
float F_power_fact = 1;

String receivedData;
String receivedData_1;
String receivedData_2;

void setup()
{
  // Initialize Serial ports
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, 4, 2);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for data...");
}

void loop()
{
  // Check if data is available on Serial (IRON)
  while (Serial.available() > 0)
  {
    receivedData = Serial.readStringUntil('\n'); // Read the data line
    // Calling Function to calculate values of iron
    ironData(receivedData);
  }

  while (Serial2.available() > 0) 
  {
    receivedData_1 = Serial2.readStringUntil('\n'); // Read the data line
    // Calling Function to calculate values of vacuum cleaner
    vacuumData(receivedData_1); 
  }

  while (Serial1.available() > 0)
  {
    receivedData_2 = Serial1.readStringUntil('\n'); // Read the data line
    // Calling Function to calculate values of fridge
    fridgeData(receivedData_2);
  }

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

  delay(4000); // Delay to avoid flooding with data
}

// Function to Iron data
void ironData(String data) {
  int ivoltageIndex = data.indexOf("Voltage:");
  int icurrentIndex = data.indexOf("Current:");
  int ipower_factIndex = data.indexOf("Power Factor:");
  int iapp_powerIndex = data.indexOf("Apparent Power:");
 
  if (ivoltageIndex != -1)
  {
    I_voltage = data.substring(ivoltageIndex + 8, data.indexOf("V", ivoltageIndex)).toFloat();
  }

  if (icurrentIndex != -1)
  {
    I_current = data.substring(icurrentIndex + 8, data.indexOf("A", icurrentIndex)).toFloat();
  }

  if (ipower_factIndex != -1)
  {
    I_power_fact = data.substring(ipower_factIndex + 13, data.indexOf("\n", ipower_factIndex)).toFloat();
  }

  if (iapp_powerIndex != -1)
  {
    I_app_power = data.substring(iapp_powerIndex + 15, data.indexOf("VA", iapp_powerIndex)).toFloat();
    I_act_power = I_app_power * I_power_fact;
    I_react_power = sqrt(pow(I_app_power,2) - pow(I_act_power,2));
  }
  if (I_current >= 0.5)
  {
    I_cycle = 1;
  }
  if (I_current <= 0.5)
  {
    I_cycle = 0;
  }
}

// Function to Vacuum data
void vacuumData(String data) {
  int vvoltageIndex = data.indexOf("Voltage:");
  int vcurrentIndex = data.indexOf("Current:");
  int vapp_powerIndex = data.indexOf("Apparent Power:");
  int vpower_factIndex = data.indexOf("Power Factor:");

  if (vvoltageIndex != -1)
  {
    V_voltage = data.substring(vvoltageIndex + 8, data.indexOf("V", vvoltageIndex)).toFloat();
  }

  if (vcurrentIndex != -1)
  {
    V_current = data.substring(vcurrentIndex + 8, data.indexOf("A", vcurrentIndex)).toFloat();
  }

  if (vpower_factIndex != -1)
  {
    V_power_fact = data.substring(vpower_factIndex + 13, data.indexOf("\n", vpower_factIndex)).toFloat();
  }

  if (vapp_powerIndex != -1)
  {
    V_app_power = data.substring(vapp_powerIndex + 15, data.indexOf("VA", vapp_powerIndex)).toFloat();
    V_act_power = V_app_power * V_power_fact;
    V_react_power = sqrt(pow(V_app_power,2) - pow(V_act_power,2));
  }
  if (V_current >= 0.5)
  {
    V_cycle = 1;
  }
  if (V_current <= 0.5)
  {
    V_cycle = 0;
  }
}

// Function to Fridge data
void fridgeData(String data) {
  // Example: "Voltage: 230.0 V Current: 10.00 A Power: 2300.00 W"
  int fvoltageIndex = data.indexOf("Voltage:");
  int fcurrentIndex = data.indexOf("Current:");
  int fapp_powerIndex = data.indexOf("Apparent Power:");
  int fpower_factIndex = data.indexOf("Power Factor:");

  if (fvoltageIndex != -1)
  {
    F_voltage = data.substring(fvoltageIndex + 8, data.indexOf("V", fvoltageIndex)).toFloat();
  }

  if (fcurrentIndex != -1)
  {
    F_current = data.substring(fcurrentIndex + 8, data.indexOf("A", fcurrentIndex)).toFloat();
  }

  if (fpower_factIndex != -1)
  {
    F_power_fact = data.substring(fpower_factIndex + 13, data.indexOf("\n", fpower_factIndex)).toFloat();
  }

  if (fapp_powerIndex != -1)
  {
    F_app_power = data.substring(fapp_powerIndex + 15, data.indexOf("VA", fapp_powerIndex)).toFloat();
    F_act_power = F_app_power * F_power_fact;
    F_react_power = sqrt(pow(F_app_power,2) - pow(F_act_power,2));
  }
  if (F_current >= 0.5)
  {
    F_cycle = 1;
  }
  if (F_current <= 0.5)
  {
    F_cycle = 0;
  }
}
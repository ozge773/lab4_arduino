#include <ThingSpeak.h>

#include <WiFiProv.h>

#include <WiFi.h>
#include <Wire.h>
#include <ThingSpeak.h>

// Thingspeak is initialized at the end of wifiSetup()
// Writing to Thingspeak is to be implemented in part B

struct imu_sensor {
  float x=0;
  float y=0;
  float z=0;
  float temp_c=0;
  float temp_f=0;
};

struct pressure_sensor {
  float temp_c=0;
  float temp_f=0;
  float pressure_hp=0;
};

float ard_millis=0; // loop duration from arduino
float prev_millis=0; // For timers

imu_sensor a; // Structs for message parsing use
imu_sensor g; // Do not use
imu_sensor m;
pressure_sensor b;

imu_sensor accel; // Structs for manipulation
imu_sensor gyro;  // Use to get data from sensors
imu_sensor magno;
imu_sensor imu_temp;
pressure_sensor bmp_280;

float estimated_altitude = 0;

//****************************************************************************
// Begin Wifi Implementation
// Feel free to see or change how the esp32 connects to PAWS-Secure,
// but this code is provided functional.
//
// To authenticate with the school's network, the user must supply EAP_IDENTITY,
// EAP_USERNAME and EAP_PASSWORD.
// SSO = Single Sign On, the account used for most logins at UGA
// EAP_IDENTITY = SSO username
// EAP_USERNAME = SSO username
// EAP_PASSWORD = SSO password

#include "esp_wpa2.h"
//for the sake of security this part should be filled by user
#define EAP_IDENTITY ""
#define EAP_USERNAME ""
#define EAP_PASSWORD ""
#define MAX_DISCONNECTS 4
const char* ssid = "PAWS-Secure";
unsigned char disconnectNum = 0;
WiFiClient client;

// Use for writing to thingspeak
//unsigned long myChannelNumber = 1;
//const char * myWriteAPIKey = "";
//const long CHANNEL = ;
//const char *WRITE_API = "";
#define API_WRITE "F26FJA7SP55NVMTF"
#define CHANNEL_ID 1908356
//int status = 0;
//int field[8] = {1,2,3,4,5,6,7,8};
/**
 * @brief Executes when a WiFi connection event occurs.
 * 
 * @param event WiFi connection event.
 */
void WiFiStationConnected(WiFiEvent_t event){
  Serial.println("Connected to AP successfully!");
}

/**
 * @brief Executes when a IP assignment event occurs.
 * 
 * @param event IP assignment event
 */
void WiFiGotIP(WiFiEvent_t event){
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
/**
 * @brief Executes when a WiFi disconnection event occurs, automatically reconnects.
 * 
 * @param event WiFi disconnection event
 */
void WiFiStationDisconnected(WiFiEvent_t event){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection.");
  Serial.println("Trying to Reconnect");
  if (disconnectNum < MAX_DISCONNECTS) {
    WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
    delay(30000);
  }
  disconnectNum++;
}

/**
 * @brief Sets up WiFi connection.
 * 
 */
void wifiSetup() {
  // Removes previous WiFi config if any
  WiFi.disconnect(true);
  // Sets events and functions to be called when events occur
  WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  // Sets WiFi mode to STA, a device that can use 802.11 WiFi protocol
  WiFi.mode(WIFI_STA);
  // Begin WiFi
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  // Initializes Thingspeak
  ThingSpeak.begin(client);
}

// End WiFi Implementation
//****************************************************************************

/**
 * @brief Called when I2C message is recieved.
 * 
 * @param len Length of incoming message.
 */
void onReceive(int len){
  // Creates char array to store message in
  char message[len];
  // Iterator
  char num = 0;
  // Writes each received char to array at index num
  while(Wire.available()){
    message[num] = Wire.read();
    num++;
  }
  // Parses message from char array to series of sensor values
  parseMessage(message, len);
}

void setAccel(){
  accel.x = a.x;
  accel.y = a.y;
  accel.z = a.z;
}

void setGyro(){
  gyro.x = g.x;
  gyro.y = g.y;
  gyro.z = g.x;
}

void setMagno(){
  magno.x = m.x;
  magno.y = m.y;
  magno.z = m.z;
}

void setIMUTemp(){
  imu_temp.temp_c = a.temp_c;
  imu_temp.temp_f = (a.temp_c * 9/5) + 32;
}

void setBMP280(){
  bmp_280.pressure_hp = b.pressure_hp;
  bmp_280.temp_c = b.temp_c;
  bmp_280.temp_f = (b.temp_c * 9/5) + 32;
}

void updateSensors(){
  setAccel();
  setGyro();
  setMagno();
  setBMP280();
  setIMUTemp();
}

/**
 * @brief Parses Arduino I2C messages into suitable data.
 * 
 * @param message Message from the Arduino.
 * @param len Length of recieved message.
 * @return true If parse is succesful.
 * @return false If parse is unsuccesful.
 */
bool parseMessage(char message[], int len) {
  char *end_ptr;
  char msg[len-1];
  for(int i=1; i < len; i++){
    msg[i-1] = message[i];
  }
  switch (message[0]){
  case 'A':
    a.x = strtof(msg, &end_ptr);
    a.y = strtof(end_ptr, &end_ptr);
    a.z = strtof(end_ptr, NULL);
    return true;
  case 'G':
    g.x = strtof(msg, &end_ptr);
    g.y = strtof(end_ptr, &end_ptr);
    g.z = strtof(end_ptr, NULL);
    return true;
  case 'M':
    m.x = strtof(msg, &end_ptr);
    m.y = strtof(end_ptr, &end_ptr);
    m.z = strtof(end_ptr, NULL);
    return true;
  case 'P':
    b.temp_c = strtof(msg, &end_ptr);
    b.pressure_hp = strtof(end_ptr, &end_ptr);
    ard_millis = strtof(end_ptr, &end_ptr);
    a.temp_c = strtof(end_ptr, NULL);
    return true;
  default:
    return false;
  }
}

/**
 * @brief Prints all measured values once per execution, in a comma seperated format.
 */
void printInputCSV(){
  Serial.print(a.x);
  Serial.print(',');
  Serial.print(a.y);
  Serial.print(',');
  Serial.print(a.z);
  Serial.print(',');
  Serial.print(g.x);
  Serial.print(',');
  Serial.print(g.y);
  Serial.print(',');
  Serial.print(g.z);
  Serial.print(',');
  Serial.print(m.x);
  Serial.print(',');
  Serial.print(m.y);
  Serial.print(',');
  Serial.print(m.z);
  Serial.print(',');
  Serial.print(b.temp_c);
  Serial.print(',');
  Serial.print(b.pressure_hp,4);
  Serial.print(',');
  //Serial.print(ard_millis,0);
  //Serial.print(',');
  Serial.println(a.temp_c);
}

void setup() {
  // Begins Serial
  Serial.begin(115200);
  // Begins WiFi
  wifiSetup();
  // Sets I2C message reception behavior
  Wire.onReceive(onReceive);
  // Begins I2C
  Wire.setPins(5,4);
  Wire.begin(uint8_t(0x1A));
  // Initialize prev_millis
  prev_millis = millis();
}
void thingspeak()
{
ThingSpeak.setField(1,float (m.x));
ThingSpeak.setField(2,float (m.y));
ThingSpeak.setField(3,float (m.z));//3
ThingSpeak.setField(4,float (a.temp_c));//4.1
ThingSpeak.setField(5,float (b.temp_c));//4.2
ThingSpeak.setField(6,float (b.pressure_hp));//5
ThingSpeak.setField(7,float (accel.x));//1
ThingSpeak.setField(8,float (g.x));//2
ThingSpeak.writeFields(CHANNEL_ID, API_WRITE); 

delay(15000);
}
void loop() {
  // Sets Accel, Gyro, Magno and BMP280 structs once per loop from most recent data.
  updateSensors();

  // Print sensor outputs as a comma seperated list
  printInputCSV();
  thingspeak();

  delay(15000);
}

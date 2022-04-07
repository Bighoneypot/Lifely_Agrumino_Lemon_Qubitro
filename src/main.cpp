
/*Lifely Agrumino Lemon For Qubitro - Sample project to use Agrumino Lemon Library and Qubitro IoT Platform.
  Created by gabriele83x@gmail.com     April 2022
  Edit private.h for wifi setting and Qubitro settings (Device id, device token, Mqtt parameters)
  Send data (illuminance, temperature, soilmoisture, battery Voltage, battery Level, Last status watering) in Qubitro Portal
  and shows them in the dashboard.
  Is possible to enable automatic watering 
  To use this firmware required Agrumino Lemon with pump https://bit.ly/3JnuXYC
  and Qubitro Account https://bit.ly/3rawiM3
  */

#include <Agrumino.h>
#include <ESP8266WiFi.h>
#include <private.h>
#include <QubitroMqttClient.h>
#define DEBUG_WIFI_LOCATION 1
#define BAUDRATE 115200 //Serial baudrate
#define MS_TO_MIN (1000000 * 60) //Conv ms to minute
unsigned int time_to_send_ms = 30000; //Set this time in ms
unsigned int sleep_time_min = 2; //Set deepsleep time in minute, min 1 max 60 minutes. *****Work only deepsleep = true***
bool deepsleepbool= true; //Set deepsleep= true if you want to save battery power
long int deviceName = ESP.getChipId();//Your Agrumino Chip ID
IPAddress ip_add;

///Watering
bool enable_watering = true; // if you want automatic watering set true
int threshold_soil_moisture_perc = 25;//soil moisture limit threshold for watering, value in % 
int watering_time = 5000; //Watering time in ms. On/Off pump 3.7V
int watering_enabled = 0; //Don't change

Agrumino agrumino;

///Agrumino data
float temperature = 0.0;
unsigned int soilMoisture = 0;
float illuminance = 0.0;
float batteryVoltage = 0.0;
unsigned int batteryLevel = 0;

WiFiClient agruminoWifiClient;
QubitroMqttClient agruminoMqttClient(agruminoWifiClient);


void agruminoData(){
  temperature = agrumino.readTempC();
  soilMoisture = agrumino.readSoil();
  illuminance = agrumino.readLux();
  batteryVoltage = agrumino.readBatteryVoltage();
  batteryLevel = agrumino.readBatteryLevel();
}

void setupWiFi() {

  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.begin(ssid, password);
  Serial.print("Try to connect to ssid :  " + String(ssid) );
  while(true)
  {
    delay(1000);
    Serial.print(".");
    if (WiFi.status() == WL_CONNECTED)
    {
      ip_add= WiFi.localIP();
      Serial.print("RSSI = " + String(WiFi.RSSI()) + " WiFi Connected, local IP : ");
      Serial.print(ip_add);
      break;
    }
  }
}


void setupQubitroMqtt() {
  agruminoMqttClient.setId(deviceID);
  agruminoMqttClient.setDeviceIdToken(deviceID, deviceToken);
  Serial.println(" Connecting to Qubitro...");
  if (!agruminoMqttClient.connect(host, port))
  {
    Serial.print("Connection failed. Error code: " + String(agruminoMqttClient.connectError()));
  }
  Serial.print(" Connected to Qubitro.");
  agruminoMqttClient.subscribe(deviceID);
}

void watering(){

  if(enable_watering == true && soilMoisture <= threshold_soil_moisture_perc) {

    agrumino.turnWateringOn();
    Serial.println("Start Watering ");
    delay(watering_time);
    agrumino.turnWateringOff();
    Serial.println("End watering ");
    watering_enabled = 1;

  }

}

void setup() {

  Serial.begin(BAUDRATE);
  agrumino.setup();
  setupWiFi();
  delay(300);
  setupQubitroMqtt();
  
  }

void loop() {

  agrumino.turnBoardOn(); 
  delay(200);
  agruminoData();
  watering();
  String myDeviceData = "{\"Device Name\":" +  String(deviceName) + ",\"Temperature\":" + String(temperature)    + ",\"Soilmoisture %\":" + String(soilMoisture) + ",\"Illuminance\":" + String(illuminance) +   ",\"Battery Voltage\":" + String(batteryVoltage) + ",\"Watering enabled \":" + String(watering_enabled) + ",\"Battery Level\":" + String(batteryLevel) + "}";
  Serial.println(myDeviceData); //Print data in serial monitor
  agruminoMqttClient.poll();
  agruminoMqttClient.beginMessage(deviceID);
  agruminoMqttClient.print(myDeviceData);
  agruminoMqttClient.endMessage();
  delay(1000);
  agruminoMqttClient.flush();
  if(deepsleepbool == false){
    delay(time_to_send_ms); 
  }
  watering_enabled = 0;
  if (deepsleepbool == true)
  { Serial.println("I go into deep sleep, and I wake up in  " + String(sleep_time_min) + "  minutes");
    WiFi.mode(WIFI_OFF);
    ESP.deepSleep(MS_TO_MIN * sleep_time_min);

}
}
/***************************************************
  Adafruit MQTT Library ESP8266 and Other Librarys

 ****************************************************/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <JSON_Decoder.h>
#include <DarkSkyWeather.h>
#include <ArduinoJson.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "WIFI SSID"
#define WLAN_PASS       "WIFI PASSWORD"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "adafruit username"
#define AIO_KEY         "*********KEY********** "

/************************* DarkSky Weather Setup *********************************/

String api_key = "******darksky api key*******";
String latitude =  "*******";
String longitude = "*******";
String units = "si";
String language = "en";

DS_Weather dsw;

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

float hometemp;
float outsidetemp;
float homehumidit;
String result;


/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
// Setup a feed called 'temperature', 'humidity', 'hometemp' for Publishing respective values.
Adafruit_MQTT_Publish tempe = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humid = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish home_temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/hometemp");

// Setup a feed called 'temperature' for subscribing to changes.
//Adafruit_MQTT_Subscribe tempmax = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/temperature");

/*************************** Sketch Code ************************************/

void setup() {
  Wire.begin(0, 2); 
  Serial.begin(9600);
  // Connect to WiFi access point.
  Serial.print(" \n");
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID); 

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());


 // Serial.println(F("BME280 test"));
  unsigned status;
  status = bme.begin();
    
 //   Serial.println("-- Default Test --");
 //   Serial.println(); 
  printValues();
  
  printCurrentWeather();
      Serial.print(F("Inside Temperature : "));
      Serial.println(hometemp);
      Serial.print(F("Outside Temperature : "));
      Serial.println(outsidetemp);
      Serial.print(F("Humidity: "));
      Serial.println(homehumidit); 

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

      Serial.println();
    if (! home_temp.publish(hometemp)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }

       if (! humid.publish(homehumidit)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
  // This is to publish only valid temperature provided by DarkSky api.    
    if (outsidetemp >= 1){
      if (! tempe.publish(outsidetemp)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
      }
      }

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    
    }
    
 // DeepSleep for 10 min including the processing and delay time.    
 // Serial.println("going to sleep now ");
  ESP.deepSleep(620*1000000);
}

void loop() {
  
  }

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

//  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
//  Serial.println("MQTT Connected!");
}

// Function to get temperature value from DarkSky api.
void printCurrentWeather()
{
  // Create the structures that hold the retrieved weather
  DSW_current *current = new DSW_current;
  DSW_hourly *hourly = new DSW_hourly;
  DSW_daily  *daily = new DSW_daily;

 // time_t time;

//  Serial.print("\nRequesting weather information from DarkSky.net... ");

  dsw.getForecast(current, hourly, daily, api_key, latitude, longitude, units, language);

//  Serial.println("Weather from Dark Sky\n");

  // We can use the timezone to set the offset eventually...
  // Serial.print("Timezone            : "); Serial.println(current->timezone);
  
 // Serial.println("############### Current weather ###############\n");
  outsidetemp = current->temperature;

//  Serial.println();

  // Delete to free up space and prevent fragmentation as strings change in length
  delete current;
  delete hourly;
  delete daily;

}

// Function to get temperature and humidity values from BMP 280 sensor.

void printValues() {
    hometemp = bme.readTemperature();
   /* Serial.print("Temperature = ");
    Serial.print(hometemp);
    Serial.println(" *C");
    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");  */
    homehumidit = bme.readHumidity();
  /*  Serial.print("Humidity = ");
    Serial.print(homehumidit);
    Serial.println(" %"); */
}

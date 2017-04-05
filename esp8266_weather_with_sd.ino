/**
   03/07/2017
   Arthur A Garcia
   This project will connect an ESP8266 to a display and DHT11 Temperature sensor
   The data will then be sent to Azure and an alert will be generated
   DEVICE : Node MCU ESP8266 (0.9)
   
   3-10-17 : added url and device id to sd card reader
*/


// include wifi library for nodemcu 8266

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <ESP8266WiFi.h>
// need this lib for Secure SSL for ESP 8266 chip
#include <WiFiClientSecure.h>  

// include SD library
#include <SD.h>

// new ping
#include <NewPing.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include <SPI.h>

// http://easycoding.tn/tuniot/demos/code/
// D3 -> SDA
// D4 -> SCL      display( address of display, SDA,SCL)
#include "SSD1306.h"
SSD1306  display(0x3C, 2, 0);

// common include file with additional user functions ise 
// To use tabs with a .h extension, you need to #include it (using "double quotes" not <angle brackets>).     
#include "esp8266_common.h"                 

// for dht11 temp sensor on esp8266 chip
#include <DHT.h>
#define DHTPIN 4 //D4 on nodemcu 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//File dataFile;
 
String netid,pwd,deviceId,url;
long duration, distance, lastDistance;

String passData[4];


#define TRIGGER_PIN 1
#define ECHO_PIN 3
#define MAX_DISTANCE 400

NewPing sonar(TRIGGER_PIN,ECHO_PIN,MAX_DISTANCE);

void setup() {

  Serial.begin(9600);
  Serial.println("");

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  // init done
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  // stup for display
  display.init();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  sendToDisplay(0, 0, "Screen Init");
  delay(2000);
  
  sendToDisplay(0,0,"Init SD Card");
  
  // get data from sd card
  // passing an array to house sd card information
  getSDData(passData);
 
  // move sd card data to global variables
  netid = passData[0];
  pwd = passData[1];
  deviceId = passData[2];
  url = passData[3];

  // verify variables from sd card got into globals
  Serial.print("NETID:");
  Serial.println(netid);
  Serial.print("PWD:");
  Serial.println(pwd);
  Serial.print("DEVICEID:");
  Serial.println(deviceId);
  Serial.print("URL:");
  Serial.println(url);

  // initialize wifi
  WiFi.disconnect();
  WiFi.begin( (const char*)netid.c_str() , (const char*)pwd.c_str() );

  display.clear();
  Serial.print("Connecting to SSID:");
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.macAddress() );
  
  sendToDisplay(0, 0, "Connecting:" + netid);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
	
	switch (WiFi.status())	
	{
	case WL_CONNECTION_LOST:
		Serial.println("Connection Lost");
		break;
	case WL_CONNECT_FAILED:
		Serial.println("Connection Failed");
		break;
	case WL_DISCONNECTED:
		Serial.println(" Not Connected");
		break;
	default:
		Serial.print("Status:");
		Serial.println(WiFi.status());
		break;
	}

    sendToDisplay(0,15,"...");

  }

  // confirm connection to WiFi
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  display.clear();
  sendToDisplay(0,0,"Connected:" + netid);

  //Define inputs and outputs
  //pinMode(TRIGGER_PIN, OUTPUT);
  //pinMode(ECHO_PIN, INPUT);
  
  // start temp sensor
  dht.begin();
  
  // start time client - used to get curren time.
  timeClient.begin();

}



void loop() {

  Serial.println("");
  Serial.println("Loop");

  timeClient.update();
  Serial.println(timeClient.getEpochTime());

  display.clear();
  sendToDisplay(0,0, "Temp from:" + deviceId);

  float humidity = dht.readHumidity();
  delay(200);
  float t = dht.readTemperature();
  // fahrenheit = t * 1.8 + 32.0;
  float temp = t *1.8 + 32;
  delay(200);
  
  sendToDisplay(0, 18, "Temp:" + String(temp));

  sendToDisplay(0, 32, "Humidity:" + String(humidity) + "%" );

  Serial.print("Temperature:");
  Serial.println(temp);

  Serial.print("Humidity:");
  Serial.println(humidity);

  String key = (String)timeClient.getEpochTime();
  
  // format data into Json object to pass to Azure
  //String myKeys[] = { "deviceId","Temp","Humidity","KeyValue" };

  String tempJson = createJsonData(deviceId, temp, humidity,key);
 
  // send json to Azure
  httpRequest("POST", url, "application/atom+xml;type=entry;charset=utf-8", tempJson);
  
  sendToDisplay(0,45,"key:" + key);
  
  Serial.print("Json Sent:");
  Serial.println(tempJson);
  
  //delay(60000);   // delay 60000 miliseconds = 60 sec
   delay(10000);

   Serial.println("Get Distance");
   getDistance(TRIGGER_PIN,ECHO_PIN);
}

void sendToDisplay( int col,int row, String data)
{
	display.drawString(col, row, data);
    display.display();
}



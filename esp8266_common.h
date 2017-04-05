#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <SD.h>

// Include the correct display library
// For a connection via I2C using Wire include
#include <SPI.h>

WiFiUDP ntpUDP;

// setup https client for 8266 SSL client
WiFiClientSecure client;

// endpoint to use to send message /devices/{device name}/messages/events?api-version=2016-02-03
// host name address for your Azure IoT Hub
// on device monitor generate a sas token on config page.
//String uri = "/devices/esp8266v2/messages/events?api-version=2016-02-03";
char hostnme[] = "ArtTempIOT.azure-devices.net";
char authSAS[] = "SharedAccessSignature sr=ArtTempIOT.azure-devices.net&sig=vmUF6p3IANfHmNWrvk4Zf%2BlpngD365hUX9f%2FB2zNaUM%3D&se=1515799811&skn=iothubowner";

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
  
// declare json functions
String createJsonData(String devId, float temp ,float humidity,String keyid);



String createJsonData(String devId, float temp, float humidity,String keyid)
{

  // create json object
  StaticJsonBuffer<200> jsonBuffer;
  char* buff;
  String outdata;

  JsonObject& root = jsonBuffer.createObject();
  root["DeviceId"] = devId;
  root["KeyId"] = keyid;
  root["temperature"] = temp;
  root["humidity"] = humidity;

  // convert to string
  root.printTo(outdata);
  return outdata;

}


void httpRequest(String verb, String uri, String contentType, String content)
{
	Serial.println("--- Start Process --- ");
	if (verb.equals("")) return;
	if (uri.equals("")) return;

	// close any connection before send a new request.
	// This will free the socket on the WiFi shield
	client.stop();

	// if there's a successful connection:
	if (client.connect(hostnme, 443)) {
		Serial.println("--- We are Connected --- ");
		Serial.print("*** Sending Data To:  ");
		Serial.println(hostnme + uri);

		Serial.print("*** Data To Send:  ");
		Serial.println(content);

		client.print(verb); //send POST, GET or DELETE
		client.print(" ");
		client.print(uri);  // any of the URI
		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(hostnme);  //with hostname header
		client.print("Authorization: ");
		client.println(authSAS);  //Authorization SAS token obtained from Azure IoT device explorer
								  //client.println("Connection: close");

		if (verb.equals("POST"))
		{
			Serial.println("--- Sending POST ----");
			client.print("Content-Type: ");
			client.println(contentType);
			client.print("Content-Length: ");
			client.println(content.length());
			client.println();
			client.println(content);
		}
		else
		{
			Serial.println("--- client status --- ");
			Serial.println(client.status());

			client.println();

		}
	}

	while (client.available()) {
		char c = client.read();
		Serial.write(c);
	}

	Serial.println("--- Send complete ----");
}

void getDistance(int TRIGGER_PIN,int ECHO_PIN)
{
  long duration, cm, inches;
  Serial.println("In GetDistance");
  Serial.print("trigger:");
  Serial.println(TRIGGER_PIN);
  
  Serial.print("Echo:");
  Serial.println(ECHO_PIN);
  
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(ECHO_PIN, INPUT);
  duration = pulseIn(ECHO_PIN, HIGH);

  // convert the time into a distance
  cm = (duration/2) / 29.1;
  inches = (duration/2) / 74; 

  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

}

void getSDData(String *passData)
{
	String str, netid, pwd, deviceId, url;

	File dataFile;
	Serial.println("In getSDData");

	// initialize sd card 
	// for nodemcu use begin() else use begin(4)
	Serial.print("Initializing SD card...");

	if (!SD.begin()) {
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	// open the file. note that only one file can be open at a time,
	// so you have to close this one before opening another.
	dataFile = SD.open("wifiFile.txt");
	int index = 0;

	if (dataFile)
	{
		Serial.println("data from sd card");
		while (dataFile.available())
		{
			if (dataFile.find("SSID:"))
			{
				str = dataFile.readStringUntil('|');
				netid = str;
				Serial.println(netid);
				sendToDisplay(0, 15, netid);
			}
			if (dataFile.find("PASSWORD:"))
			{
				str = dataFile.readStringUntil('|');
				pwd = str;
				Serial.println(pwd);
				sendToDisplay(30, 15, pwd);
			}
			if (dataFile.find("DEVICEID:"))
			{
				str = dataFile.readStringUntil('|');
				deviceId = str;
				Serial.println(deviceId);
				sendToDisplay(0, 30, deviceId);
			}
			if (dataFile.find("URL:"))
			{
				str = dataFile.readStringUntil('|');
				url = str;
				Serial.println(url);
				sendToDisplay(50, 30, url);
			}
		}
		// close the file
		dataFile.close();
	}
	passData[0] = netid;
	passData[1] = pwd;
	passData[2] = deviceId;
	passData[3] = url;


}





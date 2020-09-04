#include <ESP8266WiFi.h>
#include <WiFiClient.h>;
#include <ThingSpeak.h>;
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>
//for dht sensor part
#define DHT_dpin 0 // Analog Pin sensor is connected to
#define DHTTYPE DHT11

//for network connection to wifi
#define WIFI_SSID "Mi A2a"
#define WIFI_PASS "bb819361dac4"

//for mqtt coneection to adafruit
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "sacket201987"
#define MQTT_PASS "aio_MKJT63cw0EMe1Xg1q08J8iEdRiRc"


int ir;
int intr;
float TEMP;
float HMDT;
DHT dht(DHT_dpin,DHTTYPE);

//Set up MQTT and WiFi clients
WiFiClient client;
unsigned long myChannelNumber = 1123373; //Your Channel Number (Without Brackets)

const char * myWriteAPIKey = "5ZOQTZVCV8WAG03X"; //Your Write API Key for thingspeak channel

Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/onoff");
Adafruit_MQTT_Subscribe fan = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/fan");
void MQTT_connect();
void setup()
{
  pinMode(D4, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(A0, INPUT);
  pinMode(D6, OUTPUT);
  //digitalWrite(D4, HIGH);
  dht.begin();
  //dht.begin dht sensor
  Serial.begin(9600);

  //Connect to WiFi
  Serial.print("\n\nConnecting Wifi... ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("OK!");
  mqtt.subscribe(&onoff);
  mqtt.subscribe(&fan);
   //Subscribe to the onoff feed
  ThingSpeak.begin(client);
  //connect to thingspeak
 
}

void loop()
{
  MQTT_connect();

  //Read from our subscription queue until we run out, or
  //wait up to 5 seconds for subscription to update
  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    //If we're in here, a subscription updated...
    if (subscription == &onoff)
    {
      //Print the new value to the serial monitor
      Serial.print("led1: ");
      Serial.println((char*) onoff.lastread);

      //If the new value is  "ON", turn the light on.
      //Otherwise, turn it off.
      if (strcmp((char*) onoff.lastread, "OFF"))
      {
        //Active low logic
        digitalWrite(D4, HIGH);
        Serial.println("Light is ON");
      }
      else if (strcmp((char*) onoff.lastread, "ON"))
      {
        digitalWrite(D4, LOW);
        Serial.println("Light is OFF");
      }
    }
    else if (subscription == &fan)
    {
      //Print the new value to the serial monitor
      Serial.print("FAN: ");
      Serial.println((char*) fan.lastread);

      //If the new value is  "ON", turn the light on.
      //Otherwise, turn it off.
      if (strcmp((char*) fan.lastread, "OFF"))
      {
        //Active low logic
        digitalWrite(D6, HIGH);
        Serial.println("Fan is ON");
      }
      else if (strcmp((char*) fan.lastread, "ON"))
      {
        digitalWrite(D6, LOW);
        Serial.println("Fan is OFF");
      }
    }
  }//Adafruit_MQTT_Subscribe * subscription;
 
  
  intr = digitalRead(D3);
  if (intr == 1)
  {
    digitalWrite(D7, HIGH);
  }
  else
  {
    digitalWrite(D7, LOW);
  }
  ThingSpeak.writeField(1123373, 1,intr,"5ZOQTZVCV8WAG03X"); //Update in ThingSpeak
  ir = digitalRead(D2);
  if (ir == 0)
  {
    digitalWrite(D1, HIGH);
  }
  else
  {
    digitalWrite(D1, LOW);
  }
  ThingSpeak.writeField(1123373, 2,ir,"5ZOQTZVCV8WAG03X"); //Update in ThingSpeak
  TEMP = dht.readTemperature();
  int n = digitalRead(D6);
  if(TEMP>float(30) && n==0)
  {
    digitalWrite(D6, HIGH);
  }
  ThingSpeak.writeField(1123373, 3,TEMP,"5ZOQTZVCV8WAG03X"); //Update in ThingSpeak
  HMDT = dht.readHumidity();
  ThingSpeak.writeField(1123373, 4,HMDT,"5ZOQTZVCV8WAG03X"); //Update in ThingSpeak
  
  delay(1000);
  // ping the server to keep the mqtt connection alive
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }
}

void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

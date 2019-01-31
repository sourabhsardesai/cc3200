#include <SPI.h>
//#include <Ethernet.h>
#include <GSM.h>
#include <PubSubClient.h>


// PIN Number
#define PINNUMBER ""
// APN data
#define GPRS_APN       "airtelgprs.com" // replace your GPRS APN
#define GPRS_LOGIN     ""    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password


char* outTopic     = "Home/Room/temp"; // * MQTT channel where physical updates are published
char* inTopic      = "Home/Room/Light"; // * MQTT channel where lelylan updates are received
char* clientId     = "Hitron_arduino";
char* payloadOn  = "on";
char* payloadOn1  = "temp";
char* payloadOff = "off";


bool state;
char buf1[30];


byte mac[] = { 0xA1, 0xA0, 0xBA, 0xAC, 0xEE, 0x12 };
byte server[] = {54,238,179,124}; // (hitron server)
//EthernetClient ethClient;
GPRS gprs;
GSM gsmAccess;
GSMClient ethClient;


void callback(char* topic, byte* payload, unsigned int length); // subscription callback
PubSubClient client(server, 1883, callback, ethClient);         // mqtt client


void setup()
{
  Serial.begin(9600);
 
   delay(500);
   Serial.println("Starting Arduino web client.");
   pinMode(A0, INPUT);        //lm
   pinMode(8, OUTPUT);        //
 // connection state
  boolean notConnected = true;

  while (notConnected) 
  {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) 
        {
          notConnected = false;
          Serial.println(" GPRS attach successful");
        } 
        else 
        {
         Serial.println("Not connected");
         delay(1000);
        }
  }
  
  if (client.connect("arduinoClient", "", "")) 
  {
    Serial.println("Publishing data on mqtt server");
    client.publish("outTopic","hello world");
    client.subscribe("inTopic");
   
  }
}

      
      
void msg_Subscribe()
{
  client.subscribe(inTopic);
}


void loop()   
   {
  char buf1[30];    
  int LM35 = analogRead(A0);
  int temp = LM35*0.4808;
  Serial.println(temp);
  delay(100);
  String k=String(temp);
  k.toCharArray(buf1,30);
  if(temp>28)
    {
      client.publish(outTopic,buf1);
      
    }

   client.loop();
   delay(2000);
  }
  

void callback(char* topic, byte* payload, unsigned int length) {
  // copy the payload content into a char*
  char* json;
  json = (char*) malloc(length + 1);
  memcpy(json, payload, length);
  json[length] = '\0';
  
  
if (String(payloadOn1) == String(json))
    {
    Serial.println("Sending the data to server");
    String k=String((analogRead(A0)),DEC);
    k.toCharArray(buf1,30);
    Serial.println(buf1);
    client.publish(outTopic, buf1); // light off
    }

if (String(payloadOn) == String(json))
    {
    state = HIGH;
    digitalWrite(8,HIGH);
    Serial.println("LIGHTS ON");
    client.publish(outTopic,"::::::Yes! turned on The LIGHT::::");
    }
  
if (String(payloadOff) == String(json))
    {
     state = LOW;
     digitalWrite(8,LOW);
     Serial.println("LIGHTS OFF");
     client.publish(outTopic,"::::Lights are turned off:::::"); 
    }

  free(json);
}

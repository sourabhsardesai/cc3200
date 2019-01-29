/*
 Hybrid code for starting and cutting off the wifi of cc3200 
 - This code can be complied on energia 
 
*/
#include <stdio.h>
#include <stdint.h>
#include <inc/hw_types.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
#include <driverlib/prcm.h>
#include <driverlib/wdt.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/interrupt.h>


#if defined(USE_TIRTOS) || defined(USE_FREERTOS) || defined(SL_PLATFORM_MULTI_THREADED)
#include "osi.h"
#endif

volatile unsigned long g_ulWatchdogCycles;
#define WD_PERIOD_MS                 30000
#define MAP_SysCtlClockGet           80000000

#define MILLISECONDS_TO_TICKS(ms)    ((MAP_SysCtlClockGet / 1000) * (ms))




#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

// your network name also called SSID
char ssid[] = "iPhone";
// your network password
char password[] = "********";
// MQTTServer to use
char server[] = "your mqtt broker ip address";

byte buffer[128];

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Got a message, publishing it");
  // If length is larger than 128,
  // then make sure we do not write more then our buffer can handle
  if(length > 128) length = 128;
  
  memcpy(buffer, payload, length);
  client.publish("outTopic", buffer, length);
}

void setup()
{
  Serial.begin(115200);
  pinMode(9,INPUT);
 // MAP_PRCMLPDSWakeupSourceEnable(PRCM_LPDS_GPIO);
  MAP_PRCMLPDSWakeupSourceEnable(9);
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  // Print the WiFi status.
  printWifiStatus();
}

void loop()
{
  // Reconnect if the connection was lost
  
//  
delay(20000);
  Serial.println("disconect");
 
  client.disconnect();
//  
    sl_WlanDisconnect();
    sl_Stop(10000);
//delay(2000);
// //while(1);
sleep(20000);
  Serial.println("running");
  
  if(Serial.available())
 {
  reeconnect();
 } 
  // Check if any message were received
  // on the topic we subsrcived to
  client.poll();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void reeconnect()
{
  if (!client.connected()) {
    sl_Start(NULL, NULL, NULL);

    //sl_Start(100);
    delay(1000);
     WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  // We are connected and have an IP address.
  // Print the WiFi status.
  printWifiStatus();
    Serial.println("Disconnected. Reconnecting....");

    if(!client.connect("energiaClient")) {
      Serial.println("Connection failed");
    } else {
      Serial.println("Connection success");
      if(client.subscribe("inTopic")) {
        Serial.println("Subscription successfull");
      }
    }
  }
}

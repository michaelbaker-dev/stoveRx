// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>
#include <WiFi101.h>
#include "Wire.h"
#include "Adafruit_LiquidCrystal.h"
#include <string.h>

#define csPin 8
int val = 0;
int status = WL_IDLE_STATUS;

/* for feather32u4 */
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 5

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

#define RADIO_POWER_LEVEL 23

// Blinky on receipt
#define LED 13

//used for getting battery level
#define VBATPIN A7
//Vars for displaying battery level
float batt_val = 0;
char batt_str[5];

// Should be a message for us now
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);

//WiFi Setup
//char auth[] = "aad942ded4404806b6b926105413eef3"; //Blynk code
char serverName[] = "blynk-cloud.com";
char ssid[] = "LakeHouseFamilyRoom";
char pass[] = "lakehouseA1";

//harrison network
//char ssid[] = "LakeHouseFamilyRoom";
//char pass[] = "TC8717TCEBB91";


WiFiClient client;

boolean radio = false;

// Connect via i2c, default address #0 (A0-A2 not jumpered)
Adafruit_LiquidCrystal lcd(0);

//vars for data from radio
int stove = 0;
int outside = 0;
float battery = 0;
int batteryPct = 0;

int packets = 0;

unsigned long start, finished, elapsed,secCounter;

int secloop;

void updateDisplay()
{
  char tmp[50];

  lcd.clear();
  lcd.setCursor(0,0);
  sprintf (tmp, "Stove Temp = %i",stove);
  lcd.print(tmp);
  lcd.write(0xDF);

  lcd.setCursor(0,1);
  sprintf (tmp, "Outside Temp = %i",outside);
  lcd.print(tmp);
  lcd.write(0xDF);

  lcd.setCursor(0,2);
  sprintf (tmp, "Battery Volt = %.2f",battery);
  lcd.print(tmp);

  lcd.setCursor(0,3);
  sprintf (tmp, "Battery = %i%%",batteryPct);
  lcd.print(tmp);

  lcd.setCursor(15,3);
  sprintf (tmp, "%i",rf95.lastRssi());
  lcd.print(tmp);
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  byte numSsid = WiFi.scanNetworks();

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet<numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    Serial.println(WiFi.encryptionType(thisNet));
  }
}

void printMacAddress() {
  // the MAC address of your Wifi shield
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}

void setupWiFi()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Setting Up...");
  digitalWrite(csPin, LOW);
  lcd.setCursor(0, 1);
  WiFi.setPins(8,7,4,2);

  //try and reset WiFi
  if ( !client.connected())
  {
    Serial.println("WiFi101 disconnect");
    WiFi.disconnect();
    Serial.println("WiFi101 end");
    WiFi.end();
  }

  // Check for the presence of the shield
  Serial.println("WiFi101 shield check for Board");

  //Try to connect to network
  status = WiFi.begin(ssid, pass);
  Serial.print("WiFi Status=");
  Serial.println (status);
  if (status == WL_NO_SHIELD) {
    Serial.println("NOT PRESENT");
    lcd.print("WiFi NOT Detected");
    return; // don't continue
  }
  Serial.println("WiFi Board Detected");
  lcd.print("WiFi Board Detected");

  for (int i = 0; i < 25; i++)
    {
      if ( WiFi.status() != WL_CONNECTED ) {
        delay ( 250 );
        Serial.print ( "." );
        delay ( 250 );
      }
    }


  // Print firmware version on the shield
  String fv = WiFi.firmwareVersion();
  Serial.print("Firmware version installed: ");
  Serial.println(fv);

  // attempt to connect using WEP encryption:
  Serial.println("Initializing Wifi...");
  printMacAddress();

  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  lcd.setCursor(0,2);
  lcd.print(ip);
}

void term ( String s)
{
  delay(1000);
  s.replace(" ","%20");
  String tData = "GET /aad942ded4404806b6b926105413eef3/update/V5?value=" + s + "%0D%0A";
  client.stop();

  // if there's a successful connection:
   if (client.connect(serverName, 80)) {
     Serial.println("Sending Data to Blynk Terminal");
     // Make a HTTP request:
     client.print(tData);
     client.println(" HTTP/1.1");
     client.println("Host: blynk-cloud.com");
     client.println("Connection: close");
     client.println();
   }
   else {
     // if you couldn't make a connection:
     Serial.println("WiFi Term failed");
     lcd.setCursor (0,2);
     lcd.print("WiFi Term Failed    ");
     //WiFi.disconnect();
     setupWiFi();
   }

   // if there are incoming bytes available
   // from the server, read them and print them:
   while (client.available()) {
     char c = client.read();
     Serial.write(c);
   }
 }

void setupRadio()
{
  lcd.setCursor(0,0);
  lcd.print("Setting Up Radio");
  pinMode(csPin, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  // manual reset
  digitalWrite(csPin,HIGH);  //********************************** EXTERNAL SPI
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  lcd.setCursor(0,1);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    lcd.setCursor (0,1);
    lcd.println("LoRa init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
  lcd.setCursor (0,2);
  lcd.print("LoRa radio init OK!");
  delay(2000);

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(RADIO_POWER_LEVEL, false);
  digitalWrite(csPin,LOW);  //*************************************** EXTERNAL SPI
}

boolean readRadio()
{
  digitalWrite(csPin,HIGH);  //*************************************** EXTERNAL SPI
  if (rf95.available())
  {
    if (rf95.recv(buf, &len))
    {
 //     RH_RF95::printBuffer("Received: ", buf, len);
      digitalWrite(LED, HIGH);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      delay(10);

      // Send a reply
      uint8_t data[] = "ACK";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent ACK");
      digitalWrite(LED, LOW);
      delay (100);
      digitalWrite(csPin,LOW);  //******************************** EXTERNAL SPI
      return true;
    }
    else
    {
      Serial.println("LoRa Receive failed");
      lcd.setCursor (0,3);
      lcd.print ("LoRa failed");
      term("Radio connect failed");
      return false;
    }
  }
  return false;
}

void sendHTML ( String s)
{
  delay(1000);
  client.stop();

  // if there's a successful connection:
   if (client.connect(serverName, 80)) {
     Serial.println("Sending Data to Blynk Controls");
     // Make a HTTP request:
     client.print(s);
     client.println(" HTTP/1.1");
     client.println("Host: blynk-cloud.com");
     client.println("Connection: close");
     client.println();
   }
   else {
     //if you couldn't make a connection:
     Serial.println("WiFi Write fail");
     lcd.setCursor (0,3);
     lcd.println("WiFi Write fail");
     //WiFi.disconnect();
     setupWiFi();
   }

  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
}

void connectToBlynk()
{

//************************* CONNECT TO BLYNK ************
  String auth = "aad942ded4404806b6b926105413eef3";
  String V0 = "GET /" + auth + "/update/V0?value=" + stove;
  String V1 = "GET /" + auth + "/update/V1?value=" + outside;
  String V2 = "GET /" + auth + "/update/V2?value=" + battery;
  String V3 = "GET /" + auth + "/update/V3?value=" + batteryPct;
  //Serial.println (V0);
  //Serial.println (V1);

  lcd.noBlink();
  lcd.setCursor(18,0);
  lcd.print("V0");
  sendHTML(V0);

  lcd.setCursor(18,0);
  lcd.print("V1");
  sendHTML(V1);

  lcd.setCursor(18,0);
  lcd.print("V2");
  sendHTML(V2);

  lcd.setCursor(18,0);
  lcd.print("V3");
  sendHTML(V3);

  lcd.setCursor(18,0);
  lcd.print("  ");
}


void loadVars()
{
  stove =  atoi( strtok((char*)buf, ",") );
  Serial.print(" Loading VARs.. Stove =");
  Serial.print(stove);
  outside = atoi (strtok(NULL, ","));
  Serial.print(" Outside = ");
  Serial.print(outside);
  battery = atof (strtok(NULL, ","));
  Serial.print(" Battery = ");
  Serial.print(battery);
  batteryPct = atoi (strtok(NULL, ","));
  Serial.print(" Battery % = ");
  Serial.println(batteryPct);
}

void setup()
{
  //while (!Serial);
  //  Serial.begin(9600);
  // set up the LCD's number of rows and columns:
  lcd.begin(20, 4);
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Keesler HiTech Stove");
  delay(2000);
  setupRadio();
  setupWiFi();

  term ("%0D%0A");
  term ("*** STARTUP ***");
}

void loop()
{
  delay (1000);
  secloop = secloop + 1;
  if (secloop > 999) secloop = 0;
  Serial.print(secloop);
  Serial.print(",");
  lcd.setCursor(19,0);
  if (secloop > 9) lcd.setCursor(18,0);
  if (secloop > 99) lcd.setCursor(17,0);
  lcd.print(secloop);

  if (readRadio() == true)
    {
      Serial.println ("********* GOT MESSAGE *******");
      loadVars();
      updateDisplay();
      if (packets == 10) {packets = 0; }
      //lcd.setCursor(19,3);
      //lcd.print (packets);
      //lcd.setCursor(0,0);
      connectToBlynk();
      finished=millis();
      elapsed=finished-start;
      //Serial.print(elapsed);
      int sec = ((elapsed + 500) / 1000);
      String s = ( "RSSI:" + (String)rf95.lastRssi() + " >> " + (String)sec + "s elapsed :" + (String)packets);
      //Serial.println (s);
      term ( "RSSI:" + (String)rf95.lastRssi() + " >> " + (String)sec + "s elapsed :" + (String)packets);
      start=millis();
      packets++;
      secloop = 0;
    }
}

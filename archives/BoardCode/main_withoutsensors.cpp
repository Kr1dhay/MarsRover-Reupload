/****************************************************************************************************************************
  UDPSendReceive.ino - Simple Arduino web server sample for SAMD21 running WiFiNINA shield
  For any WiFi shields, such as WiFiNINA W101, W102, W13x, or custom, such as ESP8266/ESP32-AT, Ethernet, etc
  
  WiFiWebServer is a library for the ESP32-based WiFi shields to run WebServer
  Based on and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Based on  and modified from Arduino WiFiNINA library https://www.arduino.cc/en/Reference/WiFiNINA
  Built by Khoi Hoang https://github.com/khoih-prog/WiFiWebServer
  Licensed under MIT license
 ***************************************************************************************************************************************/


//how to use it:
//download a random UDP sender online, upload the code, and write the corresponding IP address and port(printed in the monitor) to the UDP sender
//then type the data you want to send in the UDP sender and then send it 
//now w a s d is avalable command 

#define USE_WIFI_NINA         false
#define USE_WIFI101           true
#include <WiFiWebServer.h>
#include <WiFiUDP.h>
#include <Wire.h>

#define DEBUG_WIFI_WEBSERVER_PORT   Serial

int status = WL_IDLE_STATUS;     // the Wifi radio's status

unsigned int localPort = 55555;    //10002;  // local port to listen on
unsigned int upperPort = 52113; 

char packetBuffer[255];          // buffer to hold incoming packet
char ReplyBuffer[] = "ACK";      // a string to send back
char SendDetail[] = "J00A000B";

int LPWM = 8;
int LDIR = 10;
int RPWM = 9;
int RDIR = 11;   

WiFiUDP Udp;

char ssid[] = "EEERover";        // your network SSID (name)
char pass[] = "exhibition";
const int groupNumber = 200;

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  // you're connected now, so print out the data
  Serial.print(F("You're connected to the network, IP = "));
  Serial.println(WiFi.localIP());

  Serial.print(F("SSID: "));
  Serial.print(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F(", Signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  pinMode(LPWM,OUTPUT);
  pinMode(LDIR,OUTPUT);
  pinMode(RPWM,OUTPUT);
  pinMode(RDIR,OUTPUT);
  Serial.begin(9600);

  //Wait 10s for the serial connection before proceeding
  //This ensures you can see messages from startup() on the monitor
  //Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);  

  Serial.println(F("\nStarting Web Server"));
  


/*#if !(ESP32 || ESP8266)
  
  // check for the presence of the shield
  #if USE_WIFI_NINA
    if (WiFi.status() == WL_NO_MODULE)
  #else
    if (WiFi.status() == WL_NO_SHIELD)
  #endif
    {
      Serial.println(F("WiFi shield not present"));
      // don't continue
      while (true);
    }

  #if USE_WIFI_NINA
    String fv = WiFi.firmwareVersion();
    
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
      Serial.println(F("Please upgrade the firmware"));
    }
  #endif
  
#endif
*/
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println(F("WiFi shield not present"));
    while (true);
  }

  //Configure the static IP address if group number is set
  if (groupNumber)
    WiFi.config(IPAddress(192,168,0,groupNumber+1));

  // attempt to connect to WiFi network
  Serial.print(F("Connecting to WPA SSID: "));
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  printWifiStatus();

  Serial.println(F("\nStarting connection to server..."));
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

  Serial.print(F("Listening on port "));
  Serial.println(localPort);
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));

  
  Wire.begin();
}

void loop() 
{
  int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    Serial.print(F("Received packet of size "));
    Serial.println(packetSize);
    Serial.print(F("From "));
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(F(", port "));
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0)
    {
      packetBuffer[len] = 0;
    }

    Serial.println(F("Contents:"));
    Serial.println(packetBuffer);
    if (packetBuffer[packetSize - 1] == 'w'){
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,0);
      delay(500);
      analogWrite(LPWM,0);
      analogWrite(RPWM,0);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,0);
    }
    else if(packetBuffer[packetSize - 1] == 's'){
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,1);
      delay(500);
      analogWrite(LPWM,0);
      analogWrite(RPWM,0);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,1);
    }
    else if(packetBuffer[packetSize - 1] == 'a'){
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,1);
      delay(500);
      analogWrite(LPWM,0);
      analogWrite(RPWM,0);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,1);
    }
    else if(packetBuffer[packetSize - 1] == 'd'){
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,0);
      delay(500);
      analogWrite(LPWM,0);
      analogWrite(RPWM,0);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,0);
    }

    

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), 55125);

    Udp.write("RPLY");
    
    Udp.endPacket();
  }
}

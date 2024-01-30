/****************************************************************************************************************************
  UDPSendReceive.ino - Simple Arduino web server sample for SAMD21 running WiFiNINA shield
  For any WiFi shields, such as WiFiNINA W101, W102, W13x, or custom, such as ESP8266/ESP32-AT, Ethernet, etc
  
  WiFiWebServer is a library for the ESP32-based WiFi shields to run WebServer
  Based on and modified from ESP8266 https://github.com/esp8266/Arduino/releases
  Based on  and modified from Arduino WiFiNINA library https://www.arduino.cc/en/Reference/WiFiNINA
  Built by Khoi Hoang https://github.com/khoih-prog/WiFiWebServer
  Licensed under MIT license
  This script will be uploaded to the M0 board in order to control the rover
  To interact with the microcontroller, the user will run two pyton scripts seperately from a computer
 ***************************************************************************************************************************************/



//----------------------------------------------------------------------------------------------------------------------------------
//This section initialises any variables to be used and also connects to the EEERover network
//The M0 board is also initiallised and the pin properties are set
#define USE_WIFI_NINA         false
#define USE_WIFI101           true
#include <WiFiWebServer.h>
#include <WiFiUDP.h>
#include <Wire.h>

#define DEBUG_WIFI_WEBSERVER_PORT   Serial

int status = WL_IDLE_STATUS;     // the Wifi radio's status

unsigned int localPort = 5555;    //10002;  // local port to listen on
unsigned int upperPort = 57207; 

char packetBuffer[255];          // buffer to hold incoming packet
char ReplyBuffer[] = "ACK";      // a string to send back
char SendDetail[] = "J00A000B";

// variables associated with drivetrain
int LPWM = 8;
int LDIR = 9;
int RPWM = 11;
int RDIR = 12;   

// variables associated with ultrasound
int analogPin = A3;
int ultrasonicVal = 0;


//variables associated with infrared
const int IRin = 5;

// variables associated with the RF Sensors
// pins used
int f61kHzIn = A1;
int f89kHzIn = A2;

int f61kHzDemod = 3;
int f89kHzDemod = 13;

int pinState;

// wifi stuff
WiFiUDP Udp;

char ssid[] = "EEERover";        // your network SSID (name)
char pass[] = "exhibition";
const int groupNumber = 2;

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

//----------------------------------------------------------------------------------------------------------------------------------
// This section of the code is for the ultrasound sensor
//let potentiometer is connected at analog pin 4:

bool ultrasonicPresent = false;

bool ultrasonicDetect(){
  ultrasonicVal = analogRead (analogPin);
  Serial.println ("Ultrasound Value: " + ultrasonicVal);
  if(ultrasonicVal > 800 || ultrasonicVal < 700){
    return ultrasonicPresent = true;
  }
  else{
    return ultrasonicPresent = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
// This section of code is used for the radiofrequency sensor

class RFSensors{
    // class to hold the RF sensors specific code
    private:
        bool results[3];        // boolean array that holds three bools to specify what has been sensed.
                                // [0]: rock or not. If true, all others are do not care
                                // [1]: carrier frequency - false = 61k, true = 89k
                                // [2]: modulatiomn frequency - false = 151Hz, true = 239Hz
        int modPin;             // pin to check for calculating modulation frequency

        bool arrayMatchBool(bool target[3]){
            // function to check if the supplied array is the same as the results array
            bool result = true;
            for (int index = 0; index < 3; index++){
                if (results[index] != target[index]){
                    // not the same
                    result = false;
                    index = 4;      // stop loop
                }
            }
            return result;
        }

        int demodulatedSample(int pin){
            // function to sample demodulated wave
            // samples three times then returns the maximum
            // will treat any values less than 600mV as a 0 (this value comes from testing)
            int sample = 0;
            int temp = 0;
            for (int i = 0; i < 100; i++){
                temp = analogRead(pin);
                if (temp > 280)          // 10 bit resolution, 600mV out of 5V
                {
                    // suitable value gotten
                    // add to running average
                    sample = ((sample * i) + temp) / (i + 1);
 
                }
                delayMicroseconds(100);
                
            }
            return sample;
        }

        bool frequencyDetect(int pin){
            // detects whether demodualted signal is 239Hz or 151Hz. Does this by waiting till falling edge
            // then waiting again into the time period where the two frequencies differ and testing the input.
            // wait until signal is high

            const float upperFreq = 239.0;

            int pulseHigh, pulseLow; 
            float pulseTotal, frequency; 
  
            pulseHigh = pulseIn(pin,HIGH);
            pulseLow = pulseIn(pin,LOW);

 
            pulseTotal = pulseHigh + pulseLow; 
            frequency=1000000/pulseTotal;
                  
            // Serial.println(frequency);
            if (frequency > (upperFreq - (0.1 * upperFreq))){
                // greater than 216Hz so call 239Hz
                // Serial.println("239Hz Modulating");
                return true;
            } else {
                // Serial.println("151Hz Modulating"); 
                return false;
            }
        }

    public:
        RFSensors(){
          modPin = 0;
        }

        void sense(){
            // function that updates the results boolean array
            // check carrier frequency first
            int sample61 = demodulatedSample(f61kHzIn);
            int sample89 = demodulatedSample(f89kHzIn);
            // Serial.println("RF Samples" + sample61 + sample89);

            if ((sample61 == 0) && (sample89 == 0)){
                // Serial.println("No rock");
                results[0] = false;
                return;         // stop if no rock
            } else {
              if (sample61 > sample89){
            
                // Serial.println("61kHz Carrier");
                results[0] = true;
                results[1] = false;
              } else {
                // Serial.println("89kHz Carrier");
                results[0] = true;
                results[1] = true;
              }
            }
            // check modulation frequncy
            if (results[1]){
                // 89kHz carrier - check that side
                modPin = f89kHzDemod;
            } else {
                // 61kHz carrier
                modPin = f61kHzDemod;
            }
            results[2] = frequencyDetect(modPin);       // detirmine modulation frequency
        }

        bool rockPresent(){
            return results[0];
        }

        bool isGaborium(){
            bool target[3] = {true, false, false};
            return arrayMatchBool(target);
        }

        bool isLathwaite(){
            bool target[3] = {true, false, true};
            return arrayMatchBool(target);
        }

        bool isAdamantine(){
            bool target[3] = {true, true, false};
            return arrayMatchBool(target);
        }

        bool isXirang(){
            bool target[3] = {true, true, true};
            return arrayMatchBool(target);
        }
};



//----------------------------------------------------------------------------------------------------------------------------------
// This section of code is used for the infrared sensor


float ir_freqcheck(){
  int pulseHigh, pulseLow; 
  float pulseTotal, frequency; 
  
  pulseHigh = pulseIn(IRin,HIGH);
  pulseLow = pulseIn(IRin,LOW);
    
  pulseTotal = pulseHigh + pulseLow; 
  frequency=1000000/pulseTotal;
  return frequency;
}

boolean netherite(){
  
  float frequency = ir_freqcheck();
  Serial.println ("netherite: " + String(frequency,5));
  if( (frequency > 521) && (frequency < 621) ){
    return true;
  }
  else{
    return false;
  }
  
}

boolean thiotimoline(){

  float frequency = ir_freqcheck();
  Serial.println ("thiotimoline: " + String(frequency, 5));
  if( (frequency > 303) && (frequency < 403) ){
    return true;
  }
  else{
    return false;
  }
  
}





//----------------------------------------------------------------------------------------------------------------------------------
// This section of code is used for the Magnetic sensor
int MAG_SENSOR = A0;

// if both boolean is false, no magnetic field, move on to next sensor
bool magUP = false;
double val;

bool magUp(){
  val = 347 - analogRead(MAG_SENSOR);
  if(val >= 2){
    return  true; 
  } else {
    return false;
  } 
}

bool magDown(){
  val = 347 - analogRead(MAG_SENSOR);
   if(val <= -2){
    return true; 
   } else {
    return false;
   } 
}



//----------------------------------------------------------------------------------------------------------------------------------
// This section of code is used to deduce the type of rock from all of the sensors
String rockDetected(){

  //set up and do an RF sense
  RFSensors rfSensor;
  rfSensor.sense();

  if(rfSensor.isGaborium() && ultrasonicDetect()){
    return "Gaborium";
  }
  else if(rfSensor.isLathwaite()){
   return "Lathwhite";
  }
  else if(rfSensor.isAdamantine() && magUp()){
    return "Adamantine";
  }
  else if(rfSensor.isXirang() && magDown()){
    return "Xirang";
  }
  else if(thiotimoline()){
    return "Thiotimoline";
  }
  else if(netherite() && ultrasonicDetect()){
    return "Netherite";
  }
  else{
    return "No Rock Detected";
  }
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  pinMode(LPWM,OUTPUT);
  pinMode(LDIR,OUTPUT);
  pinMode(RPWM,OUTPUT);
  pinMode(RDIR,OUTPUT);
  pinMode(IRin,INPUT);
  pinMode(f61kHzDemod,INPUT);
  pinMode(f89kHzDemod,INPUT);
  Serial.begin(9600);

  //Wait 10s for the serial connection before proceeding
  //This ensures you can see messages from startup() on the monitor
  //Remove this for faster startup when the USB host isn't attached
  while (!Serial && millis() < 10000);    

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
    digitalWrite(LED_BUILTIN,1);
    delay(100);
    Serial.print('.');
    digitalWrite(LED_BUILTIN,0);
  }

  printWifiStatus();

  Serial.println(F("\nStarting connection to port..."));
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

  Serial.print(F("Listening on port "));
  Serial.println(localPort);
  Serial.println(static_cast<IPAddress>(WiFi.localIP()));
  digitalWrite(LED_BUILTIN, 1);
  delay(200);
  digitalWrite(LED_BUILTIN, 0);
  delay(200);
  digitalWrite(LED_BUILTIN, 1);
  delay(200);
  digitalWrite(LED_BUILTIN, 0);
  delay(200);
  digitalWrite(LED_BUILTIN, 1);
  
  Wire.begin();
}

//----------------------------------------------------------------------------------------------------------------------------------
// This section of code is the main loop
//It constantly checks for UDP signals from the user and when received the rover's movement is controlled or the sensors are activated as required

void loop() 
{
  int packetSize = Udp.parsePacket();
  Serial.println(rockDetected());
  if (packetSize)
  {
    Serial.print(F("Received packet of size "));
    Serial.println(packetSize);
    Serial.print(F("From "));
    IPAddress remoteIp = Udp.remoteIP();
    IPAddress ip(192, 168, 0, 197);
    Serial.print(remoteIp);
    Serial.print(F(", port "));
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0)
    {
      packetBuffer[len] = 0;
    }
    // print content of packet received by rover
    Serial.println(F("Contents:"));
    Serial.println(packetBuffer);
    // execute command based on packet received
    if (packetBuffer[0] == 'w'){ // forward  
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,1);
    }
    else if(packetBuffer[0] == 's'){ // backward
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,0);
    }
    else if(packetBuffer[0] == 'a'){ // left
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,1);
    }
    else if(packetBuffer[0] == 'd'){ // right
      analogWrite(LPWM,255);
      analogWrite(RPWM,255);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,0);
    }
    else if (packetBuffer[0] == 'i'){ //slow forward
      analogWrite(LPWM,50);
      analogWrite(RPWM,50);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,1);
    }
    else if(packetBuffer[0] == 'k'){ //slow backward
      analogWrite(LPWM,50);
      analogWrite(RPWM,50);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,0);
    }
    else if(packetBuffer[0] == 'j'){ //slow left
      analogWrite(LPWM,20);
      analogWrite(RPWM,20);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,1);
    }
    else if(packetBuffer[0] == 'l'){ //slow right
      analogWrite(LPWM,20);
      analogWrite(RPWM,20);
      digitalWrite(LDIR,1);
      digitalWrite(RDIR,0);
    }
    else if(packetBuffer[0] == '1'){ // stop (executed upon key release)
      analogWrite(LPWM,0);
      analogWrite(RPWM,0);
      digitalWrite(LDIR,0);
      digitalWrite(RDIR,0);
      Udp.beginPacket(ip, 60472);
      String rock = String(rockDetected());
      Udp.print("Rocks");
      
      Udp.endPacket();
    }
    else if(packetBuffer[0] == 'f'){ // detect
      Udp.beginPacket(ip, 60472);
      String rock = String(rockDetected());
      Udp.print(rock);
      
      Udp.endPacket();
    }
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), 55125);

    Udp.write("RPLY");
    
    Udp.endPacket();
  }
}

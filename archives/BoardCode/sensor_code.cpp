// follows the code flowchart
#include <Arduino.h>
// pins used
int f61kHzIn = A1;
int f89kHzIn = A2;

int f61kHzDemod = 3;
int f89kHzDemod = 13;

int pinState;

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
            Serial.println("Frequency Detect");

            const float upperFreq = 239.0;

            int pulseHigh, pulseLow; 
            float pulseTotal, frequency; 
  
            pulseHigh = pulseIn(pin,HIGH);
            pulseLow = pulseIn(pin,LOW);

 
            pulseTotal = pulseHigh + pulseLow; 
            frequency=1000000/pulseTotal;
                  
            Serial.println(frequency);
            if (frequency > (upperFreq - (0.1 * upperFreq))){
                // greater than 216Hz so call 239Hz
                Serial.println("239Hz Modulating");
                return true;
            } else {
                Serial.println("151Hz Modulating"); 
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
            Serial.println(sample61);
            Serial.println(sample89);
            if ((sample61 == 0) && (sample89 == 0)){
                Serial.println("No rock");
                results[0] = false;
                return;         // stop if no rock
            } else {
              if (sample61 > sample89){
            
                Serial.println("61kHz Carrier");
                results[0] = true;
                results[1] = false;
              } else {
                Serial.println("89kHz Carrier");
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

String RFRockDetect(RFSensors sensor){
     //RF Sensor sensing method
     //uses RF stuff to generate two bools in array that correspond to 
    sensor.sense();         // run sensors
    if(sensor.rockPresent()){
        if(sensor.isGaborium()){
            return "Gaborium";
        }
    } 
    Serial.println("Sensed");
    return "EEEEE";
    
}


void setup(){
    delay(1000);
    Serial.begin(9600);
    pinMode(f61kHzDemod, INPUT);
    pinMode(f89kHzDemod, INPUT);
    Serial.println("Rock detect test");
}

void loop(){
    RFSensors rfSense;
    while (true){
        delay(500);
        Serial.println(RFRockDetect(rfSense));
    }

}
        delay(500);
        Serial.println(RFRockDetect(rfSense));
    }

}

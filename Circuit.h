#ifndef _CIRCUIT_
#define _CIRCUIT_

// Abstraction for the IR Detector 
//
// Basic operation:
//   turn IR LED OFF and read ambient IR level
//   turn IR LED ON  and read newly bright IR level
//
//   If there is a reasonable difference, presume detection
//   Keep resetting hysteresis timeout to 0 every time there is detection
//   if no detection for longer than hysteresis, declare block as empty
//
// Onboard feedback LEDS follow real time differences in IR levels to allow tuning
// IO4-feedback follows the detection with hysteresis

#include <elapsedMillis.h>

//#define DEBUG

#define HEADROOM 40
#define HYSTERESIS 2000    // in mS units, 2000 = 2.0 seconds

#define OCCUPIED LOW
#define EMPTY    ~OCCUPIED

#define LIT 0
#define DIM 1


#include <Arduino.h>

class Circuit {
public:
    Circuit(void) {};
              //          IRTX       IRRX      IO4      ledFeedback
    void init(int number, int irout, int irin, int io4, int led) {
        num = number;
        outPin  = irout;
        inPin   = irin;
        io4Pin  = io4;
        ledPin  = led;
        
        detected = false;
        delaytime = 0;
        pinMode(outPin,  OUTPUT);
        pinMode(io4Pin,  OUTPUT);
        pinMode(ledPin,  OUTPUT);
        pinMode(inPin,   INPUT);
        
#ifdef DEBUG
        digitalWrite(io4Pin,LIT); digitalWrite(ledPin,LIT);  delay(100); 
#endif
        digitalWrite(io4Pin,DIM); // Leave the detection pin 
        digitalWrite(ledPin,DIM); // and LED OFF 
    };
    
    int check(void) {
        digitalWrite(outPin, 0);       // turn off IR transmitter
        delay(5);
        int r1 =  analogRead(inPin);   // read ambient light intensity
        
        digitalWrite(outPin, 1);       // turn ON IR source
        delay(5);
        int r2 = analogRead(inPin);    // see if anything is reflecting
        
        digitalWrite(outPin, 0);       // make sure things are turned off when done

        if ((r2 - r1) > HEADROOM) {    // if a major positive difference, something has been detected...
            delaytime = 0;  // expiration timer is reset every time detection is seen
            digitalWrite(ledPin, OCCUPIED); 
            if (detected == false) {   // newly triggered
#ifdef DEBUG
                Serial.print("ON  ");
                Serial.print(num, DEC);
                Serial.print(": ambient=");
                Serial.print(r1, DEC);
                Serial.print(", reflected=");
                Serial.print(r2, DEC);
                Serial.print(", diff=");
                Serial.print(r2-r1, DEC);
#endif
            }
            detected = true;
        } else {
            digitalWrite(ledPin, EMPTY);
        }
        if (detected) {
            if (delaytime < HYSTERESIS) {
                digitalWrite(io4Pin, OCCUPIED); 
            } else {
#ifdef DEBUG
                Serial.print("OFF "); 
                Serial.println(num, DEC);
#endif
                digitalWrite(io4Pin, EMPTY); 
                detected = false;
            }
        }
        return detected ? 1 : 0;
    };
    
private:  
    int num;
    elapsedMillis delaytime;
    int detected;
    int outPin;
    int inPin;
    int ledPin;
    int io4Pin;
};

#endif

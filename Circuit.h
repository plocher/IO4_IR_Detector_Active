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

//#define SHOW_IMMEDIATE

#define HYSTERESIS 2000    // in mS units, 2000 = 2.0 seconds

#define OCCUPIED LOW
#define EMPTY    ~OCCUPIED

#define LIT 0
#define DIM 1
#define SAMPLES 30
#define DWELL 4

#include <Arduino.h>

class Circuit {
private:
    enum State { R1, R2 };
public:
    Circuit(void) {};
              //          IRTX       IRRX      IO4      headroom, ledFeedback
    void init(int number, int irout, int irin, int io4, int head, int led = -1) {
        num = number;
        outPin  = irout;
        inPin   = irin;
        io4Pin  = io4;
        ledPin  = led;
        headroom = head;

        state = R1;
        sample  = 0;
        for (int x = 0; x < SAMPLES; x++) {
            r1[x] = r2[x] = 0;
        }
        
        detected = false;
        delaytime = 0;
        dwelltime = 0;
        pinMode(outPin,  OUTPUT);
        pinMode(io4Pin,  OUTPUT);
        pinMode(inPin,   INPUT);
        if (ledPin != -1) { 
            pinMode(ledPin,  OUTPUT);
            digitalWrite(ledPin,DIM); // and LED OFF 
        }
        digitalWrite(io4Pin,DIM); // default the detection pin to EMPTY
    };
    
    int check(void) {
        if (dwelltime > DWELL)  { // state machine ticks in increments of DWELLTIME...
            dwelltime = 0;
            
            switch (state) {
                case R1:    r1[sample] =  analogRead(inPin);
                            digitalWrite(outPin, 1);       // turn ON IR source
                            state = R2;
                            break;
                            
                case R2:    r2[sample] =  analogRead(inPin);
                            digitalWrite(outPin, 0);       // make sure things are turned off when done


                            // calculate the runnuing average difference
                            long int t1 = 0, t2 = 0;
                            for (int x = 0; x < SAMPLES; x++) {
                                t1 += r1[x];
                                t2 += r2[x];
                            }
                            t1 /= SAMPLES;
                            t2 /= SAMPLES;


                            // Visual Feedback LED
                            if (ledPin != -1) {   // real time feedback on a per-reading basis...
                                int v1, v2;
#ifdef SHOW_IMMEDIATE
                                v1 = r1[sample];
                                v2 = r2[sample];
#else
                                v1 = t1;
                                v2 = t2;
#endif
                                if ((v2 - v1) >= headroom) {
                                      digitalWrite(ledPin, OCCUPIED);
#ifdef DEBUG
                                      if (num == DEBUG) {
                                        Serial.print("a=");
                                        Serial.print(v1, DEC);
                                        Serial.print(", r=");
                                        Serial.print(v2, DEC);
                                        Serial.print(", diff=");
                                        Serial.println(v2-v1, DEC);
                                      }
#endif
                                } else {
                                    digitalWrite(ledPin, EMPTY);
                                }
                            }

                            // IO4 Feedback signal - always smoothed version and add hysteresis
                            if ((t2 - t1) > headroom) {    // if different, something has been detected...
                                delaytime = 0;  // expiration timer is reset every time detection is seen

#ifdef DEBUG
                                if (detected == false) {   // newly triggered
                                    if (num == DEBUG) {
                                      Serial.print("ON  ");
                                      Serial.print(num, DEC);
                                      Serial.print(": ambient=");
                                      Serial.print(t1, DEC);
                                      Serial.print(", reflected=");
                                      Serial.print(t2, DEC);
                                      Serial.print(", diff=");
                                      Serial.print(t2-t1, DEC);
                                  }                         
                              }
#endif   
                              detected = true;
                          }
                          
                          if (detected) {
                              if (delaytime < HYSTERESIS) {
                                  digitalWrite(io4Pin, OCCUPIED); 
                              } else {  // no detection seen for a while...
                                  digitalWrite(io4Pin, EMPTY); 
                                  detected = false;
#ifdef DEBUG
                                  if (num == 0) {
                                      Serial.print("OFF "); 
                                      Serial.println(num, DEC);
                                  }
#endif
                              }
                          }
                          state = R1;
                          sample++;
                          if (sample >= SAMPLES) sample = 0;
                          break;    
            }
        }
        return detected ? 1 : 0;
    };
    
private:  
    int num;
    elapsedMillis delaytime;
    State state;
    elapsedMillis dwelltime;
    int r1[SAMPLES];
    int r2[SAMPLES];
    int headroom;
    byte sample;
    int detected;
    int outPin;
    int inPin;
    int ledPin;
    int io4Pin;
};

#endif

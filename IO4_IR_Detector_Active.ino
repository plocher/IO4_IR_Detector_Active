//DESC:Arduino based proximity sensor
//BOARD:IO4-IR-Detector-Active
//AUTHOR:John Plocher
//LICENSE:MIT License
//
//   Based very loosely on work done by Dustin Andrews 2012
//  
//   Copyright (c) 2014, 2020 John Plocher, released under the terms of the MIT License (MIT)
//
 
#include <elapsedMillis.h>
// #define V21A      // older board version based on Duemilanove Mega-128

//#define DEBUG 2 // both here and in the Circuit.h file...   for serial debug messages

elapsedMillis t1;

#define ACTIVITY_LED 18

#define BLINKTIME 2000

#include "Circuit.h"

Circuit circuit[4];

void setup() 
{       
#ifdef DEBUG
    // Configure the serial port
    Serial.begin(115200);
    Serial.println("IR Detector");  
    t1 = 0; 
#endif
    analogReference(DEFAULT);
    pinMode(ACTIVITY_LED, OUTPUT);
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);

    //              IRTX,IRRX,IO4, headroom, ledFeedback

#ifdef V21A // has no independent LED feedback...
    circuit[0].init(0, 2, A0, 6, 15);
    circuit[1].init(1, 3, A1, 7, 15);
    circuit[2].init(2, 4, A2, 8, 15);
    circuit[3].init(3, 5, A3, 9, 15);
#else
    //  The headroom parameter compensates for sensor differences and fluorescent light interference
    circuit[0].init(0, 2, A0, 6, 50, 10);
    circuit[1].init(1, 3, A1, 7, 90, 11);
    circuit[2].init(2, 4, A2, 8, 20, 12);
    circuit[3].init(3, 5, A3, 9, 20, 13);
#endif
}

bool activity = 0;

void loop() 
{
    if (t1 > BLINKTIME) { 
        activity = !activity;
        digitalWrite(ACTIVITY_LED, activity);
        if (activity) t1 = 0; else t1 = BLINKTIME - 1; // long off, short ON...
    }
    for (int x = 0; x < 4; x++) {
        circuit[x].check();
    }
}

#define ITEMS 30
int r1[ITEMS];
int r2[ITEMS];
long int av1, av2;
int item = 0;

// DELAY = 4: 1:8 (7)  for old style sensor

void xxloop() {
#define IRLED 3
#define IRRX  A1
#define DELAY 4
  digitalWrite(IRLED, 0); delay(DELAY); int v1 =  analogRead(IRRX);        // IR is off
  digitalWrite(IRLED, 1); delay(DELAY); int v2 =  analogRead(IRRX);        // IR is on

  r1[item] = v1;
  r2[item] = v2;
  av1 = av2 = 0;
  for (int x = 0; x < ITEMS; x++) {
    av1 += r1[x];
    av2 += r2[x];
  }
  av1 /= ITEMS;
  av2 /= ITEMS;

  item++;
  if (item >= ITEMS) item = 0;
  
  Serial.print(av1); Serial.print(' ');
  Serial.print(av2); Serial.print(' ');
  Serial.print(av2-av1); Serial.println();

}

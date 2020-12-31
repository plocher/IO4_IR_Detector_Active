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

//#define DEBUG // both here and in the Circuit.h file...   for serial debug messages

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

    circuit[0].init(0, 2, A0, 6, 10);
    circuit[1].init(1, 3, A1, 7, 11);
    circuit[2].init(2, 4, A2, 8, 12);
    circuit[3].init(3, 5, A3, 9, 13);
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

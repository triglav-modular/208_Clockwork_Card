/*
 ________     ___          ________     ________     ___  __       ___       __      ________     ________     ___  __           
|\   ____\   |\  \        |\   __  \   |\   ____\   |\  \|\  \    |\  \     |\  \   |\   __  \   |\   __  \   |\  \|\  \         
\ \  \___|   \ \  \       \ \  \|\  \  \ \  \___|   \ \  \/  /|_  \ \  \    \ \  \  \ \  \|\  \  \ \  \|\  \  \ \  \/  /|_       
 \ \  \       \ \  \       \ \  \\\  \  \ \  \       \ \   ___  \  \ \  \  __\ \  \  \ \  \\\  \  \ \   _  _\  \ \   ___  \      
  \ \  \____   \ \  \____   \ \  \\\  \  \ \  \____   \ \  \\ \  \  \ \  \|\__\_\  \  \ \  \\\  \  \ \  \\  \|  \ \  \\ \  \     
   \ \_______\  \ \_______\  \ \_______\  \ \_______\  \ \__\\ \__\  \ \____________\  \ \_______\  \ \__\\ _\   \ \__\\ \__\    
    \|_______|   \|_______|   \|_______|   \|_______|   \|__| \|__|   \|____________|   \|_______|   \|__|\|__|   \|__| \|__|                                                                                                                                                                                                                                                             
                       ________     ________     ________     ________                                                                 
                      |\   ____\   |\   __  \   |\   __  \   |\   ___ \                  _________________                                                 
                      \ \  \___|   \ \  \|\  \  \ \  \|\  \  \ \  \_|\ \                 __  _  _       __                                               
                       \ \  \       \ \   __  \  \ \   _  _\  \ \  \ \\ \                 _)/ \(_) ___ |_                                                
                        \ \  \____   \ \  \ \  \  \ \  \\  \|  \ \  \_\\ \               /__\_/(_)     __)                                                
                         \ \_______\  \ \__\ \__\  \ \__\\ _\   \ \_______\              _________________                                                 
                          \|_______|   \|__|\|__|   \|__|\|__|   \|_______|                                                             


    ———————————————————————————————————————————————————————————————————————————————————
    Shifting trigger sequencer, clock divider, and timing distributor for the 208.
    ———————————————————————————————————————————————————————————————————————————————————
    
    Version 1

    Code by Triglav Modular
    Anti-copyright 2023

    triglavmodular.hu/clockwork-card

    Dedicated to Charles Cohen.

    Thanks to Michael Jurczak & moylando 
    for their help on the CLIX patterns.

    ————————————————————————————————

*/                                                                                                                               

#include <avr/pgmspace.h>
#include "fscale.c"
#include "elapsedMillis.h"
#include "patterns.h"
#include "hardwareSetup.h"
#include "clix.h"
#include "uncertainty.h"

// Define constants
const unsigned int hysteresis = 2;
const unsigned long minClock = 10000;
const unsigned long maxClock = 5000000;
const long trigLength = 4000;

int externalInState = 0;
int lastExternalInState = 0;
int lastResetInState = 0;
int resetInState = 0;

int lastIntExtSwitch = 0;

int divideRatio = 1;
int clockPot = 0;
int bankPot = 0;
int newBank;
int intExtSwitch = 0;

bool clixMode = 0;
const int clixVariations = 21;

bool uncertaintyMode = 0;

int selectedBank = 0;

unsigned long previousMicros = 0;
unsigned long currentMicros = 0;
unsigned long trigMicros = 0;  // will store last time LED was updated
long clockInterval = 1000000;

int offsetPot_A = 0;
int offsetPot_B = 0;
int offsetPot_C = 0;
int rotatePot = 0;

int CVIn = 0;
int rotateCV = 0;

volatile int currentStep = 0;
int rotation = 0;
int potRotation = 0;

unsigned int offsetSequencer = 0;
unsigned int offsetEnvelope = 0;
unsigned int offsetPulser = 0;
int divideCounter = 0;

void trigLengthEnforcer() {
  clearPin(pinSequencer);
  clearPin(pinEnvelope);
  clearPin(pinPulser);
  clearPin(pinRandom);
  clearPin(clockLED);
}

void changeBank(unsigned int newBank) {

  selectedBank = newBank;
  if (newBank == 31) {
    clixMode = 1;
    uncertaintyMode = 0;
    currentBank.length = 32;
  }
  else if (newBank == 30) {
    uncertaintyMode = 1;
    clixMode = 0;
  }
  else {
    clixMode = 0;
    uncertaintyMode = 0;
    currentBank.length = pgm_read_byte_near(&bank[newBank].length);
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < currentBank.length; j++) {
        currentBank.pattern[i][j] = pgm_read_byte_near(&bank[newBank].pattern[i][j]);
      }
    }
  }

  // Update BANK LEDs

  for (int i = 0; i<5; i++) {
    if (bitRead(selectedBank, i)) {
      setPin(bankLEDs[i]); 
    }
    else {
      clearPin(bankLEDs[i]);
    }
    
  }
  
}

long getWeight(long pot, long weight) {
  long sumWeight = (long)fscale(0, 1023, 0, 1023, pot, (float)weight-50/100.0);
  sumWeight = sumWeight + map(weight, 0, 100, 0, 1023);
  return sumWeight;
}

void nextStep() {
  trigLengthEnforcer();

  readCVIn();
  readOffsetPot_A();
  readOffsetPot_B();
  readOffsetPot_C();
  readRotatePot();
  
  previousMicros = currentMicros;
  trigMicros = currentMicros;
  currentStep++;
  if (currentStep % currentBank.length == 0) {
    currentStep = 0;
  }
  
  setPin(clockLED);
  
  if (clixMode == 1) {
    if ( clixBank [map(offsetPot_A, 0, 1023, 0, clixVariations)][currentStep]) {
      setPin(pinSequencer);
    }
    if ( clixBank [map(offsetPot_B, 0, 1023, 0, clixVariations)][currentStep]) {
      setPin(pinEnvelope);
    }
    if ( clixBank [map(offsetPot_C, 0, 1023, 0, clixVariations)][currentStep]) {
      setPin(pinPulser);
    }
    if ( clixBank [map(rotatePot, 0, 1023, 0, clixVariations)][currentStep]) {
      setPin(pinRandom);
    }
  } 
  else if (uncertaintyMode == 1) {
    int currentWeight = pgm_read_byte_near(&uncertaintyWeights[potRotation][currentStep]);
    if (random(1023)<getWeight(offsetPot_A, currentWeight)) {
      setPin(pinSequencer);
    }
    if (random(1023)<getWeight(offsetPot_B, currentWeight)) {
      setPin(pinEnvelope);
    }
    if (random(1023)<getWeight(offsetPot_C, currentWeight)) {
      setPin(pinPulser);
    }
    if (random(1023)>712) {
      setPin(pinRandom);
    }
  }
  else {
    rotation = 4 - (potRotation + rotateCV) % 4;
    if (currentBank.pattern[(0 + rotation) % 4][(currentStep + offsetSequencer) % currentBank.length]) {
      setPin(pinSequencer);
    }
    if (currentBank.pattern[(1 + rotation) % 4][(currentStep + offsetEnvelope) % currentBank.length]) {
      setPin(pinEnvelope);
    }
    if (currentBank.pattern[(2 + rotation) % 4][(currentStep + offsetPulser) % currentBank.length]) {
      setPin(pinPulser);
    }
    if (currentBank.pattern[(3 + rotation) % 4][currentStep % currentBank.length]) {
      setPin(pinRandom);
    }
  }
}




void readClockPot() {
  long newClock = analogRead(A0);
  if (clockPot + hysteresis < newClock || clockPot - hysteresis > newClock) {
    clockPot = newClock;
    divideRatio = map(1023-clockPot, 0, 1023, 1, 8);
    
  Serial.begin(9600);

    double tempclock;

   newClock = (long)fscale(0, 1023, minClock, maxClock, 1023-newClock, -7);
  // tempclock = maxClock - (minClock-1023)*newClock - (500000 * sin(((2*PI)/1024)*newClock));

    clockInterval = (long)newClock;
  }
}

void readBankPot() {
  int newBank = analogRead(A1);
  if (newBank > 1010) {
    changeBank(31); // Hack for stable maximum
  } 
  else if (bankPot + hysteresis < newBank || bankPot - hysteresis > newBank) {
    bankPot = newBank;
    newBank = (int) map(newBank, 0, 1023, 0, 31);
    if (newBank != selectedBank) { 
      changeBank(newBank);
    }
  }
}


void readOffsetPot_A() {
   int newOffsetPot_A = analogRead(A2);
  if (offsetPot_A + hysteresis < newOffsetPot_A || offsetPot_A - hysteresis > newOffsetPot_A) {
    offsetPot_A = newOffsetPot_A;
    newOffsetPot_A = map(newOffsetPot_A, 0, 1023, 0, currentBank.length);
    offsetSequencer = newOffsetPot_A;
  }
}

void readOffsetPot_B() {
  int newOffsetPot_B = analogRead(A3);
  if (offsetPot_B + hysteresis < newOffsetPot_B || offsetPot_B - hysteresis > newOffsetPot_B) {
    offsetPot_B = newOffsetPot_B;
    newOffsetPot_B = map(newOffsetPot_B, 0, 1023, 0, currentBank.length);
    offsetEnvelope = newOffsetPot_B;
  }
}

void readOffsetPot_C() {
   int newOffsetPot_C = analogRead(A4);
  if (offsetPot_C + hysteresis < newOffsetPot_C || offsetPot_C - hysteresis > newOffsetPot_C) {
    offsetPot_C = newOffsetPot_C;
    newOffsetPot_C = map(newOffsetPot_C, 0, 1023, 0, currentBank.length);
    offsetPulser = newOffsetPot_C;
  }
}

void readRotatePot() {
   int newRotatePot = analogRead(A5);
  if (rotatePot + hysteresis < newRotatePot || rotatePot - hysteresis > newRotatePot) {
    rotatePot = newRotatePot;
    potRotation = map(rotatePot, 0, 1023, 0, 4);
  }
  if (uncertaintyMode == 1) {
    switch (potRotation) {
      case 0:
        currentBank.length = 32;
        break;
      case 1:
        currentBank.length = 18;
        break;
      case 2:
        currentBank.length = 10;
        break;
      case 3:
        currentBank.length = 14;
        break;
      case 4:
        currentBank.length = 5;
        break;
    }
  }
}

void readCVIn() {
   int newCVIn = analogRead(A7);
  if (CVIn + hysteresis < newCVIn || CVIn - hysteresis > newCVIn) {
    CVIn = newCVIn;
    rotateCV = map(newCVIn, 0, 1023, 0, 4);
  }
}



void readIntExtSwitch() {
  int newIntExtSwitch = analogRead(A6);
  if (newIntExtSwitch != lastIntExtSwitch) {
    if (newIntExtSwitch < 512) {
      intExtSwitch = 0;
    } 
    else {
      intExtSwitch = 1;
    } 
    lastIntExtSwitch = newIntExtSwitch;
  }
}

void resetHandler() {
  currentStep = -1;
}


void setup() {
  clockInterval = 100000;
  changeBank(0);
  pinMode(pinSequencer, OUTPUT);
  pinMode(pinEnvelope, OUTPUT);
  pinMode(pinPulser, OUTPUT);
  pinMode(clockLED, OUTPUT);
  pinMode(pinRandom, OUTPUT);
  pinMode(pinExternalIn, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinResetIn), resetHandler, RISING);
  
   
  for (int i = 0; i < 5; i++) {
    pinMode(bankLEDs[i], OUTPUT);    
  }
}

void externalHandler() {
  readClockPot();
  divideCounter++;
  if (divideCounter >= divideRatio) {
    divideCounter = 0;
    nextStep();
  }
}

void checkExternalIn() {
    externalInState = (PIND & bit (pinExternalIn)) == 0;
    if (externalInState != lastExternalInState) {
      if (externalInState == HIGH) {
          divideCounter++;
          if (divideCounter >= divideRatio) {
            nextStep();
            divideCounter = 0;
          }
      }
      lastExternalInState = externalInState;
    }
}

void loop() {

  currentMicros = micros();
  
  if (currentMicros - trigMicros > trigLength) {
    trigLengthEnforcer();
  }
  
  readIntExtSwitch();
  readClockPot();
  readBankPot();


  if (intExtSwitch == 1) {
    checkExternalIn();
  }
  
  else {
    if (currentMicros - previousMicros >= clockInterval) {
      nextStep();
    } 
  }
}
#include "directIO.h"

// Each signal is a port letter and a bit, used through directIO.h. The board chosen
// in the IDE picks the set: Arduino Nano for the ATmega328P cards (P.3, P.4),
// Arduino Leonardo for the ATmega32U4 cards (P.5).

#if defined(__AVR_ATmega32U4__)
#define pinPulser      B, 4  // /D4
#define pinRandom      B, 5  // /D5
#define pinSequencer   B, 6  // /D6
#define pinEnvelope    C, 6  // /D7
#define clockLED       D, 6  // /CLK_OUT
#define bankLED0       B, 7  // /D9
#define bankLED1       E, 6  // /D10
#define bankLED2       B, 2  // /D11/MOSI
#define bankLED3       B, 3  // /D12/MISO
#define bankLED4       B, 1  // /D13/SCK
#define pinExternalIn  D, 1  // /D2
const unsigned int pinResetIn = 3;  // /D3 on PD0: Arduino pin 3, INT0, on the Leonardo core
#else
#define pinPulser      D, 4
#define pinRandom      D, 5
#define pinSequencer   D, 6
#define pinEnvelope    D, 7
#define clockLED       B, 0
#define bankLED0       B, 1
#define bankLED1       B, 2
#define bankLED2       B, 3
#define bankLED3       B, 4
#define bankLED4       B, 5
#define pinExternalIn  D, 2
const unsigned int pinResetIn = 3;
#endif

#define setPin(pin) dWrite(pin, 1)
#define clearPin(pin) dWrite(pin, 0)

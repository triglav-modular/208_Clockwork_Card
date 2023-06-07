#define setPin(b) ( (b)<8 ? PORTD |=(1<<(b)) : PORTB |=(1<<(b-8)) )
#define clearPin(b) ( (b)<8 ? PORTD &=~(1<<(b)) : PORTB &=~(1<<(b-8)) )
#define readPin(b) ( (b)<8 ? (PORTD &(1<<(b)))!=0 : (PORTB &(1<<(b-8)))!=0 )

const unsigned int pinSequencer = 6;
const unsigned int pinEnvelope = 7;
const unsigned int pinPulser = 4;
const unsigned int pinRandom = 5;
const unsigned int pinExternalIn = 2; 
const unsigned int pinResetIn = 3;

const unsigned int clockLED = 8;
const int bankLEDs[5] = {9,10,11,12,13};
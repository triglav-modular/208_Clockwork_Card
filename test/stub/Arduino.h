// Just enough of the Arduino core to run the sketch on a computer.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <deque>

typedef uint8_t byte;
#define _Bool bool
#define PROGMEM
#define pgm_read_byte_near(p) (*(const uint8_t *)(p))
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define RISING 3
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define A6 20
#define A7 21
#define bit(b) (1UL << (b))
#define bitRead(value, b) (((value) >> (b)) & 1)

extern uint8_t PORTB, PORTD, PIND;
extern unsigned long fakeMicros;
extern int fakeAnalog[8];

inline unsigned long micros() { return fakeMicros; }
inline unsigned long millis() { return fakeMicros / 1000; }
inline int analogRead(int pin) { return fakeAnalog[pin - A0]; }
inline void pinMode(int, int) {}
inline int digitalPinToInterrupt(int pin) { return pin - 2; }
inline void attachInterrupt(int, void (*)(), int) {}
inline long random(long howBig) { return rand() % howBig; }
inline long map(long x, long inMin, long inMax, long outMin, long outMax) {
  return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

struct FakeSerial {
  std::deque<uint8_t> rx;
  void begin(unsigned long baud) { this->baud = baud; }
  int available() { return rx.size(); }
  int read() { int b = rx.front(); rx.pop_front(); return b; }
  unsigned long baud = 0;
};
extern FakeSerial Serial;

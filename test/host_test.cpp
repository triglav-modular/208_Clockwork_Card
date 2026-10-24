// Runs the sketch on a computer against fake pins, a fake clock and a fake UART,
// and checks how it follows MIDI clock. Build and run: make -C test
// Each boot() runs setup() but keeps the globals, so only the first test sees power-up state.
#include "Arduino.h"
#include <stdio.h>
#include <vector>

uint8_t PORTB, PORTC, PORTD, DDRB, DDRC, DDRD, PINB, PIND;
unsigned long fakeMicros = 1000;
int fakeAnalog[8];
FakeSerial Serial;

void readCVIn();
void readOffsetPot_A();
void readOffsetPot_B();
void readOffsetPot_C();
void readRotatePot();
void readClockPot();
void readBankPot();
void readIntExtSwitch();
void checkExternalIn();

#include "../Clockwork_Code/Clockwork_Code.ino"

static int failures = 0;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

const int potDivide1 = 1023;  // clock pot fully clockwise: divide by 1
const int potDivide2 = 876;   // divide by 2
const int potSlowest = 0;     // internal clock at its slowest, 5 s

// One loop pass, 1 ms later, with the given bytes waiting on the UART.
// Returns whether a step was played in that pass.
bool pass(std::vector<uint8_t> bytes = {}) {
  fakeMicros += 1000;
  for (uint8_t b : bytes) Serial.rx.push_back(b);
  unsigned long before = trigMicros;
  loop();
  return trigMicros != before && trigMicros == fakeMicros;
}

// Sends n clock ticks, one per pass; returns the tick indexes that played a step
// and the step each played.
std::vector<std::pair<int, int>> ticks(int n) {
  std::vector<std::pair<int, int>> played;
  for (int i = 0; i < n; i++) {
    if (pass({0xF8})) played.push_back({i, currentStep});
  }
  return played;
}

std::vector<std::pair<int, int>> expect(std::vector<std::pair<int, int>> v) { return v; }

void boot(int clockPot, bool ext) {
  for (int i = 0; i < 8; i++) fakeAnalog[i] = 0;
  fakeAnalog[0] = clockPot;
  fakeAnalog[6] = ext ? 1023 : 0;
  PIND = 0;
  Serial.rx.clear();
  setup();
  pass();
}

void print(const char *label, const std::vector<std::pair<int, int>> &v) {
  printf("  %s:", label);
  for (auto &p : v) printf(" t%d=s%d", p.first, p.second);
  printf("\n");
}

int main() {
  boot(potDivide1, true);
  CHECK(Serial.baud == 31250, "UART at %lu baud", Serial.baud);

  // Without a Start (a card that came up mid-song), ticks are followed from where it is.
  auto played = ticks(7);
  print("no start", played);
  CHECK(played == expect({{0, 1}, {6, 2}}), "ticks without a Start are followed");

  // Start, then ticks: one step every 6 ticks, from step 0 on the first tick.
  pass({0xFA});
  played = ticks(24);
  print("start, divide 1", played);
  CHECK(played == expect({{0, 0}, {6, 1}, {12, 2}, {18, 3}}), "steps every 6 ticks from step 0");

  // Stop holds the card; Continue picks up where the count left off.
  pass({0xFC});
  CHECK(ticks(12).empty(), "no steps after Stop");
  pass({0xFB});
  played = ticks(12);
  print("continue", played);
  CHECK(played == expect({{0, 4}, {6, 5}}), "Continue resumes at the next step");

  // The clock pot divides the 16th-note pulses as it divides the jack.
  fakeAnalog[0] = potDivide2;
  pass({0xFA});
  played = ticks(48);
  print("start, divide 2", played);
  CHECK(divideRatio == 2, "divide ratio %d", divideRatio);
  CHECK(played == expect({{0, 0}, {12, 1}, {24, 2}, {36, 3}}), "steps every 12 ticks at divide 2");

  // Song Position Pointer on a step: the first tick after Continue plays that step.
  // Bank 0 is 10 steps long; position 24 sixteenths at divide 2 is step 12, so step 2.
  pass({0xFC});
  pass({0xF2, 24, 0});
  pass({0xFB});
  played = ticks(13);
  print("SPP 24, divide 2", played);
  CHECK(played == expect({{0, 2}, {12, 3}}), "SPP on a step boundary");

  // Song Position Pointer between steps: position 5 at divide 2 is half-way through
  // step 2, so step 3 plays 6 ticks in, on the seventh tick after Continue.
  pass({0xFC});
  pass({0xF2, 5, 0});
  pass({0xFB});
  played = ticks(20);
  print("SPP 5, divide 2", played);
  CHECK(played == expect({{6, 3}, {18, 4}}), "SPP between steps");

  // Song Position Pointer above 127 sixteenths uses both data bytes, and a tick
  // between them is still a tick: 200 = 0x48 + 1 * 128; 200 / 2 = step 100, so step 0.
  pass({0xFC});
  pass({0xF2, 0x48});
  pass({0xF8});
  pass({0x01});
  pass({0xFB});
  played = ticks(1);
  CHECK(played == expect({{0, 0}}), "two-byte SPP with a tick in between");

  // Channel messages are ignored.
  fakeAnalog[0] = potDivide1;
  pass({0xFA});
  pass({0x90, 0x3C, 0x64});
  pass({0x3E, 0x64});  // running status
  played = ticks(7);
  CHECK(played == expect({{0, 0}, {6, 1}}), "note messages leave the clock alone");

  // While MIDI clock arrives the jack is ignored; 500 ms after the last tick it is followed again.
  pass({0xF8});
  int before = currentStep;
  PIND = bit(2);
  bool stepped = pass();
  PIND = 0;
  pass();
  CHECK(!stepped && currentStep == before, "jack edge right after a tick is ignored");
  fakeMicros += 500000;
  PIND = bit(2);
  stepped = pass();
  PIND = 0;
  pass();
  CHECK(stepped, "jack followed again after 500 ms without ticks");

  // With the switch on INT, MIDI clock and Start are ignored.
  boot(potSlowest, false);
  before = currentStep;
  pass({0xFA});
  played = ticks(48);
  CHECK(played.empty() && currentStep == before, "INT mode ignores MIDI");


  printf(failures ? "%d FAILED\n" : "all passed\n", failures);
  return failures != 0;
}

// MIDI clock in, through the USB-C port. On ATmega32U4 cards the card is a USB-MIDI
// device and the DAW sends to it directly. On ATmega328P cards the port is the
// FT232RL's serial port: the bytes arrive on the UART (D0/RX) at 31250 baud, and the
// computer needs tools/midi_bridge.py (or another serial-to-MIDI bridge) in between.
//
// With the INT/EXT switch on EXT, MIDI clock drives the card while it arrives and the
// clock jack takes over again when it stops. One pulse is a 16th note (6 ticks at
// 24 PPQN), and the clock pot divides those pulses exactly as it divides the jack.

#if defined(USBCON)
#include "MIDIUSB.h"
#endif

void nextStep();

const unsigned long midiBaud = 31250;
const unsigned int midiTicksPerPulse = 6;
const unsigned long midiTimeout = 500000;  // us without a tick before the jack is followed again

bool midiRunning = true;  // until a Stop, so a card that missed the Start still follows
bool midiTickSeen = false;
unsigned long lastMidiTickMicros = 0;
unsigned int midiCountdown = 0;  // ticks until the next step

byte midiStatus = 0;
byte midiData[2];
byte midiDataCount = 0;

bool midiClockActive() {
  return midiTickSeen && (currentMicros - lastMidiTickMicros < midiTimeout);
}

void midiTick() {
  midiTickSeen = true;
  lastMidiTickMicros = currentMicros;
  if (!midiRunning || intExtSwitch == 0) {
    return;
  }
  if (midiCountdown == 0) {
    nextStep();
    midiCountdown = midiTicksPerPulse * divideRatio;
  }
  midiCountdown--;
}

void midiStart() {
  midiRunning = true;
  if (intExtSwitch == 1) {
    currentStep = -1;  // the first tick after Start plays step 0
    midiCountdown = 0;
  }
}

// Song Position Pointer: 16th notes since the start of the song, sent before a Continue.
void midiSongPosition(unsigned int sixteenths) {
  if (intExtSwitch == 0) {
    return;
  }
  unsigned long ticks = (unsigned long)sixteenths * 6;
  unsigned int stepTicks = midiTicksPerPulse * divideRatio;
  int steps = (ticks / stepTicks) % currentBank.length;
  unsigned int remainder = ticks % stepTicks;
  if (remainder == 0) {
    currentStep = steps - 1;  // the next tick plays this step
    midiCountdown = 0;
  }
  else {
    currentStep = steps;
    midiCountdown = stepTicks - remainder;
  }
}

void midiByte(byte b) {
  if (b >= 0xF8) {  // real-time bytes can arrive between the bytes of any other message
    switch (b) {
      case 0xF8: midiTick(); break;
      case 0xFA: midiStart(); break;
      case 0xFB: midiRunning = true; break;
      case 0xFC: midiRunning = false; break;
    }
  }
  else if (b & 0x80) {
    midiStatus = b;
    midiDataCount = 0;
  }
  else if (midiStatus == 0xF2) {
    midiData[midiDataCount++] = b;
    if (midiDataCount == 2) {
      midiSongPosition(midiData[0] | ((unsigned int)midiData[1] << 7));
      midiStatus = 0;
    }
  }
}

void midiBegin() {
#if !defined(USBCON)
  Serial.begin(midiBaud);
#endif
}

#if defined(USBCON)
// Bytes in a USB-MIDI event packet, by its Code Index Number (USB MIDI 1.0, table 4-1).
const byte midiPacketLength[16] = {0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1};
#endif

void readMidi() {
#if defined(USBCON)
  midiEventPacket_t p;
  while ((p = MidiUSB.read()).header != 0) {
    byte length = midiPacketLength[p.header & 0x0F];
    if (length > 0) midiByte(p.byte1);
    if (length > 1) midiByte(p.byte2);
    if (length > 2) midiByte(p.byte3);
  }
#else
  while (Serial.available() > 0) {
    midiByte(Serial.read());
  }
#endif
}

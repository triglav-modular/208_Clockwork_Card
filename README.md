# Clockwork Card 208-5

This is an extension card for the 208 that generates and distributes trigger sequences to the various submodules.

Installation instructions:

1. Download the latest firmware from triglavmodular.hu/208-cards/clockwork-card
2. Download and install the Arduino IDE: https://www.arduino.cc/en/software
3. Remove the card from the 208.
4. Plug in the card to your computer with the USB-C port.
5. Open the downloaded firmware project. (Clockwork_Code.ino)
6. Select the USB port in the IDE in the top left corner, and the board that goes with it:
   - A port like "/dev/cu.usbserial-XXXXXXXX" is a card with an ATmega328P: choose Arduino Nano.
   - A port like "/dev/cu.usbmodemXXXX" is a card with an ATmega32U4: choose Arduino Leonardo. These cards need the MIDIUSB library, installed once from Tools > Manage Libraries.
7. Press upload in the top left corner and wait for the flashing to finish.
8. Unplug the device from the USB port.
9. You’re done.

**MIDI CLOCK**

The card follows MIDI clock from a computer over the same USB-C port while it is in the 208. Set the INT/EXT switch to EXT and connect the USB-C port to the computer.

A card with an ATmega32U4 is a USB-MIDI device: in the DAW, send MIDI clock and song position to it. It appears as "Arduino Leonardo".

A card with an ATmega328P has a USB serial port rather than a MIDI device, so a small bridge program runs on the computer and offers the DAW a MIDI destination called "Clockwork Card":

1. Install Python 3 and the bridge's libraries: `pip install mido python-rtmidi pyserial`
2. Run `python3 tools/midi_bridge.py`. Starting it restarts the card, so start it before pressing play.
3. In the DAW, send MIDI clock and song position to "Clockwork Card".

On Windows, create a port in loopMIDI, send the clock to it, and run `python3 tools/midi_bridge.py --input "loopMIDI Port"`.

Each 16th note is one clock pulse. With the clock pot at its fastest setting the card steps on every 16th; turning it down divides the pulses by up to 8, as it does for the clock jack. Start begins the pattern at its first step, Stop holds it, and Continue resumes from the song position. While MIDI clock is arriving the clock jack is ignored, and half a second after it stops the jack is followed again. With the switch on INT the card runs on its own clock and ignores MIDI.


**CHANGELOG**

1.3

Follows MIDI clock over USB. Builds for cards with the ATmega32U4 (board: Arduino Leonardo) as well as the ATmega328P.
CLIX mode (bank 31) plays the 16 Second Digital Delay's full set of click patterns: 33, from silence to every step, spread over each pot as on the original.

1.2

Fixed a bug that caused trigger lengths to be shorter than 4ms.

1.1

Fixed a bug causing external and pulser mode triggering on the falling edge.

1.0 

Initial release.

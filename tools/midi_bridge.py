#!/usr/bin/env python3
"""Send a DAW's MIDI clock to the Clockwork Card over its USB-C port.

The card's USB port is a serial port, not a MIDI device, so this bridge sits between
the two: it offers a MIDI destination called "Clockwork Card" (macOS and Linux) and
passes clock, Start, Stop, Continue and Song Position Pointer to the card at 31250 baud.
Everything else is dropped.

    pip install mido python-rtmidi pyserial
    python3 tools/midi_bridge.py              # the card's port is found on its own
    python3 tools/midi_bridge.py --list       # serial ports and MIDI inputs
    python3 tools/midi_bridge.py --input "loopMIDI Port"   # Windows: listen to an existing port

Opening the port restarts the card, so the bridge waits for it to boot before it
forwards anything.
"""

import argparse
import sys
import time

import mido
import serial
from serial.tools import list_ports

BAUD = 31250
BOOT_SECONDS = 2.0
FORWARDED = {"clock", "start", "continue", "stop", "songpos"}
VIRTUAL_NAME = "Clockwork Card"


def find_card():
    ports = [p.device for p in list_ports.comports() if p.vid == 0x0403 and p.pid == 0x6001]
    if len(ports) == 1:
        return ports[0]
    if not ports:
        sys.exit("No FT232R serial port found. Is the card plugged in? Use --port to name one.")
    sys.exit("Several FT232R ports found (%s). Use --port to pick the card." % ", ".join(ports))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--port", help="the card's serial port (default: the only FT232R port)")
    ap.add_argument("--input", help="an existing MIDI input to listen to, instead of a virtual one")
    ap.add_argument("--list", action="store_true", help="list serial ports and MIDI inputs, then exit")
    args = ap.parse_args()

    if args.list:
        print("Serial ports:")
        for p in list_ports.comports():
            print("  %s  %s" % (p.device, p.description))
        print("MIDI inputs:")
        for name in mido.get_input_names():
            print("  %s" % name)
        return

    port = args.port or find_card()
    card = serial.Serial(port, BAUD)
    print("Opened %s; waiting for the card to boot." % port)
    time.sleep(BOOT_SECONDS)
    card.reset_input_buffer()

    def forward(msg):
        if msg.type in FORWARDED:
            card.write(msg.bin())

    if args.input:
        midi_in = mido.open_input(args.input, callback=forward)
        print('Forwarding clock from "%s". Ctrl-C to stop.' % args.input)
    else:
        midi_in = mido.open_input(VIRTUAL_NAME, virtual=True, callback=forward)
        print('Send MIDI clock to "%s". Ctrl-C to stop.' % VIRTUAL_NAME)

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        midi_in.close()
        card.close()


if __name__ == "__main__":
    main()

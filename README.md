# Clockwork Card 208-5

This is an extension card for the 208 that generates and distributes trigger sequences to the various submodules.

Installation instructions:

1. Download the latest firmware from triglavmodular.hu/208-cards/clockwork-card
2. Download and install the Arduino IDE: https://www.arduino.cc/en/software
3. Remove the card from the 208.
4. Plug in the card to your computer with the USB-C port.
5. Open the downloaded firmware project. (Clockwork_Code.ino)
6. Select the USB port in the IDE in the top left corner. It will look something like "/dev.cu.usbserial-XXXXXXXX". Choose Arduino Nano for board if asked.
7. Press upload in the top left corner and wait for the flashing to finish.
8. Unplug the device from the USB port.
9. You’re done.


**CHANGELOG**

1.2

Fixed a bug that caused trigger lengths to be shorter than 4ms.

1.1

Fixed a bug causing external and pulser mode triggering on the falling edge.

1.0 

=======
Increased trigger length to be 4ms to address potentially missed triggers.

1.1
Fixed a bug causing external and pulser mode triggering on the falling edge.

1.0 

Initial release.

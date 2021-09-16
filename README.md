evil mass storage *AT90USBKEY2 (poc-malware-tool for offline system)

This is the official post to ask about this project: https://www.driverentry.com/node/104

WARNING: this is a DIY-POC just for fun and the code is pure crap x-), btw my english sucks and I am a hardware noob. Thx to esTse for the english fixes! 

![lastprot](https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/lastprot.PNG)

my roapt v1 board

The objective of this project is to create an USB device to exfiltrate data from an isolated environment via radio frequency using 433MHz ASK, this allows us to exfiltrate small amounts of information such as digital certificates without the need for an internet connection and at a considerable distance with great penetration unlike 2.4GHz.

In a physically isolated environment such as a Faraday cage we can use an alternative version which exfiltrates the information (crypted-first) to a micro SD. It's important to understand that this is very different from a rubber ducky, while the rubber ducky acts as a fake keyboard the evil mass storage its composed of keyboard firmware + radio frequency exfiltration system + mass storage, this gives us a very versatile tool.

Now let's see it in a more detailed way:

infect a target machine without internet
hardware: at90usb1287 + atmega328p + ts3usb221 + mosfet + sd card reader (SPI) + rf 433MHz ASK …
multi-stage malware: only visible when connected to target
exfiltrate info via:
mass storage: crypts/decrypts & hides sectors (only crap-XOR, its an AVR-8 bit!)
radio: RF 433MHz ASK (crap-XOR encryption)
Firmware: keyboard + mass storage (USB composite device). LUFA + FatFs + Dreg adaptation “USB Mass storage SD card for Teensy2/ATMEGA32U4 by Mathieu Sonet”
Dynamic: serial, VID, PID, USB Descriptor, decrypt/delete sectors…
All the source code its available in my github:

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2
 
WARNING: This is not a rubber ducky! xD

if you want to play with this kind of stuff the prototype board is: AT90USBKEY2 (at90usb1287)

http://aem-origin.microchip.com/en-us/development-tool/AT90USBKEY2

![usbkey](https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/usbkey.PNG)

at90usb1287
USB software interface for Device Firmware Upgrade (DFU bootloader) 
Power supply flagged by “VCC-ON” LED:
regulated 3.3V
from an external battery connector (for reduced host or OTG operation) 
from the USB interface (USB device bus powered application)
JTAG interface (connector not mounted):
for on-chip ISP
for on-chip debugging using JTAG ICE
Serial interfaces: 1 USB full/low speed device/host/OTG interface
On-board resources:
4+1-ways joystick
2 Bi-Color LEDs – temperature sensor
serial dataflash memories
all microcontroller I/O ports access on 2x8pin headers (not mounted)
On-board RESET button
On-board HWB button to force bootloader section execution at reset.
System clock: 8 MHz
If you dont like AT90USBKEY2 and you know what are you doing, you can port my code easily to Teensy++ 2.0 Development Board (AT90USB1286):

https://www.pjrc.com/store/teensypp.html
I developed a private version of evil mass storage using Teensy++ 2.0. This version will never be public (dont worry, its pure crap xD). Btw, Teensy++2.0 board (16 MHz) is faster than AT90USBKEY2 (8 MHz).

USB can be a little pain in the ass. I recommend to read two essentials books by Jan Axelson:

USB Mass Storage: Designing and Programming Devices and Embedded Hosts
USB Complete: The Developer's Guide (Complete Guides series), Fifth edition.
The book's code its not for AVR-8 bit, but its very well explained.

The first prototype was this chaos:

![evilmass](https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass.jpg)

Well, wtf is this then?

The POC is very simple, it has a SD card with the malware encrypted (a crap-XOR, remember this is an AVR-8 mic).

If you connect the USB N times, the microcontroller removes the malware from the SD. To do this, the usb device only needs POWER. Then, if the researcher plugs some times the device ... bad luck xD 

The idea of the POC is only to infects one target machine. If you connect the USB to other PC the device will work as a normal mass storage (btw very slow because SPI for SD).

Demo video (in Spanish): https://youtu.be/-K6MMVyKEv0?t=346

Steps to reproduce an attack:

The victim connects the USB device.
 
To make forensic work more difficult the device can randomize the VID/PID, serial disk and all relevant forensic-USB-data in each connection (the uploaded POC only changes this info in some stages):

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Descriptors.c#L112

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SCSI.c#L88

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SDCardManager.c#L150
 
The USB device its an USB composite device (not an USB HUB, again... read the books!!). Windows will detect it as a new keyboard and a new mass storage device.

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Descriptors.c#L193
 
The keyboard-device opens a run window (WIN + R) and starts to bruteforce the asigned letter for mass storage in order to execute the stored .exe in our mass storage. This exe its not the malware, its the first stage. It retrieves useful information like user name and writes it into the mass storage.

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/MassStorageKeyboard.c#L342

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/stage1/stage1/main.c#L90
 
The microcontroller gets the SCSI command and if the info it's correct it resets the USB connection, at this moment the malware is at the mass storage. This malware its decrypted (the POC uploaded its only a crap-XOR) using the information written in the mass storage... if evil mass storage it's not connected to the target computer, malware won't be in it. 

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/MassStorageKeyboard.c#L317

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SDCardManager.c#L381

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SCSI.c#L370

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SDCardManager.c#L587
 
The malware is executed and the microcontroller removes all sectors of the malware from the SD.

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SDCardManager.c#L163

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/MassStorageKeyboard.c#L125
 
From this moment, the USB device will only work as a regular USB mass storage (keyboard part is removed). The VID-PID + other USB info gets changed again.

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Descriptors.c#L97

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Descriptors.c#L288
 
The malware exfiltrates data writting the mass storage and the microcontroller resends the information via rf 433MHz ASK (helped by a atmega328p). It also supports the exfiltration via the SD card (encrypting the information first).

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilmass/evilmass/Lib/SDCardManager.c#L438

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/evilard/evilard.ino#L176

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/stage2/stage2/main.c#L183
This attack its only useful to steal little info because SPI slow, RF 433MHz bandwich..

I'm working in a new version. Currently experimenting with two ARM Cortex-M4 32 bit boards: FRDM-K66F and Teensy 3.6 (Paul J Stoffregen + an awesome community pjrc + a lot of code).

What I am looking for:

Fast microcontroller ARM Cortex-M4 at 180 MHz
A real SDIO interface (fast SD access)
Cryptographic Acceleration & Random Number Generator (I want to use AES to encrypt/decrypt sectors...).
NOTE: ARM Cortex-M4 its very very complex compared to AVR-8 bit, you should read this (hard) book:

The Definitive Guide to ARM Cortex-M3 and Cortex-M4 Processors Third Edition by Joseph Yiu. ARM Ltd., Cambridge, UK
Teensy 3.6 ARM Cortex-M4 (NXP Kinetis MK66FX1M0VMD18) 180MHz:

ARM Cortex-M4 at 180 MHz
Float point math unit, 32 bits only
1024 Flash, 256K RAM, 4K EEPROM
USB device 12 Mbit/sec, USB host 480 Mbit/sec
64 digital input/output pins, 22 PWM output pins
25 analog input pins, 2 analog output pins, 11 capacitive sense pins
6 serial, 3 SPI, 4 I2C ports
1 I2S/TDM digital audio port
2 CAN bus
1 SDIO (4 bit) native SD Card port
32 general purpose DMA channels
Cryptographic Acceleration & Random Number Generator
RTC for date/time

https://www.pjrc.com/store/teensy36.html

![teensy](https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/teensy.PNG)

FRDM-K66F (NXP Kinetis MK66FN2M0VMD18):

Performance

Up to 180 MHz ARM Cortex-M4 based core with DSP instructions and Single Precision Floating Point unit
System and Clocks

Multiple low-power modes to provide power optimization based on application requirements
Memory protection unit with multi-master protection
3 to 32 MHz main crystal oscillator
32 kHz low power crystal oscillator
48 MHz internal reference
Security

Hardware random-number generator
Supports DES, AES, SHA accelerator (CAU)
Multiple levels of embedded flash security
Timers

Four Periodic interrupt timers
16-bit low-power timer
Two 16-bit low-power timer PWM modules
Two 8-channel motor control/general purpose/PWM timers
Two 2-ch quad decoder/general purpose timers
Real-time clock
Human-machine interface

Low-power hardware touch sensor interface (TSI)
General-purpose input/output
Memories and memory expansion

Up to 2 MB program flash memory on nonFlexMemory devices with 256 KB RAM
Up to 1 MB program flash memory and 256 KB of FlexNVM on FlexMemory devices
4 KB FlexRAM on FlexMemory devices
FlexBus external bus interface and SDRAM controller
Analog modules

Two 16-bit SAR ADCs and two 12-bit DAC
Four analog comparators (CMP) containing a 6-bit DAC and programmable reference input
Voltage reference 1.2V
Communication interfaces

Ethernet controller with MII and RMII interface to external PHY and hardware IEEE 1588 capability
USB high-/full-/low-speed On-the-Go with on-chip high speed transceiver
USB full-/low-speed OTG with on-chip transceiver
Two CAN, three SPI and four I2C modules
Low Power Universal Asynchronous Receiver/ Transmitter 0 (LPUART0) and five standard UARTs
Secure Digital Host Controller (SDHC)
I2S module

![FRDMK66F](https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/FRDM-K66F.PNG)

https://www.nxp.com/docs/en/data-sheet/K66P144M180SF5V2.pdf

https://www.nxp.com/design/development-boards/freedom-development-boards/mcu-boards/freedom-development-platform-for-kinetis-k66-k65-and-k26-mcus:FRDM-K66F

my pull request adding new ClassDriver MassStorageSDKeyboard Demo for LUFA - the Lightweight USB Framework for AVRs:

https://github.com/abcminiuser/lufa/pull/158
my talk in english (translated by who knows):

https://www.youtube.com/watch?v=5-ly4IyrD1Q
Just my own adaptation for mass storage sd card and keyboard for AT90USBKEY2:

https://github.com/David-Reguera-Garcia-Dreg/lufa-sdcard-mass-storagekeyboard-fatfs-AT90USBKEY2
Presentation: 

https://github.com/David-Reguera-Garcia-Dreg/evilmass_at90usbkey2/blob/master/Roapt%20evil%20mass%20storage.pdf
FatFS + TTL UART + MICRO SD + ATMEL ICE JTAG DEBUGGING:



NOTE: I have no plans to make/sell more roapt v1 boards. I don't want to spend money on this xD.

ARM POC version is coming, DM via Twitter @fr33project


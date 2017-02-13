# sesame
Arduino library for a Arduino Uno with the ITEAD SIM5216A 3G Shield that is used to receive an SMS and then phone a number to open a gate 

## Introduction
This was my first Arduino project born out of necessity

I live in a gated environment where you need to dial a number to open the gate. One problem with this is that the body corporate can only register three numbers per tennant. I needed more, so this project was born.

## Parts
I used the following parts to build the project:

1.	KeyStudio UNO R3
2.  Arduino 3G GSM Shield (SIM5216A)
3.  Pre-Paid SIM card (I used a SIM card without a pin. If you're using one with a pin, you will need to implement some code to unlock the SIM)
4.  12 V Power Supply (You need power the arduino with external power as USB does not provide enough power to properley power the GSM Shield.)

## Before you start
I wasted time because I did not understand the following items properley:

1.  You need to power the Arduino with external power. Everything seems ok if you use USB only to power the setup. You can even send AT commands to the Modem and get answes back. It will however not properley power the radio so you will not be able to register the modem on the network.
2.  You need to change the GSM shield jumpers to select diffrent RX and TX digital pins to operate on. When I got mine the jumpers was set to digital pins 1 & 2. This is the same as the pins that the Arduino use for serial communications. I ended up setting mine to pins 6 & 7. Keep in mind that the TX pin on the modem becomes the RX pin on the arduino.
3.  You will need to use SoftwareSerial to communicate with the GSM Shield. This means that you will need to lower the standard Baudrate (115200) on the GSM shield to reliabily comunicate with the shield. In my case I lowerd it to 9600. You can do this with two medhods. Either create a small Arduino sketch to achieve this or use the usb port on the GSM Shield to set it via a terminal session.


# Arduino-Repeater-controller
Controll a UHF emergency repeater with Arduino Board

The required hardware up to version 4.3.

The full project link, here:
https://yo3hjv.blogspot.com/2016/02/low-profile-duplex-uhf-repeater-for-ham.html?m=0

The repeater consists pf two UHF portable radio, in my case, GP300.
From the Rx Radio, we have Carrier Detect / PL Detect output, and AUDIO OUT-Deemphasized audio at around 450 mVpp
From the Tx Radio, we have PTT input (tied to +5V) and AUDIO IN - Tx modulation INPUT at 25 mVpp.
From the battery we have a 1:3 divider. At 15V we have at A0 ADC input, 5V.
The Carrier detect / PL detect goes +5V when a proper signal is detected.
The PTT goes to Ground for transmission to start.

Please check the pinout definitions for the wiring to both Tx and Rx radios.



This is it!
73 de Adrian YO3HJV

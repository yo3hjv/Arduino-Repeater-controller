# Arduino-Repeater-controller
Controll a UHF emergency repeater with Arduino Board

The required hardware up to version 4.3.

The repeater consists pf two UHF portable radio, in my case, GP300.
From the Rx Radio, we have Carrier Detect / PL Detect output, and AUDIO OUT-Deemphasized audio at around 450 mVpp
From the Tx Radio, we have PTT input (tied to +5V) and AUDIO IN - Tx modulation INPUT at 25 mVpp.
From the battery we have a 1:3 divider. At 15V we have at A0 ADC input, 5V.
The Carrier detect / PL detect goes +5V when a proper signal is detected.
The PTT goes to Ground for transmission to start.
The AUDIO OUT goes to a 10kOhm multiturn potentiometer.
The cursor from that potentiometer goes to the AUDIO IN on the TX radio.
The Carrier detect / PL Detect output goes to the Base pin of a NPN transistor. The Emitter is to Ground and the Collector goes to 
Digital pin 10. 
The Digital pin 10 is tied to +5V (Arduino Vcc) via a 10 kOhm resistor.
The DPin 10 goes LOW when a proper signal is detected at the receiving radio.
Further more, the DPin 13 has a resistor to the Base pin of the second NPN transistor.
Between the Base and the Emitter, a 10 kOhm resistor is connected and the Emitter is tied to the Ground.
The Collector of the second transistor goes to the PTT port of the Transmitting radio.
The Courtesy Beep and the Beacon is audio signal generated at DPin 8.
From the DPin8, a 10 kOhm resistor, in series with a 47 nF capacitor goes to the 10 kOhm multiturn potentiometer, 
in parallel with the AUDIO OUT from the receving radio.
At the A0 ADC input, we have a 1:3 resistive divider in order to have max 5V at A0 when the battery voltage is at 15V
This comprise of a resistor of 5 kOhm from A0 to Ground and a 10 kOhm resistor to the SLA Battery PLUS connector.


This is it!
73 de Adrian YO3HJV

/*
   This junk is meant to controll a small repeater in UHF made of two portable radios
   Motorola GP300.
   v_8.2

   MAIN FEATURES
   -The repeater can be activated by RSSI (read by A2) or by logical Carrier Detection. This must be selected at compiling!
   -It identifies via CW message; the Callsign, the QTH and the time interval can be set by user
   -The Identifier (Beacon 1) can be triggered by manual switch
   -During the QSO, a 4 tone sequence is superimposed when the timer for beacon is reached
   -Courtesy tone that indicate the voltage of the battery with tones rising or decreasing in frequency and single flat tone (for voltage >=11.5 && volts < 13.6). 
   -A "very low battery" tone to alert the operators to use it only if necessary (440 Hz for 300 msec)
   
   
   Many thanks to RW6HHL, Vladimir, for getting me out of my comfort zone and asking for new features and also pointed to some issues!
      
       This code is released under "Beerware License" (buy me a beer when you see me).
       Author: Adrian Florescu YO3HJV
       http:///www.yo3hjv@blogspot.com

 */

/////////////// Simple Arduino CW Beacon //////////////////

// Written by Mark VandeWettering K6HX
// Email: k...@arrl.net
//
// Thx Mark de YO3HJV !
//
struct t_mtab { char c, pat; } ;
struct t_mtab morsetab[] = {
      {'.', 106},
    {',', 115},
    {'?', 76},
    {'/', 41},
    {'A', 6},
    {'B', 17},
    {'C', 21},
    {'D', 9},
    {'E', 2},
    {'F', 20},
    {'G', 11},
    {'H', 16},
    {'I', 4},
    {'J', 30},
    {'K', 13},
    {'L', 18},
    {'M', 7},
    {'N', 5},
    {'O', 15},
    {'P', 22},
    {'Q', 27},
    {'R', 10},
    {'S', 8},
    {'T', 3},
    {'U', 12},
    {'V', 24},
    {'W', 14},
    {'X', 25},
    {'Y', 29},
    {'Z', 19},
    {'1', 62},
    {'2', 60},
    {'3', 56},
    {'4', 48},
    {'5', 32},
    {'6', 33},
    {'7', 35},
    {'8', 39},
    {'9', 47},
    {'0', 63}
        } ;
#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))
#define CW_SPEED  (28)
#define DOTLEN  (1200/CW_SPEED)
#define DASHLEN  (3.5*(1200/CW_SPEED))  // CW weight  3.5 / 1

////////// end CW section ////////////

///////// define repeater beacon Messages ////////////

#define  RPT_call       "YO3KSR"   // Call sign of the repeater
   
   // #define  RPT_ctcss      ("79.7")    // CTCSS tone uncomment if needed

#define  RPT_loc        ""    // Repeater QTH locator   
#define  END_msg        "QRV"        // Goodbye Message
   
int RssiThrs = 650;     // arbitrary value to detect the threshold of RSSI analog signal

//CW   tone output

#define  CW_PITCH        (700)   // CW pitch
   
 #define   REPEATbyRSSI   // Comment this line if the receiving radio have Carrier Detect logical output
                         // leave it like this if the receiving is determined by RSSI
// I/O pinout

int TxLedPin = 2;                  // pin for TX LED 
int rx = 6;                        // Carrier Detect digital input pin
int PttPin = 12;                   // PTT digital output pin
int CarDetPin = 6;                 // Carrier Detect Logic pin
int VccBatteryPin = A0;            // Analog input for battery voltage detection
int RssiPin = A2;                  // Read RSSI to decide Repeat in absence of Carrier Detect logical input 
int BeaconPin = 4;                 // Input pullup pin that will transmit Beacon 1 when tied to Ground
int  CW_pin = 5;                   // Pin for audio Beacons Courtesy tone output
////////////////////////////////////////////////////////////////////////////////////////////////////////////
float slaV = 0;                //  variable to store voltage from the ADC after the resistive divider
int vMath = 0;                 //  variable used for maths with ADC and voltage
float volts = 0;               //  volts from ADC readings

unsigned long previousMillis = 0;         // will store last time beacon transmitted           
const long interval = 450000;            // 60 min interval at which to //beacon (milliseconds)  3600000 15 min=900000 
          

int cDet = 0;                  // current state of the Carrier Detect
int lastcDet = 0;              // previous state of the Carrier Detect
bool isRepeating = false;      // To track repeater state for RSSI mode
   
  
 
void setup () {
    
            Serial.begin(9600);              // for debugging
              
               pinMode(CW_pin, OUTPUT);      // Output pin for beacon and audio
                //  digitalWrite(CW_pin, LOW);           // CW pin init
   
               pinMode(CarDetPin, INPUT_PULLUP) ;   //  Carrier Detect on pin 10 with transistor, tied up with 10 kOhm.
                                             //  when signal is received goes LOW.                             
               pinMode(PttPin, OUTPUT);          //  Here we have PTT. When transmission is needed, this pin goes HIGH.
              
               pinMode(TxLedPin, OUTPUT);           //  TX LED
               
               pinMode(BeaconPin, INPUT_PULLUP);    // Input pullup pin for manual beacon trigger
               
                Serial.println("ready");
                
                                     }
 
             

       
void CdetRepeat ()   
                   {
                cDet = digitalRead(CarDetPin);
                //Serial.print("CarDet state: ");
                //Serial.println(cDet);
                              if (cDet != lastcDet) {                 // if the state has changed                                                                    
                                   if (cDet == LOW) {                 // if the current state is HIGH then the button
                                                                      // went from off to on:
                                   digitalWrite(PttPin, HIGH);           //  Tx PTT @ +5V
                                   digitalWrite(TxLedPin, HIGH);         //  TX LED ON
                                                     }
                              else {                                  // same state @cDet input
                                      delay(20);                      // wait a little before courtesy tone
                                      courtesy();                     // execute 'courtesy' function
                                      delay(70);                      // Wait a little while TX
                                      digitalWrite(PttPin, LOW);         // put Tx PTT @ GND, go to StandBy
                                      digitalWrite(TxLedPin, LOW);
                                   }
                             delay(50);                               // Delay a little bit to avoid bouncing
                                                      }
                                                  
                 lastcDet = cDet;                                     // save the current state as the last state
                                                                      // for next time through the loop
                   }  // END of 'repeat' function


void RssiRepeat() {
    // 1. Take 3 measurements with 7ms delay
    int rssiValue1 = analogRead(RssiPin);
    delay(7);
    int rssiValue2 = analogRead(RssiPin);
    delay(7);
    int rssiValue3 = analogRead(RssiPin);

    // 2. Calculate average
    int avgRssi = (rssiValue1 + rssiValue2 + rssiValue3) / 3;

    // 3. Check against threshold
    if (avgRssi > RssiThrs) {
        // Signal is strong enough
        if (!isRepeating) {
            // If not already repeating, start repeating
            digitalWrite(PttPin, HIGH);
            digitalWrite(TxLedPin, HIGH);
            isRepeating = true;
        }
        // If already repeating, do nothing, just keep transmitting.
    } else {
        // Signal is weak or absent
        if (isRepeating) {
            // If we were repeating, stop it now.
            delay(20);
            courtesy();
            delay(70);
            digitalWrite(PttPin, LOW);
            digitalWrite(TxLedPin, LOW);
            isRepeating = false;
        }
        // If not repeating, do nothing.
    }
}
     
     
     
     
          // Function to return the battery voltage     
          // Better to call it when transmitting to have a 'worst case' measurement      
                   
float battery() {
                     delay(50);                                       //  hold on before ADC reading
                     int vMath = analogRead(VccBatteryPin);           //  Load the SLA BAttery voltage through resistive divider on ADC A0
                     float slaV = vMath * (15 / 1023.0);              //  slaV will be calculated in Volts  with max value 15V                       
                     return slaV;                                     //  The 'battery' function returning voltage measuring
                     }   
 
  
      // *************  MAIN LOOP  ***************
              
void loop() {   
                // Read and print RSSI value continuously
                int rssiValue = analogRead(RssiPin);
                Serial.print("RSSI: ");
                Serial.println(rssiValue);
                
                // Check if manual beacon button is pressed (pin D4 is LOW)
                if (digitalRead(BeaconPin) == LOW) {
                    // Manual beacon trigger detected
                    digitalWrite(PttPin, HIGH);              // Start Tx
                    digitalWrite(TxLedPin, HIGH);            // PTT LED ON
                    delay(100);                              // Wait a little
                    beacon1();                               // Transmitting Audio Beacon 1
                    delay(100);                              // Wait a little
                    digitalWrite(PttPin, LOW);               // Go in StandBy
                    digitalWrite(TxLedPin, LOW);             // LED TX OFF
                    delay(500);                              // Debounce delay to prevent multiple triggers
                }
                
                #ifdef REPEATbyRSSI
                    RssiRepeat();                             // Detect receiving from RSSI level
                #else
                    CdetRepeat();                             // Detect receiving from Carrier Detect 
                #endif
                                                          // called  at the beggining
                      
                delay(100);                                  // AntiKerchunk delay
                                                             // Beacon timer  
                 unsigned long currentMillis = millis();     // Read millis (from first start of the board) and load value to current Millis variable

                                if (currentMillis - previousMillis >= interval)  { 
                                    previousMillis = currentMillis;                  // Time to beacon (INTERVAL) has come
                                    cDet = digitalRead(CarDetPin);                   // Read the Carrier Detect Input pin
                            
                                       if (cDet == HIGH){                            // If not QSO on the Rx frequency
                                            digitalWrite(PttPin, HIGH);              // Start Tx
                                            digitalWrite(TxLedPin, HIGH);            // PTT LED ON
                                            delay(100);                              // Wait a little
                                             beacon1();                              // Transmitting Audio Beacon 1
                                            delay(100);                              // Wait a little
                                            digitalWrite(PttPin, LOW);               // Go in StandBy
                                            digitalWrite(TxLedPin, LOW);             // LED TX OFF
                                                         }
                          
                                        else if (cDet == LOW) {                      // If a QSO on the RX frequency
                                           delay(100);                               // Wait a little          
                                             beacon2();                              // Transmitting Audio Beacon 2
                                           digitalWrite(PttPin, HIGH);   }           // Keep the repeater transmitting
                                           digitalWrite(TxLedPin, HIGH);             // LED TX ON
                                                                                   }
                                else {}                                              // If time to beacone hasn't come (INTERVAL), do nothing     
   
               }
 
 
        // **** Audio Beacon 1 - for StandBy (No QSO in progress)
void beacon1() {
      
          delay (500);
       
  delay (100);                                     // Wait a little
              sendmsg(RPT_call) ;                  //send call
              delay(7*DOTLEN) ;
              //  sendmsg("QRV") ;                     //send info
              //  delay(7*DOTLEN) ;
              //  sendmsg(RPT_ctcss);                // uncomment this two lines if there is a CTCSS enabled on the repeater
              //  delay(7*DOTLEN) ;
              sendmsg(RPT_loc) ;                   //send qth locator
              delay(7*DOTLEN) ;
              sendmsg(END_msg);
              delay(3*DOTLEN) ;
                         return;  }
                          
        // **** Audio Beacon 2 - For repeating (QSO in progress)
       
        void beacon2() {
              delay(100);
            tone(CW_pin, 2880, 100);
          delay(200);
            tone(CW_pin, 1740, 100);
          delay(200);
            tone(CW_pin, 1200, 100);
          delay(500);
            tone(CW_pin, 880, 200);
                          return; }
   
      void courtesy() {
                                                                     // We read the voltage of the battery via 'battery' external function
               volts = battery() ;
                    
               //Serial.println();
               //Serial.print(volts);
                    delay(50);                                      // Wait for the portable radio who stops transmitting to get into the receiving mode to be able to hear the tone


            if ((volts >= 0) && (volts < 10.5)) {                         // Practically, this will end the normal operation leaving only the receiver section open to monitor the frequency. Beep is just for tests.
   
                tone(5, 440, 300);
                delay(301);
                              }
           
            else if ( volts >= 10.5 && volts < 11.5) {     // Watch out! The battery goes low, limit comms!

                tone(5, 1880, 200);
                delay(201);
                tone(5, 440, 200);
                delay(201);
                                                   }
           
           
            else if ( volts >=11.5 && volts < 13.6) {     // This is OK, normal operations

                tone(5, 2900, 70);
                delay(71);
                                                    }
           
            else if (volts >= 13.6) {                     // Over voltage for some reasons. Not normal!

                tone (5, 1880, 200);
                delay(201);
                tone(5, 2880, 200);                   // We have to do something to discharge the battery!
                delay(201);
                                    }

              
                       }




/////////////////////////    CW GENERATION ROUTINES  //////////////////////////
void dash() {
    tone(CW_pin,CW_PITCH);
    delay(DASHLEN);
    noTone(CW_pin);    
    delay(DOTLEN) ;
    }
////////////////////////////
void dit() {
  tone(CW_pin,CW_PITCH);
  delay(DOTLEN);
  noTone(CW_pin);   
  delay(DOTLEN);
  }
///////////////////////////
void send(char c) {
  int i ;
    if (c == ' ') {
    delay(7*DOTLEN) ;
    return ;
    }
   if (c == '+') {
    delay(4*DOTLEN) ;
    dit();
    dash();
    dit();
    dash();
    dit();
    delay(4*DOTLEN) ;
    return ;
    }   
   
  for (i=0; i<N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat ;
      while (p != 1) {
          if (p & 1)
            dash() ;
          else
            dit() ;
          p = p / 2 ;
          }
      delay(2*DOTLEN) ;
     // wdt_reset();        // Reset Watchdog timer
      return ;
      }
    }
  }
///////////////////////////
void sendmsg(char *str) {
  while (*str)
    send(*str++) ;
  }
 

        
/////////////////////////  END  CW GENERATION ROUTINES  //////////////////////////

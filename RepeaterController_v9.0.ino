/*
   This junk is meant to controll a small repeater in UHF made of two portable radios
   Motorola GP300.
   v_9.0

   MAIN FEATURES
   -The repeater can be activated by RSSI (read by A2) or by logical Carrier Detection, selectable at power-on via pin 8
   -Pin 8 LOW (grounded) = RSSI mode, HIGH = Carrier Detect mode
   -Improved hysteresis and non-blocking timing for both RSSI and Carrier Detect modes to prevent erratic toggling
   -Dual thresholds for RSSI (RssiThrsHigh = 12, RssiThrsLow = 9) with configurable timing constants
   -Serial output of RSSI values when in RSSI mode (controlled by RssiDebug define)
   -Beacon 1. The Identifier is triggered at fixed, user defined interval (in minutes). Can also be triggered by manual switch
   -It identifies via CW message; the Callsign, the QTH and the time interval can be set by user
   -Beacon 2. During the QSO, a 4 tone sequence is superimposed when the timer for beacon is reached
   -Courtesy tone that indicate the voltage of the battery with tones rising or decreasing in frequency and single flat tone (for voltage >=11.5 && volts < 13.6). 
   -A "very low battery" tone to alert the operators to use it only if necessary (740 Hz for 300 msec)
   -Beacon 1 timer is resetting after each QSO; if no QSO, the Beacon 1 is transmitted at fixed defined intervals
   
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
#define CW_SPEED  (30)
#define DOTLEN  (1200/CW_SPEED)
#define DASHLEN  (3.5*(1200/CW_SPEED))  // CW weight  3.5 / 1

////////// end CW section ////////////

///////// define repeater beacon Messages ////////////

#define  RPT_call       "YO3KSR"   // Call sign of the repeater
   
 #define  RPT_ctcss      ("79.7")    // CTCSS tone uncomment if needed

#define  RPT_loc        ""    // Repeater QTH locator   
#define  END_msg        "QRV"        // Goodbye Message
   
// Forward declarations for functions
void dash();
void dit();
void send(char c);
void sendmsg(char *str);
void courtesy();
float battery();
void beacon1();
void beacon2();
void RssiRepeat();
void CdetRepeat();

// RSSI thresholds with hysteresis
int RssiThrsHigh = 12;                // High threshold to activate repeater
int RssiThrsLow = 9;                  // Low threshold to deactivate repeater
// Timing constants for hysteresis
const unsigned long timeOpen = 120;     // Time in ms signal must be present to activate
const unsigned long timeSustain = 250;  // Time in ms signal must be absent to deactivate

unsigned long previousMillis = 0;         // will store last time beacon transmitted           
const long Beacon1_timer = 7;             // Beacon 1 interval in minutes 

//CW tone output
#define  CW_PITCH        (700)   // CW pitch
   
// Mode selection will now be done via pin 8 at startup
// #define REPEATbyRSSI   // No longer needed as mode is selected at runtime
#define RssiDebug          // Uncommented to see current RSSI on serial when in RSSI mode
                         
// I/O pinout

int TxLedPin = 2;                  // pin for TX LED 
int PttPin = 12;                   // PTT digital output pin
int CarDetPin = 6;                 // Carrier Detect Logic pin
int VccBatteryPin = A0;            // Analog input for battery voltage detection
int RssiPin = A2;                  // Read RSSI to decide Repeat in absence of Carrier Detect logical input 
int BeaconPin = 4;                 // Input pullup pin that will transmit Beacon 1 when tied to Ground
int CW_pin = 5;                    // Pin for audio Beacons Courtesy tone output
int ModeSelectPin = 8;             // Pin to select between RSSI mode (LOW) and Carrier Detect mode (HIGH)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
float slaV = 0;                //  variable to store voltage from the ADC after the resistive divider
int vMath = 0;                 //  variable used for maths with ADC and voltage
float volts = 0;               //  volts from ADC readings

// Mode selection variable
bool useRssiMode = false;      // Will be set in setup() based on ModeSelectPin
          


// Variables for RSSI hysteresis
unsigned long rssiHighStartTime = 0;  // When signal first went above high threshold
unsigned long rssiLowStartTime = 0;   // When signal first went below low threshold
bool signalAboveHigh = false;         // Tracking if signal is above high threshold
bool signalBelowLow = false;          // Tracking if signal is below low threshold

// Variables for Carrier Detect hysteresis
unsigned long cdetOnStartTime = 0;    // When carrier detect first went active
unsigned long cdetOffStartTime = 0;   // When carrier detect first went inactive
bool cdetWasOn = false;               // Tracking if carrier detect was active
bool cdetWasOff = false;              // Tracking if carrier detect was inactive

int cDet = 0;                  // current state of the Carrier Detect
int lastcDet = 0;              // previous state of the Carrier Detect
bool isRepeating = false;      // To track repeater state for RSSI mode


void setup () {
    
            Serial.begin(9600);              // for debugging
              
               pinMode(CW_pin, OUTPUT);      // Output pin for beacon and audio
                //  digitalWrite(CW_pin, LOW);           // CW pin init
   
               pinMode(CarDetPin, INPUT_PULLUP);    // Carrier Detect when signal is received goes LOW                             
               pinMode(PttPin, OUTPUT);             // Here we have PTT. When transmission is needed, this pin goes HIGH              
               pinMode(TxLedPin, OUTPUT);           // TX LED               
               pinMode(BeaconPin, INPUT_PULLUP);    // Input pullup pin for manual beacon trigger
               pinMode(ModeSelectPin, INPUT_PULLUP); // Mode selection pin
               
               // Check mode selection pin at startup
               useRssiMode = (digitalRead(ModeSelectPin) == LOW); // If LOW (grounded), use RSSI mode
               
               if (useRssiMode) {
                   Serial.println("Mode: RSSI Repeat");
               } else {
                   Serial.println("Mode: Carrier Detect");
               }
               
               Serial.println("ready");
                                     }
 
             

       
void CdetRepeat() {
    cDet = digitalRead(CarDetPin);
    unsigned long currentMillis = millis();
    
    // Check if carrier detect is active (LOW)
    if (cDet == LOW) {
        if (!cdetWasOn) {
            // Carrier detect just became active
            cdetWasOn = true;
            cdetOnStartTime = currentMillis;
        }
        // Reset the off timer since we're active
        cdetWasOff = false;
        
        // Check if carrier has been active long enough to activate repeater
        if (currentMillis - cdetOnStartTime >= timeOpen && lastcDet != LOW) {
            // Activate repeater
            digitalWrite(PttPin, HIGH);
            digitalWrite(TxLedPin, HIGH);
            previousMillis = millis();  // Reset Beacon 1 timer at each new QSO
            lastcDet = LOW;  // Update state
        }
    }
    // Check if carrier detect is inactive (HIGH)
    else {
        if (!cdetWasOff) {
            // Carrier detect just became inactive
            cdetWasOff = true;
            cdetOffStartTime = currentMillis;
        }
        // Reset the on timer since we're inactive
        cdetWasOn = false;
        
        // Check if carrier has been inactive long enough to deactivate repeater
        if (currentMillis - cdetOffStartTime >= timeSustain && lastcDet != HIGH) {
            // Deactivate repeater
            courtesy();
            digitalWrite(PttPin, LOW);
            digitalWrite(TxLedPin, LOW);
            lastcDet = HIGH;  // Update state
        }
    }
}


void RssiRepeat() {
    // 1. Take RSSI measurements (no delays)
    int rssiValue1 = analogRead(RssiPin);
    int rssiValue2 = analogRead(RssiPin);
    int rssiValue3 = analogRead(RssiPin);
    int avgRssi = (rssiValue1 + rssiValue2 + rssiValue3) / 3;
    
    // 2. Apply hysteresis logic with time validation
    unsigned long currentMillis = millis();
    
    // Check if signal is above high threshold
    if (avgRssi > RssiThrsHigh) {
        if (!signalAboveHigh) {
            // Signal just crossed above high threshold
            signalAboveHigh = true;
            rssiHighStartTime = currentMillis;
        }
        // Reset the low threshold timer since we're above high threshold
        signalBelowLow = false;
        
        // Check if signal has been above threshold long enough to activate
        if (!isRepeating && (currentMillis - rssiHighStartTime >= timeOpen)) {
            // Activate repeater
            digitalWrite(PttPin, HIGH);
            digitalWrite(TxLedPin, HIGH);
            isRepeating = true;
            previousMillis = millis();  // Reset Beacon 1 timer at each new QSO
        }
    }
    // Check if signal is below low threshold
    else if (avgRssi < RssiThrsLow) {
        if (!signalBelowLow) {
            // Signal just crossed below low threshold
            signalBelowLow = true;
            rssiLowStartTime = currentMillis;
        }
        // Reset the high threshold timer since we're below low threshold
        signalAboveHigh = false;
        
        // Check if signal has been below threshold long enough to deactivate
        if (isRepeating && (currentMillis - rssiLowStartTime >= timeSustain)) {
            // Deactivate repeater
            courtesy();
            digitalWrite(PttPin, LOW);
            digitalWrite(TxLedPin, LOW);
            isRepeating = false;
        }
    }
    // Signal is between thresholds - maintain current state
    else {
        // Reset both timers since we're in the hysteresis zone
        signalAboveHigh = false;
        signalBelowLow = false;
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

// Forward declarations for CW functions
void dash();
void dit();
void send(char c);
void sendmsg(char *str);

// Courtesy tone function that indicates battery voltage
void courtesy() {
    // We read the voltage of the battery via 'battery' external function
    volts = battery();
    
    delay(50);    // Wait for the portable radio who stops transmitting to get into the receiving mode to be able to hear the tone

    if ((volts >= 0) && (volts < 10.5)) {    // Practically, this will end the normal operation leaving only the receiver section open to monitor the frequency
        tone(5, 740, 300);    // Very low battery warning
        delay(301);
    }
    else if (volts >= 10.5 && volts < 11.5) {    // Watch out! The battery goes low, limit comms!
        tone(5, 1680, 200);
        delay(201);
        tone(5, 1040, 200);
        delay(201);
    }
    else if (volts >= 11.5 && volts < 13.6) {    // This is OK, normal operations
        tone(5, 1700, 90);
        delay(71);
    }
    else if (volts >= 13.6) {    // Over voltage for some reasons. Not normal!
        tone(5, 1880, 200);
        delay(201);
        tone(5, 2480, 200);    // We have to do something to discharge the battery!
        delay(201);
    }
}
 
  
      // *************  MAIN LOOP  ***************
              
void loop() {   
                // Read RSSI value and print it if in RSSI mode and debug is enabled
                int rssiValue = analogRead(RssiPin);
                if (useRssiMode) {
                    #ifdef RssiDebug
                      Serial.print("RSSI: ");
                      Serial.println(rssiValue);
                    #endif
                }
                
                // Check if manual beacon button is pressed (pin D4 is LOW)
                if (digitalRead(BeaconPin) == LOW) {
                    // Manual beacon trigger detected
                    digitalWrite(PttPin, HIGH);              // Start Tx
                    digitalWrite(TxLedPin, HIGH);            // PTT LED ON
                    delay(50);                              // Wait a little
                    beacon1();                               // Transmitting Audio Beacon 1
                    delay(50);                              // Wait a little
                    digitalWrite(PttPin, LOW);               // Go in StandBy
                    digitalWrite(TxLedPin, LOW);             // LED TX OFF
                    delay(150);                              // Debounce delay to prevent multiple triggers
                }
                
                // Use the appropriate repeater mode based on pin 8 at startup
                if (useRssiMode) {
                    RssiRepeat();                             // Detect receiving from RSSI level
                } else {
                    CdetRepeat();                             // Detect receiving from Carrier Detect 
                }
                                                          // called  at the beggining
                      
                delay(50);                                  // AntiKerchunk delay
                                                             // Beacon timer  
                 unsigned long currentMillis = millis();     // Read millis (from first start of the board) and load value to current Millis variable

                                if (currentMillis - previousMillis >= (Beacon1_timer * 60000))  { 
                                    previousMillis = currentMillis;                  // Time to beacon has come
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
                                           beacon2();                                // Transmitting Audio Beacon 2
                                           digitalWrite(PttPin, HIGH);               // Keep the repeater transmitting
                                           digitalWrite(TxLedPin, HIGH);             // LED TX ON
                                        }
                                    }
                                else {}                                              // If time to beacone hasn't come (INTERVAL), do nothing     
   
               }
 
 
        // **** Audio Beacon 1 - for StandBy (No QSO in progress)
void beacon1() {
      
          //delay (200);
       
  delay (50);                                     // Wait a little
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
              delay(50);
            tone(CW_pin, 2880, 80);
          delay(200);
            tone(CW_pin, 1740, 80);
          delay(200);
            tone(CW_pin, 1200, 80);
          delay(500);
            tone(CW_pin, 880, 80);
                          return; }
   
/////////////////////////  CW GENERATION ROUTINES  //////////////////////////
void dash() {
    tone(CW_pin,CW_PITCH);
    delay(DASHLEN);
    noTone(CW_pin);    
    delay(DOTLEN) ;
    }
//////////////////////////
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
/////////////////////////
void sendmsg(char *str) {
  char c;
  while ( (c = *str++) )
    send(c) ;
  }
 

        
/////////////////////////  END  CW GENERATION ROUTINES  //////////////////////////

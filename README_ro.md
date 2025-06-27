
# Controller repetor radio cu Arduino v9.0

Un controler de repetor simplu, bazat pe Arduino pentru stații radio (precum Motorola GP300), cu mod  dublu de activare, histerezis, temporizare non-blocantă și funcții variate pentru balize și ton de curtoazie.

## Caracteristici

### Funcționalitate de bază
- **Moduri duble de activare**: Alege între activare pe bază de RSSI sau pe bază de Detectare a conditiei logice a modului la pornire**: Selectează modul la pornire prin pinul 8 (LOW = modul RSSI, HIGH = modul Carrier Detect)
  - **Fără recompilare**: Schimbă modul doar conectând/deconectând pinul 8 la masa (GND)

### Procesarea semnalului
- **Hsterezis îmbunătățit**: Praguri duble pentru a preveni comutarea eratică in mod RSSI
  - Prag RSSI Ridicat (implicit: 12) pentru a activa releul
  - Prag RSSI Scăzut (implicit: 9) pentru a dezactiva releul
- **Temporizare non-blocantă**: Folosește `millis()` în loc de `delay()` pentru funcționare rapidă
  - `timeOpen` (implicit: 120ms): Timpul în care semnalul trebuie să fie prezent pentru a activa (Anti-kerchunk)
  - `timeSustain` (implicit: 250ms): Timpul în care semnalul trebuie să lipsească pentru a dezactiva

### Balize și tonuri
- **Beacon 1**: Identificator CW transmis la intervale fixe
  - Interval configurabil în minute
  - Include indicativul, locatorul QTH și alte mesaje personalizabile (necesita recompilare)
  - Poate fi declanșat manual prin buton
- **Beacon 2**: Secvență de patru tonuri redată în timpul QSO-urilor active când expiră cronometrul balizei
- **Tonuri de curtoazie**: Indică starea tensiunii bateriei prin tonuri diferite
  - Baterie foarte descărcată: 740Hz timp de 300ms (< 10.5V)
  - Baterie descărcată: secvență 1680Hz + 1040Hz (10.5V - 11.5V)
  - Funcționare normală: 1700Hz timp de 90ms (11.5V - 13.6V)
  - Supratensiune: secvență 1880Hz + 2480Hz (> 13.6V)

### Monitorizare
- **Ieșire serială**: Afișează valoarea curenta a RSSI în modul RSSI pentru reglajul fin 
- **Monitorizarea bateriei**: Monitorizează constant tensiunea bateriei pentru tonurile de curtoazie

## Conexiuni hardware

### Atribuirea pinilor Arduino

| Pin | Funcție | Descriere |
|-----|----------|-------------|
| A0  | VccBatteryPin | Monitorizare tensiune baterie prin divizor de tensiune |
| A2  | RssiPin | Intrare RSSI de la receptor |
| D2  | TxLedPin | LED indicator TX |
| D4  | BeaconPin | Buton declanșare far manual (activ LOW) |
| D5  | CW_pin | Ieșire audio pentru faruri și tonuri |
| D6  | CarDetPin | Intrare Detectare purtător (Carrier Detect) (activ LOW) |
| D8  | ModeSelectPin | Selectare mod la pornire (LOW = RSSI, HIGH = Carrier Detect) |
| D12 | PttPin | Ieșire PTT către emițător |

### Diagrama conexiunilor

```
                                  +------------+
                                  |            |
Divizor tensiune baterie -------- | A0         |
                                  |            |
RSSI de la receptor ------------- | A2         |
                                  |            |
LED TX -------------------------- | D2         |
                                  |            |
Buton Beacon Manual ------------- | D4         |
                                  |            |
Ieșire Audio -------------------- | D5    ARD  |
                                  |      UINO  |
Intrare Carrier Detect ---------- | D6         |
                                  |            |
Comutator Selectare Mod --------- | D8         |
                                  |            |
PTT către emițător -------------- | D12        |
                                  |            |
                                  +------------+
```

## Variabile de Configurat

### Variabile cheie de ajustat

#### Parametrii de activare a releului
```cpp
// Praguri RSSI cu histerezis
int RssiThrsHigh = 12;                // Prag superior pentru activare
int RssiThrsLow = 9;                  // Prag inferior pentru dezactivare

// Timp de control pentru histerezis
const unsigned long timeOpen = 120;     // Timp în ms pentru activare
const unsigned long timeSustain = 250;  // Timp în ms pentru dezactivare
```

#### Setări Beacon
```cpp
// Cronometru Beacon
const long Beacon1_timer = 7;             // Interval Beacon 1 în minute

// Setări CW
#define  DOTLEN         (100)      // Durata punctului Morse în ms
#define  DASHLEN        (DOTLEN*3) // Durata liniei Morse
#define  CW_PITCH       (700)      // Frecvența CW în Hz

// Mesaje Beacon
#define  RPT_call       "YOURCALL"  // Introdu indicativul tău
#define  RPT_loc        "KN00XX"    // Introdu locatorul tău QTH
#define  RPT_ctcss      "CTCSS 88.5 HZ"  // Info CTCSS, dacă este setat in statiile repetorului
#define  END_msg        "QRV"       // Mesaj de încheiere
```

#### Opțiuni de depanare
```cpp
#define RssiDebug          // Decomentează pentru a vedea valorile RSSI pe serial în modul RSSI
```

## Instalare și Configurare

1. Conectează Arduino conform pinilor de mai sus
2. Ajustează variabilele de configurare din cod:
   - Setează indicativul și locatorul QTH
   - Ajustează pragurile RSSI în funcție de receptor
   - Setează intervalul Beacon după necesitate
3. Încarcă codul pe Arduino
4. Conectează pinul 8 la GND pentru modul RSSI sau lasă-l neconectat pentru modul Carrier Detect
5. Alimentează sistemul

## Sfaturi de Reglaj

### Modul RSSI
1. Monitorizează valorile RSSI prin ieșirea serială
2. Ajustează `RssiThrsHigh` și `RssiThrsLow` în funcție de valorile observate:
   - `RssiThrsHigh` trebuie să activeze releul la semnal valid
   - `RssiThrsLow` trebuie să prevină dezactivarea prematură
   - Diferența dintre praguri creează histerezis

### Constante de temporizare
1. Mărește `timeOpen` dacă releul se activează prea ușor la zgomot
2. Mărește `timeSustain` dacă releul se dezactivează prea repede în pauze

## Credite
Cod original de Adrian Florescu YO3HJV, cu rutine CW de Mark VandeWettering K6HX.
Îmbunătățit cu histerezis și selecție de mod în timpul rulării.

## Licență
Codul este publicat sub licența "Beerware" (cumpără autorului o bere când îl vezi).

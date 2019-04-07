#include <Keyboard.h>

// define all the pins
const int freqOutputPin = 9;      // OC1A output pin for ATmega32u4 (Arduino Micro)
const int ocr1aval  = 8;          // 0 = 8MHz, 1 = 4MHz,  3 = 2MHz, 7 = 1MHz
const int CA2Pin = 10;            // Goes high if there's a keypress
const int USBEnable = A0;         // To switch the USB KB functionality of the machine off, put this pin to ground.
const int WPin = A1;              // Goes high when the right Row/Col combination is found
const int enkbPin = A2;           // Enable KB Scan/Query : used to stop the scanning of the keyboard. Default state High, Low enables querying of the matrix.
const int BreakPin = A3;       
const int ColPins[4] = {5, 4, 3, 2};  
const int RowPins[3] = {6, 7, 8};  
const int RIGHT_LED = 15;
const int MIDDLE_LED = 14;
const int LEFT_LED = 16;
const int LED_ALL = -1;

// LED states
const int LED_ON = LOW;     // These are reversed as we need to ground the LEDs to turn them on.
const int LED_OFF = HIGH;

const int repeatDelay = 100; // Defined in ms

// needed for reading the analogue values of the 'key pressed(CA2)' and 'correct guess(W)' pins. 
// The output of these is somewhere around 2v, so not high enough for a digitalRead()
int CA2Val = 0;
int WVal = 0;
int CA2Low = 0;
int WLow = 0;
int WLowOffset = 200;     // These two values are used to stop the detection of false positives, values just found by experimentation 
int CA2LowOffset = 100;

bool shiftPressed = false;
bool shiftLockActive = false;
bool capsLockActive = false;

const bool debug = true;

//char keyboardMatrix [8][10] = {{"SHFT"},{"CTRL"},{"*"},{"*"},{"*"},{"*"},{"*"},{"*"},{"*"},{"*"},
//                {"TAB"},{"Z"},{"SPACE"},{"V"},{"B"},{"M"},{","},{"."},{"/"},{"COPY"},
//                {"SHFT LK"},{"S"},{"C"},{"G"},{"H"},{"N"},{"L"},{";"},{"]"},{"DEL"},
//                {"CAPS LK"},{"A"},{"X"},{"F"},{"Y"},{"J"},{"K"},{"@"},{":"},{"RTN"},
//                {"1"},{"2"},{"D"},{"R"},{"6"},{"U"},{"O"},{"P"},{"["},{"UP"},
//                {"F0"},{"W"},{"E"},{"T"},{"7"},{"9"},{"I"},{"0"},{"£"},{"DWN"},
//                {"Q"},{"3"},{"4"},{"5"},{"F4"},{"8"},{"F7"},{"="},{"~"},{"LFT"},
//                {"ESC"},{"F1"},{"F2"},{"F3"},{"F5"},{"F6"},{"F8"},{"F9"},{"\"}, {"RHT"}};


// explicitly define the keymappings in integers as ascii
//    -1 = go to lower
//    -2 = go to upper
//    -99 = shift lock
//    any other negative = keypress not sent
int lowerKeyboardMatrix [8][10] = { {129, 128, -50, -50, -50, -50, -50, -50, -50, -50},
                                    {179, 122, 32, 118, 98, 109, 44, 46, 47, -50},
                                    {-99, 115, 99, 103, 104, 110, 108, 59, 93, 178},
                                    {193, 97, 120, 102, 121, 106, 107, 34, 58, 176},
                                    {49, 50, 100, 114, 54, 117, 111, 112, 91, 218},
                                    {203, 119, 101, 116, 55, 105, 57, 48, 95, 217},
                                    {113, 51, 52, 53, 197, 56, 200, 45, 94, 216},
                                    {27, 194, 195, 196, 198, 199, 201, 202, 92, 215}};

int upperKeyboardMatrix [8][10] = { {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
                                    {-1, 90, -1, 86, 66, 77, -1, -1, -1, -1},
                                    {-1, 83, 67, 71, 72, 78, 76, -1, -1, -1},
                                    {-1, 65, 88, 70, 89, 74, 75, -1, -1, -1},
                                    {-1, -1, 68, 82, -1, 85, 79, 80, -1, -1},
                                    {-1, 87, 69, 84, -1, 73, -1, -1, -1, -1},
                                    {81, -1, -1, -1, -1, -1, -1, -1, -1, -1},
                                    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};
                                    
int shiftKeyboardMatrix [8][10] = { {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
                                    {-2, -2, -2, -2, -2, -2, 60, 62, 63, -2},
                                    {-2, -2, -2, -2, -2, -2, -2, 43, 125, -2},
                                    {-2, -2, -2, -2, -2, -2, -2, -2, 42, -2},
                                    {33, 64, -2, -2, 38, -2, -2, -2, 123, -2},
                                    {-2, -2, -2, -2, 39, -2, 41, -2, 35, -2},
                                    {-2, 92, 36, 37, -2, 40, -2, 61, 124, -2},
                                    {-2, -2, -2, -2, -2, -2, -2, -2, 126, -2}};

bool shiftActive(){
  // two different ways of shift being active on this board, shiftlock or holding shift.
  if (shiftPressed || shiftLockActive){
    Serial.println(true);
    return true;
  }else{
    Serial.println(false);
    return false; 
  }
}

int sendKey(int row, int col){
    int key = -1;
    if (shiftActive()){
      // try and get the key from the shift matrix, if this is -1, then it defaults to the normal kb matrix. 
      // this negates the need to rewrite both matricies to remap the kb.
      key = shiftKeyboardMatrix[row][col];
    }
    
    if (capsLockActive | key == -2){
      key = upperKeyboardMatrix[row][col];
    }

    if (key == -1){
      key = lowerKeyboardMatrix[row][col];
    }

    if (key == -99){
      toggleShiftLock();
    }

//    if (key == -98){
//      toggleCapsLock();
//    }

    if (key >= 0 ){
      if (debug){
        Serial.print(key);
        Serial.print(" | ");
        Serial.println(char(key));
      }
      Keyboard.write(char(key));
    }
}

void toggleShiftLock(){
    if (shiftLockActive){
//      Keyboard.release(KEY_LEFT_SHIFT);
      shiftLockActive = false;
    }else{
//      Keyboard.press(KEY_LEFT_SHIFT);
      shiftLockActive = true;
    }  
}

void setup(){
    if (debug){
      Serial.begin(9600);
    }
    pinMode(freqOutputPin, OUTPUT);
    // setup the clock.
    TCCR1A = ((1 << COM1A0));
    TCCR1B = ((1 << WGM12) | (1 << CS10));
    TIMSK1 = 0;
    OCR1A = ocr1aval;   

    pinMode(USBEnable, INPUT_PULLUP);
    
    // start the kb scanning
    pinMode(enkbPin, OUTPUT);
    digitalWrite(enkbPin, HIGH);  
    
    // analogue input rather than digial, as 'high' does not equate to a digital high, voltage is too low
    pinMode(CA2Pin, INPUT);
    pinMode(WPin, INPUT);
    pinMode(BreakPin, INPUT);
    
    pinMode(LEFT_LED, OUTPUT);
    pinMode(MIDDLE_LED, OUTPUT);
    pinMode(RIGHT_LED, OUTPUT);
    
    digitalWrite(LEFT_LED, LED_OFF);
    digitalWrite(MIDDLE_LED, LED_OFF);
    digitalWrite(RIGHT_LED, LED_OFF);
    
    for (int thisPin = 0; thisPin < 4; thisPin++){
      pinMode(ColPins[thisPin], OUTPUT);
      digitalWrite(ColPins[thisPin], LOW);
    }

    for (int thisPin = 0; thisPin < 3; thisPin++){
      pinMode(RowPins[thisPin], OUTPUT);
      digitalWrite(RowPins[thisPin], LOW);
    }

    CA2Low = analogRead(CA2Pin) + CA2LowOffset;
    WLow = analogRead(WPin) + WLowOffset;

    // start the USB keyboard emulation
    Keyboard.begin();
    // reset scrolllock, capslock and numlock
    Keyboard.releaseAll();
}

bool keyPressed(){
  CA2Val = analogRead(CA2Pin);
  if (debug){
    Serial.print("CA2: ");
    Serial.println(CA2Val);  
  }
  if (CA2Val <= CA2Low){
    CA2Low = CA2Val + CA2LowOffset;
    digitalWrite(LEFT_LED, LED_OFF);
    return false;
  }else{
    digitalWrite(LEFT_LED, LED_ON);
    return true;
  }
}

void digitalWritePins(int numPins, const int pinNumbers[], int states[]){
  // takes an array of pins and pin states to bulk write to them
  // could use pin groups and binary mappings, but this seems more readable
  for (int thisPin = 0; thisPin < numPins; thisPin++){
    // reset all the pins to be low every time
    digitalWrite(pinNumbers[thisPin], LOW);
    if (states[thisPin] > 0){
      digitalWrite(pinNumbers[thisPin], HIGH);
    }
  }
}


int findKey(){
  // the way this works is that it polls the row and cols ICs with binary counting, then checks the 'W' pin. 
  // if the WPin goes high, we've got the right combination of rows and cols, if not then continue to the next row/col combination.
  // The [row,col] index can be mapped to a matrix that has the values for which key that is.
  // we don't break the loop after we've found a keypress, as we need to detect more than one at once. 
  
  // http://www.skot9000.com/ttl/datasheets/251.pdf
  // row order is not the same as the diagram layout
  // 000, 011, 101, 001, 110, 010, 100, 111
  //   0,   1,   2,   3,   4,   5,   6,   7
  int numRowStates = 8;
  int rowStates[numRowStates][3] = {{0,0,0}, {0,1,1}, {1,0,1}, {0,0,1}, {1,1,0}, {0,1,0}, {1,0,0}, {1,1,1}};

  
  // http://www.ti.com/lit/ds/symlink/sn7445.pdf
  // col layout is the same as the diagram layout
  // summary:
  // 0000, 0001, 0010, 0011, 0100, 0101, 0110, 0111, 1000, 1001
  //    0,    1,    2,    3,    4,    5,    6,    7,    8,    9
  int numColStates = 10;
  int colStates[numColStates][4] = {{0,0,0,0}, {0,0,0,1}, {0,0,1,0}, {0,0,1,1}, {0,1,0,0}, {0,1,0,1}, {0,1,1,0}, {0,1,1,1}, {1,0,0,0}, {1,0,0,1}};

  for (int thisRowState = 0; thisRowState < numRowStates; thisRowState++){
    digitalWritePins(3, RowPins, rowStates[thisRowState]);
      for (int thisColState = 0; thisColState < numColStates; thisColState++){
        digitalWritePins(4, ColPins, colStates[thisColState]);
        if (wHigh()){
          if (debug){
            Serial.print("Row: ");
            Serial.print(thisRowState);
            Serial.print("| Col: ");
            Serial.println(thisColState);
          }
          sendKey(thisRowState, thisColState);
        }
      }
    }

  // reset the output pin state back to 0s.
  digitalWritePins(3, RowPins, rowStates[0]);
  digitalWritePins(4, ColPins, colStates[0]);
  return -1;
}

int wHigh(){
  WVal = analogRead(WPin);
  if (WVal > WLow){
    return true;
  }else{
    WLow = WVal + WLowOffset;
    return false;
  }
  
}

void loop(){ 
    if (digitalRead(USBEnable)){
      if (keyPressed()){
        digitalWrite(enkbPin, LOW);     // stop scanning the kb
        digitalWrite(RIGHT_LED, LED_ON);
        findKey();
        digitalWrite(enkbPin, HIGH);    // restart scanning
        digitalWrite(RIGHT_LED, LED_OFF);
        delay(repeatDelay);
      }
    }else{
      // stop the KB so we can write to the device
      Keyboard.end();
    }
}

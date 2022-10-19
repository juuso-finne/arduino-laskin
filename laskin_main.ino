#include <Keypad.h>
#include <LiquidCrystal.h>

bool halted = false; //a flag indicating that the computation is finished or an error state is reached
bool holdKey = false; //a flag for whether the last key was held
bool ignoreNextEvent = false; //a flag that tells the keyboard to ignore the next event

const int OPERATORLIMIT = 100;
const int HOLDTIME = 350; //Time (milliseconds) how long a key must be pressed to "hold"

int charCount = 0; //  Only for LCD
int screenPos = 16; // Only for LCD
int operatorCount = 0; // Keeping track of the total number of operators.

float operands[OPERATORLIMIT + 1] = {};
char operators[OPERATORLIMIT] = {};

float bufferVals[2] = {};
char bufferChars[2] = {};
int bufferCount = 0;

float currentVal = 0;

bool decimal = false;
int decimalPlace = 1;

bool sqRoot = false;
bool negative = false;

// Initialize LCD and keypad
LiquidCrystal lcd(A5, A4, A3, A2, A1, A0);

  const byte rows = 4;
  const byte cols = 4;
  char keys[rows][cols] = {
    {'1','2','3','+'},
    {'4','5','6','-'},
    {'7','8','9','*'},
    {'.','0','=','/'}
  };
  
  byte rowPins[rows] = {2,3,4,5}; //connect to the row pinouts of the keypad
  byte colPins[cols] = {6,7,8,9}; //connect to the column pinouts of the keypad
  Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600);
  keypad.setHoldTime(HOLDTIME);
  keypad.setDebounceTime(10);
  keypad.addEventListener(keyPress);
  lcd.cursor();
  lcd.blink();
}

void loop()
{
  char key = keypad.getKey();
}

//Event listener for keypad
void keyPress(KeypadEvent key){

  KeyState state = keypad.getState();

  if(state == IDLE)
    return;

  if(ignoreNextEvent){
    ignoreNextEvent = false;
    return;
  }
  
  if(state == HOLD){
    holdKey = true;
    ignoreNextEvent = true;
    }

  if (state == PRESSED) {
    holdKey = false;
    return;
  }

  if (halted){
    reset();
    return;
  }

  if(holdKey){
    switch(key){
      case '.':
        key = 'R';
        break;
      case '*':
        key = '^';
        break;
      case '/':
        key = 's';
        break;
    }
  }

  showInput (key);
  int keyType = parseKey(key);
  
  enum {num, oper, res};
  
  switch (keyType){
    
    case oper:
      operation(key);
      break;
    
    case num:
      digit(key);
      break;
    
    case res:
      reset();
      break;
    
    default:
      break;
    }
}

//Print the  user input or the end result on the LCD
void showInput(char key){

  if (key == '='){
    if (charCount > 16)
      lcd.clear();
    lcd.setCursor(0,1);
    lcd.write(key);
    return;
  }



  if (charCount >= 39){
    charCount = 4;
    screenPos = 16;
    lcd.clear();
    lcd.print("...");
  }

  lcd.write(key);
  charCount ++;

  if (key == 's'){
    lcd.write("qrt");
    charCount +=3;
  }



  while(screenPos < charCount){
    lcd.scrollDisplayLeft();
    screenPos++;
  }
}

//Determine whether a key is a numerical key
int parseKey(char key){
    if (isDigit(key))
        return 0;
  
  for (char c : "+-*/^=")
    if (c == key)
          return 1;
      
      switch (key){
      case 'R':
        return 2;
      case '.':
        decimal = true;
        return -1;
      case 's':
        sqRoot = true;
        return -1;
    }
      
     
}

void reset(){
    decimal = false;
    decimalPlace = 1;
    charCount = 0; 
  operatorCount = 0;
  currentVal = 0;
    lcd.clear();
  operands[OPERATORLIMIT] = 0;
    for (int i = 0; i < (OPERATORLIMIT); i++){
    operands[i] = 0;
    operators[i] = '\0';
  }

  halted = false;
  screenPos = 16;
  bufferCount = 0;
  sqRoot = false;
}

// Determine what to do when a numerical key is pressed
void digit(char key){
  int inputDigit = key - '0';
  
  if (decimal){
    currentVal += inputDigit / pow(10.0, decimalPlace);
    decimalPlace++;
    return;
  }
  
    currentVal *= 10;
    currentVal += inputDigit;
}

// Determine what to do when a non-numerical key is pressed
void operation(char key){
  if (operatorCount >= OPERATORLIMIT)
    error("Mem. limit");

  if (key == '-' && currentVal == 0){
    negative = true;
    return;
  }

  if(negative){
    currentVal *= -1;
    negative = false;
  }

  if(sqRoot){
    if(currentVal < 0){
      error("imaginary");
      return;
    }
    currentVal = sqrt(currentVal);
    sqRoot = false;
  }
  operands[operatorCount] = currentVal;

  if (key == '='){
    compute();
    return;
  }
  operators[operatorCount] = key;
  operatorCount++;
  currentVal = 0;
  decimal = false;
  decimalPlace = 1;
}

void error(String message){
  halted = true;
  lcd.setCursor(0,1);
  lcd.print("Error " + message);
}

void compute(){
  currentVal = operands[0];

  for (int i = 0; i < operatorCount - 1; i++){
    float nextVal = operands[i+1];
    char currentOp = operators[i];
    char nextOp = operators[i+1];

    if (goAhead(currentOp, nextOp)){
      currentVal = operate (currentVal, currentOp, nextVal);
      resolveBuffers();
    }

    else{
      bufferVals[bufferCount] = currentVal;
      bufferChars[bufferCount] = currentOp;
      bufferCount++;
      currentVal = nextVal;
    }
  }

  if (operatorCount != 0){
    currentVal = operate(currentVal, operators[operatorCount - 1], operands[operatorCount]);
    resolveBuffers();
  }

  if(!halted){
    if ((abs(currentVal) < 1000000000 && abs(currentVal) >= 0.01) || currentVal == 0)
      lcd.print(currentVal);
    else
      lcd.print(logRep(currentVal, 2));
  }
    halted = true;
}

//Convert very large or very small numbers to scientific notation
String logRep(float number, int precision){
  int exp = floor(log10(number));
  String mantissa = String(number/pow(10, exp),precision);
  return (mantissa + "E" + String(exp));
}

//Control order of operations
bool goAhead(char operA, char operB){
  
  if (operA == '^')
    return true;
  
  if (operA == '*' || operA == '/'){
    return (operB != '^');
  }
  
  return (operB == '+' || operB == '-');
}


float operate(float A, char op, float B){
  switch (op){
    case '+':
      return A + B;

    case '-':
      return A - B;

    case '/':
      if (B == 0){
        error("Div. by 0");
        return 0;
      }
      return A / B;

    case '*':
      return A * B;

    case '^':
        return pow (A,B);
  }
  
  return 0;
}

void resolveBuffers(){
  while (bufferCount > 0){
    bufferCount--;
    currentVal = operate(bufferVals[bufferCount], bufferChars[bufferCount], currentVal);
  }
}

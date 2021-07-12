#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 6, 7, 8, 9);

//<Preferences>

//General
byte framesPerSec=5; //fps
bool printDirection=false;
bool skipIntro=false; //default false
byte levelMode=true;
byte startingLevel=1;
uint16_t sensitivity=150; //joystick sensitivity
bool sounds=true;

//Protagonist
String protagonistIcon="X";
bool boundariesLoop=true; //when X hits boundary, it loops to other side
bool randomSpawn=true;

//Fruits
bool fruitMode=true;
byte numFruits=8; //set custom #fruits when levelMode is off
String fruitIcon="o";

//Spike
bool spikeMode=true;
byte numSpikes=5; //set custom #spikes when levelMode is off
String spikeIcon="*";

//Custom Level
byte numCustomFruits;
byte numCustomSpikes;


//</Preferences>

//LCD
uint16_t cursorX;
uint16_t cursorY;

//Joystick
const byte switchPin=12;
int joystickX, joystickY;
int midpoint=1024/2; //when joystick at rest
const byte joystickXPin=A0;
const byte joystickYPin=A1;

/* Each array is a fruit/spike with:
 First digit - x coordinate
 Second digit - y coordinate
 Last digit - fruit/spike's existance (if 1, exists; if 0, not exist
*/

byte level=startingLevel;
byte fruits[16*2][3];
byte spikes[16*2][3];

//button
const byte buttonPin=4;
bool buttonPressed=false;
bool pButtonPressed=false;
//Sound
const byte speakerPin=13;

//Preferences Window in Preferences.ino

void setup() {
  Serial.begin(9600);
  pinMode(switchPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(speakerPin, OUTPUT);
  digitalWrite(switchPin, HIGH);
  lcd.begin(16, 2);
  noise(8); //starting noise
  if (!skipIntro) {
    bool showFruit=true;
    for (int i=0; i<16; i++) { //5-second intro
      lcd.clear();
      lcd.print("Welcome to Eater");
      lcd.setCursor(i%16, 1);
      lcd.print(protagonistIcon);
      if (i%16==11) {
        noise(1);
        showFruit=false;
      }
      if (showFruit) {
        lcd.setCursor(11, 1);
        lcd.print(fruitIcon);
      }
      delay(300);
    }
    startGame(true);
  } else
    startGame(false);
}
void startGame(bool showInstructionsParameter) {
  lcd.begin(16, 2);
  //Intro Screen
  if (!skipIntro && showInstructionsParameter) { //from settings & parameter
    lcd.clear();
    lcd.print("Move the "); lcd.print(protagonistIcon);
    lcd.setCursor(0, 1);
    lcd.print("with joystick");
    delay(4000);
    
    lcd.clear();
    lcd.print("Goal: eat all");
    lcd.setCursor(0, 1);
    lcd.print("the fruits ("); lcd.print(fruitIcon); lcd.print(")");
    delay(4000);
    lcd.clear();
    lcd.print("Avoid the");
    lcd.setCursor(0, 1);
    lcd.print("spikes ("); lcd.print(spikeIcon); lcd.print(")");
    delay(4000);
  }
  lcd.clear();
  if (levelMode) {
    lcd.print("Level ");
    lcd.print(level);
    lcd.print("/5"); //5 levels
    delay(300);
    lcd.clear();
    if (level==1) {
      numSpikes=1;
      numFruits=3;
    }
    if (level==2) {
      numSpikes=3;
      numFruits=5;
    }
    if (level==3) {
      numSpikes=8;
      numFruits=10;
    }
    if (level==4) {
      numSpikes=15;
      numFruits=3;
    }
    if (level==5) {
      numSpikes=20;
      numFruits=4;
    }
  } else { //custom level
    lcd.print("Custom level");
    delay(1800);
    numSpikes=numCustomSpikes;
    numFruits=numCustomFruits;
  }
  if (randomSpawn) {
    cursorX=random(16);
    cursorY=random(2);
  } else {
    cursorX=0;
    cursorY=0;
  }
  if (fruitMode) {
    for (int i=0; i<numFruits; i++) { //renders fruits in fruit array
      fruits[i][0]=random(16);
      fruits[i][1]=random(2);
      fruits[i][2]=88; //fruit exists
    }
  }
  if (spikeMode) {
    for (int i=0; i<numSpikes; i++) { //renders fruits in fruit array
      int tentativeX, tentativeY;
      bool spaceAvailable=false;
      while (!spaceAvailable) {
        tentativeX=random(16);
        tentativeY=random(2);
        spaceAvailable=true;
        for (int i=0; i<numFruits; i++) {
          if ((fruits[i][0]==tentativeX && fruits[i][1]==tentativeY) ||  //fruit already occupying space
              (tentativeX==cursorX && tentativeY==cursorY)) { //protagonist already occupying place
            spaceAvailable=false; //space already occupied
          }
        }
      }
      spikes[i][0]=tentativeX;
      spikes[i][1]=tentativeY;
      spikes[i][2]=1; //fruit exists
    }
  }
  //show protagonist before spikes & fruits
  showProtagonist();
}

void loop() {
  updateJoystick();
  
  //display X
  lcd.clear();
  
  if (joystickX>midpoint+sensitivity) {
    if (printDirection)
      Serial.println("right");
    if (cursorX<15)
      cursorX++;
    else if (boundariesLoop)
      cursorX=0;
  }
  if (joystickX<midpoint-sensitivity) {
    if (printDirection)
      Serial.println("left");
    if (cursorX>0)
      cursorX--;
    else if (boundariesLoop)
      cursorX=15;
  }
  if (joystickY>midpoint+sensitivity) {
    if (printDirection)
      Serial.println("up");
    if (cursorY>0)
      cursorY--;
    else if (boundariesLoop)
      cursorY=1;
  }
  if (joystickY<midpoint-sensitivity) {
    if (printDirection)
      Serial.println("down");
    if (cursorY<1)
      cursorY++;
    else if (boundariesLoop)
      cursorY=0;
  }
  
  if (fruitMode) {
    showFruits();
    if (numFruitsLeft()==0) { //all fruits eaten
      lcd.clear();
      if (level==5 || !levelMode) { //levels finished or levelMode off
        lcd.print("You win!");
        noise(8);
        delay(3000);
        while (true) {
          lcd.setCursor(0, 1);
          lcd.print("Press to restart");
          if (digitalRead(buttonPin)==HIGH) {
            level=startingLevel;
            lcd.clear();
            restartMessage(1);
            break;
          }
        }; //infinite delay
      } else {
        showProtagonist(); //show protagonist by itself
        delay(800);
        lcd.clear();
        lcd.print("Level "); lcd.print(level); lcd.print("/5 done");
      }
      noise(7);
      level++;
      delay(1000);
      restartMessage(2);
    }
  }
  if (spikeMode)
    showSpikes();

  //show protagonist
  showProtagonist();

  buttonPressed=digitalRead(buttonPin);
  if (buttonPressed) {
    preferences();
  }
  delay(1000/framesPerSec);

}
void updateJoystick() {
  joystickX=analogRead(joystickXPin);
  joystickY=analogRead(joystickYPin);
}

void showProtagonist() {
  lcd.setCursor(cursorX, cursorY);
  lcd.print(protagonistIcon);
}

//FRUITS
int fruitX, fruitY, fruitExists;
bool showFruits() {
  for (int i=0; i<numFruits; i++) {
    fruitX=fruits[i][0];
    fruitY=fruits[i][1];
    fruitExists=fruits[i][2]; //0 or 1
    
    if (fruitX==cursorX && fruitY==cursorY && fruitExists) {
      fruits[i][2]=0; //eat fruit
      noise(1);
    } else if (fruitExists) { //fruit exists and cursor not on top
      lcd.setCursor(fruitX, fruitY);
      lcd.print(fruitIcon);
    }
  }
}
int numFruitsLeft() { //calculates if any fruits left
  int num=0;
  for (byte i=0; i<numFruits; i++) {
    if (fruits[i][2]) { //a fruit left
      num++;
    }
  }
  return num;
}

//SPIKES
int spikeX, spikeY, spikeExists;
bool showSpikes() {
  bool protagonistDied=false;
  for (int i=0; i<numSpikes; i++) {
    spikeX=spikes[i][0];
    spikeY=spikes[i][1];
    spikeExists=spikes[i][2]; //0 or 1
    
    lcd.setCursor(spikeX, spikeY);
    lcd.print(spikeIcon);
    
    if (spikeX==cursorX && spikeY==cursorY && spikeExists) //protagonist stepped on spike
      protagonistDied=true;
  }
  if (protagonistDied) {
    noise(2);
    for (int i=0; i<5; i++) { //flash protagonist
      showProtagonist();
      delay(180);
      lcd.setCursor(cursorX, cursorY);
      lcd.print(" ");
      delay(180);
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You died :(");
    delay(1000);
    restartMessage(3);
  }
}
void restartMessage(int msg) {
  for (int secLeft=3; secLeft>0; secLeft--) {
    lcd.setCursor(0, 1);
    switch (msg) {
      case 1: lcd.print("Starting"); break;
      case 2: lcd.print("Next level"); break;
      case 3: lcd.print("Respawn"); break;
    }
    lcd.print(" in ");
    lcd.print(secLeft);
    if (secLeft==1)
      noise(4);
    else
      noise(3);
    delay(1000);
  }
  startGame(false);
}

void noise(int noiseType) {
  if (noiseType==1) { //chomp
    tone(speakerPin, 780, 100);
    tone(speakerPin, 980, 100);
  }
  if (noiseType==2) { //die
    tone(speakerPin, 97, 700);
  }
  if (noiseType==3) tone(speakerPin, 357, 300); //countdown
  if (noiseType==4) tone(speakerPin, 547, 300); //start
  if (noiseType==5) tone(speakerPin, 1000, 300); //ding
  if (noiseType==6) tone(speakerPin, 400, 1000); //error
  if (noiseType==7) { //level victory
    tone(speakerPin, 390);
    delay(150);
    tone(speakerPin, 495);
    delay(150);
    tone(speakerPin, 590);
    delay(300);
    tone(speakerPin, 495);
    delay(300);
    tone(speakerPin, 780, 150);
  }
  if (noiseType==8) { //game victory
    noise(7);
    delay(150);
    tone(speakerPin, 590);
    delay(150);
    tone(speakerPin, 495);
    delay(150);
    tone(speakerPin, 390, 150);
  }
  else {
    Serial.print("Invalid number: ");
    Serial.println(noiseType);
  }
}

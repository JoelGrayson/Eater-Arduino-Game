String preferencesTopic="Select topic";
int preferencesCursorIndex=0;
String menuTopics[]={"General", "Protagonist", "Fruits", "Spikes", "Make a Custom Level", "Exit Preferences"};
int numMenuTopics=6;
String generalOptions[]={"Frames per second", "Joystick Sensitivity", "Toggle Sounds", "Make a Custom Level", "Exit General"};
int numGeneralOptions=5;
String protagonistOptions[]={"Protagonist Icon", "Loop Boundaries", "Random Spawn", "Exit Protagonist"};
int numProtagonistOptions=4;
String fruitOptions[]={"Toggle Fruit Mode", "Fruit Icon", "Make a Custom Level", "Exit Fruits"};
int numFruitOptions=4;
String spikeOptions[]={"Toggle Spike Mode", "Spike Icon", "Make a Custom Level", "Exit Spikes"};
int numSpikeOptions=4;

void preferences() {
    lcd.clear();
    lcd.print("Preferences");
    delay(1000);
    
    preferencesTopic="Select topic";
    while (true) {
      lcd.clear();
      //Menu
      if (preferencesTopic=="Select topic") {
        preferencesTopic=renderOptions(menuTopics, numMenuTopics, "0");
        continue;
      }
      if (preferencesTopic=="General") {
        String res=renderOptions(generalOptions, numGeneralOptions, "0");
        if (res=="Frames per Second") {
          framesPerSec=selectInt(res, framesPerSec);
        }
        if (res="Joystick Sensitivity") {
          sensitivity=selectInt(res, sensitivity);
        }
        if (res=="Toggle Sounds") {
          sounds=selectBool(res, "Toggle Sounds");
        }
        if (res=="Make a Custom Level") {
          preferencesTopic="Make a Custom Level";
          continue;
        }
        if (res=="Exit General") {
          preferencesTopic="Select topic";
          continue;
        }
      }
      if (preferencesTopic=="Make a Custom Level") {
        lcd.clear();
        lcd.print("Use joystick to");
        lcd.setCursor(0, 1);
        lcd.print("adjust");
        delay(1500);
        lcd.print("Press button to");
        lcd.setCursor(0, 1);
        lcd.print("confirm");
        
        lcd.clear();
        numCustomFruits=selectInt("#Fruits", 5);
        numCustomSpikes=selectInt("#Spikes", 5);
        levelMode=false;
        startGame(false);
        return;
      }
      if (preferencesTopic=="Protagonist") {
        String res=renderOptions(protagonistOptions, numProtagonistOptions, "0");
        if (res=="Protagonist Icon") {
          String icons[]={"X", "J", "A", "$"};
          res=renderOptions(icons, 4, res);
          protagonistIcon=res;
        }
        if (res=="Loop Boundaries") {
          boundariesLoop=selectBool(boundariesLoop, "Loop Boundaries");
        }
        if (res=="Random Spawn") {
          randomSpawn=selectBool(randomSpawn, "Random Spawn");
        }
        if (res=="Exit Protagonist") {
          preferencesTopic="Select topic";
          continue;
        }
      }
      if (preferencesTopic=="Fruits") {
        String res=renderOptions(fruitOptions, numFruitOptions, "0");
        if (res=="Toggle Fruit Mode") {
          fruitMode=selectBool(fruitMode, "Random Spawn");
        }
        if (res=="Fruit Icon") {
          String icons[]={"o", "l", "!", "-"};
          res=renderOptions(icons, 4, res);
          fruitIcon=res;
        }
        if (res=="Make a Custom Level") {
          preferencesTopic="Make a Custom Level";
          continue;
        }
        if (res=="Exit Fruits") {
          preferencesTopic="Select topic";
          continue;
        }
      }
      if (preferencesTopic=="Spikes") {
        String res=renderOptions(spikeOptions, numSpikeOptions, "0");
        if (res=="Toggle Spike Mode") {
          spikeMode=selectBool(spikeMode, "Spike Mode");
        }
        if (res=="Spike Icon") {
          String icons[]={"*", "%", ".", "@"};
          res=renderOptions(icons, 4, res);
          spikeIcon=res;
        }
        if (res=="Make a Custom Level") {
          preferencesTopic="Make a Custom Level";
          continue;
        }
        if (res=="Exit Spikes") {
          preferencesTopic="Select topic";
          continue;
        }
      }
      if (preferencesTopic=="Exit Preferences") {
        return;
      }

      delay(100);
    }
}
String renderOptions(String options[], int numOptions, String varName) {
  preferencesCursorIndex=0;
  while (true) {
    Serial.println("here");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.print(options[preferencesCursorIndex]);
    lcd.setCursor(0, 1);
    if (preferencesCursorIndex<numOptions-1) { //second option is within bounds
      lcd.print(" ");
      lcd.print(options[preferencesCursorIndex+1]);
    }
    //Joystick
    updateJoystick();
    if (joystickY>midpoint+sensitivity && preferencesCursorIndex>0) {
      preferencesCursorIndex--;
    }
    if (joystickY<midpoint-sensitivity && preferencesCursorIndex<numOptions-1) {
      preferencesCursorIndex++;
    }
    //Select button
    buttonPressed=digitalRead(buttonPin);
    if (buttonPressed && buttonPressed!=pButtonPressed) { //execute once when button pressed
      Serial.println(options[preferencesCursorIndex]+" selected.");
      delay(500); //makes sure click does not bleed to next screen

      String res=options[preferencesCursorIndex];
      if (varName!="0") { //return value to set & exit preferences
        confirmSelect(varName, res);
        noise(5);
        delay(1500);
        preferencesTopic="Exit Preferences";
      }
      return res;
    }
    pButtonPressed=buttonPressed;
    delay(300);
  }
  return "error";
}
void confirmSelect(String key, String val) {
  lcd.clear();
  lcd.print(key);
  lcd.setCursor(0, 1);
  lcd.print(" set to ");
  lcd.print(val);
}
boolean selectBool(boolean defaultVal, String varName) { //displays on, off, and cancel
  String vals[3];
  if (defaultVal) {
    vals[0]="On (set)";
    vals[1]="Off";
  } else {
    vals[0]="On";
    vals[1]="Off (set)";
  }
  vals[2]="Cancel";
  String res=renderOptions(vals, 3, varName);
  
  return res=="On";
}
int selectInt(String val, int startingPoint) {
  int num=startingPoint;
  while (true) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set ");
    lcd.print(val);
    lcd.setCursor(0, 1);
    lcd.print(num);
    //Joystick
    updateJoystick();
    if (joystickY>midpoint+sensitivity) {
      num++;
    }
    if (joystickY<midpoint-sensitivity) {
      num--;
    }
    //Confirm button
    buttonPressed=digitalRead(buttonPin);
    if (buttonPressed && buttonPressed!=pButtonPressed) { //execute once when button pressed
      confirmSelect(val, String(num));
      delay(1500);
      return num;
    }
    pButtonPressed=buttonPressed;
    delay(300);
  }
  return startingPoint;
}

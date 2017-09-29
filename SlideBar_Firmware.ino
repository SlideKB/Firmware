#include <ResponsiveAnalogRead.h>

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;
int dataNumber = 0;             // new for this version

int motorOut1 = 3;
int motorOut2 = 2;
int motorOut3 = 9;
int motorOut4 = 10;
int motorEnable = 6;

String ID = "l1n117";
//String ID = "m1n11";

int parts;
boolean newPart;

boolean powerSlideEnabled;

ResponsiveAnalogRead analog(A6, true, 0.01);

void setup() {
  pinMode(motorOut1, OUTPUT);
  pinMode(motorOut2, OUTPUT);
  pinMode(motorOut3, OUTPUT);
  pinMode(motorOut4, OUTPUT);
  pinMode(motorEnable, OUTPUT);
  Serial.begin(115200);
  Serial.print("");
}

void loop() {
  recvWithEndMarker();
  showNewNumber();
  WriteBack();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = ']';
  char rc;

  if (Serial.available() > 0) {
    rc = Serial.read();
//    Serial.println(rc);

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewNumber() {
  if (newData == true) {
    dataNumber = 0;             // new for this version
    dataNumber = atoi(receivedChars);   // new for this version
    //        Serial.print("This just in ...");
    //        Serial.println(receivedChars);
    //        Serial.print("Data as Number ... ");    // new for this version
    //        Serial.println(dataNumber);     // new for this version
    if (dataNumber == 2424){
      Serial.println(ID);
    }
    if (dataNumber > 6000 && dataNumber < 7000) {
      vibrate(dataNumber - 6000);
    }
    //        if (temp > 5000 && temp < 5500){
    //          scrollUp(temp - 5000, 0);
    //        }
    //        if (temp > 5500 && temp < 6000){
    //          scrollDown(temp - 5500, 0);
    //        }
    if (dataNumber > 3000 && dataNumber < 4000) {
      bumpLeft(dataNumber - 3000);
    }
    if (dataNumber > 4000 && dataNumber < 5000) {
      bumpRight(dataNumber - 4000);
    }
    if (dataNumber == 2000) {
      newPart = false;
      parts = 0;
    }
    if (dataNumber > 2000 && dataNumber < 2051) {
      parts = dataNumber - 2000;
      newPart = true;
    } else {
      if (dataNumber < 1025) {
        if (dataNumber > -1) {
          MoveToNumber(dataNumber, 8);
        }
      }
    }
    newData = false;

  }
  AvoidParts();
}

void MoveToNumber(int goal, int threshold) {
  analog.update();
  int val = analog.getValue();
  boolean toggle = true;
  while (abs(val - goal) >  threshold + 50 && toggle && !Serial.available() > 0) {
    analog.update();
    val = analog.getValue();
    printBack(val);
    if (val > goal) {
      goRight();
    } else {
      goLeft();
    }
    toggle = calculateNewValue(goal);
  }
  while (abs(val - goal) > threshold + 20 && toggle && !Serial.available() > 0) {
    analog.update();
    val = analog.getValue();
    printBack(val);
    if (val > goal) {
      pwmGoRight(230);
    } else {
      pwmGoLeft(230);
    }

    toggle = calculateNewValue(goal);
  }
  while (abs(val - goal) > threshold && toggle && !Serial.available() > 0) {
    analog.update();
    val = analog.getValue();
    printBack(val);
    if (val > goal) {
      pwmGoRight(200);
    } else {
      pwmGoLeft(200);
    }

    toggle = calculateNewValue(goal);
  }
  stop();
}

boolean calculateNewValue(int given) {
  if (dataNumber > 0 && dataNumber < 1023) {
    if (dataNumber != given) {
      return false;
    }
  }
  return true;
}

int sensorValue = 0;

void AvoidParts() {
  if (newPart == true) {
    int partSize = 1040 / parts;
    int difference = 450 / parts;
    analog.update();
    sensorValue = analog.getValue();
    for (int i = 0; i < parts + 1; i++) {
      if (abs(analogRead(A6) - partSize * i) < difference) {
        if (analogRead(A6) + 1 < partSize * i) {
          while (analogRead(A6) + 1 < partSize * i && abs(analogRead(A6) - partSize * i) < difference) {
            pwmGoRight(230);
            WriteBack();
          }
        } else if (analogRead(A6) + 1 > partSize * i) {
          while (analogRead(A6) + 1 > partSize * i && abs(analogRead(A6) - partSize * i) < difference) {
            pwmGoLeft(230);
            WriteBack();
          }
        }
        delay(2);
        stop();
      }
    }
  }
}



long previousMillis = 0;
long previouspreMillis = 0;
long previousSlideMillis = 0;
long interval = 4;
int previousSensorValue = -1;

void WriteBack() {
  long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    analog.update();
    sensorValue = analog.getValue();
    //    sensorValue = map(sensorValue, 0, 780, 0, 1024);
    if (previousSensorValue != sensorValue) {
      powerSlide(previousSensorValue - sensorValue);
      previousSensorValue = sensorValue;
      printBack(sensorValue);
      previouspreMillis = currentMillis;
      previousSlideMillis = currentMillis;
    }
  }
  if (currentMillis - previouspreMillis > interval * 200) {
    previouspreMillis = currentMillis;
//    Serial.print("  {SB1}");
  }
  if (currentMillis - previousSlideMillis > interval * 20) {
    previousSlideMillis = currentMillis;
    if (powerSlideEnabled) {
      stop();
    }
  }
}

void printBack(int value) {
  Serial.println(value);
//  Serial.print("|");
//  if (value < 1000) {
//    Serial.print("0");
//  }
//  if (value < 100) {
//    Serial.print("0");
//  }
//  if (value < 10) {
//    Serial.print("0");
//  }
//  Serial.print(value);
//  Serial.print(">");
}

void powerSlide(int amount) {
  if (powerSlideEnabled) {
    if (amount > 2) {
      pwmGoRight(100);
    }
    if (amount < -2) {
      pwmGoLeft(100);
    }
  }
}

void pwmGoLeft(int pwm) {
  analogWrite(motorEnable, pwm);
  digitalWrite(motorOut1, LOW);
  digitalWrite(motorOut2, HIGH);
  digitalWrite(motorOut3, LOW);
  digitalWrite(motorOut4, HIGH);
}

void goRight() {
  pwmGoRight(255);
}

void pwmGoRight(int pwm) {
  analogWrite(motorEnable, pwm);
  digitalWrite(motorOut1, HIGH);
  digitalWrite(motorOut2, LOW);
  digitalWrite(motorOut3, HIGH);
  digitalWrite(motorOut4, LOW);
}

void goLeft() {
  pwmGoLeft(255);
}

void stop() {
  digitalWrite(motorEnable, LOW);
  digitalWrite(motorOut1, HIGH);
  digitalWrite(motorOut2, HIGH);
  digitalWrite(motorOut3, HIGH);
  digitalWrite(motorOut4, HIGH);
}

void bumpRight(int amount) {
  goRight();
  delay(amount);
  stop();
}

void bumpLeft(int amount) {
  goLeft();
  delay(amount);
  stop();
}

void vibrate(int amount) {
  for (int i = 0; i < amount; i++) {
    bumpLeft(15);
    bumpRight(15);
  }
}

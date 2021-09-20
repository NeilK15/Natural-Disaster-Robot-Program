#include <Ultrasonic_no_DPE.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte I1 = 5;
const byte I2 = 6;
const byte I3 = 10;
const byte I4 = 11;

const byte rSpeedIncr = -4;             // Right speed value is 4 less than left speed
const byte frontThresh = 30;
const byte frontTrigPin = 8;
const byte frontEchoPin = 7;
const byte leftThresh = 15;
const byte leftEchoPin = 2;
const byte leftTrigPin = 4;
const char tempUnit = 'F';

bool activate;

float front; 
float left;
signed int pErr;
signed int dErr;
signed int output;
float temperature;
float voltage;
float initialTemp;
float finalTemp;

LiquidCrystal_I2C lcd(0x27,20,4);
Ultrasonic front_us(frontTrigPin, frontEchoPin);
Ultrasonic left_us(leftTrigPin, leftEchoPin);

void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Front: ");
  lcd.setCursor(0,1);
  lcd.print("Left: ");
  lcd.setCursor(0,2);
  lcd.print("Right: ");
  lcd.setCursor(0,3);
  lcd.print("Temp: ");
  pinMode(I1, OUTPUT);
  pinMode(I2, OUTPUT);
  digitalWrite(I1, LOW);
  digitalWrite(I2, LOW);
  digitalWrite(I3, LOW);
  digitalWrite(I4, LOW);
  
  activate = false;
  while(!activate) {
    printAll();
    activate = checkActivate();
    delay(10);
  }
}

void loop() {
  
  // put your main code here, to run repeatedly:
  while (front > frontThresh && left < 50) {
    followWall(left, leftThresh);
    printAll();
  }
  printAll();
  stopRobot();
  if ((left >= leftThresh-10 && left <= leftThresh+10) && front < frontThresh) {
    while (front < frontThresh) {
      turnRight();
      printAll();
    }
    delay(50);
    stopRobot();
  } else if (left > leftThresh+10 && front > frontThresh) {
    moveForward();
    delay(100);
    stopRobot();
    while (left > leftThresh) {
      turnLeft();
      printAll();
    }
    delay(50);
    stopRobot();
  }
  
  printAll();
  stopRobot();
  //delay(5000);
}

float measureFrontDistance() {
  return front_us.Ranging(CM, 1e6);
}

float measureLeftDistance() {
  return left_us.Ranging(CM, 1e6);
}

float measureVoltage() {
  int vSensor = analogRead(A0);
  float initialData = float(vSensor) * (25.0/1023.0);
  return (initialData - 0.331) / 0.948;
}

float measureTemperature(char unit) {
  int tSensor = analogRead(A2);
  delay(25);
  float initialData = float(tSensor) * (3.3/10.24);
  float celsius = initialData;
  float farenheight = (celsius * 1.8) + 32.0;
  if (unit == 'C')
    return celsius;
  else
    return farenheight;
}

void printAll() {
  front = measureFrontDistance();
  left = measureLeftDistance();
  //voltage = measureVoltage();
  temperature = measureTemperature(tempUnit);
  
  lcd.setCursor(7,0);
  lcd.print(front);
  delay(20);
  lcd.print("    ");

  lcd.setCursor(6,1);
  lcd.print(left);
  delay(20);
  lcd.print("    ");

  lcd.setCursor(7,2);
  lcd.print(output);
  delay(20);
  lcd.print("    ");

  lcd.setCursor(6,3);
  lcd.print(String(temperature) + " " + tempUnit);
  delay(20);
  lcd.print(" ");
}

void stopRobot() {
  digitalWrite(I1, LOW);
  digitalWrite(I2, LOW);
  digitalWrite(I3, LOW);
  digitalWrite(I4, LOW);
}

void followWall(float leftDist, float leftStop) {
   pErr = int(leftDist - leftStop);
   delay(100);
   signed int finalErr = int(measureLeftDistance() - leftStop);
   dErr = int((finalErr - pErr) / 100.0);
   output = int((3 * (pErr + (0.9 * dErr))) + 170.0);
   digitalWrite(I1, LOW);
   analogWrite(I2, 200);
   digitalWrite(I3, LOW);
   analogWrite(I4, output + rSpeedIncr);
}

void turnRight() {
  // 90 degrees is 1637 delay
  digitalWrite(I1, LOW);
  analogWrite(I2, 175);
  digitalWrite(I4, LOW);
  analogWrite(I3, 175);
}

void turnLeft() {
  digitalWrite(I2, LOW);
  analogWrite(I1, 175);
  digitalWrite(I3, LOW);
  analogWrite(I4, 175);
}

void moveForward() {
  digitalWrite(I1, LOW);
  analogWrite(I2, 150);
  digitalWrite(I3, LOW);
  analogWrite(I4, 146);
}

bool checkActivate() {
  int count = 0;
  while (count < 2) {
    if (left < 10.0) {
      count++;
      printAll();
      delay(20);
    }
    printAll();
  }
  return count >= 2;
}

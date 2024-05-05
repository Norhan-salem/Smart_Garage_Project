#include <Servo.h>
#include "LiquidCrystal_I2C.h"

Servo servoMotor;  

#define IR_SENSOR_IN_GATE_PIN         4
#define IR_SENSOR_OUT_GATE_PIN        5
#define GATE_CONTROL_PIN              9


//Parking Slots
#define MAXSLOTSNUMBER                       4
#define PARKINGSLOT1                         10
#define PARKINGSLOT2                         11
#define PARKINGSLOT3                         12
#define PARKINGSLOTWASHING4                  13

/*
  1-Check availble(slotsCheck) to see availble spaces
  2-(fees) return fees 50 or 200
  3-(openDoor) return yes
  4-(checkParking) send the path  
  5-(checkWashing) send path 
*/

//LCD
LiquidCrystal_I2C lcd(0x27,  16, 2);


uint8_t parkingSpaces[4] = {0,0,0,0};

volatile uint8_t availablePlaces = 4;

void IR_SensorSecureCloseGate(uint8_t dir);
void ServorCloseGate(void);
void ServorOpenGate(void);

/* This function returns the closet free space
 * the first parameter is to return the number of available spaces*/
String CheckAvailableSpaces(uint8_t * free_spaces);

/**/
void LcdPrintString(String first_line, String second_line);

void OpenGate(uint8_t direction);

void setup() {
  // put your setup code here, to run once:
  pinMode(IR_SENSOR_IN_GATE_PIN, INPUT);
  pinMode(IR_SENSOR_OUT_GATE_PIN, INPUT);

  //Parking Slots congfigurations
  pinMode(PARKINGSLOT1, INPUT_PULLUP);
  pinMode(PARKINGSLOT2, INPUT_PULLUP);
  pinMode(PARKINGSLOT3, INPUT_PULLUP);
  pinMode(PARKINGSLOTWASHING4, INPUT_PULLUP);

  //LCD
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on

  /* For gate control*/
  servoMotor.attach(GATE_CONTROL_PIN);
  Serial.begin(9600);

  /* For fees*/
  randomSeed(analogRead(0));
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println(digitalRead(PARKINGSLOT1));

  if(digitalRead(PARKINGSLOT1) == HIGH){
    OpenGate(IR_SENSOR_OUT_GATE_PIN);
  }

  delay(100);
}

void IR_SensorSecureCloseGate(uint8_t dir){
  int sensorStatus = digitalRead(dir);
  Serial.println(sensorStatus);

  if(sensorStatus == LOW){
    while(digitalRead(dir) == LOW);  
  }

  delay(1000);
  //close gate TODO
  ServorCloseGate();
}

void ServorCloseGate(void){
  servoMotor.write(100);
}

void ServorOpenGate(void){
  servoMotor.write(0);
}

String CheckAvailableSpaces(uint8_t * free_spaces){
  *free_spaces = MAXSLOTSNUMBER;

  parkingSpaces[0] = digitalRead(PARKINGSLOT1);
  parkingSpaces[1] = digitalRead(PARKINGSLOT2);

  parkingSpaces[2] = digitalRead(PARKINGSLOT3);
  parkingSpaces[3] = digitalRead(PARKINGSLOTWASHING4);

  for (uint8_t i = 0; i < MAXSLOTSNUMBER; i++) {
    *free_spaces -= parkingSpaces[i];
  }

  return "1," + String(parkingSpaces[0]) + "," + String(parkingSpaces[1]) + ","+ String(parkingSpaces[2]) + ","+ String(parkingSpaces[3]); 
}

void LcdPrintString(String first_line, String second_line){
  lcd.setCursor(0,0);
  lcd.print(first_line);
  lcd.setCursor(0,1);
  lcd.print(second_line);
}

void OpenGate(uint8_t direction){
    servoMotor.write(0);
    IR_SensorSecureCloseGate(direction);
}

String Fees(void){
  uint8_t randNumber = random(50, 200);
  if(randNumber < 125){
    return "2,50";
  }else{
    return "2,200";
  }
}


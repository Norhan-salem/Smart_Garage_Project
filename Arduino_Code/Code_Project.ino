#include <Servo.h>
#include "LiquidCrystal_I2C.h"

//Gate controls
Servo servoMotor;  

#define IR_SENSOR_IN_GATE_PIN         4
#define IR_SENSOR_OUT_GATE_PIN        5
#define GATE_CONTROL_PIN              9


//Parking Slots
#define MAX_SLOTS_NUMBER                       4
#define WASHING_SLOTS_NUMBER                   1
#define PARKING_SLOT1                          10
#define PARKING_SLOT2                          11
#define PARKING_SLOT3                          12
#define PARKING_SLOT_WASHING4                  13

//Parking Slots Directions
String slots[4] = {"1,Straight Until Point 1,Right", "1,Straight Until Point 1,Straight Until Point 2,Left", 
                   "1,Straight Until Point 1,Straight Until Point 2,Straight Until Point 3,Right,Straight Until Point 4,Left",
                   "1,Straight Until Point 1,Straight Until Point 2,Straight Until Point 3,Left,Straight Until Point 4,Right"};
String NotAvailable = "0, No Free Available Spaces";

/* Commands 
  1-Check availble(slotsCheck) to see available spaces
  2-(openDoor) return yes for exiting
  3-(checkParking) send the path or not available  
  4-(checkWashing) send path or not available 
*/
#define CMD_CHECK_AVAILABLE_SLOTS                     "slotsCheck"
#define CMD_OPEN_DOOR                                 "openDoor"
#define CMD_CHECK_AVAILABLE_PARKING_SLOTS             "checkParking"
#define CMD_CHECK_AVAILABLE_WASHING_SLOTS             "checkWashing"

//LCD
LiquidCrystal_I2C lcd(0x27,  16, 2);


uint8_t parkingSpaces[4] = {0,0,0,0};
String availability = "AN";

void IR_SensorSecureCloseGate(uint8_t dir);
void ServorCloseGate(void);
void ServorOpenGate(void);

/* Reads the parking slots availablility */
void GetParkingReadings(void);
/* This function returns the closet free space
 * the first parameter is to return the number of available spaces*/
String CheckAvailableSpaces(uint8_t * free_spaces);

/**/
void LcdPrintString(String first_line, String second_line);

String OpenGate(uint8_t direction);
String CheckParkingSpaces(bool* available, uint8_t * slotNumber);
String CheckWashingSpaces(bool* available, uint8_t * slotNumber);

void setup() {
  // put your setup code here, to run once:
  pinMode(IR_SENSOR_IN_GATE_PIN, INPUT);
  pinMode(IR_SENSOR_OUT_GATE_PIN, INPUT);

  //Parking Slots congfigurations
  pinMode(PARKING_SLOT1, INPUT_PULLUP);
  pinMode(PARKING_SLOT2, INPUT_PULLUP);
  pinMode(PARKING_SLOT3, INPUT_PULLUP);
  pinMode(PARKING_SLOT_WASHING4, INPUT_PULLUP);

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
  String state;
  String data; 
  uint8_t availableSpaces;
  bool isSpaceAvailable;

  //Checking if bluetooth is sending data
  if(Serial.available() > 0){
    state = Serial.readString();

    if(state.substring(0,10) == CMD_CHECK_AVAILABLE_SLOTS){
        data = CheckAvailableSpaces(&availableSpaces);
        Serial.println(data);
        LcdPrintString("1:" + String(availability[parkingSpaces[0]]) + 
                       " 2:" + String(availability[parkingSpaces[1]]) +
                       " 3:" + String(availability[parkingSpaces[2]]) +
                       " 4:" + String(availability[parkingSpaces[3]]) , 
                       "Available ST: " + String(availableSpaces));
    }else if(state.substring(0,8) == CMD_OPEN_DOOR){
        OpenGate(IR_SENSOR_OUT_GATE_PIN);
    }else if(state.substring(0,12) == CMD_CHECK_AVAILABLE_PARKING_SLOTS){
        data = CheckParkingSpaces(&isSpaceAvailable, &availableSpaces);
        if(isSpaceAvailable){
          Serial.println(data); //Direction
          LcdPrintString("Slot Number P" + String(availableSpaces) + " A", "Directions  Sent");
          OpenGate(IR_SENSOR_IN_GATE_PIN);
        }else{
          LcdPrintString(" Sorry No Free  ", "Available Spaces");
        }
    }else if(state.substring(0,12) == CMD_CHECK_AVAILABLE_WASHING_SLOTS){
        data = CheckWashingSpaces(&isSpaceAvailable, &availableSpaces);
        if(isSpaceAvailable){
          Serial.println(data); //Direction
          LcdPrintString("Slot Number W" + String(availableSpaces) + " A", "Directions  Sent");
          OpenGate(IR_SENSOR_IN_GATE_PIN);
        }else{
          LcdPrintString(" Sorry No Free  ", "Available Spaces");
        }
    }
  }
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

void GetParkingReadings(void){
  parkingSpaces[0] = digitalRead(PARKING_SLOT1);
  parkingSpaces[1] = digitalRead(PARKING_SLOT2);
  parkingSpaces[2] = digitalRead(PARKING_SLOT3);
  parkingSpaces[3] = digitalRead(PARKING_SLOT_WASHING4);
}

String CheckAvailableSpaces(uint8_t * free_spaces){
  *free_spaces = MAX_SLOTS_NUMBER;

  GetParkingReadings();

  for (uint8_t i = 0; i < MAX_SLOTS_NUMBER; i++) {
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

String OpenGate(uint8_t direction){
    servoMotor.write(0);
    IR_SensorSecureCloseGate(direction);
    return "2,Door has opened";
}

String CheckParkingSpaces(bool* available, uint8_t * slotNumber){
  GetParkingReadings();

  for (uint8_t i = 0; i < MAX_SLOTS_NUMBER - WASHING_SLOTS_NUMBER; i++) {
    if(!parkingSpaces[i]){
      *available = true;
      *slotNumber = i + 1;
      return "3," + slots[i];
    }
  }

  *slotNumber = 0;
  *available = false;
  return "3," + NotAvailable;
}

String CheckWashingSpaces(bool* available, uint8_t * slotNumber){
  GetParkingReadings();

  for (uint8_t i = MAX_SLOTS_NUMBER - WASHING_SLOTS_NUMBER; i < MAX_SLOTS_NUMBER; i++) {
    if(!parkingSpaces[i]){
      *available = true;
      *slotNumber = i + 1;
      return "4," + slots[i];
    }
  }

  *available = false;
  *slotNumber = 0;
  return "4," + NotAvailable;
}



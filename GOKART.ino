#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>    
#include <RF24.h>
#include <IRremote.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "globalVars.h"

// Radio Configuration
RF24 radio(7, 8); /*Create an RF24 object with two arguments being the CSN and CE pins*/

// Global Variables for RF
byte addresses[][6] = {"turrt","gkart","grnds"};

// Global Variables for IR
int irPin1 = 2;
int irPin2 = 4;

// Global Variables for E-Stop
int estop = 10;

// Global Variables for all delays in millis()
int period = 5000;
unsigned long time_now = 0;
int irPeriod = 3000; /*Used to control delay between printing the values for the IR on the score display*/
unsigned long irTime = 0;
int onlyprintonce = 10000;
unsigned long onlyprint1 = 0;

// Global Variables for E-Stop
bool ESTOP = false;

// Global Variables for Servos (Deployment Mechanism)
decode_results results;
Servo myservo;
Servo myservo2;

// LCD Screen Configuration
LiquidCrystal_I2C lcd(0x27, 16, 2); /*Set the LCD address to 0x27 for a 16 chars and 2 line display*/

void setup() {
  lcd.init();  /*Initialize the LCD*/
  lcd.backlight();  /*Open the backlight*/
  myservo.attach(3); /*Attach 1st servo to pin 5*/
  myservo.write(0); /*Have servo start at initial position*/
  myservo2.attach(5); /*Attach 2nd servo to pin 6*/
  myservo2.write(0); /*Have servo start at initial position*/
  Serial.begin(9600); 
  
  pinMode(irPin1, INPUT); /*Set irPin1 as input*/
  pinMode(irPin2, INPUT); /*Set irPin2 as input*/
  pinMode(estop, OUTPUT); /*Set E-Stop as output*/
  
  /*Begin RF Receiving*/
  radio.begin();
  //radio.setDataRate(RF24_250KBPS);
  /*Open reading pipe for Go-Kart to begin receiving from "gkart"*/
  radio.openReadingPipe(1, addresses[1]); /*Enables communication between receiver and transmitter*/
  radio.setPALevel(RF24_PA_HIGH); /*Min used to describe the distance between the transmitter and receiver modules*/
  radio.startListening(); /*Sets module as receiver*/
 
  radio.stopListening(); /*Sets module as transmitter*/
}

void loop() {
  lcd.setCursor(3, 0); /*Set the cursor to column 3, line 0. This is the first line of the LCD screen*/
  lcd.print("Score:");  /*Print 'Score:' to the LCD*/
  bool HitL; /*Tracks irPin1 hits*/
  bool HitR; /*Tracks irPin2 hits*/
  bool pushed = false; /*This bool will ensure the score does not print continously but just one every 5 seconds using delays in millis()*/ 
  HitL = digitalRead(irPin1); /*Digital read irPin1 and set it equal to bool HitL*/
  HitR = digitalRead(irPin2); /*Digital read irPin2 and set it equal to bool HitR*/
 
  if(pushed == false) { /*What will print to LCD screen when print once*/
  if((HitL == 0) && (millis() > irTime + irPeriod)) { /*If irPin1 is set to LOW (0) and delay in millis() is in effect*/
    irTime = millis();
    --IRcounter; /*If IR connection is made, decrement total IR score*/
    digitalWrite(estop, LOW); /*Set E-Stop to LOW because we want the Go-Kart to stop now that an IR conenction was made*/
    pushed = true; /*Now set pushed equal to true so that it only prints once on the LCD*/
    lcd.clear(); /*Clear the LCD screen, if you don't, after 100, 99 will be printed as 990*/
    lcd.setCursor(10, 0); /*Set the cursor to column 10, line 0. This will print the score number right after Score:*/
    /*The previous (3,0) and now (10,0) were figured out by counting the squares on the LCD screen and figuring out that 8 was the midpoint*/
    lcd.print(IRcounter); /*Will print the decremented score based on how many times the IR was hit. Will keep changing as the IR is hit*/
  }
 } else { 
    if(HitL == 1) { /*If irPin2 is set to HIGH (1) then the IR is not being hit*/
      pushed = false; /*In this case, set pushed equal to false as originally defined*/
      Serial.println("Off"); /*Testing purposes*/
    }
   }
   
  /*Code is the same for HitR*/
  if(pushed == false) {
  if((HitR == 0) && (millis() > irTime + irPeriod)) { 
    irTime = millis();
    --IRcounter;
    digitalWrite(estop, LOW);
    pushed = true;
    lcd.clear();
    lcd.setCursor(10, 0); /*Set the cursor to column 10, line 0*/
    lcd.print(IRcounter);
  }
 } else { 
  if(HitR==1) { 
    pushed = false;
    //Serial.println("Off");
  }
 }
  if(millis() > irTime + irPeriod) { 
    digitalWrite(estop, HIGH);
    pushed = false;
  } 

  radio.startListening(); /*Sets module as receiver*/
  if (radio.available()) {
    radio.read(&val, sizeof(val)); /*Read the value sent by the bluetooth module*/
    Serial.println(val); /*Serial print that value for testing purposes*/
  }//else{Serial.println("NA");}
  if(val == 'T') { /*RF E-Stop*/
    digitalWrite(estop, LOW); /*Set E-Stop to LOW (0)*/
    while(1); //While loop to keep it going continously*/
  }
  if(val == '0') { 
  //Serial.println("Right"); Testing purposes
    for(int i = 0; i < 180; i++) { /*This moves the servo the appropriate degrees*/
     myservo.write(i);
     delay(20);
    }
    if(millis() > time_now + period) { /*Allows for few second delay between the two*/
      time_now = millis();
    }
    for(int i = 180; i > 0; i--) { /*Now moves it back*/
      myservo.write(i);
      delay(20); 
    }
  }
  if(val == '1') { 
    for(int i = 0; i < 180; i++) { /*Same motion as right defense mechanism*/
     myservo2.write(i);
     delay(20);
    }
    if(millis() > time_now + period){
      time_now = millis();
    }
    for(int i = 180; i > 0; i--) {
      myservo2.write(i);
      delay(20);
    }
  }
  if(val == 's') { 
    lcd.clear();
    lcd.setCursor(3, 0); /*Set the cursor to column 3, line 0*/
    lcd.print("Score: ");
    lcd.setCursor(10, 0); /*Set the cursor to column 2, line 1*/
    lcd.print(IRcounter);
    lcd.setCursor(7, 1); /*Set the cursor to column 3, line 0*/
    lcd.print("GO"); /*Print a message to the LCD*/
  }
  if(val == 'x') {
    /*Must print out all these values to the lcd screen again because if not they will not print when the score updates*/
    lcd.clear();
    lcd.setCursor(3, 0); /*Set the cursor to column 3, line 0*/
    lcd.print("Score: ");
    lcd.setCursor(10, 0); /*Set the cursor to column 2, line 1*/
    lcd.print(IRcounter);
    lcd.setCursor(6, 1); /*Set the cursor to column 3, line 0*/
    lcd.print("STOP");  /*Print a message to the LCD*/
  }
  while(Serial.available()) { 
    Serial.read();
    //Serial.println("In While Loop"); For testing purposes
  }
  delay(10); 
  val = '2'; /*Set val to char 2 to reset value*/
}

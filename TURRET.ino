#include <Servo.h>
#include <SPI.h>
#include <RF24.h>

// Radio Configuration
bool radioNumber = 0;
RF24 radio(9, 10);
byte addresses[][6] = {"turrt", "gkart", "grnds"};
bool role = 0;

// Create variables to control servo value
Servo ForeAft;
Servo LeftRight;
unsigned int ForeAft_Output; /*Expected range 0 - 180 degrees*/
unsigned int LeftRight_Output; /*Expected range 0 - 180 degrees*/

void setup() {
  Serial.begin(9600); /*Get ready to send data back for debugging purposes*/
  ForeAft.attach(6); /*Plug a servo sig nal line into digital output pin 6*/
  LeftRight.attach(5); /*Plug a servo signal line into digital output pin 7*/
  radio.begin(); /*Initialize radio*/
  radio.setPALevel(RF24_PA_HIGH); /*Set the power output to low*/
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();
}

void loop() {
  delay(100);
  //for(int i = 0; i < 4; i++) {
  if (radio.available()) {
    radio.read(&ForeAft_Output, sizeof(ForeAft_Output));
    radio.read(&LeftRight_Output, sizeof(ForeAft_Output));
    Serial.println(ForeAft_Output);
    Serial.println(LeftRight_Output);
  }
  else {
    //Serial.print("No radio");
  }
  //}
  if (ForeAft_Output > 0) {
    ForeAft.write(ForeAft_Output);
  }
  if (LeftRight_Output > 0) {
    LeftRight.write(LeftRight_Output);
  }
  //Serial.println("Loop");
}

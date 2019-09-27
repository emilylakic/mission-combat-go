#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include "globalVars.h"

// Radio Configuration
RF24 radio(7,8); /*Create an RF24 object with two arguments being the CSN and CE pins*/

// Global Variables for RF
/*The value of the addresses can be changed to any 5-letter combination*/
byte addresses[][6] = {"turrt","gkart","grnds"}; /*Create a byte array which will represent the addresses, or the pipes through which the modules will communicate*/
bool radioNumber = 1;
bool role = 1;  /*Receiver defined as 1, Transmitter defined as 0*/

// Global Variables for Joystick
/*Decide where you are going to plug the Joystick into the circuit board*/
int ForeAft_Pin = 0; /*Plug Joystick Up/Down into Analog pin 0*/
int LeftRight_Pin = 1; /*Plug Joystick Left/Right into Analog pin 1*/
/*Create variables to read Joystick values*/
float ForeAft_Input; /*Variable to store data for Fore/Aft input from joystick*/
float LeftRight_Input; /*Variable to store data for for Left/Right input from joystick*/
/*Create variables to transmit servo value*/
int ForeAft_Output; /*Expected range 0 - 180 degrees*/
int LeftRight_Output; /*Expected range 0 - 180 degrees*/
/*These variables allow for math conversions and later error checking as the program evolves*/
/*Every Joystick is different and will thus have different limits*/ 
int Fore_Limit = 0; /*High ADC Range of Joystick UpDown*/
int Aft_Limit = 1023; /*Low ADC Range of Joystick UpDown*/
int Right_Limit = 1023; /*High ADC Range of Joystick LeftRight*/
int Left_Limit = 0; /*Low ADC Range of Joystick LeftRight*/

// Global Variables for E-Stop
int estop = 9; /*Sets pin of E-Stop to 9*/
char ESTOP;

void parseCommand(char input) {
  switch (input) {
    case '1': // LEFT will be used to drop the left defense mechanism 
    case '2': // UP -- Not used
    case '3': // RIGHT will be used to drop the right defense mechanism
    case '4': // DOWN -- Not used
    case '5': // SELECT -- Not used
    case '6': // START -- Not used
    case 's': // SQUARE will be used to output Go
    case 't': // TRIANGLE -- Not used
    case 'x': // X will be used to output Stop
    case 'c': // CIRCLE -- Not used
      break;
  }
}

void setup() {
  pinMode(estop, INPUT_PULLUP); /*Set E-Stop to input (input_pullup for button, input for actual E-Stop*/
  Serial.begin(9600); /*Get ready to send data back for debugging purposes*/
  radio.begin(); /*Get the transmitter ready*/
  radio.setPALevel(RF24_PA_HIGH); /*Set the power to low. Low or min recommended due to distance between RF signals*/
}

void loop() {
// Moving the servos on the Turret    
    radio.openWritingPipe(addresses[0]); /*Send to Turret by initializing the radio object to set the address of the receiver to which we will send data, in this case to "turrt"*/
    ForeAft_Input = analogRead(ForeAft_Pin); /*Read the Up/Down Joystick value*/
    LeftRight_Input = analogRead(LeftRight_Pin); /*Read the Left/Right Joystick value*/
    ForeAft_Output = convertForeAftToServo(ForeAft_Input); /*Convert the Up/Down Joystick value to a Servo value (0-180)*/
    LeftRight_Output = convertLeftRightToServo(LeftRight_Input); /*Convert the Left/Right Joystick value to a Servo value (0-180)*/
    //Serial.println(ForeAft_Output); /*For testing/debugging purposes*/
    radio.stopListening(); /*Stop listening and begin transmitting*/
    delay(100); /*Quite a long delay -- figure out how to fix this because it is causing the jittering*/
    if(radio.write(&ForeAft_Output, sizeof(ForeAft_Output)),Serial.println("sent ForeAft")); /*Send Up/Down data*/
    if(radio.write(&LeftRight_Output, sizeof(LeftRight_Output)),Serial.println("sent LeftRight")); /*Send Left/Right data*/

// Moving either defense mechanism on the Go-Kart by sending the value from pressing Left or Right on the Android device through Bluetooth
    if (Serial.available()) {
      val=Serial.read();
      parseCommand(val); /*Parse the input*/
      delay(100);
    }
    if(digitalRead(estop)==LOW) {
      val = 'T';
    } 
    radio.openWritingPipe(addresses[1]); /*This time open the writing pipe to be able to receive at the Go-Kart ("gkart")*/
    radio.write(&val, sizeof(val)); /*Transmit val to the Go-Kart to successfully deploy the defense mechanisms*/
    Serial.println(val);
    val = '2'; /*Set val to char 2 to change the value of val after last instruction*/
}

// Function to convert and scale the Up/Down data
float convertForeAftToServo(float y) {
    int result;
    result = map(y, Aft_Limit, Fore_Limit, 10, 170); /*0 to 180*/
    return result;
}

// Function to convert and scale the Left/Right data
float convertLeftRightToServo(float x) {
    int result;
    result = map(x, Left_Limit, Right_Limit, 10, 170);
    return result;
}

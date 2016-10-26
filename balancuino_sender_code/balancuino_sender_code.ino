// Code for sending device, in the balancing base

// Libraries required for MPU
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include "RunningMedian.h"

// Libraries required for communication with nrf24L01 radio
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

// MPU
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

// JOYSTICK
#define JOYSTICK_ORIENTATION 1     // 0, 1 or 2 to set the angle of the joystick
#define JOYSTICK_DIRECTION   0     // 0/1 to flip joystick direction
#define ATTACK_THRESHOLD     10000 // The threshold that triggers an attack
#define JOYSTICK_DEADZONE    5     // Angle to ignore
int joystickTilt = 0;              // Stores the angle of the joystick
int joystickWobble = 0;            // Stores the max amount of acceleration (wobble)

RunningMedian MPUAngleSamples = RunningMedian(5);
RunningMedian MPUWobbleSamples = RunningMedian(5);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);
    
  // MPU
  Wire.begin();
  accelgyro.initialize();  
  //Serial.println("hello");
    
  // Setup Mirf communication using nrf24L01
  Mirf.spi = &MirfHardwareSpi;
  Mirf.csnPin = 7; //(This is optional to change the chip select pin)
  Mirf.cePin = 8; //(This is optional to change the enable pin)
  Mirf.init(); 
  Mirf.setTADDR((byte *)"serv1");
  Mirf.payload = 32;
  Mirf.channel = 56;
  //Mirf.channel = 46;
  Mirf.config();
  
  Serial.println("Beginning ... "); // "Beginning ..." on sender, or "Listening ..." on sever (Receiver)
  
}

// Define table containing message to be sent
byte message[32];

void loop() {
  Serial.println("eil");
  delay(200);
  getInput();
  Serial.print("joystickTilt: ");
  Serial.print(joystickTilt);
  Serial.print("  joystickWobble: ");
  Serial.println(joystickWobble);  
  
  // Read Joystics positions into message table
  message[0]=map(joystickTilt,-100,100,0,255);
  message[1]=map(joystickWobble,0,33000,0,255);
  
  // Serial print for verification
  //Serial.print("message[0]: ");
  //Serial.print(message[0]);
  //Serial.print("   message[1]: ");
  //Serial.println(message[1]);  
      
  // Send message using nrf24L01 and Mirf librairy
  Mirf.send((byte *) &message);
    while(Mirf.isSending()){
      Serial.println("r");
  }
}


// ---------------------------------
// ----------- JOYSTICK ------------
// ---------------------------------
void getInput(){
    // This is responsible for the player movement speed and attacking. 
    // You can replace it with anything you want that passes a -90>+90 value to joystickTilt
    // and any value to joystickWobble that is greater than ATTACK_THRESHOLD (defined at start)
    // For example you could use 3 momentery buttons:
        // if(digitalRead(leftButtonPinNumber) == HIGH) joystickTilt = -90;
        // if(digitalRead(rightButtonPinNumber) == HIGH) joystickTilt = 90;
        // if(digitalRead(attackButtonPinNumber) == HIGH) joystickWobble = ATTACK_THRESHOLD;
   
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    int a = (JOYSTICK_ORIENTATION == 0?ax:(JOYSTICK_ORIENTATION == 1?ay:az))/166;
    //int g = (JOYSTICK_ORIENTATION == 0?gx:(JOYSTICK_ORIENTATION == 1?gy:gz));
    int g = (JOYSTICK_ORIENTATION == 0?gy:(JOYSTICK_ORIENTATION == 1?gx:gz));
    if(abs(a) < JOYSTICK_DEADZONE) a = 0;
    if(a > 0) a -= JOYSTICK_DEADZONE;
    if(a < 0) a += JOYSTICK_DEADZONE;
    MPUAngleSamples.add(a);
    MPUWobbleSamples.add(g);
       
    joystickTilt = MPUAngleSamples.getMedian();
    if(JOYSTICK_DIRECTION == 1) {
        joystickTilt = 0-joystickTilt;
    }
    joystickWobble = abs(MPUWobbleSamples.getHighest());
 //Serial.println("routineout");
}


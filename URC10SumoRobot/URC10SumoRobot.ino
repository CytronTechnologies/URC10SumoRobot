/*******************************************************************************
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTY AND SUPPORT
 * IS APPLICABLE TO THIS SOFTWARE IN ANY FORM. CYTRON TECHNOLOGIES SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR CONSEQUENTIAL
 * DAMAGES, FOR ANY REASON WHATSOEVER.
 ********************************************************************************
 * DESCRIPTION:
 *
 * This example shows how to drive 2 motors using 4 PWM pins (2 for each motor)
 * with 2-channel motor driver.
 * 
 * 
 * CONNECTIONS:
 * 
 * Arduino D3  - Motor Driver PWM 1A Input
 * Arduino D9  - Motor Driver PWM 1B Input
 * Arduino D10 - Motor Driver PWM 2A Input
 * Arduino D11 - Motor Driver PWM 2B Input
 * Arduino GND - Motor Driver GND
 *
 *
 * AUTHOR   : Kong Wai Weng
 * COMPANY  : Cytron Technologies Sdn Bhd
 * WEBSITE  : www.cytron.io
 * EMAIL    : support@cytron.io
 *
 *******************************************************************************/

#include <IRremote.h>
#include <CytronMotorDriver.h>



// Pins assignment.
#define BUTTON        13
#define LED0          0
#define LED1          1
#define RECV_PIN      2
#define LINE_L        12
#define LINE_R        11
#define OBSTACLE_L    3
#define OBSTACLE_R    10
#define OBSTACLE_FR   A0
#define OBSTACLE_FC   A1
#define OBSTACLE_FL   A2

// Direction.
#define LEFT    0
#define RIGHT   1

// IR Code.
#define IR_CODE_OK    0xFF38C7


// Global variables.
uint8_t searchDir = LEFT;
uint32_t timestamp = 0;
decode_results results;


// Configure the motor driver.
CytronMD motorL(PWM_DIR, 5, 4);
CytronMD motorR(PWM_DIR, 6, 7);

// Configure the IR Remote Rx.
IRrecv irrecv(RECV_PIN);



// U-Turn.
void uTurn(uint8_t dir) {
  // Stop the motors.
  motorL.setSpeed(0);
  motorR.setSpeed(0);
  delay(100);
  
  // Reverse.
  motorL.setSpeed(-255);
  motorR.setSpeed(-255);
  delay(200);

  // Stop the motors.
  motorL.setSpeed(0);
  motorR.setSpeed(0);
  delay(100);

  // Rotate..
  if (dir == LEFT) {
    motorL.setSpeed(-150);
    motorR.setSpeed(150);
  } else {
    motorL.setSpeed(150);
    motorR.setSpeed(-150);
  }
  delay(100);
  uint32_t uTurnTimestamp = millis();
  while (millis() - uTurnTimestamp < 300) {
    if ( !(digitalRead(OBSTACLE_FC) && digitalRead(OBSTACLE_FL) && digitalRead(OBSTACLE_FR)) ) {
      // Stop the motors.
      motorL.setSpeed(0);
      motorR.setSpeed(0);
      delay(100);

      return;
    }
  }

  // Move forward.
  motorL.setSpeed(255);
  motorR.setSpeed(255);
  delay(200);
}



void search() {
  if (searchDir == LEFT) {
    motorL.setSpeed(100);
    motorR.setSpeed(255);
  } else {
    motorL.setSpeed(255);
    motorR.setSpeed(100);
  }
}



void attack() {
  uint32_t straightTimestamp = millis();
  
  if (digitalRead(OBSTACLE_FC) == 0) {
    motorL.setSpeed(255);
    motorR.setSpeed(255);
  }
  else if (digitalRead(OBSTACLE_L) == 0) {
    motorL.setSpeed(-150);
    motorR.setSpeed(150);
    while (millis() - straightTimestamp < 400) {
      if (digitalRead(OBSTACLE_FC) == 0) {
        break;
      }
    }
  }
  else if (digitalRead(OBSTACLE_R) == 0) {
    motorL.setSpeed(150);
    motorR.setSpeed(-150);
    while (millis() - straightTimestamp < 400) {
      if (digitalRead(OBSTACLE_FC) == 0) {
        break;
      }
    }
  }
  else if (digitalRead(OBSTACLE_FL) == 0) {
    motorL.setSpeed(0);
    motorR.setSpeed(255);
  }
  else if (digitalRead(OBSTACLE_FR) == 0) {
    motorL.setSpeed(255);
    motorR.setSpeed(0);
  }
}



// The setup routine runs once when you press reset.
void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(RECV_PIN, INPUT_PULLUP);
  pinMode(LINE_L, INPUT_PULLUP);
  pinMode(LINE_R, INPUT_PULLUP);
  pinMode(OBSTACLE_L, INPUT_PULLUP);
  pinMode(OBSTACLE_R, INPUT_PULLUP);
  pinMode(OBSTACLE_FL, INPUT_PULLUP);
  pinMode(OBSTACLE_FC, INPUT_PULLUP);
  pinMode(OBSTACLE_FR, INPUT_PULLUP);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);

  // Turn off the LEDs.
  // Those LEDs are active low.
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  
  // Stop the motors.
  motorL.setSpeed(0);
  motorR.setSpeed(0);

  // Initialize the IR remote receiver.
  irrecv.enableIRIn();

  // Wait until ok signal is received.
  do {
    if (irrecv.decode(&results)){
      irrecv.resume();
    }
  } while (results.value != IR_CODE_OK);

  
  digitalWrite(LED0, LOW);
  

  // Start delay.
  delay(1000);


  digitalWrite(LED1, LOW);
  
  timestamp = millis();
}


// The loop routine runs over and over again forever.
void loop() {
//  if (digitalRead(LINE_L)) {
//    digitalWrite(0, HIGH);
//  } else {
//    digitalWrite(0, LOW);
//  }
//
//  if (digitalRead(LINE_R)) {
//    digitalWrite(1, HIGH);
//  } else {
//    digitalWrite(1, LOW);
//  }
//
//  return;
  
  if (!digitalRead(LINE_L)) {
    uTurn(RIGHT);
    searchDir ^= 1;
  }
  else if (!digitalRead(LINE_R)) {
    uTurn(LEFT);
    searchDir ^= 1;
  }
  else {
    // If opponent is not detected.
    if ( digitalRead(OBSTACLE_FC) &&
         digitalRead(OBSTACLE_FL) &&
         digitalRead(OBSTACLE_FR) &&
         digitalRead(OBSTACLE_L) &&
         digitalRead(OBSTACLE_R) ) {
      search();
    } else {
      attack();
    }
  }


  // Stop if ok signal received.
  if (irrecv.decode(&results)) {
    if (results.value == IR_CODE_OK) {
      // Stop the motors.
      motorL.setSpeed(0);
      motorR.setSpeed(0);

      // Turn off both LEDs.
      digitalWrite(LED0, HIGH);
      digitalWrite(LED1, HIGH);
      while (1);
    }
    irrecv.resume();
  }

  
//  if (millis() - timestamp > 10000) {
//    // Stop the motors.
//    motorL.setSpeed(0);
//    motorR.setSpeed(0);
//    while (1);
//  }
}

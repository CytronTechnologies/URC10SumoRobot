/*******************************************************************************
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTY AND SUPPORT
 * IS APPLICABLE TO THIS SOFTWARE IN ANY FORM. CYTRON TECHNOLOGIES SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR CONSEQUENTIAL
 * DAMAGES, FOR ANY REASON WHATSOEVER.
 ********************************************************************************
 * DESCRIPTION:
 *
 * This is the example of Sumo Robot based on the URC10 Sumo Robot Controller.
 * 
 *
 *
 * AUTHOR   : Kong Wai Weng
 * COMPANY  : Cytron Technologies Sdn Bhd
 * WEBSITE  : www.cytron.io
 * EMAIL    : support@cytron.io
 *
 *******************************************************************************/

#include <CytronMotorDriver.h>


// Wiring Connections.
#define LED0          0   // Onboard LED 0
#define LED1          1   // Onboard LED 1
#define BUTTON        13  // Start button
#define EDGE_L        12  // Left edge sensor
#define EDGE_R        11  // Right edge sensor
#define OPPONENT_L    3   // Left opponent sensor
#define OPPONENT_R    10  // Right opponent sensor
#define OPPONENT_FR   A0  // Front right opponent sensor
#define OPPONENT_FC   A1  // Front center opponent sensor
#define OPPONENT_FL   A2  // Front left opponent sensor

// Direction.
#define LEFT    0
#define RIGHT   1


// Global variables.
uint8_t searchDir = LEFT;


// Configure the motor driver.
CytronMD motorL(PWM_DIR, 5, 4);
CytronMD motorR(PWM_DIR, 6, 7);



/*******************************************************************************
 * Start Routine
 * This function should be called once only when the game start.
 *******************************************************************************/
 void startRoutine() {
  // Start delay.
  delay(1000);

  // Turn right around 45 degress.
  motorL.setSpeed(255);
  motorR.setSpeed(0);
  delay(180);

  // Go straight.
  motorL.setSpeed(255);
  motorR.setSpeed(255);
  delay(450);

  // Turn left until opponent is detected.
  motorL.setSpeed(-0);
  motorR.setSpeed(255);
  uint32_t startTimestamp = millis();
  while (digitalRead(OPPONENT_FC)) {
    // Quit if opponent is not found after timeout.
    if (millis() - startTimestamp > 400) {
      break;
    }
  }
  
 }


/*******************************************************************************
 * Back Off
 * This function should be called when the ring edge is detected.
 *******************************************************************************/
void backoff(uint8_t dir) {
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

  // Start looking for opponent.
  // Timeout after a short period.
  uint32_t uTurnTimestamp = millis();
  while (millis() - uTurnTimestamp < 300) {
    // Opponent is detected if either one of the opponent sensor is triggered.
    if ( !digitalRead(OPPONENT_FC) ||
         !digitalRead(OPPONENT_FL) ||
         !digitalRead(OPPONENT_FR) ||
         !digitalRead(OPPONENT_L) ||
         !digitalRead(OPPONENT_R) ) {
          
      // Stop the motors.
      motorL.setSpeed(0);
      motorR.setSpeed(0);
      delay(100);

      // Return to the main loop and run the attach program.
      return;
    }
  }

  // If opponent is not found, move forward and continue searching in the main loop..
  motorL.setSpeed(255);
  motorR.setSpeed(255);
  delay(200);
}


/*******************************************************************************
 * Search
 *******************************************************************************/
void search() {
  // Move in circular motion.
  if (searchDir == LEFT) {
    motorL.setSpeed(100);
    motorR.setSpeed(255);
  } else {
    motorL.setSpeed(255);
    motorR.setSpeed(100);
  }
}


/*******************************************************************************
 * Attack
 * Track and attack the opponent in full speed.
 * Do nothing if opponent is not found.
 *******************************************************************************/
void attack() {
  uint32_t attackTimestamp = millis();

  // Opponent in front center.
  // Go straight in full speed.
  if (!digitalRead(OPPONENT_FC)) {
    motorL.setSpeed(255);
    motorR.setSpeed(255);
  }

  // Opponent in front left.
  // Turn left.
  else if (!digitalRead(OPPONENT_FL)) {
    motorL.setSpeed(0);
    motorR.setSpeed(255);
  }

  // Opponent in front right.
  // Turn right.
  else if (!digitalRead(OPPONENT_FR)) {
    motorL.setSpeed(255);
    motorR.setSpeed(0);
  }

  // Opponent in left side.
  // Rotate left until opponent is in front.
  else if (!digitalRead(OPPONENT_L)) {
    motorL.setSpeed(-150);
    motorR.setSpeed(150);
    while (digitalRead(OPPONENT_FC)) {
      // Quit if opponent is not found after timeout.
      if (millis() - attackTimestamp > 400) {
        break;
      }
    }
  }

  // Opponent in right side.
  // Rotate right until opponent is in front.
  else if (digitalRead(OPPONENT_R) == 0) {
    motorL.setSpeed(150);
    motorR.setSpeed(-150);
    while (digitalRead(OPPONENT_FC)) {
      // Quit if opponent is not found after timeout.
      if (millis() - attackTimestamp > 400) {
        break;
      }
    }
  }
  
}



/*******************************************************************************
 * Setup
 * This function runs once after reset.
 *******************************************************************************/
void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(EDGE_L, INPUT_PULLUP);
  pinMode(EDGE_R, INPUT_PULLUP);
  pinMode(OPPONENT_L, INPUT_PULLUP);
  pinMode(OPPONENT_R, INPUT_PULLUP);
  pinMode(OPPONENT_FL, INPUT_PULLUP);
  pinMode(OPPONENT_FC, INPUT_PULLUP);
  pinMode(OPPONENT_FR, INPUT_PULLUP);

  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);

  // Turn off the LEDs.
  // Those LEDs are active low.
  digitalWrite(LED0, HIGH);
  digitalWrite(LED1, HIGH);
  
  // Stop the motors.
  motorL.setSpeed(0);
  motorR.setSpeed(0);

  // Wait until button is pressed.
  while (digitalRead(BUTTON)) {
    // While waiting, show the status of the edge sensor for easy calibration.
    if (!digitalRead(EDGE_L)) {
       digitalWrite(LED1, LOW);
    } else {
      digitalWrite(LED1, HIGH);
    }

    if (!digitalRead(EDGE_R)) {
       digitalWrite(LED0, LOW);
    } else {
      digitalWrite(LED0, HIGH);
    }
  }

  // Wait until button is released.
  while (!digitalRead(BUTTON));

  // Turn on the LEDs.
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);
  
  // Start routine..
  startRoutine();
}


/*******************************************************************************
 * Main program loop.
 *******************************************************************************/
void loop() {
  // Edge is detected on the left.
  if (!digitalRead(EDGE_L)) {
    // Back off and make a U-Turn to the right.
    backoff(RIGHT);

    // Toggle the search direction.
    searchDir ^= 1;
  }
  
  // Edge is detected on the right.
  else if (!digitalRead(EDGE_R)) {
    // Back off and make a U-Turn to the right.
    backoff(LEFT);
    
    // Toggle the search direction.
    searchDir ^= 1;
  }

  // Edge is not detected.
  else {
    // Keep searching if opponent is not detected.
    if ( digitalRead(OPPONENT_FC) &&
         digitalRead(OPPONENT_FL) &&
         digitalRead(OPPONENT_FR) &&
         digitalRead(OPPONENT_L) &&
         digitalRead(OPPONENT_R) ) {
      search();
    }
    
    // Attack if opponent is in view.
    else {
      attack();
    }
  }

  // Stop the robot if the button is pressed.
  if (!digitalRead(BUTTON)) {
    // Stop the motors.
    motorL.setSpeed(0);
    motorR.setSpeed(0);

    // Loop forever here.
    while (1);
  }

}

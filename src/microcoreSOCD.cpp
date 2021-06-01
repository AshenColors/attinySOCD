/*
 * LIPSOIC.c
 *
 * Created: 5/28/2021 10:54:07 PM
 * Author : ashen
 */ 
 
#include <Arduino.h>

#define LIP_SWITCH PB2 
#define L_IN PB0
#define R_IN PB1
#define L_OUT PB3
#define R_OUT PB4

void simpleSOCD();
void LIPSOCD();
void blinkenlights();

//stored by pin number, 2 is an error state that gives neutral
byte initial_input = 2; 

void setup() {
  // initialize the LED pin as an output:
  pinMode(L_IN, INPUT_PULLUP);
  pinMode(R_IN, INPUT_PULLUP);
  // not in use yet
  // pinMode(LIP_SWITCH, INPUT_PULLUP);

  // initialize the pushbutton pin as an input:
  pinMode(L_OUT, OUTPUT);
  pinMode(R_OUT, OUTPUT);
}

void loop() {
  switch (digitalRead(LIP_SWITCH)) {
    case LOW:
      simpleSOCD();
      break;
    case HIGH:
      LIPSOCD();
      break;
  }
}

void simpleSOCD() {
  byte leftRead = digitalRead(L_IN);
  byte rightRead = digitalRead(R_IN);
  
  if (leftRead == rightRead) {
    //output neutral
    digitalWrite(L_OUT, HIGH);
    digitalWrite(R_OUT, HIGH);
  } else if (leftRead == LOW) {
    // activate PCB (LED OFF):
    digitalWrite(L_OUT, LOW);
    digitalWrite(R_OUT, HIGH);
  } else if (rightRead == LOW) {
    digitalWrite(L_OUT, HIGH);
    digitalWrite(R_OUT, LOW);
  } 
}

void LIPSOCD() {
  byte leftRead = digitalRead(L_IN);
  byte rightRead = digitalRead(R_IN);
  
  if (leftRead == LOW && rightRead == LOW) {
    switch (initial_input) {
      case 2: //undefined state from startup
        digitalWrite(L_OUT, HIGH);
        digitalWrite(R_OUT, HIGH);
        break;
      case L_IN: //output the opposite of the initial input
        digitalWrite(L_OUT, HIGH);
        digitalWrite(R_OUT, LOW);
        break;
      case R_IN:
        digitalWrite(L_OUT, LOW);
        digitalWrite(R_OUT, HIGH);
        break;
      default:
        blinkenlights();
    }
  } else if (leftRead == LOW) {
    // activate PCB (LED OFF):
    digitalWrite(L_OUT, LOW);
    digitalWrite(R_OUT, HIGH);
    initial_input = L_IN;
  } else if (rightRead == LOW) {
    digitalWrite(L_OUT, HIGH);
    digitalWrite(R_OUT, LOW);
    initial_input = R_IN;
  } else if (leftRead == HIGH && rightRead == HIGH) {
    //output neutral
    digitalWrite(L_OUT, HIGH);
    digitalWrite(R_OUT, HIGH);
  } else {
      for(byte i = 0; i <= 4; i++) {
        blinkenlights();
      }
  }
}

void blinkenlights() {
    digitalWrite(L_OUT, HIGH);
    digitalWrite(R_OUT, LOW);
    delay(100);
    digitalWrite(L_OUT, LOW);
    digitalWrite(R_OUT, HIGH);
    delay(100);
}

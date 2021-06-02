/*
 * LIPSOIC.c
 *
 * Created: 5/28/2021 10:54:07 PM
 * Author : ashen
 */

#include <Arduino.h>
#include <avr/io.h>
// #include <eewl.h>

//Pin definitions
#define IP_MODE_SWITCH PB1
#define L_IN PB0
#define R_IN PB2
#define L_OUT PB3
#define R_OUT PB4

// #define BUFFER_START 0x4      // buffer start address
// #define BUFFER_LEN 50         // number of data blocks

//stored by input pin number, 1 is an error state that gives neutral
byte initial_input = 1;
byte input_priority = 1; //0 = L_IN priority, 2 = R_IN priority, 1 = neutral, 3 = last input priority
volatile byte button_pressed = 0;

// EEWL eepromIPconfig(input_priority, BUFFER_LEN, BUFFER_START);

void SOCD(); //main socd-handling function
void configIPmode(); //does the configuration
void buttonpress();

void setup()
{
  // initialize the LED pin as an output:
  // pinMode(L_IN, INPUT_PULLUP);
  // pinMode(R_IN, INPUT_PULLUP);
  // pinMode(IP_MODE_SWITCH, INPUT_PULLUP);
  // initialize the pushbutton pin as an input:
  // pinMode(L_OUT, OUTPUT);
  // pinMode(R_OUT, OUTPUT);
  DDRB = (1 << L_OUT) | (1 << R_OUT);
  PORTB =  (1 << L_IN) | (1 << R_IN) | (1<<IP_MODE_SWITCH);

  //set IP mode from eeprom, or force a fake button push on boot
  // if (!(eepromIPconfig.get(input_priority)))
  // {
      button_pressed = 1;
  // }

  //attach our interrupt
  attachInterrupt(digitalPinToInterrupt(IP_MODE_SWITCH), buttonpress, FALLING);
}

void loop()
{
  //initiate configuration if we have a button press
  if (button_pressed)
  {
    configIPmode();
  }
  SOCD();
}

//this handles everything
//replaces simpleSOCD() and LIPSOCD()
void SOCD()
{
  byte leftRead = digitalRead(L_IN);
  byte rightRead = digitalRead(R_IN);

  //same for all input methods
  if (leftRead == HIGH && rightRead == HIGH)
  {
    //output neutral
    PORTB |= (1 << L_OUT) | (1 << R_OUT);
    // digitalWrite(L_OUT, HIGH);
    // digitalWrite(R_OUT, HIGH);
  }
  else if (leftRead == LOW && rightRead == LOW)
  {
    switch (input_priority)
    {
    case L_IN:
      // digitalWrite(L_OUT, LOW);
      // digitalWrite(R_OUT, HIGH);
      PORTB &= ~(1 << L_OUT);
      PORTB |= (1 << R_OUT);
      break;
    case R_IN:
      // digitalWrite(L_OUT, HIGH);
      // digitalWrite(R_OUT, LOW);
      PORTB |= (1 << L_OUT);
      PORTB &= ~(1 << R_OUT);
      break;
    case 1:
      //neutral
      // digitalWrite(L_OUT, HIGH);
      // digitalWrite(R_OUT, HIGH);
      PORTB |= (1 << L_OUT) | (1 << R_OUT);
      break;
    case 3:
      //LIP SOCD
      switch (initial_input)
      {
      case L_IN:
        PORTB |= (1 << L_OUT);
        PORTB &= ~(1 << R_OUT);
        break;
      case R_IN:
        PORTB &= ~(1 << L_OUT);
        PORTB |= (1 << R_OUT);
        break;
      case 1:
      default:
        PORTB |= (1 << L_OUT);
        PORTB |= (1 << R_OUT);
      }
    }
  }
  else
  {
    if (leftRead == 0)
    {
      // digitalWrite(L_OUT, LOW);
      // digitalWrite(R_OUT, HIGH);
      PORTB &= ~(1 << L_OUT);
      PORTB |= (1 << R_OUT);
      initial_input = L_IN;
    } 
    else if (rightRead == 0) 
    {
      PORTB |= (1 << L_OUT);
      PORTB &= ~(1 << R_OUT);
      initial_input = R_IN;
    }
  }
}

void buttonpress()
{
  button_pressed = 1;
}

void configIPmode()
{
  while (digitalRead(IP_MODE_SWITCH) == LOW)
  {
    digitalWrite(L_OUT, digitalRead(L_IN));
    digitalWrite(R_OUT, digitalRead(R_IN));
  }
  byte leftRead = digitalRead(L_IN);
  byte rightRead = digitalRead(R_IN);
  if (leftRead == HIGH && rightRead == HIGH) //no input buttons pressed, go to neutral
    input_priority = 1;
  else if (leftRead == LOW && rightRead == HIGH)
    input_priority = L_IN;
  else if (leftRead == HIGH && rightRead == LOW)
    input_priority = R_IN;
  else if (leftRead == LOW && rightRead == LOW)
    input_priority = 3;
  // eepromIPconfig.put(input_priority);
  initial_input = 1;
  button_pressed = 0;
}
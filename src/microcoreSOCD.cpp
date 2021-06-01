/*
 * LIPSOIC.c
 *
 * Created: 5/28/2021 10:54:07 PM
 * Author : ashen
 */

#include <Arduino.h>
//#include <RingEEPROM.h>

//Pin definitions
#define IP_MODE_SWITCH PB1
#define L_IN PB0
#define R_IN PB2
#define L_OUT PB3
#define R_OUT PB4

//stored by input pin number, 1 is an error state that gives neutral
byte initial_input = 1;
byte input_priority = 3; //0 = L_IN priority, 2 = R_IN priority, 1 = neutral, 3 = last input priority
volatile byte button_pressed = 0;


void SOCD(); //main socd-handling function
void showIPmode(); //Reads from EEPROM and displays whatever it is through a sequence of outputs.
void configIPmode(); //does the configuration
void buttonpress();

void setup()
{
  // initialize the LED pin as an output:
  pinMode(L_IN, INPUT_PULLUP);
  pinMode(R_IN, INPUT_PULLUP);
  pinMode(IP_MODE_SWITCH, INPUT_PULLUP);

  // initialize the pushbutton pin as an input:
  pinMode(L_OUT, OUTPUT);
  pinMode(R_OUT, OUTPUT);

  //attach our interrupt
  attachInterrupt(digitalPinToInterrupt(IP_MODE_SWITCH), buttonpress, FALLING);
}

void loop()
{
  //initiate configuration if we have a button press
  // if (button_pressed)
  // {
  //   configIPmode();
  //   button_pressed = 0;
  // }

  //call the right SOCD function
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
    digitalWrite(L_OUT, HIGH);
    digitalWrite(R_OUT, HIGH);
  }
  else if (leftRead == LOW && rightRead == LOW)
  {
    switch (input_priority)
    {
    case L_IN:
      digitalWrite(L_OUT, HIGH);
      digitalWrite(R_OUT, LOW);
      break;
    case R_IN:
      digitalWrite(L_OUT, LOW);
      digitalWrite(R_OUT, HIGH);
      break;
    case 1:
      //neutral
      digitalWrite(L_OUT, HIGH);
      digitalWrite(R_OUT, HIGH);
      break;
    case 3:
      //LIP SOCD
      switch (initial_input)
      {
      case L_IN:
        digitalWrite(L_OUT, HIGH);
        digitalWrite(R_OUT, LOW);
        break;
      case R_IN:
        digitalWrite(L_OUT, LOW);
        digitalWrite(R_OUT, HIGH);
        break;
      case 1:
      default:
        digitalWrite(L_OUT, HIGH);
        digitalWrite(R_OUT, HIGH);
      }
    }
  }
  else
  {
    if (leftRead == 0)
    {
      digitalWrite(L_OUT, LOW);
      digitalWrite(R_OUT, HIGH);
      initial_input = L_IN;
    } 
    else if (rightRead == 0) 
    {
      digitalWrite(L_OUT, HIGH);
      digitalWrite(R_OUT, LOW);
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
  byte basetime = millis();
  while (digitalRead(IP_MODE_SWITCH) == LOW)
  {
    //freeze until the switch is released, allowing pressing of buttons
  }
  if (millis() - basetime > 1000) //if held for more than a second
  {                               //start real configuration
    byte leftRead = digitalRead(L_IN);
    byte rightRead = digitalRead(R_IN);
    if (leftRead == HIGH && rightRead == HIGH) //no input buttons pressed, go to neutral
      input_priority = 2;
    else if (leftRead == LOW && rightRead == HIGH)
      input_priority = 0;
    else if (leftRead == HIGH && rightRead == LOW)
      input_priority = 1;
    else if (leftRead == LOW && rightRead == LOW)
      input_priority = 3;
    // byte writeBuffer[PARAM_PACKET_SIZE];
    // writeBuffer[0] = input_priority;
    // myeepRom.savePacket(writeBuffer);
  }
  showIPmode();
}

void showIPmode()
{
  switch (input_priority)
  {
  case 0:
  case 1: //absolute IP
    for (byte i = 0, t = 0; i < 6; i++, t = !t)
    {
      digitalWrite(input_priority + 3, t);
      digitalWrite((!input_priority) + 3, HIGH);
      delay(200);
    }
    break;
  case 2: //neutral IP
    for (byte i = 0; i < 3; i++)
    {
      digitalWrite(L_OUT, LOW);
      digitalWrite(R_OUT, HIGH);
      delay(200);
      digitalWrite(L_OUT, HIGH);
      digitalWrite(R_OUT, HIGH);
      delay(200);
      digitalWrite(L_OUT, HIGH);
      digitalWrite(R_OUT, LOW);
      delay(200);
      digitalWrite(L_OUT, HIGH);
      digitalWrite(R_OUT, HIGH);
      delay(200);
    }
    break;
  case 3: //last input priority
    for (byte i = 0, t = 0; i < 6; i++, t = !t)
    {
      digitalWrite(input_priority + 3, t);
      digitalWrite((!input_priority) + 3, !t);
      delay(200);
    }
    break;
  }
}


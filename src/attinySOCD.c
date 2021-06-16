// attiny SOCD cleaner
// a project by ashen_blue

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

//Pin definitions
#define IP_MODE_SWITCH PB1
#define L_IN PB0
#define R_IN PB2
#define L_OUT PB3
#define R_OUT PB4

//EEPROM definitions
//Implementing the algorithm from https://sites.google.com/site/dannychouinard/Home/atmel-avr-stuff/eeprom-longevity
#define SENOFF 8
#define SENBIT 0x80
#define EEPOOLSIZE 64 - 4 //the whole EEPROM, minus the parts avrdude uses

//stored by input pin number, 1 is an error state that gives neutral
uint8_t initial_input = 1;
uint8_t input_priority = 1; //0 = L_IN priority, 2 = R_IN priority, 1 = neutral, 3 = last input priority
volatile uint8_t button_pressed = 0;

uint8_t NVBLKPOS; //eeprom last byte position

void setup();
void SOCD();
void configIPmode();
void lastnv(), nextnv(), readnv(), writenv(), initcheck();

void setup()
{
  // initialize output pins and input pins/pullups:
  DDRB = (1 << L_OUT) | (1 << R_OUT);
  PORTB = (1 << L_IN) | (1 << R_IN) | (1 << IP_MODE_SWITCH);

  //set input priority mode from eeprom
  lastnv();
  readnv();
  initcheck();

  //attach our interrupt
  //configure interrupt to activate on falling edge
  MCUCR |= 0b10;
  MCUCR &= ~(0b01);
  //enable interrupt on INT0, which is PB1, and enable global interrupt in status register
  GIMSK |= (1 << 6);
  sei();
}

ISR(INT0_vect)
{
  button_pressed = 1;
}

int main()
{
  setup();

  while (1)
  {
    //initiate configuration if we have a button press
    if (button_pressed)
    {
      configIPmode();
    }
    SOCD();
  }
}

//this handles everything
//replaces simpleSOCD() and LIPSOCD()
void SOCD()
{
  uint8_t leftRead = (PINB >> L_IN) & 1;
  uint8_t rightRead = (PINB >> R_IN) & 1;

  //same for all input methods
  if (leftRead == 1 && rightRead == 1)
  {
    //output neutral
    PORTB |= (1 << L_OUT) | (1 << R_OUT);
  }
  else if (leftRead == 0 && rightRead == 0)
  {
    switch (input_priority)
    {
    case L_IN:
      PORTB &= ~(1 << L_OUT);
      PORTB |= (1 << R_OUT);
      break;
    case R_IN:
      PORTB |= (1 << L_OUT);
      PORTB &= ~(1 << R_OUT);
      break;
    case 1:
      //neutral
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
      // digitalWrite(L_OUT, 0);
      // digitalWrite(R_OUT, 1);
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

void configIPmode()
{
  do
  {
    //copy the IN bits to the OUT bits
    PORTB = (PORTB & ~(1 << L_OUT)) | (((PINB >> L_IN) & 1) << L_OUT);
    PORTB = (PORTB & ~(1 << R_OUT)) | (((PINB >> R_IN) & 1) << R_OUT);
  } while (!((PINB >> IP_MODE_SWITCH) & 1));
  uint8_t leftRead = (PINB >> L_IN) & 1;
  uint8_t rightRead = (PINB >> R_IN) & 1;
  if (leftRead == 1 && rightRead == 1) //no input buttons pressed, go to neutral
    input_priority = 1;
  else if (leftRead == 0 && rightRead == 1)
    input_priority = L_IN;
  else if (leftRead == 1 && rightRead == 0)
    input_priority = R_IN;
  else if (leftRead == 0 && rightRead == 0)
    input_priority = 3;
  nextnv();
  writenv();
  initial_input = 1;
  button_pressed = 0;
}

void initcheck(void)
{
  if (input_priority == ~0x00 || input_priority == ~0x80)
  {
    input_priority = 1; // default value
  }
}

// EEPROM handling code past this point. input_priority is hardcoded
// as the variable to use 'cause I'm scared of trying to make this into a library
// and getting this to work at the same time.
void lastnv()
{
  uint8_t sentinel;
  uint8_t i;
  NVBLKPOS = 0;
  sentinel = eeprom_read_byte((const uint8_t *)SENOFF) & SENBIT;
  i = 0;
  while (i < EEPOOLSIZE)
  {
    if ((eeprom_read_byte((const uint8_t *)i + SENOFF) & SENBIT) != sentinel)
      break;
    NVBLKPOS = i;
    i += sizeof(input_priority);
  }
}

void readnv()
{
  uint8_t *p;
  uint8_t i;
  p = (uint8_t *)&input_priority;
  for (i = 0; i < sizeof(input_priority); i++)
  {
    (*p) = eeprom_read_byte((const uint8_t *)NVBLKPOS + i);
    if (i == SENOFF)
      (*p) &= ~SENBIT;
    p++;
  }
}

void nextnv()
{
  NVBLKPOS += sizeof(input_priority);
  if (NVBLKPOS >= EEPOOLSIZE)
    NVBLKPOS = 0;
}

void writenv()
{
  uint8_t i;
  uint8_t sentinel;
  uint8_t *p;
  p = (uint8_t *)&input_priority;
  lastnv();
  nextnv();
  sentinel = eeprom_read_byte((const uint8_t *)SENOFF) & SENBIT;
  if (!NVBLKPOS)
    sentinel ^= SENBIT;
  for (i = 0; i < sizeof(input_priority); i++)
  {
    if (i != SENOFF)
      eeprom_update_byte((uint8_t *)NVBLKPOS + i, *(p + i));
  }
  i = (*(p + SENOFF) & (~SENBIT)) | sentinel;
  eeprom_update_byte((uint8_t *)NVBLKPOS + SENOFF, i);
}
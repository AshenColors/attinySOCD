// microcore SOCD cleaner
// a project by ashen_blue

#include <avr/io.h>
#include <avr/interrupt.h>
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
uint8_t initial_input = 1;
uint8_t input_priority = 1; //0 = L_IN priority, 2 = R_IN priority, 1 = neutral, 3 = last input priority
volatile uint8_t button_pressed = 0;

// EEWL eepromIPconfig(input_priority, BUFFER_LEN, BUFFER_START);

void SOCD();         //main socd-handling function
void configIPmode(); //does the configuration

void setup()
{
  // initialize output pins and input pins/pullups:
  DDRB = (1 << L_OUT) | (1 << R_OUT);
  PORTB = (1 << L_IN) | (1 << R_IN) | (1 << IP_MODE_SWITCH);

  //set IP mode from eeprom, or force a fake button push on boot
  // if (!(eepromIPconfig.get(input_priority)))
  // {
  button_pressed = 1;
  // }

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
    if ((PINB >> L_IN) & 1)
      PORTB |= (1 << L_OUT);
    else
      PORTB &= ~(1 << L_OUT);
    if ((PINB >> R_IN) & 1)
      PORTB |= (1 << R_OUT);
    else
      PORTB &= ~(1 << R_OUT);
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
  // eepromIPconfig.put(input_priority);
  initial_input = 1;
  button_pressed = 0;
}
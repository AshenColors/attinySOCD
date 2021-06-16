The attinySOCD project implements a configurable SOCD cleaner suitable for use with a Zero Delay board
(and presumably other encoders, but they haven't been tested). A single attiny13 can be used
for each axis that needs to be cleaned, and two is enough for any standard hitbox-style controller.
Settings are stored in EEPROM, eliminating the need to configure the controller every time it's powered on.

Configuration is done by depressing a config button, holding down input directions, and then releasing the configuration button.
- If no buttons are held down, simultaneous inputs output neutral. This is the default L+R behavior for hitbox-style controllers.
- If one button is held down, that button will take priority when simultaneous inputs are made. This is the default U+D behavior for hitbox-style controllers (up takes priority).
- If both buttons are held down, the last input will take priority. (Also known as second input priority)

More about these modes can be found here: https://www.hitboxarcade.com/blogs/faq/what-is-an-socd

An attiny13 can be had for about $0.75 in through hole form, or $0.50 for SMD packages (although once
you're using SMD, there are cheaper attiny offerings this can be ported to). Wire a configuration button
to the config pins, put in enough screw terminals to handle all the inputs and outputs, and your end BOM
is considerably tiny.

There's a basic wear leveling algorithm in use, so I think it's safe to say that this design will last
the lifetime of the controller.

My first embedded C project; I beg forgiveness from those that trace my steps.

TODO: design through hole PCB with JST XH (Zero Delay) and screw terminal/bare hole variants.
maybe an smd one too if I start feeling cocky

```
attiny13 pinout
               __ __
(D5/A0) PB5  1|o    |8  VCC
(D3/A3) PB3  2|     |7  PB2 (D2/A1)
(D4/A2) PB4  3|     |6  PB1 (D1/PWM)
        GND  4|_____|5  PB0 (D0/PWM)
```

PB1 is the only interrupt pin and must be used for the config button. One cardinal direction takes 
input on PB0 and outputs on PB4; the second does so on PB2/PB3. PB5 is the reset pin and unused.

Each axis can have its own configuration button, or they can be wired to the same button if you're 
okay with configuring them at the same time (this is my preference and how my protoboard is implemented).
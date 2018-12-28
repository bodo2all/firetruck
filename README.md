# firetruck
Firetruck Model with Arduino Nano and DFPlayer

The code reacts to user choice buttons and blinks various leds
to make the plastic scale model more interesting.
Code runs on an Arduino Nano powered by 5V.
Leds are connected to 5V via a pull-up resistors, and to microcontroler pins.
Audio is provided through DFPlayer, which is connected to TX/RX via 1K resistors
The DFPlayer must be disconnected or powered off to flash the arduino,
and reconnected afterwards.
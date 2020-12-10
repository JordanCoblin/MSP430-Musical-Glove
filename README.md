# MSP430-Musical-Glove
A Musical Glove made using an MSP430 microcontroller and accelerometers

Included are the two main files used for this project, to be viewed as a code sample.

The main.c file was used to program the MSP430, while play_sound.py was used to handle UART communication between the MSP430 and a serial port on a laptop. I used PyGame to produce sounds from my laptop when triggered by finger presses from the glove. These "finger presses" were characterized by an orientation of an accelerometer with respect to the Earth, with one accelerometer attached to each finger of the glove. Hence the idea was that one could play music in a manner similar to a piano without a physical instrument and with complete freedom in the types of sounds one could use. With only five fingers on a hand, a user may simply tilt their hand to the left or right to switch between sets of sounds (e.g. "change octaves").

Short video demos: 
- https://www.youtube.com/watch?v=z_thQZXonDY
- https://www.youtube.com/watch?v=qcQM46SUlmI

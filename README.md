# PicoComputer - Galaksija Emulator for Pi Pico

Emulates a 6k Galaksija, the famous and great Yugoslavian computer from the 1980s.  This is designed to be as simple
as possible.  The original Galaksija was the gift of computing, made accessible by a great man who wanted everyone
to have a computer if they so wanted.

This was designed in the same spirit, for the people who want to build on broadboard, or with minimal soldering.

# Hardware
See picocomp.png for circuit diagram - very simple.  Only the video is mandatory, which is two resistors and a phono connector.

# Credits

VGA driver taken with kind permission from Hunter Adams RP2040-Demos VGA_Graphics:
- https://github.com/vha3/Hunter-Adams-RP2040-Demos/tree/master/VGA_Graphics

Modified for composite video, and mixed with PIOs from Alan Reed's pico-composite-video
- git@github.com:alanpreed/pico-composite-video.git - published under MIT license.

Also I took the monospace font from Alan Reed, who in turn took it from
[here.](https://opengameart.org/content/monospace-bitmap-fonts-english-russian)

# Pi Pico composite video

This repository contains my implementation of a composite video peripheral for the Pi Pico, using the Pico's PIO subsystem. Functions are available for drawing rectangles, XBM images and text to the screen.

Below is a link to a short video demonstrating the Pico's composite video output in action:

[![Pi Pico composite video demonstration](https://img.youtube.com/vi/IoYIfUzPTYo/0.jpg)](https://www.youtube.com/watch?v=IoYIfUzPTYo)

 A writeup of the development process can be found on my personal website [here.](https://areed.me/posts/2021-07-14_implementing_composite_video_output_using_the_pi_picos_pio/)



PiOS
====

My experimental bare metal OS for the raspberry pi.

#### What it does right now:
* Interact with GPIO - Flash LED
* Framebuffer - Drawing pixels
* Terminal - Basic character based display
* Timer - Using the system timer to wait n ms
* Keyboard(en-GB) support

#### What I'm planning to do / Working on:
* Reading/writing to the SD-card
* Loading programs
* Enable MMU

Probably in that order :)

###### Building:
* YAGARTO
* Cygwin
* Windows 7/8

###### Notes:
I use [Chadderz lovely usb driver](https://github.com/Chadderz121/csud).
(I have a copy of the source in my repro as I've made some small modifications to
get it to run with my configuration).

PiOS
====

My experimental bare metal OS for the raspberry pi.

#### What it does right now:
* Interact with GPIO - Flash LED
* Framebuffer - Drawing pixels
* Terminal - Basic character based display
* Timer - Using the system timer to wait n ms
* Keyboard(en-GB) support
* Interrupts (only timer interrupts right now)

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
For Usb I'm using [Chadderz lovely usb driver](https://github.com/Chadderz121/csud).

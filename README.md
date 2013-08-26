PiOS
====

My experimental bare metal OS for the raspberry pi.

#### What it does right now:
* Interact with GPIO - Flash LED
* Framebuffer - Drawing pixels
* Terminal - Basic character based display
* Timer - Using the system timer to wait n ms

#### What I'm planning to do / Working on:
* Keyboard support (input en-GB)
* Reading/writing to the SD-card
* Loading programs
* Enable MMU

Probably in that order :)

NOTE:
I always make an effort to make sure that the source builds (using YAGARTO, Windows 7/8) however,
sometimes I do check ins of unfinished code as I work on multiple sites.
The code *should* still build (and run).

I would also like to point out that I am by no means an expert on C or assembly,
this repro serves as an exerimental platform for me where I code as I learn and learn as I code.

Using [Chadderz lovely usb driver](https://github.com/Chadderz121/csud).
(I have a copy of the source in my repro as I've made some small modifications to
get it to run with my configuration).

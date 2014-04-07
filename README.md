PiOS
====

My experimental bare metal OS for the raspberry pi.

#### What it does right now:
* GPIO - Example: Flash LED
* Framebuffer - Drawing pixels
* Terminal - Character based display
* System Timer
* Interrupts
* MMU
* eMMC reading/writing 
* Fat32 - Small subset (read-only)

#### What I'm planning to do / working on:
* Multitasking
* Fat32 - Writing
* Loading and executing binaries
* Audio
* Support for addition file systems: SFS, EXT2(?)

#### Building:
It should/might/could build under *nix, but this isn't my primary platform.
Windows is therefore listed as a requirement.

* [GNU Tools for ARM](https://launchpad.net/gcc-arm-embedded)
* [Cygwin](http://www.cygwin.com/) (for make)
* Windows 7/8

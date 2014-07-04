PiOS
====

My experimental bare metal OS for the raspberry pi.
This file is used to track its current state and what I'm currently working on.

#### What it does right now:
* GPIO - Example: Flash LED
* Framebuffer - Drawing pixels
* Terminal - Character based display
* System Timer
* Interrupts
* MMU
* eMMC reading/writing 
* Fat32 - Small subset (read-only)
* Loading and executing binaries (ELFs)
* UART and MiniUart send/receive (interrupt based receive)
* Virtual memory (high memory kernel, Low memory user)
* Stacktraces on faults

#### What I'm planning to do / working on:
* Multitasking
* Fat32 - Writing
* Audio
* Support for addition file systems: SFS, EXT2(?)

#### Build requirements:
* [GNU Tools for ARM](https://launchpad.net/gcc-arm-embedded)
* [Cygwin](http://www.cygwin.com/) (Windows only)

#### Building:
Just clone the repository and run make in the root directory

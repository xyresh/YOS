# YOS
A simple operating system built as a learning experience

YOS is a simple os,
it has a monolithic kernel design and simple multitasking.

## Multitasking
A simple round robing scheduler has been built to serve our multitasking purposes.
it still need work as the system can arbitrarily crash. and at the moment no task management is in place.

## Memory
in terms of memory management we have 4KB pages in the os with the help of ppm bitmapping.

## Other features
our os has a shell with a few implemented commands, the shell itself needs work as right now it has a yanderedev type system but we will fix it in the future.
mouse input is supported, however as of now it is of no use, will hopefully be usefull in the future.
## building  
you will need the following dependencies to build:  
`grub-mkrescue qemu-system-i386 nasm gcc`  
to build simply run:  
`make`  
and to run in qemu:  
`make run`  

  
<img width="721" height="402" alt="Screenshot From 2025-08-13 19-41-49" src="https://github.com/user-attachments/assets/8fbf3dd6-7950-442f-a8ce-d94c8cd61f32" />

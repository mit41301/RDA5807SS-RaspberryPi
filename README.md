## I2C detect

<img width="732" height="487" alt="image" src="https://github.com/user-attachments/assets/d5f5e12c-2afc-492f-b9c1-863710996511" />

at addresses 0x10, 0x11, and 0x60. This is part of what makes this device strange, and where the documentation starts to get a little fuzzy.

 

It seems like accessing the device at address 0x60 is how you perform the TEA5767 compatible commands, so this is the last I will mention it, I didn't find that in documentation, and I found no mention in the docs regarding TEA5767, it just seems to be data I've found here and there.

 

Address 0x10 seems to be an address where you write to the I2C device without specifying a register address, the device starts to write the bytes to the 16 bit registers starting at register 0x02, high byte first, then it increments the register counter internally and writes to the next register, and it does this until you have finished writing. This turned out to be a little challenging, but I was able to get it done. I'll explain it when I get to the code.

 

Address 0x11 purports to behave like a regular I2C address in that it expects a register number to be included in the command, I was not able to successfully write to the device in this manner, but I was able to successfully read from the device registers, so that's something.

there are 2 files

radio.c and

radio.h

 

the compile command line is

gcc -Wall -o radio radio.c -lwiringPi

 

then run it with

 

./radio (if you have an issue, use sudo ./radio)

 

you can look in the loop() function for the commands, a few are

+ - Seek Up

- - Seek Down

r - reset tuning to inital station (103.7)

x - power off

o - power on

v - volume up

c - volume down

d - display register values

 

follow any of the commands with the enter key.

 

just look in the big ugly switch statement for the rest

[More Here](https://community.element14.com/members-area/personalblogs/b/michael-conners-s-blog/posts/fm-radio-on-the-raspberry-pi)

 

There does seem to be a bit of a bug in the channel display after a seek, but if you hit d to dump the registers, it should show the proper tuning.

 

Hope you enjoy.

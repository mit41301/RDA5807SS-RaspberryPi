// Program for the Parallax 27984 FM Radio Receiver using RDA5807SS radio chip
// Uses the I2C bus
// Device  Ras Pi
// VCC  (9) - 1
// GND (10) - 6
// SDA  (1) - 3
// SCL  (2) - 5

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <endian.h>
#include <wiringPiI2C.h>
#include "radio.h"

#define CONFIGADDR 0x10
#define STATUSADDR 0x11
#define CONFIGREGLEN 5

int deviceFd;
int statusFd;

unsigned int configRegisters[] ={
  0xc001,0x0000,0x0400,0x86d3,0x4000};

int volume = 0;
int mute = 0;
int stereo = 0;
int bass = 0;


int main()
{
	deviceFd = openDevice(CONFIGADDR);
	statusFd = openDevice(STATUSADDR);
 
	tuneToChannel(1037);
	printChannel();
	loop();
	exit(0);
}

int openDevice(int i2cAddress)
{
	int fd;
	if((fd = wiringPiI2CSetup(i2cAddress)) < 0)
	{
		printf("Error opening device at I2C Address %x\n",i2cAddress);
		exit(fd);
	}
	return fd;
}

void loop()
{
	int c = 0;
	while((c = getchar()) != EOF)
	{
		switch(c)
		{
		case'+':
		  seekUp();
		  break;
		case '-' :
		  seekDown();
		  break;
		case '>' :
		  muteOn();
		  break;
		case '<' :
		  muteOff();
		  break;
		case '1' :
		  freqUp();
		  break;
		case '2' :
		  freqDown();
		  break;
		 case 'x' :
		  powerOff();
		  break;
		case 'o' :
		  powerOn();
		  break;
		case 'v' :
		  volumeUp();
		  break;
		case 'c' :
		  volumeDown();
		  break;
		case 'd' :
		  dumpRegisters();
		  break; 
		case 'r' :
		  resetTuning();
		  break;  
		case 'q' :
		  quit();
		  break;
		}
	if(c != '\n')		
		printChannel();
  }
  return;
}

void initModule()
{
  powerOn();
  reset();
  
  printf("Device initialized\n");
}

void powerOn()
{
  configRegisters[1] |= 0x10;
  configRegisters[0] |= 0x1;
  writeConfigRegisters();
  configRegisters[1] &= 0xffef;
}

void reset()
{
  configRegisters[0] |= 0x2; 
  printConfigRegisters();
  writeConfigRegisters();
  configRegisters[0] &= 0xfffd;
}

void quit()
{
	powerOff();
	close(deviceFd);
	close(statusFd);
	exit(0);
}

void resetTuning()
{
  reset();
  tuneToChannel(1037);
}


void powerOff()
{
  configRegisters[0] &= 0xfffe;
  writeConfigRegisters();
}

void volumeUp()
{
  volume = (configRegisters[3] & 0xf) != 0xf ? (configRegisters[3] & 0xf) : 0xe;
  volume++;
  configRegisters[3] = (configRegisters[3] & 0xfff0) | volume;
  writeConfigRegisters();
}

void volumeDown()
{
  volume = (configRegisters[3] & 0xf) != 0 ? (configRegisters[3] & 0xf) : 0x1;
  volume--;
  configRegisters[3] = (configRegisters[3] & 0xfff0) | volume;
  writeConfigRegisters();
}

void seekUp()
{
  printf("Seeking up\n");
  configRegisters[0] |= 0x300;
  writeConfigRegisters();
  configRegisters[0] &= 0xfcff;
  checkTuneComplete();

}

void seekDown()
{
  printf("Seeking down\n");
  configRegisters[0] |= 0x100;
  writeConfigRegisters();
  configRegisters[0] &= 0xfeff;
  checkTuneComplete();
}


void writeConfigRegisters()
{
  int index,ii=0;
  unsigned char tempRegisters[CONFIGREGLEN * 2];
  
 
  for(index = 0;index < CONFIGREGLEN;index++)
  {
   tempRegisters[ii++] = (configRegisters[index] >> 8) & 0xff;
   tempRegisters[ii++] = configRegisters[index] & 0xff;
  }
  write(deviceFd,tempRegisters,CONFIGREGLEN * 2);
 }


void dumpRegisters()
{
  int ii;
  for(ii=0x02;ii < 0x0c;ii++)
    printRegisterValue(ii);
  printConfigRegisters();
}

void printConfigRegisters()
{
	int ii;
	for(ii=0;ii< CONFIGREGLEN;ii++){
	  printf("Value of config register 0x%x",ii);
	  printf(" is 0x%x\n",configRegisters[ii]);
  }
}


void printRegisterValue(char reg)
{
  printf("Value of register 0x%x",reg);
  printf(" is 0x%x\n",readRegister(reg));
}

void freqUp()
{
  printf("Tuning up\n");
  configRegisters[1] |= 0x10;
  configRegisters[1] = (configRegisters[1] & 0x3f) | ((configRegisters[1] &0xffc0) + (1 <<6));
  writeConfigRegisters();
  configRegisters[1] &= 0xffef;
  checkTuneComplete();}

void freqDown()
{
  printf("Tuning down\n");
  configRegisters[1] |= 0x10;
  configRegisters[1] = (configRegisters[1] & 0x3f) | ((configRegisters[1] &0xffc0) - (1 <<6));
  writeConfigRegisters();
  configRegisters[1] &= 0xffef;
  checkTuneComplete();
}

void muteOn()
{
  printf("Mute on\n");
  mute = 1;
  configRegisters[0] &= 0xbfff;
  writeConfigRegisters();
}

void muteOff()
{
  printf("Mute off\n");
  mute = 0;
  configRegisters[0] |= 0x4000;
  writeConfigRegisters();
}

void printChannel()
{
  printf("Tuned to channel: ");
  int channel = readRegister(0x0a) & 0xff;
  printf("%d",87 + channel/10);
  printf(".");
  printf("%d",channel%10);
  printf(" Mhz\n"); 
}

unsigned int readRegister(int regAddr)
{
  return be16toh(wiringPiI2CReadReg16(statusFd,regAddr)); 
}

void tuneToChannel(int frequency)
{
  frequency -= 870;
  configRegisters[1] |= 0x10;
  configRegisters[1] = (configRegisters[1] & 0x3f) | (frequency << 6);
  writeConfigRegisters();
  configRegisters[1] &= 0xffef;
  checkTuneComplete();
}

void checkTuneComplete()
{
    while((readRegister(0x0a) & 0x4000) != 0) 
     usleep(25);
}

void tuneComplete()
{
  int val = readRegister(0x04);
  wiringPiI2CWriteReg16(deviceFd,0x04,val | 0x4);
  printf("Interrupt fired.");
  printChannel();
}




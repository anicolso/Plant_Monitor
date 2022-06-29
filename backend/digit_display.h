#ifndef _DISPLAYDIGITS_H_
#define _DISPLAYDIGITS_H_

int initI2cBus(char* bus, int address);
void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);
int display_digits(void);
void Digit_startup(void);
void Digit_shutdown(void);

#endif
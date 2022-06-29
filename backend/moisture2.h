// moisture2.h
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <pthread.h>

#ifndef MOISTURE_H
#define MOISTURE_

#define MOISTURE_REF_V 1.8
#define MOISTURE_MAX_READING 4095

void Moisture_runMoisture();
int Moisture_getMoisture();
int initI2cBus(char* bus, int address);
uint16_t readSensor(int i2cFD);
bool read2(int i2cFD, uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num);
void Moisture_startSampling();
void Moisture_stopSampling();

#endif
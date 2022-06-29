// Temp + Humidity + Moisture + Light monitor thread
// To call just create extern variables for each status variable "humidityStatus" etc.
// and call the get functions below which returns 1 if good or 0 if bad

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "bbb_dht_read.h"
#include "util.h"
#include "moisture2.h"
#include <time.h>

#define LIGHT_SENSOR "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095

#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"
#define I2C_DEVICE_ADDRESS 0x36
#define I2C_TOUCH_BASE 0x0f
#define I2C_OFFSET 0x10
#define MILLISECONDS 250
#define DISPLAY_ADDRESS 0x70

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define REG0 0x00
#define REG1 0x01
#define REG2 0x02
#define REG3 0x03
#define REG4 0x04
#define REG5 0x05
#define REG6 0x06
#define REG7 0x07
#define REG8 0x08
#define REG9 0x09

#define REG21 0x21
#define REG22 0x22
#define REG23 0x23
#define REG24 0x24
#define REG25 0x25
#define REG26 0x26
#define REG27 0x27

#define ALL_BITS_ON 0xFF
#define ALL_BITS_OFF 0x00

#define REG_TURN_ON 0x81
#define REG_TURN_OFF 0x80

int fileD;

// --- List of Codes for Different Letters / Numbers (For first symbol on alphanumeric display) --- 
/* 	A: 0x00 0xf7
 	B: 0x00 0x8f, 0x01 0x12
 	b: 0x00 0xfc
 	C: 0x00 0x39
 	D: 0x00 0x0f, 0x01 0x12
 	d: 0x00 0xde
	E: 0x00 0xf9
	F: 0x00 0xf1
	G: 0x00 0xbd
	H: 0x00 0xf6
	h: 0x00 0xf4
	I: 0x00 0x09, 0x01 0x12
	J: 0x00 0x1e
	K: 0x00 0x70, 0x01 0x24 
	L: 0x00 0x38
	M: 0x00 0x36, 0x01 0x05 or 0x00 0x36, 0x01 0x07
	N: 0x00 0x36, 0x01 0x21
	O: 0x00 0x3f
	P: 0x00 0xf3
	Q: 0x00 0x3f, 0x01 0x20
	R: 0x00 0xf3, 0x01 0x20
	S: 0x00 0xed
	T: 0x00 0x01, 0x01 0x12
	U: 0x00 0x3e
	V: 0x00 0x30, 0x01 0x0c
	W: 0x00 0x36, 0x01 0x28 or 0x00 0x36, 0x01 0x38
	X: 0x01 0x2d
	Y: 0x01 0x15 or 0x01 0x17
	Z: 0x00 0x09, 0x01 0x0c
	
	0: 0x00 0x3f, 0x01 0x0c
	1: 0x00 0x06
	2: 0x00 0xdb
	3: 0x00 0xcf
	4: 0x00 0xe6
	5: 0x00 0xed
	6: 0x00 0xfd
	7: 0x00 0x07 or 0x00 0x27
	8: 0x00 0xff
	9: 0x00 0xe7 or 0x00 0xef
*/

// the status variables for the sensors 
bool humidityStatus = true;
bool temperatureStatus = true;
bool lightStatus = true;
bool moistureStatus = true;


// Thread initialization
pthread_t thread_id5;

int initI2cBus_display(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);

	if (result < 0) {
		perror("I2C: Unable to set I2C device to slave address.");
		exit(1);
	}

	return i2cFileDesc;
}

static void writeI2cReg_display(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);

	if (res != 2) {
		perror("I2C: Unable to write i2c register.");
		exit(1);
	}
}

void printLetter(int fileDesc, int displayCharacter, char letter) {

	unsigned int reg1 = 0x00;
	unsigned int reg2 = 0x00;
	
	if (displayCharacter == 0) {
		writeI2cReg_display(fileDesc, REG0, 0x00);
		writeI2cReg_display(fileDesc, REG1, 0x00);
		
		reg1 = REG0;
		reg2 = REG1;
	}
	
	else if (displayCharacter == 1) {
		writeI2cReg_display(fileDesc, REG2, 0x00);
		writeI2cReg_display(fileDesc, REG3, 0x00);
		
		reg1 = REG2;
		reg2 = REG3;
	}
	
	else if (displayCharacter == 2) {
		writeI2cReg_display(fileDesc, REG4, 0x00);
		writeI2cReg_display(fileDesc, REG5, 0x00);
		
		reg1 = REG4;
		reg2 = REG5;
	}
	
	else if (displayCharacter == 3) {
		writeI2cReg_display(fileDesc, REG6, 0x00);
		writeI2cReg_display(fileDesc, REG7, 0x00);
		
		reg1 = REG6;
		reg2 = REG7;
	}
	
	if (letter == 'A') {
		writeI2cReg_display(fileDesc, reg1, 0xf7);
	}
	
	else if (letter == 'B') {
		writeI2cReg_display(fileDesc, reg1, 0x8f);
		writeI2cReg_display(fileDesc, reg2, 0x12);
	}
	
	else if (letter == 'C') {
		writeI2cReg_display(fileDesc, reg1, 0x39);
	}
	
	else if (letter == 'D') {
		writeI2cReg_display(fileDesc, reg1, 0x0f);
		writeI2cReg_display(fileDesc, reg2, 0x12);
	}
	
	else if (letter == 'E') {
		writeI2cReg_display(fileDesc, reg1, 0xf9);
	}
	
	else if (letter == 'F') {
		writeI2cReg_display(fileDesc, reg1, 0xf1);
	}
	
	else if (letter == 'G') {
		writeI2cReg_display(fileDesc, reg1, 0xbd);
	}
	
	else if (letter == 'H') {
		writeI2cReg_display(fileDesc, reg1, 0xf6);
	}
	
	else if (letter == 'I') {
		writeI2cReg_display(fileDesc, reg1, 0x09);
		writeI2cReg_display(fileDesc, reg2, 0x12);
	}
	
	else if (letter == 'J') {
		writeI2cReg_display(fileDesc, reg1, 0x1e);
	}
	
	else if (letter == 'K') {
		writeI2cReg_display(fileDesc, reg1, 0x70);
		writeI2cReg_display(fileDesc, reg2, 0x24);
	}
	
	else if (letter == 'L') {
		writeI2cReg_display(fileDesc, reg1, 0x38);
	}
	
	else if (letter == 'M') {
		writeI2cReg_display(fileDesc, reg1, 0x36);
		writeI2cReg_display(fileDesc, reg2, 0x05);
	}
	
	else if (letter == 'N') {
		writeI2cReg_display(fileDesc, reg1, 0x36);
		writeI2cReg_display(fileDesc, reg2, 0x21);
	}
	
	else if (letter == 'O') {
		writeI2cReg_display(fileDesc, reg1, 0x3f);
	}
	
	else if (letter == 'P') {
		writeI2cReg_display(fileDesc, reg1, 0xf3);
	}

	else if (letter == 'Q') {
		writeI2cReg_display(fileDesc, reg1, 0x3f);
		writeI2cReg_display(fileDesc, reg2, 0x20);
	}
	
	else if (letter == 'R') {
		writeI2cReg_display(fileDesc, reg1, 0xf3);
		writeI2cReg_display(fileDesc, reg2, 0x20);
	}
	
	else if (letter == 'S') {
		writeI2cReg_display(fileDesc, reg1, 0xed);
	}
	
	else if (letter == 'T') {
		writeI2cReg_display(fileDesc, reg1, 0x01);
		writeI2cReg_display(fileDesc, reg2, 0x12);
	}
	
	else if (letter == 'U') {
		writeI2cReg_display(fileDesc, reg1, 0x3e);
	}
	
	else if (letter == 'V') {
		writeI2cReg_display(fileDesc, reg1, 0x30);
		writeI2cReg_display(fileDesc, reg2, 0x0c);
	}
	
	else if (letter == 'W') {
		writeI2cReg_display(fileDesc, reg1, 0x36);
		writeI2cReg_display(fileDesc, reg2, 0x28);
	}
	
	else if (letter == 'X') {
		writeI2cReg_display(fileDesc, reg2, 0x2d);
	}
	
	else if (letter == 'Y') {
		writeI2cReg_display(fileDesc, reg2, 0x15);
	}
	
	else if (letter == 'Z') {
		writeI2cReg_display(fileDesc, reg1, 0x09);
		writeI2cReg_display(fileDesc, reg2, 0x0c);
	}

	else if (letter == '0') {
		writeI2cReg_display(fileDesc, reg1, 0x3f);
		writeI2cReg_display(fileDesc, reg2, 0x0c);
	}

	else if (letter == '1') {
		writeI2cReg_display(fileDesc, reg1, 0x06);
	}

}


// functions to retrieve the status
bool getHumidityStatus(void) {
    return humidityStatus;
}

bool getTemperatureStatus(void) {
    return temperatureStatus;
}

bool getLightStatus(void) {
    return lightStatus;
}

bool getMoistureStatus(void) {
    return moistureStatus;
}

void displayStatus(int status, int value) {

	printLetter(fileD, 0, ' ');
	printLetter(fileD, 1, ' ');
	printLetter(fileD, 2, ' ');
	printLetter(fileD, 3, ' ');

	if (status == 0) {
		printLetter(fileD, 0, 'H');
		printLetter(fileD, 1, 'U');
		printLetter(fileD, 2, 'M');
		
		if (value == 1) {
			printLetter(fileD, 3, '1');
		}

		else if (value == 0) {
			printLetter(fileD, 3, '0');
		}
	}

	if (status == 1) {
		printLetter(fileD, 0, 'T');
		printLetter(fileD, 1, 'E');
		printLetter(fileD, 2, 'M');
		
		if (value == 1) {
			printLetter(fileD, 3, '1');
		}

		else if (value == 0) {
			printLetter(fileD, 3, '0');
		}
	}

	if (status == 2) {
		printLetter(fileD, 0, 'L');
		printLetter(fileD, 1, 'I');
		printLetter(fileD, 2, 'G');
		
		if (value == 1) {
			printLetter(fileD, 3, '1');
		}

		else if (value == 0) {
			printLetter(fileD, 3, '0');
		}
	}

}

// Continuously read sensors and check their outputs
void sensorStatus(void)
{
    //float humidity;

    //float temperature;

    //int reading;

    //double voltage;

    //int i2cFD;

    // uint16_t moisture;

    // printf("Drive display (assumes GPIO #61 and #44 are output and 1\n");
	int i2cFileDesc = initI2cBus_display(I2CDRV_LINUX_BUS1, DISPLAY_ADDRESS);
	fileD = i2cFileDesc;
    //printf("%d\n", i2cFileDesc);
	
	// Initialize all the symbols on the display 
	writeI2cReg_display(i2cFileDesc, REG0, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG1, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG2, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG3, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG4, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG5, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG6, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG7, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG8, 0xFF);
	writeI2cReg_display(i2cFileDesc, REG9, 0xFF);
	
	writeI2cReg_display(i2cFileDesc, REG21, 0xFF);
	
	// Fully turn on the display
	writeI2cReg_display(i2cFileDesc, REG_TURN_ON, 0x00);

	printLetter(i2cFileDesc, 0, ' ');
	printLetter(i2cFileDesc, 1, ' ');
	printLetter(i2cFileDesc, 2, ' ');
	printLetter(i2cFileDesc, 3, ' ');

	// displayStatus(i2cFileDesc, 1, 1);<

	while (1) {
		//humidity = 0; 
		//temperature = 0;

        // Get light reading in Volts
        //  reading = getVoltage0Reading(LIGHT_SENSOR);
        //  voltage = ((double)reading / A2D_MAX_READING) * A2D_VOLTAGE_REF_V;
	
        // get humidity and temperature reading
		//  bbb_dht_read(DHT22, 1, 17, &humidity, &temperature);

        // get moisture reading
        //  i2cFD = initI2cBus(I2CDRV_LINUX_BUS2, I2C_DEVICE_ADDRESS);
        //  moisture = readSensor(i2cFD);

        // check if they are good values
        /*if (humidity <= 0 || humidity > 200) {
            humidityStatus = false;
        } else {
            humidityStatus = true;
        }
        if (temperature <= 0 || temperature > 50) {
            temperatureStatus = false;
        } else {
            temperatureStatus = true;
        }
        if (voltage < 0 || voltage > 3) {
            lightStatus = false;
        } else {
            lightStatus = true;
        }

        if (moisture < 0 || moisture > 1050) {
            moistureStatus = false;
        } else {
            moistureStatus = true;
        }*/

        // PRINTS STATUS FOR NOW
		//printf("HUMIDITY: %f - Status: %s\n", humidity, humidityStatus ? "GOOD" : "BAD");
		//printf("TEMP: %f - Status: %s\n", temperature, temperatureStatus ? "GOOD" : "BAD");
        //printf("LIGHT: %f - Status: %s\n", voltage, lightStatus ? "GOOD" : "BAD");
        //printf("MOISTURE: %d - Status: %s\n", moisture, moistureStatus ? "GOOD" : "BAD");

	}

}



// start thread
void StartSensorStatus(void) {
    pthread_create(&thread_id5, NULL, (void*)sensorStatus, NULL);
}

// stop thread
void StopSensorStatus(void) {
    printf("Stop process\n");

    pthread_join(thread_id5, NULL);

    printf("Process stopped\n");

}

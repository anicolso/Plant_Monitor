#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "sampler.h"
#include "util.h"

#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define I2CDRV_LINUX_BUS0 "/dev/i2c-0"
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"

#define I2C_RIGHT_DIGIT  "/sys/class/gpio/gpio44/value"
#define I2C_LEFT_DIGIT  "/sys/class/gpio/gpio61/value"

#define I2C_SETUP_SCRIPT "\
config-pin p9_18 i2c \n\
config-pin p9_17 i2c \n\
echo 61 > /sys/class/gpio/export \n\
echo 44 > /sys/class/gpio/export \n\
echo out > /sys/class/gpio/gpio61/direction \n\
echo out > /sys/class/gpio/gpio44/direction \n\
echo 1 > /sys/class/gpio/gpio61/value \n\
echo 1 > /sys/class/gpio/gpio44/value \n"


static pthread_t tid3;
static bool displaying_digit = true;

static int initI2cBus(char* bus, int address)
{
    int i2cFileDesc = open(bus, O_RDWR);
    int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
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

// static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
// {
//     // To read a register, must first write the address
//     int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
//     if (res != sizeof(regAddr)) {
//         perror("I2C: Unable to write to i2c register.");
//         exit(1);
//     }
//     // Now read the value and return it
//     char value = 0;
//     res = read(i2cFileDesc, &value, sizeof(value));
//     if (res != sizeof(value)) {
//         perror("I2C: Unable to read from i2c register");
//         exit(1);
//     }
//     return value;
// }

// 0: 134, 161
// 1: 3,   128
// 2: 15,  49
// 3: 6,   176
// 4: 139, 144
// 5: 141, 176
// 6: 141, 177
// 7: 20,  4
// 8: 142, 177
// 9: 142, 144

// Turn on/off left or right digit function
void change_LED(char *LED_type, char *action){
	FILE *pLedFile = fopen(LED_type, "w");
	error_msg("ERROR OPENING %s.\n", pLedFile == NULL, LED_type);

	// Write action to file
	int charWritten = fprintf(pLedFile, "%s", action);
	error_msg("ERROR WRITING DATA TO: %s\n", charWritten <= 0, LED_type);

	fclose(pLedFile);
}

// preset values for digits from 0-9, index i = digit i. i[0] = top half, i[1] = bottom half
int nums[10][2] = {{134, 161}, {3, 128}, {15, 49}, {6, 176}, {139, 144}, {141, 176}, {141, 177}, {20, 4}, {142, 177}, {142, 144}};


int display_digits(void)
{
    while(displaying_digit){

        // Turn off both digits
        change_LED(I2C_LEFT_DIGIT, "0");
        change_LED(I2C_RIGHT_DIGIT, "0");

        int num_dips = Sampler_getNumDips();
        if(num_dips > 99){
            num_dips = 99;
        }

        // Extract individual ints
        int num_dips_ones = num_dips % 10;
        num_dips /= 10;
        int num_dips_tens = num_dips % 10;

        int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
        writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
        writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

        // set top and bottom of digit
        writeI2cReg(i2cFileDesc, REG_OUTB, nums[num_dips_ones][0]);
        writeI2cReg(i2cFileDesc, REG_OUTA, nums[num_dips_ones][1]);

        // Turn on right digit
        change_LED(I2C_LEFT_DIGIT, "0");
        change_LED(I2C_RIGHT_DIGIT, "1");

        nano_sleep(0, 5000000); // 5ms

        // Turn off both digits
        change_LED(I2C_LEFT_DIGIT, "0");
        change_LED(I2C_RIGHT_DIGIT, "0");

        writeI2cReg(i2cFileDesc, REG_OUTB, nums[num_dips_tens][0]);
        writeI2cReg(i2cFileDesc, REG_OUTA, nums[num_dips_tens][1]);

        // Turn on left digit
        change_LED(I2C_LEFT_DIGIT, "1");
        change_LED(I2C_RIGHT_DIGIT, "0");

        nano_sleep(0, 5000000);

        // Cleanup I2C access;
        close(i2cFileDesc);

        nano_sleep(0, 1000000); // sleep for 100 ms 
    }
    return 0;
}

void Digit_startup(void){
    system(I2C_SETUP_SCRIPT);
    pthread_create(&tid3, NULL, (void*)display_digits, NULL);
}

void Digit_shutdown(void){
    displaying_digit = false;  
    pthread_join(tid3, NULL);
}
// moisture2.c
#include "moisture2.h"

#define I2CDRV_LINUX_BUS2 "/dev/i2c-2"
#define I2C_DEVICE_ADDRESS 0x36
#define I2C_TOUCH_BASE 0x0f
#define I2C_OFFSET 0x10

#define MILLISECONDS 250

pthread_t thread_id6;

int Moisture_getMoisture(){
    int i2cFD = initI2cBus(I2CDRV_LINUX_BUS2, I2C_DEVICE_ADDRESS);
    
    uint16_t moist = readSensor(i2cFD);
    int status = -1;
    if(moist < 400){
        status = 1;
        //printf("Very dry\n");
    } else if (moist >= 400 && moist < 475){
        status = 2;
        //printf("Dry\n");
    } else if(moist >= 475 && moist < 550){
        status = 3;
        //printf("Moist\n");
    } else if(moist >= 550 && moist < 625){
        status = 4;
    } else if(moist >= 625 && moist < 700){
        status = 5;
    } else if(moist >= 700 && moist < 775){
        status = 6;
    } else if(moist >= 775 && moist < 850){
        status = 7;
    } else if(moist >= 850 && moist < 925){
        status = 8;
    } else if(moist >= 925 && moist < 1000){
        status = 9;
    } else {
        status = 10;
    }
    return status;
}

// Runs and reads the values to terminal
void Moisture_runMoisture(){
    int i2cFD = initI2cBus(I2CDRV_LINUX_BUS2, I2C_DEVICE_ADDRESS);
    while(1){
        uint16_t moist = readSensor(i2cFD);
        printf("moist = %d", moist);

    }
    close(i2cFD);


    /*
    int v1 = 10;
    int v2 = 10;

    // v1 = v2 = 3000 seems very consistent, clean reads 
    for(int i = 0; i < 300; i++){
        uint16_t ret = readSensor(i2cFD, v1, v2);
        printf("Reading: %d\n", ret);

        v1 += 10;
        v2 += 10;
    }
    */
}


int initI2cBus(char* bus, int address)
{
    int i2cFD = open(bus, O_RDWR);
    int result = ioctl(i2cFD, I2C_SLAVE, address);
    if(result < 0){
        perror("I2C: Unable to set I2C device to slave address.");
        exit(1);
    }
    return i2cFD;
}

// Reads the analog value on capacitative touch-enabled pin
uint16_t readSensor(int i2cFD)
{
    uint8_t buf[2];
    uint16_t ret = 65535;

    while(ret > 1050){
    // Check if good read
        if(read2(i2cFD, I2C_TOUCH_BASE, I2C_OFFSET, buf, 2)){
            ret = ((uint16_t)buf[0] << 8) | buf[1];
        } else{
            printf("ERROR DURING MOISTURE READ.\n");
            break;
        }
    }

    return ret;
}

bool read2(int i2cFD, uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num)
{
    long delayer = 300000000;
    struct timespec ns ={0, delayer};

    // 2 byte 
    uint8_t pos = 0;
    uint8_t prefix[2];
    prefix[0] = (uint8_t)regHigh;
    prefix[1] = (uint8_t)regLow;

    while(pos < num){
        uint8_t read_m;
        if(32 <= (num - pos)){
            read_m = 32;
        } else{
            read_m = (num - pos);
        }

        if(!write(i2cFD, prefix, 2)){
            return false;
        }
        //printf("Prefix 1: %d Prefix 2: %d\n", prefix[0], prefix[1]);
        nanosleep(&ns, NULL);
        //printf("Reading: %d bytes ", read_m);
        if(!read(i2cFD, (buf+pos), read_m)){
            return false;
        }        
        pos += read_m;
        //printf("pos: %d, num: %d\n", pos, num);
    }

    return true;
}

void Moisture_startSampling(){
    pthread_create(&thread_id6, NULL, (void*)Moisture_runMoisture, NULL);
}

void Moisture_stopSampling(void){
    pthread_join(thread_id6, NULL);
}
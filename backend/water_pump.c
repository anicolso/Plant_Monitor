#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "util.h"
#include "water_pump.h"

// export pin 2 AND set direction as out
void initWaterPumpGPIO(void) {
    FILE *pFile = fopen("/sys/class/gpio/export", "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open export file.\n");
        exit(1);
    }
    // Write to data to the file using fprintf():
    fprintf(pFile, "%d", 2);
    // Close the file using fclose():
    fclose(pFile);

    nano_sleep(0, 300);

    FILE *pFileDirection = fopen("/sys/class/gpio/gpio2/direction", "w");
    if (pFileDirection == NULL) {
        printf("ERROR: Unable to open directino file.\n");
        exit(1);
    }
    // Write to data to the file using fprintf():
    fprintf(pFileDirection, "%s", "out");
    // Close the file using fclose():
    fclose(pFileDirection);

    // Call nanosleep() to sleep for ~300ms before use.
}

// turn water pump on
void waterPumpOn() {
    nano_sleep(0, 300);
    FILE *pFile = fopen("/sys/class/gpio/gpio2/value", "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open valueOn file.\n");
        exit(1);
    }
    // Write to data to the file using fprintf():
    fprintf(pFile, "%d", 1);
    // Close the file using fclose():
    fclose(pFile);

    // Call nanosleep() to sleep for ~300ms before use.

}

// turn wawter pump off
void waterPumpOff(void) {
    FILE *pFile = fopen("/sys/class/gpio/gpio2/value", "w");
    if (pFile == NULL) {
        printf("ERROR: Unable to open valueOff file.\n");
        exit(1);
    }
    // Write to data to the file using fprintf():
    fprintf(pFile, "%d", 0);
    // Close the file using fclose():
    fclose(pFile);

    // Call nanosleep() to sleep for ~300ms before use.
}

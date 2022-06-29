#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "sampler.h"
#include "util.h"

#define POT "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

pthread_t tid3;

static bool is_printing = true;


void Printer_outputData(void){
     while(is_printing){
        nano_sleep(10, 0);
        writeToOutput();
     }
}

void Printer_startPrinting(void){
    pthread_create(&tid3, NULL, (void*)Printer_outputData, NULL); 

}

void Printer_stopPrinting(void){
    is_printing = false;
    pthread_join(tid3, NULL);

}
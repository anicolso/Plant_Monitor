#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include "sampler.h"
#include "util.h"

#define POT "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"

static pthread_t tid1;
bool is_sampling = true;

void Pot_Sample(void){
    while(is_sampling){
        // sleep for 1 second
        nano_sleep(1, 0);

        int reading = getVoltage0Reading(POT);

        if(reading == 0){
            reading = 1;
        }

        // if(Sampler_getHistorySize() != reading && ){
        //     Sampler_setHistorySize(reading);
        // }
    }
}

void Pot_StartSampling(void){
    pthread_create(&tid1, NULL, (void*)Pot_Sample, NULL);

}

void Pot_StopSampling(void){
    is_sampling = false;
    pthread_join(tid1, NULL);
}
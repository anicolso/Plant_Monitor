#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "sampler.h"
#include "util.h"
#include "potentiometer.h"
#include "digit_display.h"
#include "udp.h"
#include "terminal_output.h"
#include "bbb_dht_read.h"
#include "bbb_mmio.h"
#include "water_pump.h"
#include "sensorStatus.h"
#include "moisture2.h"

static bool is_running = true;

// Shutdown all threads
void Main_Shutdown(void){
    // Uncomment below to stop humidity/temp sensor
    //humiditySensor_stop();
    //Moisture_stopSampling();

    // Sensor Status Monitoring
    StopSensorStatus();


    // MAIN FUNCTION CALLS
    /*Printer_stopPrinting();
    Digit_shutdown();
    Pot_StopSampling();
    Sampler_stopSampling();
    UDP_StopListening();
    */
    is_running = false;
}

// Startup all threads
int main(void){
    // Uncomment below to start humidity/temp sensor
    // humiditySensor_start();
    // Moisture_startSampling();<

    // Sensor Status Monitoring
    StartSensorStatus();

    // nano_sleep(0, 300);
    initWaterPumpGPIO();
    nano_sleep(0, 300);

    // MAIN FUNCTION CALLS
    UDP_StartListening();
    Sampler_startSampling();
    // Pot_StartSampling();
    // Digit_startup();
    Printer_startPrinting();

    while(is_running){
        // do nothing
    }


    /*
    // Example pump controls, make sure to sleep for 300 ns and to also init first!
    nano_sleep(0, 300);
    initWaterPumpGPIO();
    nano_sleep(0, 300);
    waterPumpOn();
    nano_sleep(5, 0);
    waterPumpOff();
    */

    return 0;
}
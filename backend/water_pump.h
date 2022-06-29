#ifndef _WATERPUMP_H_
#define _WATERPUMP_H_

// initialize the GPIO - export pin and set direction as out
void initWaterPumpGPIO(void);

// turn the water pump on
void waterPumpOn();

// turn the water pump off
void waterPumpOff(void);

#endif 
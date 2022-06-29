#ifndef _SENSORSTATUS_H_
#define _SENSORSTATUS_H_

// Start and Stop status monitoring thread
void StartSensorStatus(void);
void StopSensorStatus(void);

// get the humidity status
bool getHumidityStatus(void);

// get the temperature status
bool getTemperatureStatus(void);

// get the light status
bool getLightStatus(void);

void displayStatus(int status, int value);

// get the moisture status
//bool getMoistureStatus(void);


#endif
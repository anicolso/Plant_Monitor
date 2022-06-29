#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdbool.h>

// get voltage reading for device
int getVoltage0Reading(char* device);
// sleep
void nano_sleep(long seconds, long nanoseconds);
// generic error handling
void error_msg(char* msg, bool condition, char* file);
// write value to file with timestamp
void writeToFile(float value, char* filename);

#endif
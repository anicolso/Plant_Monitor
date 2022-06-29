
#ifndef BBB_DHT_READ_H
#define BBB_DHT_READ_H

#include "common_dht_read.h"

// Read DHT sensor connected to GPIO bin GPIO<base>_<number>, for example P8_11 is GPIO1_13 with
// base = 1 and number = 13.  Humidity and temperature will be returned in the provided parameters.
// If a successfull reading could be made a value of 0 (DHT_SUCCESS) will be returned.  If there
// was an error reading the sensor a negative value will be returned.  Some errors can be ignored
// and retried, specifically DHT_ERROR_TIMEOUT or DHT_ERROR_CHECKSUM.

/***** WHEN RUNNING CALL WITH SUDO ******/
int bbb_dht_read(int type, int gpio_base, int gpio_number, float* humidity, float* temperature);

// start thread 
void humiditySensor_start(void);

// stop thread
void humiditySensor_stop(void);

#endif
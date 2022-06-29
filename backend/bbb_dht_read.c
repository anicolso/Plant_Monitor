// PARTS OF CODE TAKEN FROM: 
//    https://github.com/adafruit/Adafruit_Python_DHT/tree/master/source/Beaglebone_Black
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>

#include "bbb_dht_read.h"
#include "bbb_mmio.h"
#include "util.h"

#define INT_STR_LEN (sizeof(int)*CHAR_BIT/3 + 2)

// Thread initialization
pthread_t thread_id4;


// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
#define DHT_MAXCOUNT 32000

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES 41

int bbb_dht_read(int type, int gpio_base, int gpio_number, float* humidity, float* temperature) {
  // Validate humidity and temperature arguments and set them to zero.
  if (humidity == NULL || temperature == NULL) {
    return DHT_ERROR_ARGUMENT;
  }
  *temperature = 0.0f;
  *humidity = 0.0f;

  // Store the count that each DHT bit pulse is low and high.
  // Make sure array is initialized to start at zero.
  int pulseCounts[DHT_PULSES*2] = {0};

  // Get GPIO pin and set it as an output.
  gpio_t pin;
  if (bbb_mmio_get_gpio(gpio_base, gpio_number, &pin) < 0) {
    return DHT_ERROR_GPIO;
  }
  bbb_mmio_set_output(pin);

  // Bump up process priority and change scheduler to try to try to make process more 'real time'.
  set_max_priority();

  // Set pin high for ~500 milliseconds.
  bbb_mmio_set_high(pin);
  sleep_milliseconds(500);

  // The next calls are timing critical and care should be taken
  // to ensure no unnecssary work is done below.

  // Set pin low for ~20 milliseconds.
  bbb_mmio_set_low(pin);
  busy_wait_milliseconds(20);

  // Set pin as input.
  bbb_mmio_set_input(pin);


  // Wait for DHT to pull pin low.
  uint32_t count = 0;
  while (bbb_mmio_input(pin)) {
    if (++count >= DHT_MAXCOUNT) {
      // Timeout waiting for response.
      set_default_priority();
      return DHT_ERROR_TIMEOUT;
    }
  }

  // Record pulse widths for the expected result bits.
  for (int i=0; i < DHT_PULSES*2; i+=2) {
    // Count how long pin is low and store in pulseCounts[i]
    while (!bbb_mmio_input(pin)) {
      if (++pulseCounts[i] >= DHT_MAXCOUNT) {
        // Timeout waiting for response.
        set_default_priority();
        printf("ERROR 1\n");
        return DHT_ERROR_TIMEOUT;
      }
    }
    // Count how long pin is high and store in pulseCounts[i+1]
    while (bbb_mmio_input(pin)) {
      if (++pulseCounts[i+1] >= DHT_MAXCOUNT) {
        // Timeout waiting for response.
        set_default_priority();
        printf("ERROR 2\n");
        return DHT_ERROR_TIMEOUT;
      }
    }
  }

  // Done with timing critical code, now interpret the results.

  // Drop back to normal priority.
  set_default_priority();

  // Compute the average low pulse width to use as a 50 microsecond reference threshold.
  // Ignore the first two readings because they are a constant 80 microsecond pulse.
  uint32_t threshold = 0;
  for (int i=2; i < DHT_PULSES*2; i+=2) {
    threshold += pulseCounts[i];
  }
  threshold /= DHT_PULSES-1;

  // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
  // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
  // then it must be a ~70us 1 pulse.
  uint8_t data[5] = {0};
  for (int i=3; i < DHT_PULSES*2; i+=2) {
    int index = (i-3)/16;
    data[index] <<= 1;
    if (pulseCounts[i] >= threshold) {
      // One bit for long pulse.
      data[index] |= 1;
    }
    // Else zero bit for short pulse.
  }

  // Useful debug info:
  //printf("Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n", data[0], data[1], data[2], data[3], data[4]);

  // Verify checksum of received data.
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    if (type == DHT11) {
      // Get humidity and temp for DHT11 sensor.
      *humidity = (float)data[0];
      *temperature = (float)data[2];
    }
    else if (type == DHT22) {
      // Calculate humidity and temp for DHT22 sensor.
      *humidity = (data[0] * 256 + data[1]) / 10.0f;
      *temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
      if (data[2] & 0x80) {
        *temperature *= -1.0f;
      }
    }

    char* humidityFile = "/humidity.txt";
    char* temperatureFile = "/temperature.txt";
    writeToFile(*humidity, humidityFile);
    writeToFile(*temperature, temperatureFile);
    
    
    return DHT_SUCCESS;
  }
  else {
    return DHT_ERROR_CHECKSUM;
  }
}


// Continuously read Humidity and Temperature and print it out
void humidityTempSensor(void)
{
	while (1) {
		float humidity = 0; 
		float temperature = 0;
	
		bbb_dht_read(DHT22, 1, 17, &humidity, &temperature);

		printf("HUMIDITY: %f\n", humidity);
		printf("TEMP: %f\n", temperature);

    char* humidityFileName = "/humidity.txt";
    char* temperatureFileName = "/temperature.txt";

    writeToFile(humidity, humidityFileName);
    writeToFile(temperature, temperatureFileName);
	}

}


// start thread 
void humiditySensor_start(void) {
    pthread_create(&thread_id4, NULL, (void*)humidityTempSensor, NULL);
}

// stop thread
void humiditySensor_stop(void) {
    printf("Stop process\n");

    pthread_join(thread_id4, NULL);

    printf("Process stopped\n");

}
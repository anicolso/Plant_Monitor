// Simple fast memory-mapped GPIO library for the Beaglebone Black.
// Allows reading and writing GPIO at very high speeds, up to ~2.6mhz!

// TAKEN FROM: https://github.com/adafruit/Adafruit_Python_DHT/tree/master/source/Beaglebone_Black

/*
  // Example usage:
  #include <stdio.h>
  #include "bbb_mmio.h"
  int main(int argc, char* argv[]) {
    // Get GPIO pin.  
    // See the giant table of of pins in the system reference manual for details 
    // on the base and number for a given GPIO:
    //  https://github.com/CircuitCo/BeagleBone-Black/blob/master/BBB_SRM.pdf?raw=true
    // Section 7 Connectors, table 12 shows P8_11 maps to GPIO1_13, so 1 is the 
    // gpio base and 13 is the gpio number.
    gpio_t p8_11;
    if (bbb_mmio_get_gpio(1, 13, &p8_11) < 0) {
      printf("Couldn't get requested GPIO pin!\n");
      return 1;
    }
    // Set pin as output.
    bbb_mmio_set_output(p8_11);
    // Toggle the pin high and low as fast as possible.
    // This generates a signal at about 2.6mhz in my tests.
    // Each pulse high/low is only about 200 nanoseconds long!
    while (1) {
      bbb_mmio_set_high(p8_11);
      bbb_mmio_set_low(p8_11);
    }
    return 0;
  }
*/

#ifndef BBB_MMIO_H
#define BBB_MMIO_H

#include <stdint.h>

#define MMIO_SUCCESS 0
#define MMIO_ERROR_ARGUMENT -1
#define MMIO_ERROR_DEVMEM -2
#define MMIO_ERROR_MMAP -3

#define MMIO_OE_ADDR 0x134
#define MMIO_GPIO_DATAOUT 0x13C
#define MMIO_GPIO_DATAIN 0x138
#define MMIO_GPIO_CLEARDATAOUT 0x190
#define MMIO_GPIO_SETDATAOUT 0x194 

// Define struct to represent a GPIO pin based on its base memory address and number.
typedef struct {
  volatile uint32_t* base;
  int number;
} gpio_t;

int bbb_mmio_get_gpio(int base, int number, gpio_t* gpio);

static inline void bbb_mmio_set_output(gpio_t gpio) {
  gpio.base[MMIO_OE_ADDR/4] &= (0xFFFFFFFF ^ (1 << gpio.number));
}

static inline void bbb_mmio_set_input(gpio_t gpio) {
  gpio.base[MMIO_OE_ADDR/4] |= (1 << gpio.number);
}

static inline void bbb_mmio_set_high(gpio_t gpio) {
  gpio.base[MMIO_GPIO_SETDATAOUT/4] = 1 << gpio.number;
}

static inline void bbb_mmio_set_low(gpio_t gpio) {
  gpio.base[MMIO_GPIO_CLEARDATAOUT/4] = 1 << gpio.number;
}

static inline uint32_t bbb_mmio_input(gpio_t gpio) {
  return gpio.base[MMIO_GPIO_DATAIN/4] & (1 << gpio.number);
}

#endif
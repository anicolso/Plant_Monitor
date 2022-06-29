
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bbb_mmio.h"

#define GPIO_LENGTH 4096
#define GPIO0_ADDR 0x44E07000
#define GPIO1_ADDR 0x4804C000
#define GPIO2_ADDR 0x481AC000
#define GPIO3_ADDR 0x481AF000

// Store mapping of GPIO base number to GPIO address.
static uint32_t gpio_addresses[4] = { GPIO0_ADDR, GPIO1_ADDR, GPIO2_ADDR, GPIO3_ADDR };

// Cache memory-mapped GPIO addresses.
static volatile uint32_t* gpio_base[4] = { NULL };

int bbb_mmio_get_gpio(int base, int number, gpio_t* gpio) {
  // Validate input parameters.
  if (gpio == NULL) {
    printf("PIN SETUP ERROR 1\n");
    return MMIO_ERROR_ARGUMENT;
  }
  if (base < 0 || base > 3) {
    printf("PIN SETUP ERROR 2\n");
    return MMIO_ERROR_ARGUMENT;
  }
  // Map GPIO memory if its hasn't been mapped already.
  if (gpio_base[base] == NULL) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1) {
      // Error opening /dev/mem.  Probably not running as root.
      printf("PIN SETUP ERROR 3\n");
      return MMIO_ERROR_DEVMEM;
    }
    // Map GPIO memory to location in process space.
    gpio_base[base] = (uint32_t*)mmap(NULL, GPIO_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_addresses[base]);
    if (gpio_base[base] == MAP_FAILED) {
      // Don't save the result if the memory mapping failed.
      gpio_base[base] = NULL;
      printf("PIN SETUP ERROR 4\n");
      return MMIO_ERROR_MMAP;
    }
  }
  // Initialize and set GPIO fields.
  memset(gpio, 0, sizeof(gpio[0]));
  gpio->base = gpio_base[base];
  gpio->number = number;
  return MMIO_SUCCESS;
}
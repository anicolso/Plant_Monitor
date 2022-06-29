CC_C = arm-linux-gnueabihf-gcc
CFLAGS = -Wall -g -std=c99 -pthread -D_DEFAULT_SOURCE -D_BSD_SOURCE -D _POSIX_C_SOURCE=200809L -Werror

all: project

project:
	$(CC_C) $(CFLAGS) main.c \
	sampler.c queue.c util.c potentiometer.c digit_display.c udp.c terminal_output.c \
	water_pump.c bbb_dht_read.c bbb_mmio.c common_dht_read.c sensorStatus.c moisture2.c -o project
	
	cp project $(HOME)/cmpt433/public/myApps/


clean: 
	rm project

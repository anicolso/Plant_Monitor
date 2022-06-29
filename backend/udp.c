#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <netinet/in.h>
#include "main.h"
#include "sampler.h"
#include "bbb_dht_read.h"
#include "water_pump.h"
#include "moisture2.h"
#include "util.h"
#include "sensorStatus.h"


#define MAX_LEN 1024
#define PORT 8088

static pthread_t tid;
static bool is_listening = false;
static struct sockaddr_in sin;

// might need to be in listen func
static struct sockaddr_in sinRemote;
static unsigned int sin_len = sizeof(sinRemote);
static char PREV_COMMAND[MAX_LEN];

int display_num = 0;

float last_hum = 0;
float last_temp = 0;
float last_light = 0;

static char * help_msg[8] = {"Accepted command examples:", 
                            "count    -- display total number of samples taken.", 
                            "length   -- display number of samples in history (both max, and current)",
                            "history  -- display the full sample history being saved ",
                            "get 10   -- display the 10 most recent history values.",
                            "dips     -- display number of.",
                            "stop     -- cause the server program to end.",
                            "<enter>  -- repeat last command."};


void UDP_StopListening(void){
    is_listening = false;
    pthread_join(tid, NULL);
}

int UDP_init(void){
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    int socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr*) &sin, sizeof(sin));

    return socketDescriptor;
}

// Taken from https://stackoverflow.com/questions/13399594/how-to-extract-numbers-from-string-in-c, hmjd's answer.
int find_number(char* s){
    int i1 = -1;
    if (1 == sscanf(s,"%*[^0123456789]%d",&i1))
    {
        // do nothing
    }
    return i1;
}

void reply(char messageRx[], int socketDescriptor){
    char messageTx[MAX_LEN];
    sin_len = sizeof(sinRemote);

    // enter 
    if(strlen(messageRx) == 1){
        memcpy(messageRx, PREV_COMMAND, MAX_LEN*sizeof(char));
    } else {
        // copy command to the prev command array.
        memcpy(PREV_COMMAND, messageRx, MAX_LEN*sizeof(char));
    }


    if(strncmp(messageRx, "help", strlen("help")) == 0){
        // Iterate through the help messages
        for(int i=0; i<8; i++){
            const char * msg = help_msg[i];
            sprintf(messageTx, "%s\n", msg);
            sendto(socketDescriptor, messageTx, strlen(messageTx), 0, (struct sockaddr *) &sinRemote, sin_len);
        }
        return;
    }
    else if(strncmp("light-reading", messageRx, strlen("light-reading")) == 0){
        // get the newest light reading
        int length;
        double * newest = Sampler_getNewestSamples(1, &length);
        sprintf(messageTx, "light-reading %f", newest[0]);

        last_light = newest[0];

        free(newest);
    } else if(strncmp("humidity-temperature-reading", messageRx, strlen("humidity-temperature-reading")) == 0){
        // get the newest humidity and temperature reading
        float humidity = 0;
		float temperature = 0;
	
		bbb_dht_read(DHT22, 1, 17, &humidity, &temperature);

		printf("HUMIDITY: %f\n", humidity);
		printf("TEMP: %f\n", temperature);

        last_hum = humidity;
        last_temp = temperature;
        // send humidity and temperature together
        sprintf(messageTx, "humidity-temperature-reading %f-%f", humidity, temperature);

    } else if(strncmp("moisture-reading", messageRx, strlen("moisture-reading")) == 0){
        // get the newest moisture reading
        int status = Moisture_getMoisture();
        sprintf(messageTx, "moisture-reading %d", status);

    } else if(strncmp("pump-btn", messageRx,strlen("pump-btn")) == 0){
        // turn on the pump for x seconds (TBD)
        printf("TURNING ON PUMP");
        waterPumpOn();

        printf("TURNING OFF PUMP");
        waterPumpOff();

        // send back response so we know pump is done watering
        sprintf(messageTx, "pump-btn");

    } else if(strncmp("pump-on", messageRx,strlen("pump-on")) == 0){
        // turn on the pump for x seconds (TBD)
        printf("TURNING ON PUMP-on\n");
        waterPumpOn();

        // send back response so we know pump is done watering
        sprintf(messageTx, "pump-on");
    } else if(strncmp("pump-off", messageRx,strlen("pump-off")) == 0){
        // turn on the pump for x seconds (TBD)
        printf("TURNING ON PUMP-off\n");
        waterPumpOff();

        // send back response so we know pump is done watering
        sprintf(messageTx, "pump-off");
    }  else if(strncmp("display", messageRx,strlen("pump-off")) == 0){
        // change the var that cycles
        display_num = (display_num + 1) % 3;

        if(display_num == 0){
            if(last_hum <= 0){
                displayStatus(display_num, 0);
            } else {
                displayStatus(display_num, 1);
            }
        } else if(display_num == 1){
            if(last_temp <= 0){
                displayStatus(display_num, 0);
            } else {
                displayStatus(display_num, 1);
            }
        } else if(display_num == 2){
            if(last_light <= 0){
                displayStatus(display_num, 0);
            } else {
                displayStatus(display_num, 1);
            }
        }
        sprintf(messageTx, "display");
    }      
    else {
        sprintf(messageTx, "Unknown command. Type 'help' for command list\n");
    }

    sendto(socketDescriptor, messageTx, strlen(messageTx), 0, (struct sockaddr *) &sinRemote, sin_len);

}

void listening(void){
    // might not need to make this everytime
    int socketDescriptor = UDP_init();
    
    while(is_listening){

        char messageRx[MAX_LEN];

        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, (struct sockaddr *) &sinRemote, &sin_len);

        // NULL terminated (string)
        messageRx[bytesRx] = 0;

        reply(messageRx, socketDescriptor);
    }
    
    close(socketDescriptor);
}


void UDP_StartListening(void){
    is_listening = true;
    pthread_create(&tid, NULL, (void*)listening, NULL);
}
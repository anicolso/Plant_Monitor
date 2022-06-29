#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>

#define INT_STR_LEN (sizeof(int)*CHAR_BIT/3 + 2)

int getVoltage0Reading(char* device)
{
    // Open file
    FILE *f = fopen(device, "r");
    if (!f) {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0) {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }
    // Close file
    fclose(f);
    return a2dReading;
}

void nano_sleep(long seconds, long nanoseconds){
	struct timespec reqDelay = {seconds, nanoseconds};
	nanosleep(&reqDelay, (struct timespec *) NULL);
}

// Handle error msging and exits dynamically
void error_msg(char* msg, bool condition, char* file){
	if (condition) {
		printf(msg, file);
		exit(1);
	}
}

void writeToFile(float value, char* filename){
    const char format[] =  "%d-%d-%d %02d:%02d:%02d";

    time_t t;
    time(&t);
    struct tm * timeinfo = localtime( &t );
    char* time = malloc((INT_STR_LEN*6 + sizeof(format) + 1) * sizeof(char));

    int n = sprintf(time, "%d-%d-%d %02d:%02d:%02d", 1900 + timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    if(n < 0 && n >= sizeof(char*)){
      printf("Failed");
    }

    char cwd[PATH_MAX];

    // create output file
    /* File pointer to hold reference to our file */
    FILE * fPtr;

    strncat(getcwd(cwd, sizeof(cwd)), filename, sizeof(cwd)+(strlen(filename)-1 * sizeof(char)));
    printf("\n%s\n", cwd);

    // might have to run: 'chmod 777 sample.csv' to gain access to this file.
    fPtr = fopen(cwd, "a");

    /* fopen() return NULL if last operation was unsuccessful */
    if(fPtr == NULL)
    {
        /* File not created hence exit */
        perror(cwd);
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(fPtr, "%f, ", value);
    fprintf(fPtr, "%s\n", time); // play around with this to get the proper date format we want


    /* Close file to save file data */
    fclose(fPtr);
}
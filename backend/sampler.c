#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#include "queue.h"
#include <malloc.h>
#include <memory.h>

#include "util.h"

#define POT "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define LIGHT_SENSOR "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define A2D_VOLTAGE_REF_V 1.8
#define A2D_MAX_READING 4095
#define INT_STR_LEN (sizeof(int)*CHAR_BIT/3 + 2)

struct Queue * dip_indexes;

// * Implementation for circular buffer was taken from https://www.equestionanswers.com/c/c-circular-buffer.php * 
typedef struct cbuff_{
    double * buffer;
    char ** times;
    int start;
    int end;
    int size;
    int count;
} cbuff_t;

// Create a new buffer with length 'size'
cbuff_t * cbuff_new(int size)
{
  cbuff_t * cb = (cbuff_t*)malloc(sizeof(cbuff_t));
  memset(cb, 0, sizeof(cbuff_t));
  cb->size = size;
  cb->buffer = (double*)malloc(sizeof(double)*size);

  const char format[] =  "%d-%d-%d %02d:%02d:%02d";
  cb->times = (char**)malloc(size * sizeof(char*));
  for (int i = 0; i < size; i++){
    cb->times[i] = malloc((INT_STR_LEN*6 + sizeof(format) + 1) * sizeof(char)); // yeah, I know sizeof(char) is 1, but to make it clear...
  }
  return cb;
}

// Add element to buffer cb
void cbuff_add(cbuff_t * cb, double elem, char* time_elem)
{
  time_t t;
  time(&t);
  struct tm * timeinfo = localtime( &t );

  int end = cb->end;
  if(cb->count && (end % cb->size) == cb->start){
    cb->start = (cb->start + 1 ) %cb->size;
    cb->count --;
  }
  cb->buffer[cb->end] = elem;

  if(time_elem == NULL){
    printf("Adding current time\n");
    int n = sprintf(cb->times[cb->end], "%d-%d-%d %02d:%02d:%02d", 1900 + timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    if(n < 0 && n >= sizeof(char*)){
      printf("Failed");
    }
  } else {
    printf("%s\n", time_elem);
    cb->times[cb->end] = time_elem;
  }
  cb->end = (cb->end+1) % cb->size;
  cb->count ++;

}

// copt items from one buffer to another
void cbuff_copy(cbuff_t * cb, cbuff_t* copy)
{
  int start = cb->start ;
  int end = cb->end ;
  int i, count = 0;
  for(i = start; count < cb->count; i = (i + 1)%cb->size){
    cbuff_add(copy, cb->buffer[i], cb->times[i]);
    count++;
    if(i == (end - 1)) {
      break;
    }
  }
}

// Free up memory from buffer
void cbuff_delete(cbuff_t * cb)
{
  free(cb->buffer);
  free(cb->times);
  free(cb);
}

static int HISTORY_SIZE = 1000;
static bool is_sampling = false;
static long long TOTAL_LIGHT_SAMPLES = 0;
static double weighted_avg = 0;
static int num_dips = 0;
static bool ready_for_detecting_dips = true;
static double last_dip_value = 0.0;

// buffer for samples
static cbuff_t* circular_buff;

pthread_t tid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

double Sampler_getWeightedAvg(void){
    return weighted_avg;
}

long long Sampler_getNumSamplesTaken(void){
    return TOTAL_LIGHT_SAMPLES;
}

int Sampler_getNumSamplesInHistory(void){
    return circular_buff->count;
}

int Sampler_getNumDips(void){
    return num_dips;
}


void detectDip(double voltage){
    if(front(dip_indexes) == circular_buff->end){
        dequeue(dip_indexes);
        num_dips --;
    }

    // use weighted avg to detect a dip
    if(ready_for_detecting_dips){
        // detected dip
        if(weighted_avg - voltage >= 0.1){
            ready_for_detecting_dips = false;
            last_dip_value = weighted_avg;
            num_dips ++;
        }
    } else {
        // dip has come back up to normal
        if(voltage - last_dip_value >= 0.03){
            ready_for_detecting_dips = true;
            enqueue(dip_indexes, circular_buff->end); // append end of dip to queue
        }
    }

}

void Sample(void){
    while(is_sampling){

        // higher V is more Light
        int reading = getVoltage0Reading(LIGHT_SENSOR);
        double voltage = ((double)reading / A2D_MAX_READING) * A2D_VOLTAGE_REF_V;

        pthread_mutex_lock(&mutex);
        {

            // write to history buffer
            cbuff_add(circular_buff, voltage, NULL);
            TOTAL_LIGHT_SAMPLES ++;

            // Look for dip here
            detectDip(voltage);

        }
        pthread_mutex_unlock(&mutex);
        
        // Calculate weighted avg
        if(weighted_avg == 0){
            weighted_avg = voltage;
        } else {
            weighted_avg = ((weighted_avg * 99.9) + (voltage * 0.1)) / 100;
        }

        nano_sleep(1, 0); // sleep for 1ms
    }
}

void Sampler_setHistorySize(int newSize){
    int num_samples = Sampler_getNumSamplesInHistory();

    cbuff_t* tempBuff = cbuff_new(newSize);
    
    pthread_mutex_lock(&mutex);
    {

        // Moving to a smaller buffer
        if(num_samples > newSize){
            int starting_index = circular_buff->end - newSize;

            // check if buffer.count == prev HISTORY_SIZE to see where we will start from. Head or next pointer.
            if(starting_index < 0){
                starting_index = HISTORY_SIZE - (starting_index * -1);
            }

            struct Queue * temp = createQueue(A2D_MAX_READING);

            // move pointer to the Nth newest element
            for(int i=starting_index; i<starting_index + newSize; i++){
                cbuff_add(tempBuff, circular_buff->buffer[i%circular_buff->size], circular_buff->times[i%circular_buff->size]);

            }

            while(!isEmpty(dip_indexes)){
                int dip = dequeue(dip_indexes);
                // only add dips that can be in the new history
                int offset = dip - (num_samples - newSize);
                if(dip - (num_samples - newSize) >= 0){
                    enqueue(temp, offset);
                } else {
                    num_dips --;
                }
            }

            // move everything back into original queue
            while(!isEmpty(temp)){
                enqueue(dip_indexes, dequeue(temp));
            }

            delete_queue(temp);

        } else {
            // going to a bigger buffer
            for(int i=0; i<num_samples; i++){
                cbuff_add(tempBuff, circular_buff->buffer[i], circular_buff->times[i]);
            }
        }

        cbuff_delete(circular_buff);
        circular_buff = cbuff_new(newSize);

        // Add all newest elements back into original buffer
        for(int i=0; i<tempBuff->count; i++){
            cbuff_add(circular_buff, tempBuff->buffer[i%circular_buff->size], circular_buff->times[i%circular_buff->size]);
        }

        cbuff_delete(tempBuff);

    }
    pthread_mutex_unlock(&mutex);

    HISTORY_SIZE = newSize;

}

int Sampler_getHistorySize(void){
    return HISTORY_SIZE;
}

void Sampler_startSampling(void){
    HISTORY_SIZE = getVoltage0Reading(POT); // get starting POT value for history
    if(HISTORY_SIZE == 0){
        HISTORY_SIZE = 1;
    }
    circular_buff = cbuff_new(HISTORY_SIZE);
    dip_indexes = createQueue(A2D_MAX_READING);
    is_sampling = true;
    pthread_create(&tid, NULL, (void*)Sample, NULL);
}

void Sampler_stopSampling(void){
    is_sampling = false;
    pthread_join(tid, NULL);

    cbuff_delete(circular_buff);
    delete_queue(dip_indexes);
}

double* Sampler_getHistory(int *length){
    int num_samples = Sampler_getNumSamplesInHistory();

    double * history = (double*)malloc(num_samples * sizeof(double));
    memcpy(history, circular_buff->buffer, num_samples * sizeof(double));

    *length = num_samples;

    return history;
}

double* Sampler_getNewestSamples(int n, int *length){
    int num_samples = Sampler_getNumSamplesInHistory();
    double * history = (double*)malloc(n * sizeof(double));

    if(num_samples > n){
        int starting_index = circular_buff->end - n;

        // check if buffer.count == prev HISTORY_SIZE to see where we will start from. Head or next pointer.
        if(starting_index < 0){
            starting_index = HISTORY_SIZE - (starting_index * -1);
        }

        int idx = 0;
        // move pointer to the Nth newest element
        for(int i=starting_index; i<starting_index + n; i++){
            history[idx] = circular_buff->buffer[i%circular_buff->size];
            idx ++;
        }

        *length = n;

    } else {
        // going to a bigger buffer
        for(int i=0; i<num_samples; i++){
            history[i] = circular_buff->buffer[i];
        }

        *length = num_samples;
    }    

    return history;
}

char** Sampler_getNewestTimes(int n, int *length){
    int num_samples = Sampler_getNumSamplesInHistory();
    
    const char format[] =  "%d-%d-%d %02d:%02d:%02d";
    char ** history = (char**)malloc(num_samples * sizeof(char*));
    for (int i = 0; i < num_samples; i++){
      history[i] = malloc((INT_STR_LEN*6 + sizeof(format) + 1) * sizeof(char)); // yeah, I know sizeof(char) is 1, but to make it clear...
    }

    if(num_samples > n){
        int starting_index = circular_buff->end - n;

        // check if buffer.count == prev HISTORY_SIZE to see where we will start from. Head or next pointer.
        if(starting_index < 0){
            starting_index = HISTORY_SIZE - (starting_index * -1);
        }

        int idx = 0;
        // move pointer to the Nth newest element
        for(int i=starting_index; i<starting_index + n; i++){
            history[idx] = circular_buff->times[i%circular_buff->size];
            idx ++;
        }

        *length = n;

    } else {
        // going to a bigger buffer
        for(int i=0; i<num_samples; i++){
            history[i] = circular_buff->times[i];
        }

        *length = num_samples;
    }    

    return history;
}

void writeToOutput(void){
    // get all newest from history
    int length;
    int time_length;
    double * history = Sampler_getNewestSamples(HISTORY_SIZE, &length);
    char ** times = Sampler_getNewestTimes(HISTORY_SIZE, &time_length);

    // char* data[HISTORY_SIZE];
    char cwd[PATH_MAX];

    // create output file
     /* File pointer to hold reference to our file */
    FILE * fPtr;

    /* 
     * Open file in w (write) mode. 
     * "data/file1.txt" is complete path to create file
     */
    char* outputFile = "light.txt";

    strncat(getcwd(cwd, sizeof(cwd)), "/light.txt", sizeof(cwd)+(strlen(outputFile) * sizeof(char)));
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


    // /* Input contents from user to store in file */
    // printf("Enter contents to store in file : \n");
    // fgets(history, sizeof(double)*length, stdin);

    for(int i=0; i<length; i++){

        //   int tm_mday;			/* Day.		[1-31] */
        //   int tm_mon;			/* Month.	[0-11] */
        //   int tm_year;			/* Year	- 1900.  */

        fprintf(fPtr, "%f, ", history[i]);
        fprintf(fPtr, "%s\n", times[i]); // play around with this to get the proper date format we want

    }


    /* Close file to save file data */
    fclose(fPtr);


    // free data
    free(history);
    free(times);

    cbuff_delete(circular_buff);
    circular_buff = cbuff_new(HISTORY_SIZE);

}
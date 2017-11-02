#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "interface.h"


#define Ki 800.0
#define Kp 10.0
#define REFERENCE 1.0
#define PERIOD_S 0.002
#define PERIOD_MS PERIOD_S*1000
#define PERIOD_US PERIOD_MS*1000
#define RUNNING_TIME 0.5
#define I_COUNT (RUNNING_TIME / PERIOD_S)


volatile int RUN_THREADS = 1;
volatile double y;

struct udp_conn interfaceConnection;

pthread_mutex_t locker;
sem_t receive_y;
sem_t receive_signal;


void sender(char *);
void* receiver( void *);
void* controller( void *);
void* responder( void* a );


//--- THREAD FONCTIONS --------------------------------

void sender(char * data) {	
	pthread_mutex_lock(&locker);
	udp_send(&interfaceConnection, data , strlen(data)+1);
	pthread_mutex_unlock(&locker);
}


void* controller(void* parameters) {
	struct timespec period,end;
	char control_buffer[128];
	double error,integral,u,len;


	sender("START");

	clock_gettime(CLOCK_REALTIME, &period);
	clock_gettime(CLOCK_REALTIME, &end);
	timespec_add_us(&end, 500 * 1000);

	int i=0;
	while (i < I_COUNT){
		sender("GET");
		sem_wait(&receive_y);
		//--- Control ---
		error = REFERENCE - y; 
		integral += error * PERIOD_S;
		u = Kp * error + Ki * integral;

		len = sprintf(control_buffer,"SET:%f",u);
		sender(control_buffer);

		timespec_add_us(&period, PERIOD_US);
		clock_nanosleep(&period);
		i++;
	}	
	return parameters; 
} 

void* receiver(void* parameters){
	char receive_buffer[128];
	while(1){	
		if (udp_receive(&interfaceConnection, receive_buffer, 128) > 0){	
			if(strncmp(receive_buffer,"GET_ACK",7) == 0){
				y = atof( &receive_buffer[8] );
				sem_post(&receive_y);
			} else if (strncmp(receive_buffer,"SIGNAL",6) == 0){
				sem_post(&receive_signal);
			}	
		} 
		if (RUN_THREADS == 0) break;
	}
	
	return parameters;
}

// PARTIE 2

void* responder(void* parameters)
{
	char answer[11] = "SIGNAL_ACK";
	while (1)
	{
		sem_wait(&receive_signal);
		sender(answer);
		if (RUN_THREADS == 0) break;
	}
	return parameters;
} 

// --- MAIN ---------------------------------------------

int main (void) {	

	udp_init_client(&interfaceConnection, 9999, "192.168.0.1");

	pthread_t preceiver;
	pthread_create(&preceiver, NULL, receiver, NULL);
	sem_init(&receive_y,1,0);

	pthread_t pcontroller;
	pthread_create(&pcontroller, NULL, controller, NULL);
	sem_init(&receive_signal,1,0);

	pthread_t presponder;
	pthread_create(&presponder, NULL, responder, NULL);
	
	pthread_join(pcontroller,NULL);


	sender("STOP");


	udp_close(&interfaceConnection);

	return 0;
}
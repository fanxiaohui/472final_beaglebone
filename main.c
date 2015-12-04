//main.c

/////////// This should definitely be cleaned up :)
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <time.h>
#include <stropts.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <sys/signalfd.h>
#include <string.h>

#define NUMPOLL 2

//ADC VALUES
#define ADC_FRONT_THR
#define ADC_LEFT_THR
#define ADC_RIGHT_THR
#define ADC_BACK_THR


//Directions
#define OFF 0
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3

//MOTORS
#define M_RIGHT 0
#define M_LEFT 1

void setMotor(int whichMotor, int direction, int percent);
// void makeMotorDecision(int* );


void cmdCarFwdFull(void){
		printf("Car go!\n");
		setMotor(M_RIGHT, FORWARD, 100);
		setMotor(M_LEFT, FORWARD, 100);
}

void cmdCarStop(void){
		printf("Car stop!\n");
		setMotor(M_RIGHT, OFF, 100);
		setMotor(M_LEFT, OFF, 100);
}

void cmdCarLeftFull(void){
		setMotor(M_RIGHT, FORWARD, 100);
		setMotor(M_LEFT, BACKWARD, 100);
}

void cmdCarLeftFwd(void){
		setMotor(M_RIGHT, FORWARD, 100);
		setMotor(M_LEFT, FORWARD, 20);
}

void cmdCarRightFull(void){
		setMotor(M_RIGHT, BACKWARD, 100);
		setMotor(M_LEFT, FORWARD, 100);
}

void cmdCarRightFwd(void){
		setMotor(M_RIGHT, FORWARD, 20);
		setMotor(M_LEFT, FORWARD, 100);
}

//0 : normal
//1 : front obstacle
//2 : left obstacle
//3 : right obstacle
//state machine for car behavior, coupled with appropriate actions for entering each state
typedef struct CarState {
	int state;
	int oldstate;
	void (* actions[2])(void);
} CarState;

//initialize state to full forward, create action pointer values
void initCarState(CarState* state){
	state->state = 0;
	state->actions[0] = cmdCarFwdFull;
	state->actions[1] = cmdCarRightFull;
    state->actions[state->state]();
}

//where behavior changing is written
void updateCarState(CarState* state, int adcVals[4]){
	if(adcVals[2] > 1600){
		state->state = 1;
	} else{
		state->state = 0;
	}
}

void refreshCarState(CarState* state){
	state->oldstate = state->state;
}

int main() {
	printf("Control Program\n");

	pid_t pid = fork(); 
	if (pid < 0) 
		printf("Couldn't launch process\n");
	else if (pid == 0) {
	//MAIN PROGRAM

	/* Interesting Links: https://gabrbedd.wordpress.com/2013/07/29/handling-signals-with-signalfd/ <- use signalfd for kernel module?
	*/

	struct pollfd pfd[NUMPOLL];
	int ret;
	ssize_t bytes;
	int fd, fd2;
	char* tok;
	int motorflag;

	

	fd = open("/tmp/adcData", O_RDONLY);
	fd2 = open("/dev/ib", O_RDONLY); //fd2

   //FILE* fp2 = fopen("/dev/ib", "r");
   //fd2 = fileno(fp2);

	//pfd[0].events = POLLIN | POLLERR | POLLHUP;



	pfd[0].fd = fd;	
	pfd[0].events = POLLIN;
	pfd[1].fd = fd2;
	pfd[1].events = POLLIN;

	char databuf[1024] = "";

	int adcVals[4];
	//[0] back
	//[1] left
	//[2] front
	//	2000 : 7 cm
	//[3] right

    // makeMotorDecision(adcVals);
    CarState cs;
    initCarState(&cs);

	while(1) {
		ret = poll(pfd, NUMPOLL, -1);
      if(ret > 0) { //Something happened
         if (pfd[0].revents & POLLIN) {
            printf("ADC stored:\n");
            read(fd, databuf, 1024);
            tok = strtok(databuf, ",\n");
            int i = 0;
            while(i < 4 && tok != NULL) 
            {
               adcVals[i] = atoi(tok);
               printf(" %d\n", adcVals[i]);
               tok = strtok(NULL, ",\n");
               i++;
            }

            updateCarState(&cs, adcVals);
			if(cs.state != cs.oldstate){
				printf("new state: %d\n", cs.state);
				cs.actions[cs.state]();
			}
            refreshCarState(&cs);


         }
         if (pfd[1].revents & POLLIN) {
            read(fd2, databuf, 2);
            printf("Button: %s\n", databuf);
         }
      }
	}

	//END MAIN PROGRAM
	} else {
		pid_t pid2 = fork(); 
		if (pid2 < 0) 
			printf("Couldn't launch process\n");
		else if (pid2 == 0) {
			system ("./adc");
			printf("ADC\n");
		} else {
			system ("./motor");
			printf("MOTOR\n");
		}
	}
}

//decision: do we write makeMotorDecision so so that setMotor is called infrequently, or 
void makeMotorDecision(int adcVals[4], CarState* cs) {
	// setMotor(M_RIGHT, FORWARD, 100);
		// setMotor(M_RIGHT, OFF, 100);
		// setMotor(M_LEFT, OFF, 100);
	// if(cs->state != cs->oldstate){
	// 	printf("new state: %d\n", cs->state);
	// 	// cs->actions[cs->state];
	// }
	// setMotor(M_LEFT, FORWARD, 100);
	// // setMotor(M_RIGHT, OFF, 100);
	// setMotor(M_LEFT, OFF, 100);
}

void setMotor(int whichMotor, int direction, int percent) {
	//printf("setmoto\n");
	FILE* motorOut = fopen("/tmp/motorData", "w");
	fprintf(motorOut, "%d,%d,%d\n", whichMotor, direction, percent);
	printf("To Motor: %d,%d,%d\n", whichMotor, direction, percent);
	fflush(motorOut);
	fclose(motorOut);
}
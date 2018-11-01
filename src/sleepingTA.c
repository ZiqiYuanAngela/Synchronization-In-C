/*
 ============================================================================
 Name        : sleepingTA.c
 Author      : Ziqi_Yuan
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_SLEEP_TIME 3
#define NUM_OF_STUDENTS 5
#define NUM_OF_TAS 2
#define NUM_OF_HELPS 2
#define NUM_OF_SEATS 2

pthread_mutex_t mutex_lock;//Declare global variable mutex_lock,initialize in main()

int waiting_students=0;//Declare and initialize waiting_students to 0
sem_t students_sem;//Declare semaphore for students, initialize in main()
sem_t ta_sem;//Declare semaphore for students, initialize in main()

pthread_t tAthreads[NUM_OF_TAS];//Declare threads for TA
pthread_t studentT[NUM_OF_STUDENTS];//Declare threads for students

typedef struct tracker{//Declare struct for each TA thread and each Student thread
	int help_number;
	int id;

}tracker;

struct tracker *data[NUM_OF_STUDENTS];//Array of struct of type tracker for Student threads
struct tracker *data2[NUM_OF_TAS];//Array of struct of type tracker for TA threads



void *tAHelp(void *params){
	tracker *pointer=(tracker *)params;
	unsigned int seed=1;
	int rand;
	int id=pointer->id;
	while(1){//keeps helping any more students
		rand=(rand_r(&seed)%MAX_SLEEP_TIME)+1;
		sem_wait(&students_sem);//waits for student arrival
		pthread_mutex_lock(&mutex_lock);//lock waiting_students to decrement
        if(waiting_students>0){
			--waiting_students;//decrement by one
			pointer->help_number++;
			printf("TA[%d,%d]: helps a student for %d seconds,# of waiting students= %d\n",id,pointer->help_number,rand
    			,waiting_students);//print statement
			fflush(NULL);
			pthread_mutex_unlock(&mutex_lock);//unlock waiting_students for open access
			sem_post(&ta_sem);//signal the availability of a TA thread
			sleep(rand);//helping student in progress

	}
		else{pthread_mutex_unlock(&mutex_lock); sem_post(&ta_sem);}//Free the lock

	}
}

void *studentForHelp(void *params){
     tracker *pointer=(tracker *)params;
     unsigned int seed=1;
     int id=pointer->id;
     int rand=(rand_r(&seed)%MAX_SLEEP_TIME)+1;
     sleep(rand);//Students start working on programming
     printf("student[%d,%d]:programs for %d seconds\n",id,pointer->help_number,rand);
     fflush(NULL);
     while(pointer->help_number<NUM_OF_HELPS){//students keep asking for help unless help_number>=2
    	 pthread_mutex_lock(&mutex_lock);//Lock waiting_students for access and modification
    	 if(waiting_students<NUM_OF_SEATS){//check if wait list is full

    		 ++waiting_students;//if not full,take students into wait list
    		 printf("student[%d,%d]: takes a seat, # of waiting students = %d\n",id,pointer->help_number,
						waiting_students);//print statement
    		 fflush(NULL);
    		 sem_post(&students_sem);//signal the student arrival
             pthread_mutex_unlock(&mutex_lock);//unlock waiting_students
             sem_wait(&ta_sem);//wait for available TAs
    		 pointer->help_number++;
    		 printf("student[%d,%d]:receives help\n",id,pointer->help_number);
    		 fflush(NULL);

    		 if(pointer->help_number<NUM_OF_HELPS){//if more help is allowed, continue programming
    			 rand=(rand_r(&seed)%MAX_SLEEP_TIME)+1;

    			 printf("student[%d,%d]:programs for %d seconds\n",id,pointer->help_number,rand);
    			 fflush(NULL);
    			sleep(rand);//continue programming
        }

	}
    	 else{
    		 pthread_mutex_unlock(&mutex_lock);//unlock waiting_students and let the student try later
    		 printf("student[%d,%d]: will try later\n",id,pointer->help_number);
    		 fflush(NULL);
    		 rand=(rand_r(&seed)%MAX_SLEEP_TIME)+1;
    		 printf("student[%d,%d]: programs for %d seconds\n",id,pointer->help_number,rand);
    		 fflush(NULL);
    		 sleep(rand);//let students program by itself


	}           }
	pthread_exit(NULL);//exit the thread once all maximum help(2) for each student is reached

}

int main(void) {

	printf("CS149 SleepingTA from Ziqi Yuan\n");
	fflush(NULL);

	pthread_mutex_init(&mutex_lock, NULL);
	sem_init(&students_sem,1,0);//initialize the Student semaphore to 0
	sem_init(&ta_sem,1,0);//initialize the TA semaphore to 0
	for (int a=0;a<NUM_OF_STUDENTS;a++){//allocate memory for struc
			data[a]=(tracker *)malloc(sizeof(tracker));
			data[a]->help_number=0;
	        data[a]->id=a;}
	for(int d=0;d<NUM_OF_TAS;d++){//allocate memory for struc
		data2[d]=(tracker *)malloc(sizeof(tracker));
		data2[d]->help_number=0;
		data2[d]->id=d;
	}


	for(int i=0;i<NUM_OF_TAS;i++){//Create TA threads
         pthread_create(&tAthreads[i],NULL,tAHelp,data2[i]);}

	for(int j=0;j<NUM_OF_STUDENTS;j++){//Create Student threads
         pthread_create(&studentT[j],NULL,studentForHelp,data[j]);
	}

	  for(int h=0;h<NUM_OF_STUDENTS;h++){//Wait for all Student threads to finish and join
		    	 pthread_join(studentT[h],NULL);
		 }
	  for(int k=0;k<NUM_OF_TAS;k++){//Cancel all TA threads
		  pthread_cancel(tAthreads[k]);
	  }


	  //Destroy mutex_lock && semaphores before termination
	  pthread_mutex_destroy(&mutex_lock);
	  sem_destroy(&students_sem);
	  sem_destroy(&ta_sem);

	  printf("main:Done!\n");
	  fflush(NULL);

	  //free allocation for data[] & data2[]
	  	  for(int c=0;c<NUM_OF_STUDENTS;c++){
	  		  free(data[c]);
	  	  }
	  	  for(int e=0;e<NUM_OF_TAS;e++){
	  		  free(data2[e]);
	  	  }

	return EXIT_SUCCESS;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>


#define NUM_ASTRONOMERS 10
int AVG_EAT_TIME; 
int MAX_WAIT_TIME;

int ordering[NUM_ASTRONOMERS];  // 0 means symmetric, 1 means asymmetric
int NUM_ASYMMETRIC; // Number of asymmetric astronomers
// In case of using monitor, you can define it here and manipulate 
// the code accordingly.
// typedef struct {
    // ...
   
// Only one process can be active in a Monitor
// A process can pick up a chopstick only if both are available
// A process can only have a state=EATING if both neighbours are not eating and current philosopher.state == HUNGRY

// mutex is to protect critical section and next is used to make processes wait in a queue
sem_t mutex, next;
// Count number of suspended processes
int next_count = 0;

// States that a philosopher can be in
enum {THINKING, HUNGRY, EATING} state[NUM_ASTRONOMERS];

// Used to delay a process if the resources are not available
// A process will wait() until some other process invokes signal()
// signal() resumes 1 suspended process -> If no process is waiting -> nothing will happen
// count - To determine processes waiting for self.sem


typedef struct{
    sem_t sem;
    int count;
} condition;
condition self[NUM_ASTRONOMERS];

//Astronomer type buffer
char type[NUM_ASTRONOMERS][50];

//Chopstick buffer
char chopsticks[NUM_ASTRONOMERS][50];


     

void pickup(int i){
    // Secure Critical Section
    sem_wait(&mutex);

    // Set state to Hungry (Process is ready to perform a task)
    state[i] = HUNGRY;

        // Print states of Astronomers
        printf("\n%s Astronomer %i is hungry\n", type[i], i);
        printf("\nStates: ");
     for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %d ", state[m]);
}

    test(i);

    // If the process could not eat
    if (state[i] != EATING){
            printf("\n%s Astronomer %i could't pick up left chopstick %i and right chopstick %i in a preferred way\n",type[i], i, i, (i+1)%NUM_ASTRONOMERS);
        wait(i);
    }
   

    // If processes are waiting to enter the monitor
    if (next_count > 0){
        // Signal the next process to come into the monitor
        sem_post(&next);
    }
    else{
        // Change mutex to original state
        sem_post(&mutex);
    }
}

void putdown(int i){
    // Secure Critical Section
    sem_wait(&mutex);

        // Print states of chopsticks
        printf("\n%s Astronomer %i is done eating\n", type[i], i);
     strcpy(chopsticks[(i+1)%NUM_ASTRONOMERS]," down ");
    strcpy(chopsticks[i]," down ");

    // Set philosopher back to resting state (THINKING)
	state[i] = THINKING;

        // Print states of astronomers.
    printf("\nStates: ");
     for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %d ", state[m]);
}


    // Check neighbours
	test((i + 1) % NUM_ASTRONOMERS);
	test((i + NUM_ASTRONOMERS-1) % NUM_ASTRONOMERS);

    // Change mutex to original state
	sem_post(&mutex);
}


void test(int i){

    //Assymetric
    if (ordering[i] == 1){

        // Hungry and right chopstick avavilable?
        if ((state[i] == HUNGRY) && (state[(i+1)%NUM_ASTRONOMERS] != EATING)){
        printf("\n%s Astronomer %d picked up right chopstick %i", type[i], i, (i+1)%NUM_ASTRONOMERS);
        strcpy(chopsticks[(i+1)%NUM_ASTRONOMERS]," up ");

            // Left chopstick avavailable? 
        if ((state[i] == HUNGRY) && (state[(i+NUM_ASTRONOMERS-1)%NUM_ASTRONOMERS] != EATING)){
        printf("\n%s Astronomer %d picked up left chopstick %i\n", type[i], i, i);
        strcpy(chopsticks[i]," up ");
                      // Print states of chopsticks
              printf("\nChopsticks: ");
        for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %s ", chopsticks[m]);
}

// If left chopstick available, eat
        state[i] = EATING;
         eat(i);

// Does not do anything during pickup()
        signal(i);
        }



// If left chopstick not immediately available
        else {
             printf("\n%s Astronomer %d is waiting to pick up left chopstick %i", type[i], i, i);
             MAX_WAIT_TIME = rand() % (2 + 1 - 0) + 0;
            sleep(MAX_WAIT_TIME);
            
            // If left is available after wait, eat
            if ((state[i] == HUNGRY) && (state[(i+NUM_ASTRONOMERS-1)%NUM_ASTRONOMERS] != EATING)){
              printf("\n%s Astronomer %d picked up left chopstick %i\n", type[i], i, i);
                strcpy(chopsticks[i], " up ");
        
        // Print states of chopsticks
        printf("\nChopsticks: ");
 for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %s ", chopsticks[m]);
}
        state[i] = EATING;
         eat(i);
// Does not do anything during pickup()
        signal(i);
        }


            // Drop the right chopstick since left never became available
        else {
              strcpy(chopsticks[(i+1)%NUM_ASTRONOMERS]," down ");
        } 

    }
        }
    }


//Symmetric
else {

    // Check if neighbours are NOT EATING and if current philosopher is HUNGRY
    if ((state[(i+NUM_ASTRONOMERS-1)%NUM_ASTRONOMERS] != EATING) && (state[i] == HUNGRY) && (state[(i+1)%NUM_ASTRONOMERS] != EATING)){
          printf("\n%s Astronomer %d picked up both chopsticks %i and %i\n", type[i], i, i, (i+1)%NUM_ASTRONOMERS);
             strcpy(chopsticks[(i+1)%NUM_ASTRONOMERS]," up ");
            strcpy(chopsticks[i]," up ");

              // Print states of astronomers
       printf("\nChopsticks: ");
 for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %s ", chopsticks[m]);
}
        // Set state of current astronomer to EATING
        state[i] = EATING;
         eat(i);

        // Does not do anything during pickup()
        signal(i);
    }
}
}

void wait(int i){
    // Process is now waiting
    self[i].count++;
       printf("\n%s Astronomer %d dropped any chopsticks and is WAITING in line\n", type[i], i);
                    
     // Print states of chopsticks
       printf("\nChopsticks: ");
        for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %s ", chopsticks[m]);
}

MAX_WAIT_TIME = rand() % (2 + 1 - 0) + 0;
            sleep(MAX_WAIT_TIME);

    // If processes are already waiting
    if (next_count > 0){
        // Signal to the next waiting process to enter the monitor
        sem_post(&next);

    }
    else{
        // If no process is waiting, set the mutex to original state
        sem_post(&mutex);
    }

    // Set the semaphore and count back to original state
    sem_wait(&self[i].sem);
    self[i].count--;

}

void signal(int i){

    // If the philosopher is waiting
    if (self[i].count > 0){
        next_count++;
                     
        // Signal the philosophers semaphore
       printf("\n%s Astronomer %d is SIGNALING to the other processes in line who were waiting", type[i], i);

        sem_post(&self[i].sem);
        sem_wait(&next);
        next_count--;
             

    }
}

void initialisation(){
    // Initialize semaphores and philosopher states
    sem_init(&mutex, 0, 1);
    sem_init(&next, 0, 0);

    for (int i=0; i<NUM_ASTRONOMERS; i++){
        state[i] = THINKING;
        sem_init(&self[i].sem, 0, 0);
        self[i].count = 0;
    }
}


void think(int i) {
    printf("\n%s Astronomer %d is THINKING for now\n", type[i], i);
    sleep(2);
    }

void eat(int i) {
      printf("\n%s Astronomer %d is EATING for now\n", type[i], i);
    
  // Print states of astronomers
    printf("\nStates: ");
     for (int m= 0; m < NUM_ASTRONOMERS;m++){
    printf(" %d ", state[m]);
}

   AVG_EAT_TIME = rand() % (2 + 1 - 0) + 0;
    sleep(AVG_EAT_TIME);
    
    }
  


// template for the philosopher thread
// it is ran by each thread
void* philosopher(void* num) {
 while (1) {

        int* i = num;
        think(*i);
        pickup(*i);
        putdown(*i);
        }


}


// a function to generate random ordering of 0 and 1
// 0 means symmetric, 1 means asymmetric
void place_astronomers(int *ordering){
    // generate random ordering
    // 0 means symmetric, 1 means asymmetric

    NUM_ASYMMETRIC = rand() % ((NUM_ASTRONOMERS - 1) + 1 - 3) + 3;

    for (int i = 0; i < NUM_ASYMMETRIC; i++) {
        ordering[i] = 1;
    }
    for (int i = NUM_ASYMMETRIC; i < NUM_ASTRONOMERS; i++) {
        ordering[i] = 0;
    }
    // shuffle the ordering
    for (int i = 0; i < NUM_ASTRONOMERS; i++) {
        int j = rand() % NUM_ASTRONOMERS;
        int temp = ordering[i];
        ordering[i] = ordering[j];
        ordering[j] = temp;
    }

     for (int i = 0; i < NUM_ASTRONOMERS; i++) {
    if (ordering[i] == 1){
strcpy(type[i], "Asymmetric");
      }
      else {
strcpy(type[i], "Symmetric");
      }
     }
}


int main(){

    // Initialize philosophers
        int phil_ids[NUM_ASTRONOMERS];
     pthread_t philosophers[NUM_ASTRONOMERS];

     sem_init(&mutex, 0, 1);
    sem_init(&next, 0, 0);

    for (int i=0; i<NUM_ASTRONOMERS; i++){
        state[i] = THINKING;
        sem_init(&self[i].sem, 0, 0);
        self[i].count = 0;
                                   strcpy(chopsticks[i], " down ");

    }
        

    // Start philosophers

     place_astronomers(&ordering);

    for (int i = 0; i < NUM_ASTRONOMERS; i++){
        // To prevent race conditions
        phil_ids[i] = i;

        // Create Philosopher Processes
        pthread_create(&philosophers[i], NULL, philosopher, &phil_ids[i]);
    }

    

    // call join on philosophers
    for (int i = 0; i < NUM_ASTRONOMERS; i++){
        pthread_join(philosophers[i], NULL);
    }


    return 0;
}



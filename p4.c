#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <error.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <semaphore.h> 
#include <sys/sem.h>
#include <signal.h>


// Page size in memory
#define BLOCK_SIZE 4096
#define SHM_SIZE 1024  // Size of the shared memory
#define NUM_CHILDREN 3

struct SharedMemory {
    char message[NUM_CHILDREN][256];
    int status[NUM_CHILDREN];
};


sem_t mutex;  // Semaphore ID
int shmid;  // Shared memory ID
struct SharedMemory* shm_ptr;  // Pointer to the shared memory

//timers depending on process
int update = 0;
int downdate = 0;
int altdate = 5;
int timer = 0;


pid_t ids[NUM_CHILDREN];



void child_process(int child_id) {
    while (1) {
         printf("%d",child_id);

        // Wait for exclusive write access to shared memory
sem_wait(&mutex);

        if (shm_ptr->status[child_id] == 0) {
        if (child_id == 0){
            // Write a message to the shared memory
            snprintf(shm_ptr->message[child_id], sizeof(shm_ptr->message), "Child %d message %i", child_id, update);
                    update++;

        } if (child_id == 1){
            // Write a message to the shared memory
            snprintf(shm_ptr->message[child_id], sizeof(shm_ptr->message), "Child %d message %i", child_id, downdate);
            downdate--;

        } if (child_id == 2){
            // Write a message to the shared memory
            snprintf(shm_ptr->message[child_id], sizeof(shm_ptr->message), "Child %d message %i", child_id, altdate);
            altdate *= -1; 
        }
        
            shm_ptr->status[child_id] = 1;  // Message is pending
        
            // Release the semaphore
           sem_post(&mutex);

        }
       else{
        printf("%i is not active", child_id);
       }

        sleep(1);  // Simulate some work
       

        // Child process can continue after the parent reads the message
    }
}

void parent_process() {
  while(1) {
        // Wait for all child processes to complete their messages
        for (int i = 0; i < NUM_CHILDREN; i++) {
            while (shm_ptr->status[i] == 0);
        }

        // Print messages from child processes
        printf("\nMessages from child processes:\n");
        for (int i = 0; i < NUM_CHILDREN; i++) {
            printf("Child %d: %s\n", i, shm_ptr->message[i]);
            timer++;
           shm_ptr->status[i] = 0;  // Message has been processed
           
            
        }
 for (int i = 0; i < NUM_CHILDREN; i++) {
        

            if (timer > 40){
                      if (kill(ids[i], SIGKILL) == 0) {
            printf("Parent process killed the child process %i\n", i);  
           
        }

       }
    
  }
  if (timer >40) {
    break;
  }
  
}
}

int main(int argc, char* argv[]){
     
       sem_init(&mutex, 1, 1); 

    // Converts a pathname and project identifier to a unique System V IPC key
    key_t key = ftok("p4", 0);

    // Allocates a shared memory segment and returns the identifier of the shared memory segment
    // Memory segment has permissions 0666 and flag ensures it is created
     shmid = shmget(key, BLOCK_SIZE, 0644 | IPC_CREAT);

    // Attaches the shared memory segment associated with the shared memory identifier specified
    // by shmid to the address space of the calling process and returns the starting address of
    // the shared memory block
    shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (struct SharedMemory*)-1){
            perror("Failed to attach shared memory");
        return -1;
    }
    // Initialize the status array
for (int i = 0; i < NUM_CHILDREN; i++) {
    shm_ptr->status[i] = 0;  // 0 indicates no message pending
}

 for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        ids[i] = pid;
        if (pid == 0) {
            // Child process
            child_process(i);
        }
    }

    // Parent process
parent_process();
 

printf("end parent");
    // Detaches the shared memory segment from the address space of the calling process
    shmdt(shm_ptr);
    // Shared memory control operation - Destroy the shared memory segment
    shmctl(shmid, IPC_RMID, NULL);

    sem_destroy(&mutex); //Destroy semaphore

    return 0;
}
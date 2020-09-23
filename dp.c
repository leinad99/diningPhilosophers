/*
  Author:  Daniel DeGraaf and Lake Pry
  Course:  COMP 340, Operating Systems
  Date:    18 November 2019
  Description:  This file implements the
                functionality required for
                Project 2, Task 2.
  Compile with: gcc -o dp dp.c -lpthread
  Run with:     ./dp sample1.out 
                        or 
                ./dp sample2.out

*/

#include "dp.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    //For the random number files 
    for(int i = 0; i < MAX_LENGTH; i++){
        rand_numbers[i] = 0;
    }
    FILE *file;
    //try to open the file for the random numbers
    if ((file = fopen(argv[1],"r")) == NULL){
        printf("File failed to open\n");
        return 1;
    }
    //put the random numbers in the file into an array
    int counter = 0;
    while((fscanf(file,"%d",&rand_numbers[counter])) != EOF){
        counter++;
    }
    fclose(file);//close the file

    int rand_position = 0;

    pthread_t philosophers[NUMBER]; //initialize threads for the philosophers
    if (pthread_mutex_init(&mutex_lock, NULL) != 0){//initialize mutex lock
        printf("Could not make mutex lock");
        return -1;
    }
    for (int i = 0; i < NUMBER; i++){ //initiallize the 5 chopstick semaphores
        if (sem_init(&sem_vars[i], 0, 1) == -1){
            printf("Could not make semaphores");
            return -1;
        }
    }
    for (int i = 0; i < NUMBER; i++){//initiallize id's
        thread_id[i] = i;
    }
    for (int i = 0; i < NUMBER; i++){ //Create all the philosopher threads
        pthread_create(&philosophers[i],0,philosopher,(void *)&thread_id[i]);
    }
    for (int i = 0; i < NUMBER; i++){ //join all the threads back together
        pthread_join(philosophers[i],NULL);
    }   

    printf("\n\n\t<<< Normal Termination >>>\n\n");
    return 0;
}

void *philosopher(void *param) {
    int *lnumber = (int *)param;
    int id = *lnumber; //id of the philosopher
    int wait;
    for(int i = 0; i < 5; i++) { //think, get hungry, eat, rapeat for 5 times
        wait = get_next_number();
        printf("Philosopher %d is thinking for %d seconds...\n", id, wait);
        sleep(wait); //think for a certain amount of time
        pickup_chopsticks(id); //try to pick up chopsticks and eat
        sleep(get_next_number()); //eat for a random amount of time
        return_chopsticks(id); //put the chopsticks back
    }
    printf("Philosopher %d left the table.\n", id);
}
 
void pickup_chopsticks(int number) {
    printf("\tPhilosopher %d is hungry\n",number);
    pthread_mutex_lock(&mutex_lock);
    state[number] = HUNGRY;
    pthread_mutex_unlock(&mutex_lock);

    //if odd try to pick up right then middle, if even pick up middle then right.
    if(number % 2 == 0){
        sem_wait(&sem_vars[number]); //middle
        sem_wait(&sem_vars[(number + 1) % NUMBER]); //right
        pthread_mutex_lock(&mutex_lock);
        state[number] = EATING;
        pthread_mutex_unlock(&mutex_lock);
        printf("\t\tPhilosopher %d is eating with chopsticks %d and %d\n", number, number, (number + 1) % NUMBER);
        
    }
    else{
        sem_wait(&sem_vars[(number + 1) % NUMBER]); //right
        sem_wait(&sem_vars[number]); //middle
        pthread_mutex_lock(&mutex_lock);
        state[number] = EATING;
        pthread_mutex_unlock(&mutex_lock);
        printf("\t\tPhilosopher %d is eating with chopsticks %d and %d\n", number, number, (number + 1) % NUMBER);
    }
   
}

void return_chopsticks(int number) {
    pthread_mutex_lock(&mutex_lock);
    state[number] = THINKING;
    pthread_mutex_unlock(&mutex_lock);
    printf("\t\tPhilosopher %d is puts down chopsticks %d and %d\n", number, number, (number + 1) % NUMBER);
    //put back the chopsticks, order doesn't matter
    sem_post(&sem_vars[(number + 1) % NUMBER]); 
    sem_post(&sem_vars[number]);
}

int get_next_number(){
    //return the next int in the array of random ints
    return rand_numbers[rand_position++]; 
}



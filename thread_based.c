#ifndef _THREAD_H
#define _THREAD_H

/**
 * @file thread_based.c
 * @author Amith Gopi (431000927)
 * @brief This demonstrates the multiple producer consumer problem using pthread - single process creating multiple threads for producers and consumers.
 * Inter thread communication is handles by a common cycle message queue - implemnted by a bounded array - access to which is controlled usin two 
 * semaphoes and a mutex for the critical sections
 * @version 1.0
 * @date 2022-02-25
 * 
 */
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define max(a, b) ((a<b) ? b : a)

// Define constants
const size_t BUFFER_SIZE_T = 10;               //Buffer size
size_t NUM_PRODUCERS_T = 5; //Number of producers
size_t NUM_CONSUMERS_T = 5; //Number of consumers
int DELAY_T = 500; 
int ITERATIONS_T = 100;

struct timespec delay_t = {.tv_sec = 0, .tv_nsec = 500*1000000L};

// Circular message queue implemented as an array
void** buffer;
size_t current_buffer_write_index = 0;
size_t current_buffer_read_index = 0;

// Semaphoes
sem_t queue_full_semaphore;
sem_t queue_empty_semaphore;
pthread_mutex_t mutex;

// Declare the producer and consumer functions
void* producer_t(void*);
void* consumer_t(void*);

/**
 * @brief Main method.
 * 
 * @return int 
 */
int runThreadBasedExec(size_t _num_producers, size_t _num_consumers, int _delay, int _iterations ) {
     // Seed rand
     srand (time(NULL));

     //Init variables
     NUM_PRODUCERS_T = _num_producers;      //Number of producers
     NUM_CONSUMERS_T = _num_consumers;     //Number of consumers
     DELAY_T = _delay;
     ITERATIONS_T = _iterations;

     // Allocate buffer
     buffer = malloc(BUFFER_SIZE_T);
     
     // Initialize semaphores
     // Second argument indicates that these are shared between threads od process
     // Init empty semaphore as equal to buffer size
     sem_init(&queue_empty_semaphore, 0, BUFFER_SIZE_T);
     // Init fill semaphore as equal to 0
     sem_init(&queue_full_semaphore, 0, 0);
     // Initizlize the mutex
     pthread_mutex_init(&mutex, NULL);

     //Store producer and consumer threads
     pthread_t producers[NUM_PRODUCERS_T];
     pthread_t consumers[NUM_CONSUMERS_T];
     //Create and initialize an array to store an identifier for each thread
     int thread_id[max(NUM_CONSUMERS_T, NUM_PRODUCERS_T)];
     for(int i = 0; i < max(NUM_CONSUMERS_T, NUM_PRODUCERS_T); i++) {
          thread_id[i] = i+1;
     }

     // Create producer threads
     for(int i=0; i < NUM_PRODUCERS_T; i++) {
          pthread_create(&producers[i], NULL, producer_t, &thread_id[i]);
     }

     // Create consumer threads
     for(int i=0; i < NUM_CONSUMERS_T; i++) {
          pthread_create(&consumers[i], NULL, consumer_t, &thread_id[i]);
     }

     // Let the main thread wait for the producer threads
     for(int i = 0; i < NUM_PRODUCERS_T; i++) {
        pthread_join(producers[i], NULL);
     }
     // Let the main thread wait for the consumer threads
     for(int i = 0; i < NUM_CONSUMERS_T; i++) {
        pthread_join(consumers[i], NULL);
     }

     //Destroy semaphores and mutex
     pthread_mutex_destroy(&mutex);
     sem_destroy(&queue_empty_semaphore);
     sem_destroy(&queue_full_semaphore);
     //Deallocate buffer
     free(buffer);

     return 0;
}

/**
 * @brief Struct to act as a wrapper fro messages to be placed in the message queue/buffer.
 * This contains a message id and a pointer to the message.
 * 
 */
struct Message { 
     int message_id;
     void* message_ptr;
};

/**
 * @brief Implements the producer part of the problem. It is an infinite loop function where in each loop iteration, it tries to send a message to
 * the message queue if it is able to acquire the necessary mutex locks and semaphores
 * 
 * @param pid value used to identify the process pass from the thread creation process
 * @return void* 
 */
void* producer_t(void* pid) {
     int count = ITERATIONS_T;
     while(count) {
          // Decrease the empty semaphore lock by one to indicate decremnting the number of empty slots
          // This reserves the slot for the producer
          sem_wait(&queue_empty_semaphore);

          // Lock mutex for critical section of code
          pthread_mutex_lock(&mutex);
          // Create a message with a random number and place it in message queue
          struct Message message = {rand(), NULL };
          buffer[current_buffer_write_index] = (void *)&message;
          // Increase buffer size pointer
          current_buffer_write_index = (current_buffer_write_index + 1)%BUFFER_SIZE_T;
          printf("Producer ID: %d - Placing message - %d | Queue: %d\n", *(int *)pid, message.message_id, (int)current_buffer_write_index);
          // Unlock mutex for critical section of code
          pthread_mutex_unlock(&mutex);

          // Increase the full semaphore lock by one to indicate incrementing the number of full slots
          // This indicates that a message is in the queue
          sem_post(&queue_full_semaphore);
          count--;
          
     }
}

/**
 * @brief Implements the consumer part of the problem. It is an infinite loop function where in each loop iteration, it tries to recieve a message from
 * the message queue if it is able to acquire the necessary mutex locks and semaphores
 * 
 * @param pid value used to identify the process pass from the thread creation process
 * @return void* 
 */
void* consumer_t(void* cid) {
     while(true) {
          // Decrease the full semaphore lock by one to indicate decrementing the number of empty slots
          // This reserves the current message for this consumer
          sem_wait(&queue_full_semaphore);

          // Lock mutex for critical section of code
          pthread_mutex_lock(&mutex);
          // Create a message with a random number and place it in message queue
          struct Message message =  *(struct Message*)buffer[current_buffer_read_index];
          // Increase buffer size pointer
          current_buffer_read_index = (current_buffer_read_index + 1)%BUFFER_SIZE_T;
          printf("Consumer ID: %d - Recieved message - %d | Queue: %d\n", *(int *)cid, message.message_id, (int)current_buffer_read_index);
          // Unlock mutex for critical section of code
          pthread_mutex_unlock(&mutex);
          // Create a delay to simmulate processing time
          if(DELAY_T) {
               delay_t.tv_nsec = DELAY_T*1000000L;
               nanosleep(&delay_t, &delay_t);
          }
          
          // Increase the empty semaphore lock by one to indicate incrementing the number of empty slots
          // This indicates that a message has been consumed
          sem_post(&queue_empty_semaphore);

     }
}

#endif
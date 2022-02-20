#include<iostream>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include<unistd.h>

using namespace std;

// Define constants
const size_t BUFFER_SIZE = 10;               //Buffer size
const unsigned int NUM_PRODUCER_THREADS = 5; //Number of producers
const unsigned int NUM_CONSUMER_THREADS = 5; //Number of consumers

// Circular message queue implemented as an array
void** buffer;
size_t current_buffer_write_index = 0;
size_t current_buffer_read_index = 0;

// Semaphoes
sem_t queue_full_semaphore;
sem_t queue_empty_semaphore;
pthread_mutex_t mutex;

// Declare the producer and consumer functions
void* producer(void*);
void* consumer(void*);

/**
 * @brief Main method.
 * 
 * @return int 
 */
int main() {
     // Seed rand
     srand (time(NULL));

     // Allocate buffer
     buffer =  new void*[BUFFER_SIZE];
     // Initialize semaphores
     // Second argument indicates that these are shared between threads od process
     // Init empty semaphore as equal to buffer size
     sem_init(&queue_empty_semaphore, 0, BUFFER_SIZE);
     // Init fill semaphore as equal to 0
     sem_init(&queue_full_semaphore, 0, 0);
     // Initizlize the mutex
     pthread_mutex_init(&mutex, NULL);

     //Store producer and consumer threads
     pthread_t producers[NUM_PRODUCER_THREADS];
     pthread_t consumers[NUM_CONSUMER_THREADS];
     //Create and initialize an array to store an identifier for each thread
     int thread_id[max(NUM_CONSUMER_THREADS, NUM_PRODUCER_THREADS)];
     for(int i = 0; i < max(NUM_CONSUMER_THREADS, NUM_PRODUCER_THREADS); i++) {
          thread_id[i] = i+1;
     }

     // Create producer threads
     for(int i=0; i < NUM_PRODUCER_THREADS; i++) {
          pthread_create(&producers[i], NULL, producer, &thread_id[i]);
     }

     // Create consumer threads
     for(int i=0; i < NUM_CONSUMER_THREADS; i++) {
          pthread_create(&consumers[i], NULL, consumer, &thread_id[i]);
     }

     // Let the main thread wait for the producer threads
     for(int i = 0; i < NUM_PRODUCER_THREADS; i++) {
        pthread_join(producers[i], NULL);
     }
     // Let the main thread wait for the consumer threads
     for(int i = 0; i < NUM_CONSUMER_THREADS; i++) {
        pthread_join(consumers[i], NULL);
     }

     //Destroy semaphores and mutex
     pthread_mutex_destroy(&mutex);
     sem_destroy(&queue_empty_semaphore);
     sem_destroy(&queue_full_semaphore);
     //Deallocate buffer
     delete[] buffer;

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
void* producer(void* pid) {
     while(true) {
          // Decrease the empty semaphore lock by one to indicate decremnting the number of empty slots
          // This reserves the slot for the producer
          sem_wait(&queue_empty_semaphore);

          // Lock mutex for critical section of code
          pthread_mutex_lock(&mutex);
          // Create a message with a random number and place it in message queue
          struct Message message = {rand(), nullptr };
          buffer[current_buffer_write_index] = (void *)&message;
          // Increase buffer size pointer
          current_buffer_write_index = (current_buffer_write_index + 1)%BUFFER_SIZE;
          cout<<"Producer ID: "<<*(int *)pid<<" - Placing message  - "<<message.message_id<<" | Queue: "<<current_buffer_write_index<<endl;
          // Unlock mutex for critical section of code
          pthread_mutex_unlock(&mutex);

          // Increase the full semaphore lock by one to indicate incrementing the number of full slots
          // This indicates that a message is in the queue
          sem_post(&queue_full_semaphore);
          sleep(5);
     }
}

/**
 * @brief Implements the consumer part of the problem. It is an infinite loop function where in each loop iteration, it tries to recieve a message from
 * the message queue if it is able to acquire the necessary mutex locks and semaphores
 * 
 * @param pid value used to identify the process pass from the thread creation process
 * @return void* 
 */
void* consumer(void* cid) {
          while(true) {
          // Decrease the full semaphore lock by one to indicate decrementing the number of empty slots
          // This reserves the current message for this consumer
          sem_wait(&queue_full_semaphore);

          // Lock mutex for critical section of code
          pthread_mutex_lock(&mutex);
          // Create a message with a random number and place it in message queue
          struct Message message =  *(Message*)buffer[current_buffer_read_index];
          // Increase buffer size pointer
          current_buffer_read_index = (current_buffer_read_index + 1)%BUFFER_SIZE;
          cout<<"Consumer ID: "<<*(int *)cid<<" - Recieved message - "<<message.message_id<<" | Queue: "<<current_buffer_read_index<<endl;
          // Unlock mutex for critical section of code
          pthread_mutex_unlock(&mutex);

          // Increase the empty semaphore lock by one to indicate incrementing the number of empty slots
          // This indicates that a message has been consumed
          sem_post(&queue_empty_semaphore);

     }
}
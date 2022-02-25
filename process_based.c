#ifndef _PROCESS_H
#define _PROCESS_H

#include<semaphore.h>
#include<time.h>
#include<mqueue.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
  #include <sys/wait.h>
  #include <unistd.h>

// Set to enable debug loggers
#define DEBUG_LOGS false
// Define loggers levels
enum LOG_LEVEL {DEBUG, INFO};
// define name of message queue
#define MESSAGE_QUEUE "/msg_queue"
// Name of file to output the messages to
#define OUTPUT_FILE "./output.txt"
#define INPUT_FILE "./input.txt"

// Names for shares semaphores
#define FULL_SEM "/queue_full_semaphore"
#define EMPTY_SEM "/queue_empty_semaphore"
#define CRITICAL_SECTION_MUTEX "/cs_mutex"

// NUmber of iterations to run the producer process for, set to -1 for infinite iterations
int ITERATIONS = 100;

// Macro to convert millisseconds to nanoseconds
#define msTons(delay) delay*1000000L

// Define constants
#define BUFFER_SIZE 10        //Buffer size
#define MAX_MESSAGE_SIZE 100
size_t NUM_PRODUCERS = 5;      //Number of producers
size_t NUM_CONSUMERS = 5;     //Number of consumers
int DELAY = 500;            //Max time in ms to wait while consuming messages, set to 0 for no dealy

// Declare the producer and consumer functions
void* producer(int);
void* consumer(int);
void initlializeSemaphores();
void createMessageQueue();
void cleanSemaphores();
void logger(enum LOG_LEVEL, char *s, ...);

// Initlialize a timespec arry to be used as delay
struct timespec delay = {.tv_sec = 0, .tv_nsec = msTons(500)};

/**
 * @brief Main method.
 * 
 * @return int 
 */
int runProcessBased(size_t _num_producers, size_t _num_consumers, int _delay, int _iterations ) {
     // Seed rand
     srand(time(NULL));
     //Init variables
     NUM_PRODUCERS = _num_producers;      //Number of producers
     NUM_CONSUMERS = _num_consumers;     //Number of consumers
     DELAY = _delay; 
     ITERATIONS = _iterations;
     // Clean any remant shared resoures from previous runs
     cleanSemaphores();
     // Init a new set of semaphores
     initlializeSemaphores();
     // Create a new message queue
     createMessageQueue();

     FILE *filePointer ; 
     // Create a output file
     filePointer = fopen(OUTPUT_FILE, "w");
     fclose(filePointer);
     // Create a input file
     filePointer = fopen(INPUT_FILE, "w");
     fclose(filePointer);

     int i, pid;
     for(i = 0; i<NUM_PRODUCERS; i++) {
          pid = fork();
          if(pid == 0) {
               // isConsumer = true;
               printf("Started Producer child pid %d from [parent] pid %d\n",getpid(),getppid());
               producer(i);
               exit(0);
          }
     }

     for(i = 0; i<NUM_CONSUMERS; i++) {
          pid = fork();
          if(pid == 0) {
               // isProducer = true;
               printf("Started Consumer child pid %d from [parent] pid %d\n",getpid(),getppid());
               consumer(i);
               exit(0);
          }
     }

     // Wait for child process to complete
     if(pid != 0) while(wait(NULL) > 0);
     // Destroy the message queue
     mq_unlink(MESSAGE_QUEUE);
     // Clear semaphores
     cleanSemaphores();

     return 0;
}

// /**
//  * @brief Struct to act as a wrapper fro messages to be placed in the message queue/buffer.
//  * This contains a message id and a pointer to the message.
//  * 
//  */
// struct Message { 
//      int message_id;
//      void* message_ptr;
// };

/**
 * @brief Implements the producer part of the problem. It is an infinite loop function where in each loop iteration, it tries to send a message to
 * the message queue if it is able to acquire the necessary mutex locks and semaphores
 * 
 * @param pid value used to identify the process pass from the thread creation process
 * @return void* 
 */
void* producer(int producer_id) {
     // Open the message queue
     mqd_t mqd = mq_open(MESSAGE_QUEUE, O_EXCL | O_WRONLY);
     if (mqd == -1) {
          perror ("Error opening queue");
          exit (1);
     }

     // Open shared semaphoes
     sem_t *queue_full_semaphore = sem_open(FULL_SEM, O_RDWR);
     sem_t *queue_empty_semaphore = sem_open(EMPTY_SEM, O_RDWR);
     sem_t *mutex = sem_open(CRITICAL_SECTION_MUTEX, O_RDWR);
     // Pointer to file to print output
     FILE *filePointer;

     // Set number of messages to produce equal to iterations
     int count = ITERATIONS;
     while(count) {
          // Decrease the empty semaphore lock by one to indicate decremnting the number of empty slots
          // This reserves the slot for the producer
          logger(DEBUG, "PORD %d, wait\n", producer_id);
          sem_wait(queue_empty_semaphore);
          logger(DEBUG, "PORD %d, mutex wait\n", producer_id);
          // Lock mutex for critical section of code
          sem_wait(mutex);
          filePointer = fopen(INPUT_FILE, "a");
          logger(DEBUG, "PORD %d, CS\n", producer_id);

          // Create a message with a random number
          char input[MAX_MESSAGE_SIZE] ;
          sprintf(input, "%d", rand()%1000);   
          //Create a message by adding producer info and pid to the random number
          char msg[100];
          sprintf(msg, "PROD: %d | PID: %d | COUNT: %d | MSG: ", producer_id, getpid(), count);
          strcat(msg, input);
          
          // Send message to the message queue
          mq_send (mqd, msg, strlen(msg)+1, 0);

          // Log message to file
          fprintf(filePointer, "%s\n", msg);
          fclose(filePointer);
          // Unlock mutex for critical section of code
          sem_post(mutex);
          logger(DEBUG, "PORD %d, unlock\n", producer_id);
          // Increase the full semaphore lock by one to indicate incrementing the number of full slots
          // This indicates that a message is in the queue
          sem_post(queue_full_semaphore);
          // delay.tv_nsec = msTons(rand()%1000);
          // nanosleep(&delay, &delay);
          count--;
     }
     // Close semaphores
     sem_close(queue_full_semaphore);
     sem_close(queue_empty_semaphore);
     sem_close(mutex);
}

/**
 * @brief Implements the consumer part of the problem. It is an infinite loop function where in each loop iteration, it tries to recieve a message from
 * the message queue if it is able to acquire the necessary mutex locks and semaphores
 * 
 * @param consumer_id value used to identify the process pass from the thread creation process
 * @return void* 
 */
void* consumer(int consumer_id) {
     // Open the message queue
     mqd_t mqd = mq_open(MESSAGE_QUEUE, O_RDONLY);
     if (mqd == -1) {
          perror ("Error opening queue");
          exit (1);
     }

     // Open shared semaphoes
     sem_t *queue_full_semaphore = sem_open(FULL_SEM, O_RDWR);
     sem_t *queue_empty_semaphore = sem_open(EMPTY_SEM, O_RDWR);
     sem_t *mutex = sem_open(CRITICAL_SECTION_MUTEX, O_RDWR);
     // Pointer to file to print output
     FILE *filePointer ; 
     
     int count = NUM_PRODUCERS*ITERATIONS;
     while(count) {
          // Decrease the full semaphore lock by one to indicate decrementing the number of empty slots
          // This reserves the current message for this consumer
          logger(DEBUG, "CON %d, wait\n", consumer_id);
          sem_wait(queue_full_semaphore);
          logger(DEBUG, "CON %d, mnutex wait\n", consumer_id);
          // Lock mutex for critical section of code
          sem_wait(mutex);
          logger(DEBUG, "CON %d, CS\n", consumer_id);
          // Open file in append mode to add the received message
          filePointer = fopen(OUTPUT_FILE, "a");
          // Receive message
          char output[MAX_MESSAGE_SIZE+1];
          mq_receive(mqd, output, 100, NULL);
          logger(INFO, "CON - %d | MSG - %s\n", consumer_id, output);
          // Place message in file
          fprintf(filePointer, "CON - %d | %s\n", consumer_id, output);
          fclose(filePointer);

          // Unlock mutex for critical section of code  
          sem_post(mutex);
          logger(DEBUG, "CON %d unlock\n", consumer_id);
          // Set process to sleep for a random interval based on the DELAY parameter
          if( DELAY ) {
               delay.tv_nsec = msTons(rand()%500);
               nanosleep(&delay, &delay);
          }

          // Increase the empty semaphore lock by one to indicate incrementing the number of empty slots
          // This indicates that a message has been consumed
          sem_post(queue_empty_semaphore);
          count--;
     }
     // Close semaphores
     sem_close(queue_full_semaphore);
     sem_close(queue_empty_semaphore);
     sem_close(mutex);
}

/**
 * @brief Initialize/create the semaphores
 * 
 */
void initlializeSemaphores() {
     sem_t* full = sem_open(FULL_SEM, O_CREAT | O_EXCL, 0660, 0);
     sem_t* empty = sem_open(EMPTY_SEM, O_CREAT | O_EXCL, 0660, BUFFER_SIZE);
     sem_t* mutex = sem_open(CRITICAL_SECTION_MUTEX, O_CREAT | O_EXCL, 0660, 1);

     sem_close(full);
     sem_close(empty);
     sem_close(mutex);
}

/**
 * @brief Create a Message Queue object
 * 
 */
void createMessageQueue() {
     struct mq_attr attr;
     attr.mq_msgsize = MAX_MESSAGE_SIZE;
     attr.mq_maxmsg = BUFFER_SIZE;
     attr.mq_flags = 0;
     attr.mq_curmsgs = 0;
     mqd_t mqd = mq_open(MESSAGE_QUEUE, O_CREAT | O_WRONLY,  0600, &attr);
     if (mqd == -1) {
          perror ("Error opening queue");
          exit (1);
     }
     mq_close(mqd);
}

/**
 * @brief Clean up all semaphores
 * 
 */
void cleanSemaphores() {
     mq_unlink(MESSAGE_QUEUE);
     sem_unlink(FULL_SEM);
     sem_unlink(EMPTY_SEM);
     sem_unlink(CRITICAL_SECTION_MUTEX);
}

/**
 * @brief Better loggers based on levels
 * 
 * @param level 
 * @param s 
 * @param ... 
 */
void logger(enum LOG_LEVEL level, char *s, ...) {
     va_list args;
     va_start(args, s);
     if (level == DEBUG && DEBUG_LOGS) {
          vprintf(s, args);
     } else if (level == INFO) {
          vprintf(s, args);
     }
     va_end(args);
}

#endif
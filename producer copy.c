
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include<unistd.h>
#include<mqueue.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MESSAGE_QUEUE "/msg_queue"

// using namespace std;

// Define constants
const size_t BUFFER_SIZE = 10;               //Buffer size
const unsigned int NUM_PRODUCERS = 1; //Number of producers
const unsigned int NUM_CONSUMERS = 1; //Number of consumers

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
     srand(time(NULL));
     pthread_mutex_init(&mutex, NULL);

     struct mq_attr attr;
     attr.mq_msgsize = 100;
     attr.mq_maxmsg = BUFFER_SIZE;
     attr.mq_flags = 0;
     attr.mq_curmsgs = 0;
     mqd_t mqd = mq_open(MESSAGE_QUEUE, O_CREAT | O_WRONLY,  0600, &attr);
     if (mqd == -1) {
          perror ("Error opening queue");
          exit (1);
     }
     mq_close(mqd);


     int i, pid;
     bool isConsumer, isProducer;
     // for(i=0;i<NUM_CONSUMERS;i++) {
     //      pid = fork();
     //      if(pid == 0) {
     //           isConsumer = true;
     //           printf("Started Producer child pid %d from [parent] pid %d\n",getpid(),getppid());
               producer((void *)&i);
     //           exit(0);
     //      }
     // }

     // for(i=0;i<NUM_PRODUCERS;i++) {
     //      pid = fork();
     //      if(pid == 0) {
     //           isProducer = true;
     //           printf("Started Consumer child pid %d from [parent] pid %d\n",getpid(),getppid());
               consumer((void *)&i);
     //           exit(0);
     //      }
     // }

     // if(pid != 0) while(wait(NULL) > 0);
     mq_unlink(MESSAGE_QUEUE);
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
     mqd_t mqd = mq_open(MESSAGE_QUEUE, O_EXCL | O_WRONLY);
     if (mqd == -1) {
          perror ("Error opening queue");
          exit (1);
     }
     
     int count = 5;
     while(count) {
          // Decrease the empty semaphore lock by one to indicate decremnting the number of empty slots
          // This reserves the slot for the producer
          // sem_wait(&queue_empty_semaphore);

          // Lock mutex for critical section of code
          pthread_mutex_lock(&mutex);
          // Create a message with a random number and place it in message queue
          char input[100+1] = "test117";
          struct Message message = {rand(), NULL };
          struct mq_attr attr;
          mq_getattr(mqd, &attr);
          printf("size %d", attr.mq_curmsgs);
          char a[3];
          sprintf(a, "%d", count);
          strcat(input, a);
          mq_send (mqd, input, strlen(input)+1, 0);

          // buffer[current_buffer_write_index] = (void *)&message;
          // // Increase buffer size pointer
          // current_buffer_write_index = (current_buffer_write_index + 1)%BUFFER_SIZE;
          // cout<<"Producer ID: "<<*(int *)pid<<" - Placing message  - "<<message.message_id<<" | Queue: "<<current_buffer_write_index<<endl;
          // Unlock mutex for critical section of code
          pthread_mutex_unlock(&mutex);

          // Increase the full semaphore lock by one to indicate incrementing the number of full slots
          // This indicates that a message is in the queue
          // sem_post(&queue_full_semaphore);
          // sleep(5);
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
void* consumer(void* cid) {
     mqd_t mqd = mq_open(MESSAGE_QUEUE, O_RDONLY);
     if (mqd == -1) {
          perror ("Error opening queue");
          exit (1);
     }

     
     int count =5;
     while(count) {
          // Decrease the full semaphore lock by one to indicate decrementing the number of empty slots
          // This reserves the current message for this consumer
          // sem_wait(&queue_full_semaphore);

          // Lock mutex for critical section of code
          pthread_mutex_lock(&mutex);
          char output[100+1];
          mq_receive(mqd, output, 100, NULL);
          printf("%s", output);
          // Create a message with a random number and place it in message queue
          // struct Message message =  *(Message*)buffer[current_buffer_read_index];
          // // Increase buffer size pointer
          // current_buffer_read_index = (current_buffer_read_index + 1)%BUFFER_SIZE;
          // cout<<"Consumer ID: "<<*(int *)cid<<" - Recieved message - "<<message.message_id<<" | Queue: "<<current_buffer_read_index<<endl;
          // Unlock mutex for critical section of code
          pthread_mutex_unlock(&mutex);

          // Increase the empty semaphore lock by one to indicate incrementing the number of empty slots
          // This indicates that a message has been consumed
          // sem_post(&queue_empty_semaphore);
          count--;

     }
}
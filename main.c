/**
 * @file main.c
 * @author Amith Gopi (431000927)
 * @brief To run this file, execute the binary as ./main <args>. The following arguments must be passed to the input:
 * -p for number of producers, -c for number of consumers and -d for the consumer delay in milliseconds
 * The -a option enables an alternate mode which uses pthreads to create threads instead of processes using pthreads.h
 * -t is optional and can be used to change the number of messages each producer process would generate.
 * @version 1.0
 * @date 2022-02-25
 * 
 */

#include "process_based.c"
#include "thread_based.c"
#include <stdio.h>
#include <stdbool.h>

// define the argument options
#define OPT_STRING "p:c:d:t:a"

int main(int argc, char *argv[]) {
     int opt;
     // Init paramters
     int _num_producers = -1, _num_consumers = -1, _delay = -1, _iterations = 100;
     bool isAlternateMode = false;

     //Check for command line arguments and parse values
     while ((opt = getopt(argc, argv, OPT_STRING)) != EOF) {
         switch (opt)
         {
             case 'p':
                _num_producers = strtol(optarg, NULL, 10);
                break; 
             case 'c':
                _num_consumers = strtol(optarg, NULL, 10);
                break;
            case 'd':
                _delay = strtol(optarg, NULL, 10);
                break;
            case 't':
                _iterations = strtol(optarg, NULL, 10);
                break;
            case 'a':
                isAlternateMode = true;
                break;
            default:
             break;
         }
     }

    // Run the program based on the arguments selected
    if (_num_producers >= 0 && _num_consumers >=0 && _delay >= 0 && _iterations > 0 ) {
        printf("Running producers (%d) for %d iterations and consumers (%d) with max consumer dealy at %d.\n", _num_producers, _iterations, _num_consumers, _delay);
        if(isAlternateMode) { 
            printf("Running in alterante mode using pthreads...\n");
            runThreadBasedExec(_num_producers, _num_consumers, _delay, _iterations); }
        else { 
            runProcessBased(_num_producers, _num_consumers, _delay, _iterations); }
    } else {
        printf("Invalid arguemnts passed\nE.g: ./main -p 5 -c 6 -d 500");
    }

    return 0;

}
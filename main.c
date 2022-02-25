/**
 * @file main.c
 * @author Amith Gopi (431000927)
 * @brief To run this file, execute the binary as ./main <args>. The following areguments must be passed to the input:
 * -p for number of producers, -c for number of consumers and -d for the consumer delay in milliseconds
 * The -a option enables an alternate mode which uses pthreads to create threads instead of processes
 * @version 1.0
 * @date 2022-02-25
 * 
 */

#include "process_based.c"
#include "thread_based.c"
#include <stdio.h>
#include <stdbool.h>

#define OPT_STRING "p:c:d:a"

int main(int argc, char *argv[]) {
     int opt;
     int _num_producers = -1, _num_consumers = -1, _delay = -1;
     bool isAlternateMode = false;
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
            case 'a':
                isAlternateMode = true;
                break;
            default:
             break;
         }
     }

    
    if (_num_producers >= 0 && _num_consumers >=0 && _delay >= 0) {
        printf("Running producers (%d) and consumers (%d) with max consumer dealy at %d.\n", _num_producers, _num_consumers, _delay);
        if(isAlternateMode) { 
            printf("Running in alterante mode using pthreads...\n");
            runThreadBasedExec(_num_producers, _num_consumers, _delay); }
        else { 
            runProcessBased(_num_producers, _num_consumers, _delay); }
    } else {
        printf("Invalid arguemnts passed\nE.g: ./main -p 5 -c 6 -d 500");
    }

    return 0;

}
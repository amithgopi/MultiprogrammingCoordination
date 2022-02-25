# MultiprogrammingCoordination
Implement a correct solution to the n-dimensional multiple producer consumer problem. The program uses a POSIX message queue to implement inter process
communication between multiple producers and consumers. The producers place their messsages (generated using rand as random numbers) in to the queue and
the consumer recive them from the queue. Both write their respective messages to files called input.txt and output.txt in the same folder as the `main` binary.
This uses two semaphoes to handle the bound message buffer so that producers wait on a full buffer and consumers wait on an empty buffer.
Another semaphore (or mutex) is used to lock the critical section of the code preserving the atomicity if this section.
The process is set to run for **100** iterations of a producer by default - each producer produces 100 messages. The -t argument can be used to change this

In the alternate thread baed implementation, the same process is accomplished using an in memory array used as a cycle bounded queue and threads of the same process
instead of individual processes for each producer and consumer.

## Build
To run the program compile the files main.c, process_based.c and thread_based.c
Run `gcc -g main.c -o main -pthread -lrt`
This should generate a executable file called main.
*NOTE*: Ensure that main has the executable permission (+x) set to that it can run.

## Run
To run the program execute the binary named `main`
The following arguments must be passed to the input:
 * -p for number of producers, -c for number of consumers and -d for the consumer delay in milliseconds
 * The -a option enables an alternate mode which uses pthreads to create threads instead of processes using pthreads.h.
 Eg: 
 * `./main -p 3 -c 5 -d 500` (3 producers, 5 consumers and a max consumption delay of 500ms per message)
 * `./main -p 8 -c 2 -d 200 -t 50` (8 producers, 2 consumers and a max consumption delay of 200ms per message and producers generate 50 messages each)
 * `./main -p 5 -c 5 -d 300 -a` (Enables alternate mode)

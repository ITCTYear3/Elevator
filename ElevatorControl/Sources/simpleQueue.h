#ifndef SIMPLE_QUEUE
#define SIMPLE_QUEUE

#define QUEUE_LEN 3 //length of the queue

//Globals for the queue, head of the queue and number in queue
char queue[QUEUE_LEN] = {0};
int queuePos = 0;
int numInQueue = 0;

//Function prototypes
void addToQueue( char floorNum );
char getNextFloor ();
char peekNextFloor ();

#endif
/*
Simple circular buffer
-peekNextFloor returns the next floor on the queue without dequeueing it
-getNextFloor returns the number of the next floor to return to and pops it off the top of the queue, it returns 0 if the queue is empty
-addToQueue takes a floor number, checks to see if that floor is already in the queue, and if not, puts it at the end of the queue

*/

#include simpleQueue.h

char peekNextFloor() {
	int nextFloor;
	
	if (numInQueue == 0) 	//check to see if there is any valid data in the queue
        return 0; 			//nothing valid in the queue, simply return 0
	
	nextFloor = queue[queuePos];
	
	return nextFloor;
}

char getNextFloor () {

    int nextFloor = queue[queuePos]; //grab the floor at the front of the queue
    
    if (numInQueue == 0){ //check to see if there is any valid data in the queue
        return 0; //nothing valid in the queue, simply return 0
    }
    
    numInQueue--; //there was something valid in the queue, so decrement the number in the queue
    
    queuePos = (queuePos + 1) % QUEUE_LEN; //move the top position of the queue as it has been popped
    
    return nextFloor; //return the target floor
}

void addToQueue ( char floorNum ){
    int i;
    
    //check to see if the floor being added to the queue is already present
    for(i=0; i < QUEUE_LEN; i++){ 
        if (floorNum == queue[i]){
            return;
        }
    }
    
    //floor wasn't in queue, add it to the end of the queue and increment the queue length
    queue[(queuePos + numInQueue) % QUEUE_LEN] = floorNum;
    numInQueue++;

}
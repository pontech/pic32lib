#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <WProgram.h>
#include <Stream.h>
#include "Vector.h"
//#include "Arduino.h"

class CircleBuffer {
public:
	CircleBuffer() //int bufferSize = 32)
	{
		size = 33; //bufferSize+1;
		head = 0;
		tail = 0;
		//contents = (Vector *) malloc (sizeof (Vector) * size);
	}
	
	~CircleBuffer ()
	{
		//free(contents);
		//contents = NULL;
		size = 0;
		head = 0;
		tail = 0;
	}
	
	int isFull ()
	{
		if((tail+1)%size == head){
			return true;
		}
		else{
			return false;
		}
	}
	
	int isEmpty()
	{
		if(tail == head){
			return true;
		}
		else{
			return false;
		}
	}
	
	int push (const Vector i)
	{
		if(!isFull()){
			contents[tail] = i;
			tail = (tail + 1) % size;
			return 0;
		}
		return -1;
	}
	
	Vector pop ()
	{
		Vector elem = contents[head];
		head = (head + 1) % size;
		return elem;
	}
	
	Vector next ()
	{
		Vector elem = contents[current];
		if(head+tail > size) {
			current = (current + 1) % (size-head + tail);
		}
		else {
			current = (current + 1) % (head-tail);
		}
		return elem;
	}
	
	bool isAvailable(int i)
	{
		for(int j = 0; j <= i; j++){
			if((tail+1)%size == head){
				return false;
			}
		}
		return true;
	}

private:

    int size;
	int current;
    int head;
    int tail;
    Vector contents[33];
};

#endif // CIRCLEBUFFER_H

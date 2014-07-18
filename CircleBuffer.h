#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <WProgram.h>
#include <Stream.h>
#include "Vector.h"
//#include "Arduino.h"
#define CircleBufferSize 75
class CircleBuffer {
public:
    CircleBuffer() //int bufferSize = 32)
    {
        size = CircleBufferSize; //bufferSize+1;
        clear();
        //contents = (Vector *) malloc (sizeof (Vector) * size);
    }

    ~CircleBuffer()
    {
        //free(contents);
        //contents = NULL;
        size = 0;
        head = 0;
        tail = 0;
    }

	void clear()
    {
        head = 0;
        tail = 0;
    }
	
    int isFull()
    {
        if((tail + 1) % size == head) {
            return true;
        }
        else {
            return false;
        }
    }

    int isEmpty()
    {
        if(tail == head) {
            return true;
        }
        else {
            return false;
        }
    }

    int push(const Vector &v)
    {
        if(!isFull()) {
            contents[tail] = v;
            tail = (tail + 1) % size;
            return 0;
        }
        return -1;
    }

    Vector pop()
    {
        Vector elem = contents[head];
        head = (head + 1) % size;
        return elem;
    }

    Vector next()
    {
        Vector elem = contents[current];
        if(head + tail > size) {
            current = (current + 1) % (size - head + tail);
        }
        else {
            current = (current + 1) % (head - tail);
        }
        return elem;
    }

    bool isAvailable(int i)
    {
        for(int j = 0; j <= i; j++) {
            if((tail + 1) % size == head) {
                return false;
            }
        }
        return true;
    }

    int size;

private:
    int current;
    int head;
    int tail;
    Vector contents[CircleBufferSize];
};

#endif // CIRCLEBUFFER_H

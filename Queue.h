#ifndef QUEUE_H
#define QUEUE_H

#include <WProgram.h>

typedef struct {
  us8 exit;
  char side;
} SortPosition;

#define QueueSize 20
#define QueueType SortPosition
#define QueueTypeInit { 255, 'C' }

typedef enum {
  qeNONE = 0,
  qeOUT_OF_RANGE = 1
} QueueError;

class Queue{
public:
  QueueType table[QueueSize];
  us8 head;
  us8 tail;

  Queue() {
    clear();
  }

  inline void index_rollover(us8 &index) {
    index++;
    if(index >= QueueSize) {
      index = 0;
    }
  }

  inline us8 size() {
    return QueueSize;
  }

  inline us8 count() {
    if( head >= tail ) {
      return (head - tail);
    }
    else {
      return (head + QueueSize - tail);
    }
  }

  void remove(us8 index) {
      if(index < QueueSize) {
          table[index] = QueueTypeInit;
      }
  }

  void clear() {
    for(us8 i = 0; i < QueueSize; i++) {
      remove(i);
    }
    head = 0;
    tail = 0;
  }

  QueueType at(us8 index) {
    QueueType rval = QueueTypeInit;
    if(index < QueueSize) {
      return table[index];
    }
    return rval;
  }

  QueueError push(QueueType position) {
    if( this->count() >= (QueueSize-1) ) {
      return qeOUT_OF_RANGE;
    }

    table[head] = position;
    index_rollover(head);
    return qeNONE;
  }

  QueueType pop() {
    QueueType temp = QueueTypeInit;
    temp = at(tail);
    remove(tail);
    index_rollover(tail);
    return temp;
  }
};

#endif // QUEUE_H

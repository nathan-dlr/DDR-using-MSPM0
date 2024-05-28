// FIFO2.cpp
// Runs on any microcontroller
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size.  The file includes a transmit FIFO
// using index implementation and a receive FIFO using pointer
// implementation.  Other index or pointer implementation FIFOs can be
// created using the macros supplied at the end of the file.
// Created: 1/16/2020 
// Student names: Nathan DeLaRosa, Thompson Truong
//Chiou
// Last modification date: change this to the last modification date or look very silly


#include <stdint.h>
#include <stdio.h>  //added this
#include "../inc/FIFO2.h"
#include "../inc/ST7735.h"


// A class named Queue that defines a FIFO
Queue::Queue(){
  // Constructor - set PutI and GetI as 0. 
  // We are assuming that for an empty Queue, both PutI and GetI will be equal

// add code here to initialize on creation
    PutI = GetI = 0;
    full = false;
    empty = false;
}

// To check whether Queue is empty or not
bool Queue::IsEmpty(void){
  if (empty) {
      return true;
  }
  else {
      return false;
  }
}

  // To check whether Queue is full or not
bool Queue::IsFull(void){
    //full if the producer is 1 behind the consumer or if producer is at the last index and consumer is at the first
  if (full) {
      return true;
  }
  else {
      return false;
  }
}

  // Inserts an element in queue at rear end
bool Queue::Put(char x){
  //we can only put if not full
    if (IsFull()) {
        return false;
    }
    else {
        Buf[PutI] = x;
        PutI = (PutI + 1) % FIFOSIZE;
        if (PutI == GetI) {
            full = true;
        }
        empty = false;
        return true;
    }
}

  // Removes an element in Queue from front end. 
bool Queue::Get(char *pt){
  if (PutI == GetI) {
      empty = true;
      full = false;
  }
  if (IsEmpty()) {
      return false;
  }
  else {
      *pt = Buf[GetI];
      GetI = (GetI + 1) % FIFOSIZE;
      full = false;

      return true;
  }
}

  /* 
     Printing the elements in queue from front to rear. 
     This function is only to test the code. 
     This is not a standard function for Queue implementation. 
  */
void Queue::Print(void){
    // Finding number of elements in queue
    int x;
    x = GetI;
    for (int i = 0; i < FIFOSIZE; i++) {

        if (i%5 ==0) {
            printf("\n");
        }
        printf("%d,", Buf[x]);
        x = (x + 1) % FIFOSIZE;
    }
    printf("\n");
  // output to ST7735R
}



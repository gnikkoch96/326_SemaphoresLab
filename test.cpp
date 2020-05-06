//Steven Centeno
//Nikko Chan

//related Programs:
//semaphore.h
//shipment.o

//this program is meant to:
// have a process that acts like a online store for tennis supplies.
//Customers purchase items and if its out of stock,
//it waits to fulfil the order until it is restocked.

//this works in 3 unique processes
// I. the parent process acts as an inventory software it:
//      takes orders, fulfills orders when in stock, notifies custtomers
//      and stores all the information on the orders in an array by chronological
//      order of being fulfilled
//
// II. 10 child processes spawn from the parent process and they:
//      go through a for loop of 100 to make 100 orders each. Each order
//      has 1 overgrip and the other four items are determined by random numbers
//      which can be 1 or 0 items ordered. Each order made must wait until notified
//      that the order has been shipped before going to the next order out of 100
//
// III. an 11th child process is made to run the shipment_arrival function. This function returns
//      an array of 5 integers determining the number of overgrips, racket strings, tennis shoes
//      tennis balls and tennis accessories respectively. This is used to update the inventory


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "semaphore.h"
#include "shipment.h"

using namespace std;


//There can only be a max of 100 orders
const int MAX_ORDERS = 100;


//Semaphore Variables
enum {SEM1,SEM2,SEM3};


int main(){
    //Creation of Semaphores (and initialization)
    SEMAPHORE sem(3);
	int children_count = 0;
	//Child Creation
	int num_of_children = 2;

	int pid = 1;
    if(pid){
        for(int i = 0; i < num_of_children; i++){
		++children_count;
            pid = fork();
            if(pid == 0){ //Process Two
                if(children_count == 1){
                    sem.V(SEM2);
                    sem.V(SEM2);
                    sem.P(SEM1);
                    sem.P(SEM3);

                    cout << "Process 2 Finished" << endl;
                }else if(children_count == 2){ //Process Three
                    //sem.V(SEM3);
                    sem.V(SEM3);
                    sem.P(SEM1);
                    sem.P(SEM2);
                    cout << "Process 3 Finished" << endl;
                }
            }else if (pid < 0){
                cout << "Spawn Error - No Child Process was Created" << endl;
                break;
            }
        }

        if(pid){ //Process One
            sem.V(SEM1);
            sem.V(SEM1);
            sem.P(SEM2);
            sem.P(SEM3);

            cout << "Process 1 Finished" << endl;
        }
    }

	//Process gets Terminated
	exit(0);
}






#include <iostream>
#include <time.h>
#include <stdlib.h>

using namespace std;


//There can only be a max of 100 orders
const int MAXCHAR = 100;

//Display 5 indexes at a time
const int MAX_ITEMS = 5;

//Semaphore Variables
enum {OVER_GRIPS, RACKET_STRINGS, TENNIS_SHOES, TENNIS_BALLS, TENNIS_ACC, FFL_ORDER, WAIT_NOTIFICATION};
/*
    1. 1 - 5: Merchandise Items
    2. 6: Used to resume the fulfillment process from the customer...it makes the order after going through all levels
    3. 7:
*/
//Function Processes
void fulfillment_proc(SEMAPHORE &, int *);
void parent_cleanup(SEMAPHORE &, int);
void customer_proc(SEMAPHORE &, int *, int customer_id);
void supplier_proc(SEMAPHORE &, int *);

//Used to find the indexes to store order depending on the customer-id
int find_min_index(int);
int find_max_index(int);

//Finds the first index where
int find_open_order_index(int *);

int main(){
    //Random
    srand(time(NULL));

    //Stores the id of the shared memory (used as reference)
    int shmid;

    //Points to data in the shared memory
    int *shmITEM;

    //Customer ID (increments for each fork())
    int customer_id = 0;

    //Creation of Semaphores (and initialization)
    SEMAPHORE sem(6);

    //Creation of Shared Memory
    shmid = shmget(IPC_PRIVATE, (6 * 11)*sizeof(int), PERMS);
	shmITEM = (int *)shmat(shmid, 0, SHM_RND);


	//Initialize Inventory Values (not customer_id = 0) (I'm thinking we initialize here because we can't assume fulfillment process to execute faster than customer)
    int data[] = {0, 15, 15, 15, 15, 15};

    //Store Data in Shared Memory
    for(int i = 0; i < 6; i++){
        *(shmITEM + i) += data[i];
    }

    for(int i = 0; i < 15; i++){
        sem.V(ORDER_GRIPS);
        sem.V(RACKET_STRINGS);
        sem.V(TENNIS_SHOES);
        sem.V(TENNIS_BALLS);
        sem.V(TENNIS_ACC);
    }

	//Child Creation
	int num_of_children = 11;

	int pid = 1;
    if(pid){//Parent Process
        for(int i = 0; i < num_of_children; i++){
            customer_id++;
            pid = fork();
            if(pid == 0 && cusomter_id == 11){        //11th Process
                //Supplier Process
                suppler_proc(sem, shmITEM);
                break;
            }else if(pid == 0){  //Child Processes
                //Customer Process
                customer_proc(sem, shmITEM);
                break;
            }else{  //Couldn't Spawn Child
                cout << "Spawn Error - No Child Process was Created" << endl;
                break;
            }
        }

        //Fulfillment Process
        fulfillment_proc(sem, shmITEM);

        //Cleanup Process
        parent_cleanup(sem, shmITEM);
    }
	//Process gets Terminated
	exit(0);
}

void fulfillment_proc(SEMAPHORE &sem, int *shmITEM){
   for(int i = 0; i < 10 * MAX_ORDERS; i++){
        //Waits for a customer to make an order
       sem.P(FFL_ORDER);

        //Find index
        int min_index = find_open_order_index(shmITEM);

        //Order goes through, check through shared memory for order
        int customer_id = *(shmITEM + min_index);                           //Stores Customer Order

        //Resets customer-id index in shared memory to 0 for next order
        *(shmITEM + min_index) = 0;

        if(*(shmITEM + min_index + 1) == 1){        //Over grips
            *(shmITEM + 1) -= 1;
        }

        if(*(shmITEM + min_index + 2) == 1){        //Racket Strings
            *(shmITEM + 2) -= 1;
        }

        if(*(shmITEM + min_index + 1) == 1){        //Tennis Shoes
            *(shmITEM + 3) -= 1;
        }

        if(*(shmITEM + min_index + 1) == 1){        //Tennis Balls
            *(shmITEM + 4) -= 1;
        }

        if(*(shmITEM + min_index + 1) == 1){        //Tennis acc
            *(shmITEM + 5) -= 1;
        }

        //Print out order for customer
        cout << "ORDER ====" << endl;
        cout << "Customer ID: " << customer_id << endl;
        cout << "Order Grips: " << *(shmITEM + min_index + 1) << endl;
        cout << "Racket Strings: " << *(shmITEM + min_index + 2) << endl;
        cout << "Tennis Shoes: " << *(shmITEM + min_index + 3) << endl;
        cout << "Tennis Balls: " << *(shmITEM + min_index + 4) << endl;
        cout << "Tennis Acc: " << *(shmITEM + min_index + 5) << endl;

        sem.V(WAIT_NOTIFICATION);
   }


}

void supplier_proc(SEMAPHORE &sem, int *shmITEM){
    //Size of six for customer id
    int *restock = (int) malloc(6 * sizeof(int));

    while(true){
        //Store Restock Information in restock
        shipment_arrival(restock);

        //Update Shared Memory
        for(int i = 1; i < 6; i++){
            *(shmITEM + i) = restock[i];
        }

        //Restock Semaphores
        for(int i = 0; i < restock[1]; i++){                                //Increments Order Grips
            sem.V(ORDER_GRIPS);
        }

        for(int i = 0; i < restock[2]; i++){                                //Increments Order Grips
            sem.V(RACKET_STRINGS);
        }

        for(int i = 0; i < restock[3]; i++){                                //Increments Order Grips
            sem.V(TENNIS_SHOES);
        }

        for(int i = 0; i < restock[4]; i++){                                //Increments Order Grips
            sem.V(TENNIS_BALLS);
        }

        for(int i = 0; i < restock[5]; i++){                                //Increments Order Grips
            sem.V(TENNIS_ACC);
        }
    }

}


void customer_proc(SEMAPHORE &sem, int *shmITEM, int customer_id){
    //Find the corresponding indexes to store order in
    int min_index = find_min_index(customer_id);
    int max_index = find_max_index(customer_id);

    //Customer Order
    //0 - Customer ID, 1 - 5 -> Merchandise Order Value
    int data[] = new int[6];
    data[0] = customer_id;

    //Always order Over_grips requirement
    data[1] = 1;

    for(int i = 0; i < MAX_ORDERS; i++){
        //Generating Order
        for(int i = 2; i < 6; i++){
            data[i] = rand() % 2;
        }

        sem.P(OVER_GRIPS);

        //Waits for item if they ordered it (i.e. have a value of 1)
        if(data[2] == 1)
            sem.P(RACKET_STRINGS);
        if(data[3] == 1)
            sem.P(TENNIS_SHOES);
        if(data[4] == 1)
            sem.P(TENNIS_BALLS);
        if(data[5] == 1)
            sem.P(TENNIS_ACC);

        //Reaching this part means that the customer could order the items (Storing Customer Order)
        *(shmITEM + min_index) = customer_id;
        for(int i = 1; i < 6; i++){
            *(shmITEM + min_index + i) = data[i];
        }

        sem.V(FFL_ORDER);
        sem.P(WAIT_NOTIFICATION);
    }
}

int find_open_order_index(int *shmITEM){
    if(*(shmITEM + 6) == 1)         //Customer One made an order
        return 6;
    else if(*(shmITEM + 12) == 2)   //Customer Two made an order
        return 12;
    else if(*(shmITEM + 18) == 3)   //Customer Three made an order
        return 18;
    else if(*(shmITEM + 24) == 4)   //Customer Four made an order
        return 24;
    else if(*(shmITEM + 30) == 5)   //Customer Five made an order
        return 30;
    else if(*(shmITEM + 36) == 6)   //Customer Six made an order
        return 36;
    else if(*(shmITEM + 42) == 7)   //Customer Seven made an order
        return 42;
    else if(*(shmITEM + 48) == 8)   //Customer Eight made an order
        return 48;
    else if(*(shmITEM + 54) == 9)   //Customer Nine made an order
        return 54;
    else                            //Customer Ten made an order
        return 60;

}

int find_min_index(int customer_id){
    switch(customer_id){
        case 1: return 6;
        case 2: return 12;
        case 3: return 18;
        case 4: return 24;
        case 5: return 30;
        case 6: return 36;
        case 7: return 42;
        case 8: return 48;
        case 9: return 54;
        case 10; return 60;
    }
}

int find_max_index(int customer_id){
    switch(customer_id){
        case 1: return 11;
        case 2: return 17;
        case 3: return 23;
        case 4: return 29;
        case 5: return 35;
        case 6: return 41;
        case 7: return 47;
        case 8: return 53;
        case 9: return 59;
        case 10; return 65;
    }
}


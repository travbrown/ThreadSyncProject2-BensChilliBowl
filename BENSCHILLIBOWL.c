#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    return BENSCHILLIBOWLMenu[rand() % BENSCHILLIBOWLMenuLength];
}

/* Allocate memory for the Restaurant, 
 * then create the mutex and condition variables needed 
 * to instantiate the Restaurant */
BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
    // Allocate memory for the Restaurant.
    BENSCHILLIBOWL *bcb = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
    bcb->orders = NULL;
    bcb->current_size = 0;
    bcb->max_size = max_size;
	  bcb->next_order_number = 1;
    bcb->orders_handled = 0;
    bcb->expected_num_orders = expected_num_orders;
  
    // Create the mutex and condition variables needed to instantiate the Restaurant.
    pthread_mutex_init(&(bcb->mutex), NULL);
    pthread_cond_init(&(bcb->can_add_orders), NULL);
    pthread_cond_init(&(bcb->can_get_orders), NULL);
  
    printf("Restaurant is open!\n");
    return bcb;
}

/* check that the number of orders received is equal to the number handled (ie.fullfilled). 
 * Remember to deallocate your resources 
 * */
void CloseRestaurant(BENSCHILLIBOWL* bcb) {
    // check that the number of orders received is equal to the number handled (ie.fullfilled).
    if (bcb->orders_handled != bcb->expected_num_orders) {
        fprintf(stderr, "Some of the orders were not handled.\n");
        exit(0);
    }
  
    // deallocate resources 
    pthread_mutex_destroy(&(bcb->mutex));
    free(bcb);
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
    // applying lock so that the order is synchronized
    pthread_mutex_lock(&(bcb->mutex)); 
  
    // the restaurant has reached maximum number of orders the restaurant can handle at the moment
    // wait until the queue can take more orders
    while (IsFull(bcb)) { 
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }
    
    // add order to rear of queue 
    order->order_number = bcb->next_order_number;
    AddOrderToBack(&(bcb->orders), order);
  
    // update next order number and current size
    bcb->next_order_number += 1; 
    bcb->current_size += 1;
    
    // notify the process that it can get orders now
    pthread_cond_broadcast(&(bcb->can_get_orders));
  
    // release lock
    pthread_mutex_unlock(&(bcb->mutex));
    
    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
    pthread_mutex_lock(&(bcb->mutex));
    
    // wait until the restaurant has any order to handle at the moment
    while(IsEmpty(bcb)) { 
        // if expected number of orders have been fulfilled, 
        // unlock the mutex and return null (no more order will be taken).
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }
    
    // get order from queue(FIFO).
    Order *order = bcb->orders;
    bcb->orders = bcb->orders->next;
    
    // update the current order size and orders handled
    bcb->current_size -= 1; 
    bcb->orders_handled += 1;
    
    // notify the process that it add get orders now
    pthread_cond_broadcast(&(bcb->can_add_orders));
        
    // release the lock.
    pthread_mutex_unlock(&(bcb->mutex));   
    return order;
}

/* helper function to determine if there are any order available or not */
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  if (bcb->current_size == 0){
    return true;
  }
  else{
    return false;
  }
}

/* helper function to determine if the numbers of orders available 
 * has reached maximum capacity */
bool IsFull(BENSCHILLIBOWL* bcb) {
  if (bcb->current_size == bcb->max_size){
    return true;
  }
  else{
    return false;
  }
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) {
      *orders = order;
  } 
  else {
      Order *curr_order = *orders;
      while (curr_order->next) {
          curr_order = curr_order->next;
      }
      curr_order->next = order;
    }
}
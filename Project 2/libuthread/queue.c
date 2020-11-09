#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "queue.h"

#define ERROR -1; // define error value.
#define SUCCESS 0; // define success value
/**
 * Linked-List Node structure to store each data in queue
 * @data: item data in each node
 * @next: node pointer points to next node
 */
struct node{
    void* data;
    struct node* next;
};

/**
 * Queue implemented by linked-list
 * @head: head pointer points to the first element in queue
 * @tail: tail pointed points to the last element in queue
 * @length: queue length
 */
struct queue{
    struct node* head;
    struct node* tail;
    int length;
};

queue_t queue_create(void)
{
    // Allocate an empty queue
    struct queue* newQueue = malloc(sizeof(struct queue));
	
    // failure when allocating the new queue
    if(newQueue == NULL){
        return NULL;
    }

    // success allocate the new queue
    // make new queue empty
    newQueue->head = NULL;
    newQueue->tail = NULL;
    newQueue->length = 0;

    // return pointer to new empty queue
    return newQueue;
}

int queue_destroy(queue_t queue)
{
    // queue is NULL or is not empty
    if(queue == NULL || queue->length > 0){
        return ERROR;
    }
    // deallocate the memory associated to the queue object
    free(queue);
    return SUCCESS;


}

int queue_enqueue(queue_t queue, void *data)
{
    // if queue or data are NULL
    if(queue == NULL || data == NULL){
        return ERROR;
    }
	struct node* newNode = malloc(sizeof(struct node));
    // memory allocation error
    if(newNode == NULL){
        return ERROR;
    }
    newNode->data = data;
    newNode->next = NULL;
    
    if(queue->length == 0){//empty queue
        queue->head = newNode;
        queue->tail = newNode;
    } else {
        queue->tail->next = newNode;
        queue->tail = newNode;
    }
    queue->length++;
    return SUCCESS;
}

int queue_dequeue(queue_t queue, void **data)
{
    // if queue or data are NULL or if queue is empty
	if(queue == NULL || data == NULL || queue->length == 0){
        return ERROR;
    }
    struct node* oldest = malloc(sizeof(struct node));
    if(oldest == NULL){
        return ERROR;
    }
    oldest = queue->head;
    // assign the value of a pointer to data
    *data = oldest->data;
    // rearrange the queue
    if(queue->length == 1){ // original queue has only 1 element
        // set queue to empty
        queue->head = NULL;
        queue->tail = NULL;
    } else { // original queue has more than 1 element
        queue->head = queue->head->next;
    }
    // deduct 1 from queue length since dequeue 1 element
    queue->length--;
    free(oldest);
    return SUCCESS;
}


int queue_delete(queue_t queue, void *data)
{
	// queue ore data are NULL
    if(queue == NULL || data == NULL || queue->length == 0){
        return ERROR;
    }
    int found = 0;
    int queueLen = queue->length;
    struct node* previous = malloc(sizeof(struct node));
    struct node* current = queue->head;
    previous = NULL;
    for(int i = 0; i < queueLen; i++){
        if(current->data == data){
            found = 1;
            // goal data is the head of the queue
            if(i == 0){
                if(queue->length == 1){ // only one element in the queue
                    queue->length--;
                    queue->head = NULL;
                    queue->tail = NULL;
                    free(current); // deallocate the goal data in queue
                
                } else if(queue->length > 1) { // more than one elements in the queue
                    queue->length--;
                    
                    current = queue->head->next;// deallocate the goal data in queueQ
                    queue->head = NULL;
                    queue->head = current;
                }
            } else { // goal data is not the head of the queue
                queue->length--;
                previous->next = current->next;
                free(current); // deallocate the goal data in queue
            }
            break; // only delete the first match one
        } else { // goal data not found
            previous = current;
            current = current->next;
            previous->next = current;
        }
    }
    if(found == 0){ // data was not found
        return ERROR;
    }
    return SUCCESS;

}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if(queue == NULL || func == NULL || queue->length == 0){
        return ERROR;
    }
    struct node* current = queue->head;
    struct node* temp = malloc(sizeof(struct node));
    while(current != NULL){
        temp = current;
        current = current->next;
        func(temp->data);
    }
    temp = NULL;
    free(temp);
    return SUCCESS;



}

int queue_length(queue_t queue)
{
	if(queue == NULL){
        return ERROR;
    }
    int queueLen;
    queueLen = queue->length;
    return queueLen;
}
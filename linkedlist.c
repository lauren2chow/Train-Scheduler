#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"

struct train* createTrain(int train_num, char direction, int load_time, int cross_time){

	struct train *newTrain = (struct train *)malloc(sizeof(struct train));

	newTrain->number = train_num;
    newTrain->priority = direction;
    newTrain->load_time = load_time;
    newTrain->cross_time = cross_time;

	newTrain->next = NULL;
	return newTrain;
}

struct train* push(struct train *head, struct train *newTrain){
    if(head == NULL){
        head = newTrain;
        head->next = NULL;
    }else{
        struct train *temp_train = head;
        while(temp_train->next != NULL){
            temp_train = temp_train->next;
        }
        temp_train->next = newTrain;
        newTrain->next = NULL;
    }
    return head;
}
struct train* pop(struct train *head){
    if(head != NULL){
        struct train *temp_train = head;
        head = head->next;
        free(temp_train);
    }
    return head;
}

void pop_by_number(struct train *head, struct train *train_node){
    struct train *prev = head;
    struct train *curr = head;
    while(curr != train_node){
        prev = curr;
        curr = curr->next;
    }
    prev->next = curr->next;
    free(curr);
}

int isEmpty(struct train* head){
    return (head == NULL); 
}
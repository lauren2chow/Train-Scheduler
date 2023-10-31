#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "linkedlist.h"
#include <time.h>


pthread_mutex_t LOAD_MUTEX;
pthread_cond_t LOAD_SIGNAL;
pthread_mutex_t RUN_MUTEX;
pthread_cond_t RUN_SIGNAL;


int TOTAL_TRAINS = 0;
int TRAIN_LOADING = 0;

struct train *EH_HEAD = NULL;
struct train *EL_HEAD = NULL;
struct train *WH_HEAD = NULL;
struct train *WL_HEAD = NULL;

int EAST_COUNTER = 0;
int WEST_COUNTER = 0;

struct timespec start = {0}, mark;
pthread_t *RUN_TRAIN_ARRAY;
int LOAD_LOCK = 0;
int LOCK = 0;
int PUSHED = 0;

double get_time(int* time_array){
    /*
	* Gets the time from when the trains start loading to the current time.
    * Returns the seconds as a double and updates the time_array to contain hours and minutes.
	*/
    double time_taken,sec;
    int hour = 0, min = 0;
    time_taken = (mark.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (mark.tv_nsec - start.tv_nsec)) * 1e-9;
    hour = (time_taken/3600);
    min = (time_taken - (3600*hour))/60;
    sec = (time_taken - (3600*hour) - (min*60));
    time_array[0] = hour;
    time_array[1] = min;
    return sec;
}

void pushTrain(struct train *train_node){
    /*
	* Checks the direction that the given train node is going and the 
    * priority adds it to the end of the corresponding queue.
	*/
    switch(train_node->priority){
        case 'E':
            strcpy(train_node->station, "East");
            EH_HEAD = push(EH_HEAD, train_node);
            break;
        case 'e':
            strcpy(train_node->station, "East");
            EL_HEAD = push(EL_HEAD, train_node);
            break;
        case 'W':
            strcpy(train_node->station, "West");
            WH_HEAD = push(WH_HEAD, train_node);
            break;
        case 'w':
            strcpy(train_node->station, "West");
            WL_HEAD = push(WL_HEAD, train_node);
    }
}

void* loadtrain(void * train_node){
    /*
	* A thread that waits for the broadcast which signals that all the trains are 
    * waiting to load. Then starts the time and loads the given train accounding 
    * to the load time. Prints a message saying the train is ready to go and the 
    * time after the train finishes loading.
	*/
    struct train *trainptr = (struct train *)train_node;
    int time_array[2];
    double sec;

    pthread_mutex_lock(&LOAD_MUTEX);
    while(LOAD_LOCK != 1){
        TRAIN_LOADING ++;
        pthread_cond_wait(&LOAD_SIGNAL, &LOAD_MUTEX);
    }
    pthread_mutex_unlock(&LOAD_MUTEX);

    usleep(trainptr->load_time*100000);
    if(clock_gettime(CLOCK_MONOTONIC, &mark) == -1){
        perror("clock gettime\n");
        exit(EXIT_FAILURE);
    }
    sec = get_time(time_array);
    pushTrain(trainptr);
    printf("%02d:%02d:%04.1f Train %2d is ready to go %4s\n", time_array[0], time_array[1], 
    sec, trainptr->number, trainptr->station);
    TRAIN_LOADING -= 1;
    PUSHED = 1;
    
    
    return NULL;
}

void wait_for_trains(struct train *head){
    /*
	* Creates a thread for each train in the linked list head. Once all the threads
    * are waiting a broadcast signal will be sent to start loading the trains. The
    * time will also be started. 
	*/
    pthread_t wait_train_array[TOTAL_TRAINS];
    struct train *temp_train = head;

    while(temp_train != NULL){
        pthread_create(&wait_train_array[temp_train->number], NULL, &loadtrain, (void *) temp_train);
        temp_train = temp_train->next;
    }
    while(1){
        if(TOTAL_TRAINS == TRAIN_LOADING){
            pthread_mutex_lock(&LOAD_MUTEX);
            LOAD_LOCK = 1;
            pthread_mutex_unlock(&LOAD_MUTEX);
            pthread_cond_broadcast(&LOAD_SIGNAL);
            if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
                perror("clock gettime\n");
                exit(EXIT_FAILURE);
            }
            free(temp_train);
            return;
        }
        
    }
    
}

void* run_train(void * train_node){
    /*
	* A thread will run train_node priting when it starts and finishes running.
	*/
    int time_array[2];
    double sec;
    struct train *trainptr = (struct train *)train_node;
    
    if(clock_gettime(CLOCK_MONOTONIC, &mark) == -1){
        perror("clock gettime\n");
        exit(EXIT_FAILURE);
    }
    sec = get_time(time_array);
    printf("%02d:%02d:%04.1f Train %2d is ON the main track going %4s\n", time_array[0], time_array[1], 
    sec, trainptr->number,trainptr->station);

    usleep(trainptr->cross_time*100000);

    if(clock_gettime(CLOCK_MONOTONIC, &mark) == -1){
        perror("clock gettime\n");
        exit(EXIT_FAILURE);
    }
    sec = get_time(time_array);
    printf("%02d:%02d:%04.1f Train %2d is OFF the main track after going %4s\n", time_array[0], time_array[1], 
    sec, trainptr->number, trainptr->station);
    
    return NULL;
}

struct train* check_print_order(struct train* head){
    /*
    * If there are multiple trains in the given queue with the same load time it chooses
    * the smallest train number (the train that is writen first in the input file) and returns it.
    */
    if(head->next == NULL){
        return head;
    }
    struct train *temp_train = head->next;
    struct train *temp_high = head;
    while((temp_train != NULL) && (temp_train->load_time == head->load_time)){
        if(temp_train->number < temp_high->number){
            temp_high = temp_train;
        }
        temp_train = temp_train->next;
    }
    return temp_high;
}

void dispatch_east(){
    /*
	* Creates a train thread going east by calling the function run_train, first 
    * checking the high priority queue EH_HEAD, if it is empty then it will dispatch 
    * a train from the low priority queue EL_HEAD.
	*/
    WEST_COUNTER = 0;
    EAST_COUNTER += 1;
    struct train *temp_train;

    if(EH_HEAD != NULL){
        temp_train = check_print_order(EH_HEAD);
        pthread_create(&RUN_TRAIN_ARRAY[EH_HEAD->number], NULL, &run_train, (void *) temp_train);
        if (pthread_join(RUN_TRAIN_ARRAY[EH_HEAD->number], NULL) != 0) {
            perror("Failed to join thread");
        }
        if(temp_train != EH_HEAD){
            pop_by_number(EH_HEAD, temp_train);
        }
        else{
            EH_HEAD = pop(EH_HEAD);
        }
    }else{
        temp_train = check_print_order(EL_HEAD);
        pthread_create(&RUN_TRAIN_ARRAY[EL_HEAD->number], NULL, &run_train, (void *) temp_train);
        if (pthread_join(RUN_TRAIN_ARRAY[EL_HEAD->number], NULL) != 0) {
            perror("Failed to join thread");
        }
        if(temp_train != EL_HEAD){
            pop_by_number(EL_HEAD, temp_train);
        }
        else{
            EL_HEAD = pop(EL_HEAD);
        }
    }
    LOCK = 0;
    pthread_mutex_unlock(&RUN_MUTEX);
    pthread_cond_signal(&RUN_SIGNAL);
}

void dispatch_west(){
    /*
	* Creates a train thread going west by calling the function run_train, first 
    * checking the high priority queue WH_HEAD, if it is empty then it will dispatch 
    * a train from the low priority queue WL_HEAD.
    * Unlocks the RUN_MUTEX and sends a signal after the train is finished running
	*/
    EAST_COUNTER = 0;
    WEST_COUNTER += 1;
    struct train *temp_train;

    if(WH_HEAD != NULL){
        temp_train = check_print_order(WH_HEAD);
        pthread_create(&RUN_TRAIN_ARRAY[WH_HEAD->number], NULL, &run_train, (void *) temp_train);
        if (pthread_join(RUN_TRAIN_ARRAY[WH_HEAD->number], NULL) != 0) {
            perror("Failed to join thread");
        }
        if(temp_train != WH_HEAD){
            pop_by_number(WH_HEAD, temp_train);
        }
        else{
            WH_HEAD = pop(WH_HEAD);
        }
    }else{
        temp_train = check_print_order(WL_HEAD);
        pthread_create(&RUN_TRAIN_ARRAY[WL_HEAD->number], NULL, &run_train, (void *) temp_train);
        if (pthread_join(RUN_TRAIN_ARRAY[WL_HEAD->number], NULL) != 0) {
            perror("Failed to join thread");
        }
        if(temp_train != WL_HEAD){
            pop_by_number(WL_HEAD, temp_train);
        }
        else{
            WL_HEAD = pop(WL_HEAD);
        }
    }
    LOCK = 0;
    pthread_mutex_unlock(&RUN_MUTEX);
    pthread_cond_signal(&RUN_SIGNAL);
}

void check_west(){
    /*
	* Checks if there are any trains in the West High and West Low priority queue. If
    * not empty then dispatches a train from the west else dispatches a train east.
	*/
    if(isEmpty(WH_HEAD)!=1 || isEmpty(WL_HEAD)!=1){
        dispatch_west();
    }else{
        dispatch_east();
    }
}

void check_east(){
    /*
	* Checks if there are any trains in the East High and East Low priority queue. If
    * not empty then dispatches a train from the east else dispatches a train west.
	*/
    if(isEmpty(EH_HEAD)!=1 || isEmpty(EL_HEAD)!=1){
        dispatch_east();
    }else{
        dispatch_west();
    }
}

int all_queues_empty(){
    /*
	* Checks all four queues are empty and returning a boolean.
	*/
    if(isEmpty(EH_HEAD)==1 && isEmpty(EL_HEAD)==1 && isEmpty(WL_HEAD)==1 && isEmpty(WH_HEAD)==1){
        return 1;
    }
    return 0;
}

void track_dispatcher(){
    /*
	* Decides which train is dispatched next. 
    * If there are more than three trains that run concurrently in the same direction a train from the other
    * direction will run if there is one.
    * Runs high priority trains first than the low priority trains.
    * If both EAST and WEST queues with the same priority have ready trains it will run the train opposite of 
    * the last run train or WEST if no trains have run yet.
	*/
    if(all_queues_empty()){
        LOCK = 0;
        pthread_mutex_unlock(&RUN_MUTEX);
        return;
    }
    
    if(EAST_COUNTER >= 3){
        check_west();
        return;
    }else if(WEST_COUNTER >= 3){
        check_east();
        return;
    }

    if(isEmpty(EH_HEAD)!=1 && isEmpty(WH_HEAD)!=1){  //If both EAST and WEST have ready high priority trains
        if(EAST_COUNTER== 0 && WEST_COUNTER == 0){      //If no trains have crossed the main track
            dispatch_west();
        }else if(EAST_COUNTER == 0){                    //If west crossed last
            dispatch_east();
        }else{                                          //If east crossed last
            dispatch_west();
        }
        return;
    }

    
    if(isEmpty(EH_HEAD)!=1){
        dispatch_east();
        return;
    }else if(isEmpty(WH_HEAD)!=1){
        dispatch_west();
        return;
    }

    if(isEmpty(EL_HEAD)!=1 && isEmpty(WL_HEAD)!=1){  //If both EAST and WEST have ready low priority trains
        if(EAST_COUNTER == 0 && WEST_COUNTER == 0){     //If no trains have crossed the main track
            dispatch_west();
        }else if(EAST_COUNTER == 0){                    //If west crossed last
            dispatch_east();
        }else{                                          //If east crossed last
            dispatch_west();
        }
        return;
    }

    if(isEmpty(EL_HEAD)!=1){
        dispatch_east();
        return;
    }else if(isEmpty(WL_HEAD)!=1){
        dispatch_west();
        return;
    }
}

void track_scheduler(){
    /*
	* Locks the RUN_MUTEX allowing only one train to be dispatched and run at a time
	*/
    while(!all_queues_empty() || TRAIN_LOADING != 0){
        if(PUSHED == 1){
            pthread_mutex_lock(&RUN_MUTEX);
            while(LOCK == 1){
                pthread_cond_wait(&RUN_SIGNAL, &RUN_MUTEX);
            }
            LOCK = 1;
            track_dispatcher();
        }
    }
}


int main(int argc, char **argv){
    /*
	* Initallizes and destroys all mutexes and signals. Reads from file given
    * turning each line into a train node that stores the station, direction, 
    * load and cross time of the train. The node is then added to the all
    * trains linked list which is then sent to the wait_for_trains function
    * which waits for all the trains and loads them. After track_scheduler is
    * called to shedule and dispatch the all the trains. 
	*/
    pthread_mutex_init(&LOAD_MUTEX, NULL);
    pthread_cond_init(&LOAD_SIGNAL, NULL);
    pthread_mutex_init(&RUN_MUTEX, NULL);
    pthread_cond_init(&RUN_SIGNAL, NULL);

    char direction[1];
    int load_time;
    int cross_time;
    int train_num = 0;

    struct train *all_trains = NULL;

    FILE *ptr = fopen(argv[1], "r");
    if (ptr == NULL){
        printf("Error can't open file\n");
        exit(1);
    } 

    struct train *temp_train;
    while(fscanf(ptr, "%s %d %d", direction, &load_time, &cross_time)== 3){
        temp_train = createTrain(train_num, direction[0], load_time, cross_time);
        all_trains = push(all_trains, temp_train);
        train_num++;
    }
    
    TOTAL_TRAINS = train_num;

    wait_for_trains(all_trains);

    RUN_TRAIN_ARRAY = (pthread_t*)calloc(TOTAL_TRAINS,sizeof(pthread_t));

    track_scheduler();

    pthread_mutex_destroy(&LOAD_MUTEX);
    pthread_cond_destroy(&LOAD_SIGNAL);
    pthread_mutex_destroy(&RUN_MUTEX);
    pthread_cond_destroy(&RUN_SIGNAL);

    free(RUN_TRAIN_ARRAY);
    fclose(ptr);
    return 0;
}
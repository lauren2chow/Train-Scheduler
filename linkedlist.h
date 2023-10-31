struct train{
    int number;
    char station[4];
	char priority;
    int load_time;
    int cross_time;

	struct train* next;
};

struct train* createTrain(int train_num, char direction, int load_time, int cross_time); /*Creates and returns an new node*/
struct train* push(struct train *head, struct train *newTrain); /*Inserts the given train to the end of the queue*/
struct train* pop(struct train *head); /*removes the first train from the linked list*/
void pop_by_number(struct train *head, struct train *train_node);
int isEmpty(struct train* head); /*checks to see if the priority queue is empty*/
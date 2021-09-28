#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int MAX_CUSTOMER_TIME;
int MAX_CASHIER_QUEUE;
int MAX_CASHIERS;
int MAX_NEXT_CUSTOMERS;

typedef struct Person {
	char name;
	int time;
} Person;

typedef struct QueueNode {
	Person *person;
	struct QueueNode *next;
} QueueNode;

typedef struct Queue {
	QueueNode *head;
	QueueNode *tail;
	int size;
} Queue;

typedef struct Cashier {
	bool work;
	int served;
	Queue *queue;
} Cashier;

bool loadConfig() {
	FILE *configFile = fopen("settings.txt", "r");
	if (configFile == NULL) {
		return false;
	}
	int config[4];	  // tmp array for settings value
	char setting[32]; // string for scan
	for (int i = 0; i < 4; ++i) {
		// split before "="
		char *key = strtok(fgets(setting, sizeof(setting), configFile), "=");
		// split before ";"
		config[i] = atoi(strtok(NULL, ";"));
	}
	MAX_CUSTOMER_TIME = config[0];
	MAX_CASHIER_QUEUE = config[1];
	MAX_CASHIERS = config[2];
	MAX_NEXT_CUSTOMERS = config[3];
	return true;
}

Person *newCustomer() {
	Person *person = (Person *)malloc(sizeof(Person));
	person->name = (char)((rand() % 26) + 97);
	person->time = rand() % MAX_CUSTOMER_TIME + 1;
	return person;
}

QueueNode *newNode(Person *p) {
	QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
	node->person = p;
	node->next = NULL;
	return node;
}

Queue *newQueue() {
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;
	return queue;
}

Cashier *newCashdesk() {
	Cashier *cashdesk = (Cashier *)malloc(sizeof(Cashier));
	cashdesk->queue = newQueue();
	cashdesk->served = 0;
	cashdesk->work = false;
	return cashdesk;
}

void QueuePush(Queue *q, QueueNode *node) {
	if (!node)
		node = newNode(newCustomer());
	node->next = NULL;
	if (q->head) {
		q->tail->next = node;
		q->tail = node;
	} else
		q->head = q->tail = node;
	q->size++;
}

QueueNode *QueuePeek(Queue *queue) { return queue->head; }

QueueNode *QueuePop(Queue *queue) {
	QueueNode *node = QueuePeek(queue);
	if (node) {
		queue->head = queue->head->next;
		queue->size--;
		return node;
	} else
		return NULL;
}

void CashdeskQueuePop(Cashier *cashier) {
	QueueNode *customer = QueuePop(cashier->queue);
	cashier->served++;
	if (customer) {
		free(customer->person);
		free(customer);
	}
}

bool CashdeskPush(Cashier **cashdesks, QueueNode *node) {
	Cashier *maxCustomersCashdesk = NULL;
	for (int i = 0; i < MAX_CASHIERS; ++i) {
		if (cashdesks[i]->queue->size == MAX_CASHIER_QUEUE)
			continue;
		if (!maxCustomersCashdesk ||
			cashdesks[i]->served > maxCustomersCashdesk->served) {
			maxCustomersCashdesk = cashdesks[i];
		}
	}
	if (maxCustomersCashdesk) {
		maxCustomersCashdesk->work = true;
		QueuePush(maxCustomersCashdesk->queue, node);
	}
	return maxCustomersCashdesk == NULL;
}

bool CashdeskQueuePush(Cashier **cashdesks, Queue *nextCustomers) {
	bool GAMEOVER_FLAG = false;
	QueueNode *node = QueuePop(nextCustomers);
	while (node && !GAMEOVER_FLAG) {
		GAMEOVER_FLAG = CashdeskPush(cashdesks, node);
		node = QueuePop(nextCustomers);
	}
	return GAMEOVER_FLAG;
}

void CashdeskQueueDisplay(Cashier **cashdesks) {
	QueueNode **currentCustomers =
		(QueueNode **)malloc(MAX_CASHIERS * sizeof(QueueNode *));
	for (int i = 0; i < MAX_CASHIERS; ++i) {
		currentCustomers[i] = QueuePeek(cashdesks[i]->queue);
	}
	for (int i = 0; i < MAX_CASHIER_QUEUE; ++i) {
		for (int j = 0; j < MAX_CASHIERS; ++j) {
			if (currentCustomers[j]) {
				printf("\t%c%d", currentCustomers[j]->person->name,
					   currentCustomers[j]->person->time);
				currentCustomers[j] = currentCustomers[j]->next;
			} else
				printf("\t||");
		}
		if (i < MAX_CASHIER_QUEUE - 1)
			printf("\n");
	}
	free(currentCustomers);
}

void CashdeskServe(Cashier *cashier) {
	Person *customer;
	if (cashier->queue->head)
		customer = cashier->queue->head->person;
	else
		return;
	customer->time--;
	if (customer->time == 0)
		CashdeskQueuePop(cashier);
	if (cashier->queue->size == 0)
		cashier->work = false;
}

void QueueDisplay(Queue *q) {
	printf("\nСледующие посетители: ");
	QueueNode *node = QueuePeek(q);
	while (node) {
		printf("%c%d ", node->person->name, node->person->time);
		node = node->next;
	}
}

int QueueCustomersCountDisplay(Cashier **cashdesks) {
	int value = 0;
	for (int i = 0; i < MAX_CASHIERS; ++i) {
		value += cashdesks[i]->queue->size;
	}
	return value;
}

int CashdesksServedCustomersCountDisplay(Cashier **cashdesks) {
	int value = 0;
	for (int i = 0; i < MAX_CASHIERS; ++i) {
		value += cashdesks[i]->served;
	}
	return value;
}

int CashdesksWorkCountkDisplay(Cashier **cashdesks) {
	int value = 0;
	for (int i = 0; i < MAX_CASHIERS; ++i) {
		value += cashdesks[i]->work;
	}
	return value;
}

void renderInterface(int globalTime, Queue *nextCustomers,
					 Cashier **cashdesks) {
	system("clear");
	// display Remi
	printf("Супермаркет \"Реми\". Система моделирования очередей.\n");
	// display cashdesk number
	for (int cashiers = 1; cashiers <= MAX_CASHIERS; cashiers++)
		printf("\t%d", cashiers);
	printf("\n");
	// display cashdesk's served customers
	for (int cash = 0; cash < MAX_CASHIERS; cash++) {
		printf("\t%d", cashdesks[cash]->served);
	}
	printf("\n");
	// display cashdesk mood (on/off)
	for (int cash = 0; cash < MAX_CASHIERS; cash++) {
		printf("\t%c", cashdesks[cash]->work ? '+' : '-');
	}
	printf("\n");
	// display queues of cashdesks
	CashdeskQueueDisplay(cashdesks);
	printf("\nВремя: %d", globalTime);
	QueueDisplay(nextCustomers);
	printf("\nЧеловек в очередях: %d", QueueCustomersCountDisplay(cashdesks));
	printf("\nКасс работает: %d из %d", CashdesksWorkCountkDisplay(cashdesks),
		   MAX_CASHIERS);
	printf("\nВсего обслужено: %d",
		   CashdesksServedCustomersCountDisplay(cashdesks));
	printf("\nДопустимая очередь на кассу: %d\n", MAX_CASHIER_QUEUE);
}

int main() {
	srand(time(NULL)); // randomize seed
	// load config
	if (!loadConfig()) {
		printf("Poka Pedrila");
		exit(EXIT_FAILURE);
	}
	long long globalTime = 0; // global timer
	Cashier **cashdesks = (Cashier **)malloc(
		MAX_CASHIERS * sizeof(Cashier *)); // array of cashdesk
	Queue *nextCustomers = newQueue();	   // next customers queue

	for (int i = 0; i < MAX_CASHIERS; i++) {
		cashdesks[i] = newCashdesk();
	}

	while (true) {
		// Push to queue new next customers
		for (int i = 0; i < rand() % MAX_NEXT_CUSTOMERS + 1; ++i) {
			QueuePush(nextCustomers, NULL);
		}
		// display stats of Remi
		renderInterface(globalTime, nextCustomers, cashdesks);
		// cashdesk serve
		for (int i = 0; i < MAX_CASHIERS; ++i) {
			CashdeskServe(cashdesks[i]);
		}
		// distribution to the cashdesk queue
		if (CashdeskQueuePush(cashdesks, nextCustomers)) {
			renderInterface(globalTime, nextCustomers, cashdesks);
			printf("SUCK MY BALLS!");
			break;
		}

		++globalTime;
		sleep(1);
	}
	return 0;
}

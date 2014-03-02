#ifndef PQUEUE_H
#define PQUEUE_H

// We hav eto do this as Node contains a reference to "Node"
typedef struct Node Node;

struct Node {
	void* data;
	Node* next;
};

typedef struct {
	Node* front;
	Node* back;
	unsigned int numNodes;
} Queue;

void pqueue_enqueue(Queue* queue, void* node);
void* pqueue_dequeue(Queue* queue);
void* pqueue_peek(Queue* queue);

#endif

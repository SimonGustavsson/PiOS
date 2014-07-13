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

void Queue_Enqueue(Queue* queue, void* item);
void* Queue_Dequeue(Queue* queue);
void* Queue_Peek(Queue* queue);

#endif

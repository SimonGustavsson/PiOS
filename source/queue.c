#include "queue.h"
#include "memory.h"

void Queue_Enqueue(Queue* queue, void* data)
{
	// Create a node
	Node* node = (Node*)palloc(sizeof(Node));
	node->data = data;
	node->next = 0;
	//printf("Allocated node at 0x%08x.\n", node);

	Node* prevBack = queue->back;
	if (queue->back != 0)
		prevBack->next = node;
	queue->back = node;

	// If this was the first element that was added, it's also the front
	if (queue->front == 0)
		queue->front = node;

	queue->numNodes++;
}

void* Queue_Dequeue(Queue* queue)
{
	// Get a reference to the front node so we don't lose it when modifying front
	Node* prevFront = queue->front;

	// Set the next element to be the front of the queue

	queue->front = queue->front->next;

	queue->numNodes--;

	if (queue->numNodes == 0)
	{
		queue->front = 0;
		queue->back = 0;
	}

	void* data = prevFront->data;

	phree(prevFront);

	return data;
}

void* Queue_Peek(Queue* queue)
{
	if (queue->front != 0)
		return queue->front->data;

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q->size == MAX_QUEUE_SIZE) exit(1);
        q->proc[q->size] = proc; // Thêm tiến trình vào hàng đợi
        q->size++;               // Cập nhật kích thước của hàng đợi
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (q == NULL || q->size == 0) return NULL;
        struct pcb_t *proc = q->proc[0];
        for (int i = 0; i < q->size-1; i++) {
          q->proc[i] = q->proc[i+1];
        }
        q->proc[q->size-1] = NULL;
        q->size--;
	return proc;
}


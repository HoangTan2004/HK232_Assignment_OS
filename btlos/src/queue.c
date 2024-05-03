#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/*
 * @brief:	check if the queue is empty
 * @para:	pointer the queue
 * @retval:	1   -   empty 
 *          0   -   not empty
 * */
int empty(struct queue_t *q)
{
    if (q == NULL)
        return 1;
    return (q->size == 0);
}

void enqueue(struct queue_t *q, struct pcb_t *proc)
{
    /* TODO: put a new process to queue [q] */
    if (q->size >= MAX_QUEUE_SIZE)
        return;
    q->size++;
    q->proc[q->size - 1] = proc;
}


struct pcb_t *dequeue(struct queue_t *q)
{
    /* TODO: return a pcb whose prioprity is the highest
     * in the queue [q] and remember to remove it from q
     * */
    struct pcb_t *ret_proc = NULL;
    if (empty(q)) return ret_proc;
        #ifdef MLQ_SCHED
        ret_proc = q->proc[0];
        for (int i = 0; i < q->size - 1; i++)
        {
            q->proc[i] = q->proc[i + 1];
        }
        q->proc[q->size - 1] = NULL;
        q->size--;
    return ret_proc;
}
#else
struct pcb_t *dequeue(struct queue_t *q)
{
    /* TODO: return a pcb whose prioprity is the highest
     * in the queue [q] and remember to remove it from q
     * */
    int highestPriority = MAX_PRIO * 2;
    int ind = -1;

    for (int i = 0; i < q->size; i++)
    {
        // If priority is same choose
        // the element with the
        // highest value
        if (highestPriority > q->proc[i]->priority)
        {
            highestPriority = q->proc[i]->priority;
            ind = i;
        }
    }

    if (ind == -1)
        return NULL;
    else
    {
        struct pcb_t *ret_proc = q->proc[ind];
        for (int i = ind; i < q->size - 1; i++)
        {
            q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return ret_proc;
    }
    return NULL;
}
#endif

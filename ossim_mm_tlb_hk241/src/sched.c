
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int curr_prio = 0;
static int slot_left = MAX_PRIO;

int queue_empty(void) {
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if(!empty(&mlq_ready_queue[prio])) 
			return -1;
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void) {
    int i;
	for (i = 0; i < MAX_PRIO; i ++)
		mlq_ready_queue[i].size = 0;
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

/* 
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t * get_mlq_proc(void) {
	struct pcb_t * proc = NULL;
	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if(slot_left == 0 || empty(&mlq_ready_queue[curr_prio])) {
		curr_prio = 0;
		slot_left = MAX_PRIO - curr_prio;
	}
	for (int i = 0; i < MAX_PRIO; i++) {
        if (slot_left == 0 || empty(&mlq_ready_queue[curr_prio])) {
			//if(curr_prio < 10) printf("ready queue prio %d empty\n", curr_prio);
            curr_prio = (curr_prio + 1) % MAX_PRIO;
            slot_left = MAX_PRIO - curr_prio;
        }
        else {
		    proc = dequeue(&mlq_ready_queue[curr_prio]);
            slot_left--;
            break;
        }
	}
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t * proc) {
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);	
}

struct pcb_t * get_proc(void) {
	return get_mlq_proc();
}

void put_proc(struct pcb_t * proc) {
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t * proc) {
	//printf("vao add proc\n");
	return add_mlq_proc(proc);
}



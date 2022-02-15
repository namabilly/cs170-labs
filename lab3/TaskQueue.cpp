#include <queue>
#include "TaskQueue.h"

TaskQueue::
TaskQueue()
{
    // TODO: Your code here.
    smutex_init(&mutex);
    scond_init(&cond);
}

TaskQueue::
~TaskQueue()
{
    // TODO: Your code here.
    smutex_destroy(&mutex);
    scond_destroy(&cond);
}

/*
 * ------------------------------------------------------------------
 * size --
 *
 *      Return the current size of the queue.
 *
 * Results:
 *      The size of the queue.
 *
 * ------------------------------------------------------------------
 */
int TaskQueue::
size()
{
    // TODO: Your code here.
    return q.size(); // Keep compiler happy until routine done.
}

/*
 * ------------------------------------------------------------------
 * empty --
 *
 *      Return whether or not the queue is empty.
 *
 * Results:
 *      The true if the queue is empty and false otherwise.
 *
 * ------------------------------------------------------------------
 */
bool TaskQueue::
empty()
{
    // TODO: Your code here.
    return q.empty(); // Keep compiler happy until routine done.
}

/*
 * ------------------------------------------------------------------
 * enqueue --
 *
 *      Insert the task at the back of the queue.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void TaskQueue::
enqueue(Task task)
{
    // TODO: Your code here.
    smutex_lock(&mutex);
    q.push(task);
    scond_signal(&cond, &mutex);
    smutex_unlock(&mutex);
}

/*
 * ------------------------------------------------------------------
 * dequeue --
 *
 *      Remove the Task at the front of the queue and return it.
 *      If the queue is empty, block until a Task is inserted.
 *
 * Results:
 *      The Task at the front of the queue.
 *
 * ------------------------------------------------------------------
 */
Task TaskQueue::
dequeue()
{
    // TODO: Your code here.
    smutex_lock(&mutex);
    while (empty())
        scond_wait(&cond, &mutex);
    Task task = q.front();
    q.pop();
    scond_signal(&cond, &mutex);
    smutex_unlock(&mutex);
    return task; // Keep compiler happy until routine done.
}


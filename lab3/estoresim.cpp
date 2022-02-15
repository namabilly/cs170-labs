#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "EStore.h"
#include "TaskQueue.h"
#include "RequestGenerator.h"

class Simulation
{
    public:
    TaskQueue supplierTasks;
    TaskQueue customerTasks;
    EStore store;

    int maxTasks;
    int numSuppliers;
    int numCustomers;

    explicit Simulation(bool useFineMode) : store(useFineMode) { }
};

/*
 * ------------------------------------------------------------------
 * supplierGenerator --
 *
 *      The supplier generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the supplier queue, then
 *      stop all supplier threads by enqueuing arg->numSuppliers
 *      stop requests.
 *
 *      Use a SupplierRequestGenerator to generate and enqueue
 *      requests.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
supplierGenerator(void* arg)
{
    // TODO: Your code here.
    Simulation* sim = (Simulation*) arg;
    SupplierRequestGenerator srg = SupplierRequestGenerator(&sim->supplierTasks);
    srg.enqueueTasks(sim->maxTasks, &sim->store);
    srg.enqueueStops(sim->numSuppliers);
    printf("finish srg\n");
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customerGenerator --
 *
 *      The customer generator thread. The argument is a pointer to
 *      the shared Simulation object.
 *
 *      Enqueue arg->maxTasks requests to the customer queue, then
 *      stop all customer threads by enqueuing arg->numCustomers
 *      stop requests.
 *
 *      Use a CustomerRequestGenerator to generate and enqueue
 *      requests.  For the fineMode argument to the constructor
 *      of CustomerRequestGenerator, use the output of
 *      store.fineModeEnabled() method, where store is a field
 *      in the Simulation class.
 *
 *      This thread should exit when done.
 *
 * Results:
 *      Does not return. Exit instead.
 *
 * ------------------------------------------------------------------
 */
static void*
customerGenerator(void* arg)
{
    // TODO: Your code here.
    Simulation* sim = (Simulation*) arg;
    CustomerRequestGenerator crg = CustomerRequestGenerator(&sim->customerTasks, sim->store.fineModeEnabled());
    crg.enqueueTasks(sim->maxTasks, &sim->store);
    crg.enqueueStops(sim->numCustomers);
    printf("finish crg\n");
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * supplier --
 *
 *      The main supplier thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the supplier queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
supplier(void* arg)
{
    // TODO: Your code here.
    Simulation* sim = (Simulation*) arg;
    Task task = sim->supplierTasks.dequeue();
    
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * customer --
 *
 *      The main customer thread. The argument is a pointer to the
 *      shared Simulation object.
 *
 *      Dequeue Tasks from the customer queue and execute them.
 *
 * Results:
 *      Does not return.
 *
 * ------------------------------------------------------------------
 */
static void*
customer(void* arg)
{
    // TODO: Your code here.
    Simulation* sim = (Simulation*) arg;
    Task task = sim->customerTasks.dequeue();
    return NULL; // Keep compiler happy.
}

/*
 * ------------------------------------------------------------------
 * startSimulation --
 *      Create a new Simulation object. This object will serve as
 *      the shared state for the simulation. 
 *
 *      Create the following threads:
 *          - 1 supplier generator thread.
 *          - 1 customer generator thread.
 *          - numSuppliers supplier threads.
 *          - numCustomers customer threads.
 *
 *      After creating the worker threads, the main thread
 *      should wait until all of them exit, at which point it
 *      should return.
 *
 *      Hint: Use sthread_join.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
static void
startSimulation(int numSuppliers, int numCustomers, int maxTasks, bool useFineMode)
{
    // TODO: Your code here.
    // create Simulation
    Simulation sim = Simulation(useFineMode);
    sim.maxTasks = maxTasks;
    sim.numSuppliers = numSuppliers;
    sim.numCustomers = numCustomers;

    // create threads
    printf("before gen\n");
    sthread_t sup_gen, cus_gen, sup[numSuppliers], cus[numCustomers];
    sthread_create(&sup_gen, supplierGenerator, &sim);
    sthread_create(&cus_gen, customerGenerator, &sim);

    for(int i=0;i<numSuppliers;i++)
        sthread_create(&sup[i], supplier, &sim);
    for(int i=0;i<numCustomers;i++)
        sthread_create(&cus[i], customer, &sim);
    printf("after gen\n");
    // wait for all thread to exit and then return
    sthread_join(sup_gen);
    sthread_join(cus_gen);
    for(int i=0;i<numSuppliers;i++)
        sthread_join(sup[i]);
    for(int i=0;i<numCustomers;i++)
        sthread_join(cus[i]);
    printf("finish\n");
    return;

}

int main(int argc, char **argv)
{
    bool useFineMode = false;

    // Seed the random number generator.
    // You can remove this line or set it to some constant to get deterministic
    // results, but make sure you put it back before turning in.
    srand(time(NULL));

    if (argc > 1)
        useFineMode = strcmp(argv[1], "--fine") == 0;
    startSimulation(10, 10, 100, useFineMode);
    return 0;
}


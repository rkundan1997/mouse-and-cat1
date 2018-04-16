

#include <sys/types.h>
#include <stdlib.h>

#include <pthread.h>


#if OPT_A1
#include "kitchen.h"
#endif 
extern int initialize_bowls(unsigned int bowlcount);

extern void cat_eat(unsigned int bowlnumber);
extern void mouse_eat(unsigned int bowlnumber);
extern void cat_sleep(void);
extern void mouse_sleep(void);


int NumBowls;  // number of food bowls
int NumCats;   // number of cats
int NumMice;   // number of mice
int NumLoops;  // number of times each cat and mouse should eat

/*
 * Once the main driver function (catmouse()) has created the cat and mouse
 * simulation threads, it uses this semaphore to block until all of the
 * cat and mouse simulations are finished.
 */
struct semaphore *CatMouseWait;

/*
 * 
 * Function Definitions
 * 
 */

#if OPT_A1
struct kitchen *k = NULL;  // global kitchen variable

struct kitchen *kitchen_create() {
    int i;

    // Create the top-level struct
    struct kitchen *k = kmalloc(sizeof(struct kitchen));

    if (k == NULL) {
        return NULL;
    }

    // Construct the bowl lock array
    k->bowl_locks = kmalloc(NumBowls * sizeof(struct lock *));
    if (k->bowl_locks == NULL) {
        kfree(k);
        return NULL;
    }

    // Construct each bowl lock
    for (i = 0; i < NumBowls; i++) {
        k->bowl_locks[i] = lock_create("bowl");
    }

    // Construct the entrance lock
    k->kitchen_lock = lock_create("enter");

    // Construct the kitchen cv
    k->kitchen_cv = cv_create("kitchen");

    // Construct the group queue
    k->group_list = q_create(1);

    // Initialize the current_creature flag
    k->current_creature = 2;

    // Initialize the counter
    k->creature_count = 0;

    return k;
}

void enter_kitchen(const int creature_type) {
  
    else if (!q_empty(k->group_list)) {
        // Inspect the last group in line
        int index = q_getend(k->group_list) > 0 ? q_getend(k->group_list)-1 : q_getsize(k->group_list) - 1;
        struct kgroup *g = (struct kgroup *)q_getguy(k->group_list, index);

        if (g->type == creature_type) {
            // If the last group is of your type, merge into that group
            g->amount++;
        } else {
            // Otherwise, start a new last group
            g = kmalloc(sizeof(struct kgroup));
            g->type = creature_type;
            g->amount = 1;

            // Enqueue new group
            q_addtail(k->group_list, g);
        }

        // Wait
        cv_wait(k->kitchen_cv, k->kitchen_lock);
    }
   
    else if (k->current_creature != creature_type) {
        // Create a group struct
        struct kgroup *g = kmalloc(sizeof(struct kgroup));

        // Group is of your type with 1 creature so far
        g->type = creature_type;
        g->amount = 1;

        // Enqueue new group
        q_addtail(k->group_list, g);

        // Wait
        cv_wait(k->kitchen_cv, k->kitchen_lock);
    }

    // If here, we have been granted access to the kitchen

    // Set the creature type currently owning the kitchen
    k->current_creature = creature_type;

    // Increment creature count
    k->creature_count++;

    lock_release(k->kitchen_lock);
}

void exit_kitchen() {
    // Reacquire entrance lock
    lock_acquire(k->kitchen_lock);

    // Decrement count
    k->creature_count--;

    /*
     * If there are no creatures left, let in the next group waiting.
     * If there is no other group waiting, reset the switch that
     * indicates which creature type currently owns the kitchen.
     */
    if (!q_empty(k->group_list) && k->creature_count == 0) {
        // Dequeue first group in line
        struct kgroup *g = q_remhead(k->group_list);
        int i;

        // Signal every member of that group
        for (i = 0; i < g->amount; i++) {
            cv_signal(k->kitchen_cv, k->kitchen_lock);
        }

        // Destroy the group struct
        kfree(g);
    } else if (q_empty(k->group_list) && k->creature_count == 0) {
        // 2 is the "unset" value for k->current_creature
        k->current_creature = 2;
    }

    // Release enter lock and exit
    lock_release(k->kitchen_lock);
}

void eat(const int creature_type) {
    /*
     * Convention: creature_type=0 -> cat. creature_type=1 -> mouse.
     */

    // Try to enter kitchen and potentially get blocked/grouped
    enter_kitchen(creature_type);

    // Choose a random initial bowl index
    unsigned int bowl = ((unsigned int)random() % NumBowls);
    int i = 0;

    for (i = 0; i < NumBowls; i++) {
        // Try to acquire it, but don't block if it is occupied
        if (lock_tryacquire(k->bowl_locks[bowl])) break;

        // Try another bowl
        bowl = (bowl+1) % NumBowls;
    }

    // If all the bowls are occupied
    if (i == NumBowls) {
        // Just wait to acquire the initially chosen one
        bowl %= NumBowls;
        lock_acquire(k->bowl_locks[bowl]);
    }

    assert(lock_do_i_hold(k->bowl_locks[bowl]));

    // Eat
    if (creature_type) {
        mouse_eat(bowl+1);
    } else {
        cat_eat(bowl+1);
    }

    // Release this bowl's lock
    lock_release(k->bowl_locks[bowl]);

    exit_kitchen();
}

void kitchen_destroy(struct kitchen *k) {
    int i;

    // Destroy the queue elements
    while (!q_empty(k->group_list)) {
        kfree(q_remhead(k->group_list));
    }

    // Destroy the queue
    q_destroy(k->group_list);

    // Destroy the cv
    cv_destroy(k->kitchen_cv);

    // Destroy the entrance lock
    lock_destroy(k->kitchen_lock);

    // Destroy the bowl locks
    for (i = 0; i < NumBowls; i++) {
        lock_destroy(k->bowl_locks[i]);
    }

    // Destroy the bowl lock array
    kfree(k->bowl_locks);

    // Destroy the kitchen
    kfree(k);

    // Clear the pointer
    k = NULL;
}

#endif //OPT_A1


static
void
cat_simulation(void * unusedpointer, 
	       unsigned long catnumber)
{
  int i;
  unsigned int bowl;

  /* avoid unused variable warnings. */
  (void) unusedpointer;
  (void) catnumber;


  /* your simulated cat must iterate NumLoops times,
   *  sleeping (by calling cat_sleep() and eating
   *  (by calling cat_eat()) on each iteration */
  for(i=0;i<NumLoops;i++) {

  
    cat_sleep();



#if OPT_A1
    (void)bowl; //suppress warning
    eat(0);
#else
    /* legal bowl numbers range from 1 to NumBowls */
    bowl = ((unsigned int)random() % NumBowls) + 1;
    cat_eat(bowl);
#endif // OPT_A1

  }

  /* indicate that this cat simulation is finished */
  V(CatMouseWait); 
}
	


static
void
mouse_simulation(void * unusedpointer,
          unsigned long mousenumber)
{
  int i;
  unsigned int bowl;

  /* Avoid unused variable warnings. */
  (void) unusedpointer;
  (void) mousenumber;


  /* your simulated mouse must iterate NumLoops times,
   *  sleeping (by calling mouse_sleep()) and eating
   *  (by calling mouse_eat()) on each iteration */
  for(i=0;i<NumLoops;i++) {

    
    mouse_sleep();

    

#if OPT_A1
    (void)bowl; // suppress warning
    eat(1);
#else
    /* legal bowl numbers range from 1 to NumBowls */
    bowl = ((unsigned int)random() % NumBowls) + 1;
    mouse_eat(bowl);
#endif // OPT_A1

  }

  /* indicate that this mouse is finished */
  V(CatMouseWait); 
}



int
catmouse(int nargs,
	 char ** args)
{
  int index, error;
  int i;

  /* check and process command line arguments */
  if (nargs != 5) {
    kprintf("Usage: <command> NUM_BOWLS NUM_CATS NUM_MICE NUM_LOOPS\n");
    return 1;  // return failure indication
  }

  /* check the problem parameters, and set the global variables */
  NumBowls = atoi(args[1]);
  if (NumBowls <= 0) {
    kprintf("catmouse: invalid number of bowls: %d\n",NumBowls);
    return 1;
  }
  NumCats = atoi(args[2]);
  if (NumCats < 0) {
    kprintf("catmouse: invalid number of cats: %d\n",NumCats);
    return 1;
  }
  NumMice = atoi(args[3]);
  if (NumMice < 0) {
    kprintf("catmouse: invalid number of mice: %d\n",NumMice);
    return 1;
  }
  NumLoops = atoi(args[4]);
  if (NumLoops <= 0) {
    kprintf("catmouse: invalid number of loops: %d\n",NumLoops);
    return 1;
  }
  kprintf("Using %d bowls, %d cats, and %d mice. Looping %d times.\n",
          NumBowls,NumCats,NumMice,NumLoops);

  /* create the semaphore that is used to make the main thread
     wait for all of the cats and mice to finish */
  CatMouseWait = sem_create("CatMouseWait",0);
  if (CatMouseWait == NULL) {
    panic("catmouse: could not create semaphore\n");
  }

  /* 
   * initialize the bowls
   */
  if (initialize_bowls(NumBowls)) {
    panic("catmouse: error initializing bowls.\n");
  }

#if OPT_A1
  // Create the kitchen
  k = kitchen_create();
#endif

  /*
   * Start NumCats cat_simulation() threads.
   */
  for (index = 0; index < NumCats; index++) {
    error = thread_fork("cat_simulation thread",NULL,index,cat_simulation,NULL);
    if (error) {
      panic("cat_simulation: thread_fork failed: %s\n", strerror(error));
    }
  }

  /*
   * Start NumMice mouse_simulation() threads.
   */
  for (index = 0; index < NumMice; index++) {
    error = thread_fork("mouse_simulation thread",NULL,index,mouse_simulation,NULL);
    if (error) {
      panic("mouse_simulation: thread_fork failed: %s\n",strerror(error));
    }
  }

  /* wait for all of the cats and mice to finish before
     terminating */  
  for(i=0;i<(NumCats+NumMice);i++) {
    P(CatMouseWait);
  }

#if OPT_A1
  // Cleanup the kitchen lol
  kitchen_destroy(k);
#endif

  /* clean up the semaphore the we created */
  sem_destroy(CatMouseWait);

  return 0;
}

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define THREAD_COUNT 10
#define REPEAT_COUNT 50000

static void
dbgprintf (const char *fmt, ...)
{
  struct timespec tp;
  va_list ap;

  if (clock_gettime (CLOCK_MONOTONIC, &tp) == 0)
    printf ("%lu.%lu ", tp.tv_sec, tp.tv_nsec);

  va_start (ap, fmt);
  vprintf (fmt, ap);
  va_end (ap);
}

static int ready[THREAD_COUNT];
static pthread_mutex_t ready_lock[THREAD_COUNT];
static pthread_rwlock_t fire_signal[REPEAT_COUNT];
static const pthread_once_t fresh_once = PTHREAD_ONCE_INIT;
static pthread_once_t once_control;
static int performed;
static pthread_mutex_t performed_lock = PTHREAD_MUTEX_INITIALIZER;

static void
once_execute (void)
{
  pthread_mutex_lock (&performed_lock);
  performed++;
  pthread_mutex_unlock (&performed_lock);
}

static void *
once_contender_thread (void *arg)
{
  int id = (int) (long) arg;
  int repeat;

  for (repeat = 0; repeat <= REPEAT_COUNT; repeat++)
    {
      /* Tell the main thread that we're ready.  */
      pthread_mutex_lock (&ready_lock[id]);
      ready[id] = 1;
      pthread_mutex_unlock (&ready_lock[id]);

      if (repeat == REPEAT_COUNT)
        break;

      dbgprintf ("Contender %p waiting for signal for round %d\n",
                 pthread_self (), repeat);
      /* Wait for the signal to go.  */
      pthread_rwlock_rdlock (&fire_signal[repeat]);
      /* And don't hinder the others (if the scheduler is unfair).  */
      pthread_rwlock_unlock (&fire_signal[repeat]);
      dbgprintf ("Contender %p got the     signal for round %d\n",
                 pthread_self (), repeat);

      /* Contend for execution.  */
      pthread_once (&once_control, once_execute);
    }

  return NULL;
}

static void
test_once (void)
{
  int i, repeat;
  pthread_t threads[THREAD_COUNT];

  /* Initialize all variables.  */
  for (i = 0; i < THREAD_COUNT; i++)
    {
      ready[i] = 0;
      pthread_mutex_init (&ready_lock[i], NULL);
    }
  for (i = 0; i < REPEAT_COUNT; i++)
    pthread_rwlock_init (&fire_signal[i], NULL);

  /* Block all fire_signals.  */
  for (i = REPEAT_COUNT-1; i >= 0; i--)
    pthread_rwlock_wrlock (&fire_signal[i]);

  /* Spawn the threads.  */
  for (i = 0; i < THREAD_COUNT; i++)
    pthread_create (&threads[i], NULL, once_contender_thread, (void *) (long) i);

  for (repeat = 0; repeat <= REPEAT_COUNT; repeat++)
    {
      /* Wait until every thread is ready.  */
      dbgprintf ("Main thread before synchronizing for round %d\n", repeat);
      for (;;)
        {
          int ready_count = 0;
          for (i = 0; i < THREAD_COUNT; i++)
	    ready_count += ready[i];
          if (ready_count == THREAD_COUNT)
            break;
          pthread_yield ();
        }
      dbgprintf ("Main thread after  synchronizing for round %d\n", repeat);

      if (repeat > 0)
        {
          /* Check that exactly one thread executed the once_execute()
             function.  */
          if (performed != 1)
            abort ();
        }

      if (repeat == REPEAT_COUNT)
        break;

      /* Preparation for the next round: Initialize once_control.  */
      memcpy (&once_control, &fresh_once, sizeof (pthread_once_t));

      /* Preparation for the next round: Reset the performed counter.  */
      performed = 0;

      /* Preparation for the next round: Reset the ready flags.  */
      for (i = 0; i < THREAD_COUNT; i++)
        {
          pthread_mutex_lock (&ready_lock[i]);
          ready[i] = 0;
          pthread_mutex_unlock (&ready_lock[i]);
        }

      /* Signal all threads simultaneously.  */
      dbgprintf ("Main thread giving signal for round %d\n", repeat);
      pthread_rwlock_unlock (&fire_signal[repeat]);
    }

  /* Wait for the threads to terminate.  */
  for (i = 0; i < THREAD_COUNT; i++)
    pthread_join (threads[i], NULL);
}

int
main ()
{
  printf ("Starting test_once ..."); fflush (stdout);
  test_once ();
  printf (" OK\n"); fflush (stdout);

  return 0;
}

/* Local Variables:  */
/* compile-command: "gcc -o test-once test-once-pthread.c -lpthread" */
/* End:              */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>

static int numThreads = 1;
static int numIters = 1;
static long long counter;
static int opt_yield;
static int mutex;
static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static int spin;
int s1 = 0;
static int cas;

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}

void casAdd(long long *counter, int value) {
  long long old = *counter;
  do {
    old = *counter;
    if (opt_yield)
      sched_yield();
  } while (__sync_val_compare_and_swap(counter, old, old+value) != old);
}

void* addHelper(int value) {
  if (cas) {
    for (int i = 0; i < numIters; i++)
      casAdd(&counter, value);
  }
  else {
    for (int i = 0; i < numIters; i++) {
      if (mutex)
	pthread_mutex_lock(&m1);
      if (spin)
	while(__sync_lock_test_and_set(&s1, 1));
      add(&counter, value);
      if (mutex)
	pthread_mutex_unlock(&m1);
      if (spin)
	__sync_lock_release(&s1);  
    }
  }
  return NULL;
}

void* addHelpHelp() {
  addHelper(1);
  addHelper(-1);
  return NULL;
}

int main(int argc, char* argv[])
{
  static struct option long_options[] =
    {
     {"threads", required_argument, 0, 't'},
     {"iterations", required_argument, 0, 'i'},
     {"yield", no_argument, 0, 'y'},
     {"sync", required_argument, 0, 's'},
     {0, 0, 0, 0}
    };

  int c;
  pthread_t *threads;
  char* tag = "add-none";
  
  while (1)
    {
      c = getopt_long(argc, argv, "", long_options, NULL);

      if (c == -1)
	break;

      switch(c)
	{
	case 't':
	  numThreads = atoi(optarg);
	  break;
	case 'i':
	  numIters = atoi(optarg);
	  break;
	case 's':
	  if (strcmp(optarg, "m") == 0)
	    mutex = 1;
	  if (strcmp(optarg, "s") == 0)
	    spin = 1;
	  if (strcmp(optarg, "c") == 0)
	    cas = 1;
	  break;
	case 'y':
	  opt_yield = 1;
	  break;
	default:
	  fprintf(stderr, "Invalid Option\n");
	  exit(1);
	}
    }

  // start clock
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
   
  threads = (pthread_t*)malloc((numThreads)*sizeof(pthread_t));

  // create threads
  for (int i = 0; i < numThreads; i++)
    pthread_create(&threads[i], NULL, addHelpHelp, NULL);

  // join threads
  for (int i = 0; i < numThreads; i++)
    pthread_join(threads[i], NULL);

  // end clock
  clock_gettime(CLOCK_MONOTONIC, &end);
  long long time = ((long long)end.tv_sec - (long long)start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);

  if (opt_yield) {
    if (mutex)
      tag = "add-yield-m";
    else if (spin)
      tag = "add-yield-s";
    else if (cas)
      tag = "add-yield-c";
    else
      tag = "add-yield-none";
  }
  else {
    if (mutex)
      tag = "add-m";
    else if (spin)
      tag = "add-s";
    else if (cas)
      tag = "add-c";
    else
      tag = "add-none";
  }
  
  int numOps = numThreads*numIters*2;
  int avgTime = (int)(time/numOps);
  printf("%s,%d,%d,%d,%lld,%d,%lld\n", tag, numThreads, numIters, numOps, time, avgTime, counter);

  exit(0);
}

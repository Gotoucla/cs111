// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145

#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>


static long long counter = 0;
static int opt_yield;
int sync_flag=0;
int mutex_flag = 0;
int spin_flag = 0;
int compare_flag = 0;
char *tag;
pthread_mutex_t lock;

typedef struct __lock_t {
  int flag;
} lock_t; // for spin-lock

int spin;
int compare;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void spin_lock(int *lock) {
  while(__sync_lock_test_and_set(lock, 1) == 1)
    ;
}

void unlock(int *lock) {
  __sync_lock_release(lock);
}

void compare_lock(long long *counter, long long value) {
  long long old = *counter;
  do {
    old = *counter;
    if (opt_yield)
      sched_yield();  
  } while(__sync_val_compare_and_swap(counter,old,old+value)!=old);
}

	
void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}


void *addFunction(void *iteration_num) {
  int *num = (int *)iteration_num;
  int i = 0;
  if (compare_flag) {
    int t = 0;
    for (;t<*num;t++) {
      compare_lock(&counter,1);
    }

    t = 0;
    for (;t<*num;t++) {
      compare_lock(&counter,-1);
    }
  }
  else {
    for (; i<*num; i++) {
      if (mutex_flag) {
	pthread_mutex_lock(&lock);            
	add(&counter,1);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	add(&counter,-1);
	pthread_mutex_unlock(&lock);
           
      }
      else if (spin_flag) {
	spin_lock(&spin);
	add(&counter,1);
	unlock(&spin);

	spin_lock(&spin);
	add(&counter,-1);
	unlock(&spin);
      }

      else {
	add(&counter,1);
	add(&counter,-1);
      }
	
    }
  }
  return NULL;
}

char *gettag() {
  if (opt_yield) {
    if (!sync_flag)
      tag = "add-yield-none";
    else {
      if (mutex_flag)
	tag = "add-yield-m";
      if(spin_flag)
	tag = "add-yield-s";
      if(compare_flag)
	tag="add-yield-c";
    }
  }
  else if (!opt_yield) {
    if (mutex_flag)
      tag = "add-m";
    if(spin_flag)
      tag = "add-s";
    if(compare_flag)
      tag="add-c";
    if (!sync_flag)
      tag="add-none";
  }
  return tag;
}
    

int main(int argc, char * argv[]) {
  int opt = 0;
  //int option_index = 0;
  //char *string = "t:i:ys:";
  int thread_count = 0;
  int iteration_count = 0;
  struct timespec start_time, end_time;
  int errorNum=0;

  static struct option long_options[] = {
    
					 {"threads", required_argument, NULL, 't'},
					 {"iterations",required_argument, NULL, 'i'},
					 {"yield",no_argument, NULL, 'y'},
					 {"sync",required_argument, NULL, 's'},
					 {0,0,0,0},
  };

  while (1) {
    opt = getopt_long(argc,argv,"",long_options,NULL);

    if (opt == -1)
      break;

    switch (opt) {
    case 't': // threads
      thread_count = atoi(optarg);
      break;

    case 'i': // iteration
      iteration_count = atoi(optarg);
      break;

    case 'y': // yield
      opt_yield = 1;
      break;

    case 's': // sync
      sync_flag = 1;
      if(strcmp(optarg,"m")==0) { // mutex
	mutex_flag = 1;
      }
      else if(strcmp(optarg,"s")==0) { // spin-lock
	spin_flag = 1;
      }
      else if(strcmp(optarg,"c")==0) { // atomic
	compare_flag = 1;
      }
      else {
	fprintf(stderr, "Incorrect input: should be m or s or c. \n");
	exit(1);
      }
	
      break;

    default:

      fprintf(stderr, "Usage: %s [--threads=number_of_threads] [--iterations=number_of_iterations]\n", argv[0]);
      exit(1);
    }
  }

    
  if (clock_gettime(CLOCK_MONOTONIC, &start_time) < 0) {
    errorNum = errno;
    fprintf(stderr,"Unable to get the start time: %s\n", strerror(errorNum));
    exit(2);
  }

  
  pthread_t threads[thread_count];
  int i=0;
  int rc; // for pthread_create
  int jc; // for pthread_join

  // create threads 
  for (;i<thread_count;i++) {
    rc = pthread_create(&threads[i], NULL, addFunction, &iteration_count);
    if (rc != 0) {
      errorNum = errno;
      fprintf(stderr, "Unable to create a thread: %s\n", strerror(errorNum));
      exit(2);
    }
  }
  i = 0;
  // join parent thread
  for (;i<thread_count;i++) {
    jc = pthread_join(threads[i], NULL);
    if (jc != 0) {
      errorNum = errno;
      fprintf(stderr, "Unable to join a thread: %s\n", strerror(errorNum));
      exit(2);
    }
  }
      
  if (clock_gettime(CLOCK_MONOTONIC, &end_time) < 0) {
    errorNum = errno;
    fprintf(stderr,"Unable to get the end time: %s\n", strerror(errorNum));
    exit(2);
  }

  int total_operation = thread_count * iteration_count * 2;
  long long total_time;
  int average;
  total_time = ((long long)end_time.tv_sec - (long long)start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
  average = (int)(total_time/total_operation);
  tag=gettag();
  fprintf(stdout, "%s,%d,%d,%d,%lld,%d,%lld\n",tag,thread_count,iteration_count,total_operation,total_time, average, counter);
  exit(0);
}
  
    
 

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
#include <unistd.h>
#include "SortedList.h"

int opt_yield;
SortedList_t* list;
SortedListElement_t* list_element;
const size_t keys_length = 8;
char tag[20] = "list-";

static int thread_count = 1;
static int iteration_count = 1;
static int total_element;

static int sync_flag;
static int mutex_flag;
static int spin_flag;
int spin;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char yield_tag[5];

SortedList_t* init_list() {
  SortedList_t* head = malloc(sizeof(SortedList_t));
  head->prev = head;
  head->next = head;
  head->key = NULL;
  return head;
}

// for spin lock
void spin_lock(int *lock) {
  while(__sync_lock_test_and_set(lock, 1) == 1)
    ;
}

// for spin lock
void unlock(int *lock) { 
  __sync_lock_release(lock);
}

char *get_random_keys(size_t length) {
  static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
  char *randomString = NULL;

  if(length) {
    randomString = malloc(sizeof(char) * (length + 1));

    if(randomString) {
      size_t n=0;
      for (;n<length;n++) {
	int key = rand()%(int)(sizeof(charset)-1);
	randomString[n]=charset[key];
      }

      randomString[length]='\0';
    }
  }
  return randomString;
}
 
void *listFunction(void *thread_index) {
  
  int *num = (int *)thread_index;
  int i;
  //fprintf(stdout, "thread index=%d\n",*num);
  //fflush(stdout);
   

  // insert all elements
  for(i=*num;i<(*num+iteration_count);i++) {
    if(mutex_flag)
      pthread_mutex_lock(&lock);

    if(spin_flag)
      spin_lock(&spin);

    SortedList_insert(list,&list_element[i]);

    if (mutex_flag)
      pthread_mutex_unlock(&lock);

    if (spin_flag)
      unlock(&spin);
   
    
  }

  //fprintf(stdout, "element insertion done\n");
  //fflush(stdout);

  
  
  if (mutex_flag)
    pthread_mutex_lock(&lock);

  if (spin_flag)
    spin_lock(&spin);

  // get list length
  int list_length = SortedList_length(list);  
  //fprintf(stdout, "length of list=%d\n",list_length);
  //fflush(stdout);

  if (list_length == -1 ) {
    fprintf(stderr, "Unable to get the length. The list corrupted \n");
    exit(1);
  }

  if (mutex_flag)
    pthread_mutex_unlock(&lock);

  if (spin_flag)
    unlock(&spin);

  //fprintf(stdout, "length done\n");
  //fflush(stdout);

  
  
   i=0;
   // delete all keys inserted
   for(i=*num;i<(*num+iteration_count);i++) {
     
     if (mutex_flag)
       pthread_mutex_lock(&lock);

     if (spin_flag)
       spin_lock(&spin);

     SortedListElement_t *rc = SortedList_lookup(list,list_element[i].key);

     if (mutex_flag)
       pthread_mutex_unlock(&lock);

     if (spin_flag)
       unlock(&spin);
     
     if (rc != NULL) {

       if (mutex_flag)
	 pthread_mutex_lock(&lock);

       if (spin_flag)
	 spin_lock(&spin);
       
       SortedList_delete(rc); // delete successfully

       if (mutex_flag)
	 pthread_mutex_unlock(&lock);

       if (spin_flag)
	 unlock(&spin);
     }
     else { // can't find the element
       fprintf(stderr, "Unable to find the element \n");
       exit(1);
     }     
   }  
   return NULL;
}

void gettag() {
  char *sync_tag = NULL;
  if (sync_flag) {
    if (mutex_flag)
      sync_tag = "-m";
    if (spin_flag)
      sync_tag = "-s";    
    }
  else  {
    sync_tag = "-none";
  }
  
  if (opt_yield == 0)
    strcpy(yield_tag,"none");
  strcat(tag,yield_tag);
  strcat(tag,sync_tag);
  
}
    

int main(int argc, char * argv[]) {
  int opt = 0;
  struct timespec start_time, end_time;
  int errorNum=0;
  char * yield_string = NULL;

  static struct option long_options[] = {
    
					 {"threads", required_argument, NULL, 't'},
					 {"iterations",required_argument, NULL, 'i'},
					 {"yield",required_argument, NULL, 'y'},
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
      yield_string = optarg;
      int j=0;
      int length = strlen(yield_string);
      for(;j<length;j++) {
	switch(yield_string[j]) {
	case 'i': // insert
	  opt_yield |= INSERT_YIELD;
	  strcat(yield_tag,"i");
	  break;
	case 'd': // delete
	  opt_yield |= DELETE_YIELD;
	  strcat(yield_tag,"d");
	  break;
	case 'l': // lookup
	  opt_yield |= LOOKUP_YIELD;
	  strcat(yield_tag,"l");
	  break;
	default:
	  fprintf(stderr,"Invalid argument\n");
	  exit(1);
	}

      }          
      break;

    case 's': // sync
      sync_flag = 1;
      if(strcmp(optarg,"m")==0) { // mutex
	mutex_flag = 1;
      }
      else if(strcmp(optarg,"s")==0) { // spin-lock
	spin_flag = 1;
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

  // initialize a list 
  list = init_list();

  // initialize random strings to keys 
  total_element = thread_count * iteration_count;
  int i=0;
  srand(time(NULL));
  list_element = malloc(total_element*sizeof(SortedListElement_t));
  for(;i<total_element;i++) {
    list_element[i].key = get_random_keys(keys_length);
  }


      
  if (clock_gettime(CLOCK_MONOTONIC, &start_time) < 0) {
    errorNum = errno;
    fprintf(stderr,"Unable to get the start time: %s\n", strerror(errorNum));
    exit(2);
  }
  
  pthread_t *threads;
  int rc; // for pthread_create
  int jc; // for pthread_join

  // create threads
  int m=0;
  threads = (pthread_t*)malloc(thread_count*sizeof(pthread_t));
  int* index = malloc(thread_count*sizeof(int));
  for (;m<thread_count;m++) {
    index[m] = m*iteration_count;
    rc = pthread_create(&threads[m], NULL, listFunction, &index[m]);
    if (rc != 0) {
      errorNum = errno;
      fprintf(stderr, "Unable to create a thread: %s\n", strerror(errorNum));
      exit(2);
    }
  }

  m=0;
  for (;m<thread_count;m++) {
    jc = pthread_join(threads[m], NULL);
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

  // confirm the length of the list
   int list_length;
   list_length = SortedList_length(list);

   if (list_length == -1 ) {
     fprintf(stderr, "Unable to get the length. The list corrupted \n");
     exit(1);
   }

   if (list_length == 0) {
     fprintf(stdout, "The length of the list is 0. Yay!!!\n");
     fflush(stdout);
   }

   int total_operation = thread_count * iteration_count * 3;
   long long total_time;
   int average;

   total_time = ((long long)end_time.tv_sec - (long long)start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
 
   average = (int)(total_time/total_operation);
   gettag();
   int list_num=1;
   fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%d\n",tag,thread_count,iteration_count,list_num,total_operation,total_time, average);
   
  free(list);
  free(list_element);
  free(threads);
  free(index);
  exit(0);
}

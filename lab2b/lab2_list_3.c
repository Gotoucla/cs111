// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145
// final version of lab2b.c

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

static int thread_count;
static int iteration_count;
static int total_element;

static int sync_flag;
static int mutex_flag;
static int spin_flag;

unsigned long* waitTime;

static int num_sub_lists=1;
typedef struct {
  SortedList_t * list_head;
  pthread_mutex_t mutex_lock;
  int spin_lock;
} Sublist;
Sublist *sublists;

char yield_tag[5];
struct args {
  int thread_index;
  int iteration_index; 
};


SortedList_t* init_list() {
  SortedList_t* head = malloc(sizeof(SortedList_t));
  head->prev = head;
  head->next = head;
  head->key = NULL;
  return head;
}

void init_sublist() {
  int i=0;
  for (;i<num_sub_lists;i++) {
    sublists[i].list_head=init_list();

    if (mutex_flag) {
      if (pthread_mutex_init(&sublists[i].mutex_lock, NULL) != 0) {
	fprintf(stderr, "Error when initialize a mutex lock\n");
	exit(2);
      }
    }
    else if (spin_flag) 
      sublists[i].spin_lock=0;
  }
}

// for spin lock
void lock_spin(int *lock) {
  while(__sync_lock_test_and_set(lock, 1) == 1)
    ;
}

// for spin lock
void unlock_spin(int *lock) { 
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

void lock(int ind,Sublist *sub_list) {
  if(mutex_flag) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    pthread_mutex_lock(&sub_list->mutex_lock);
    clock_gettime(CLOCK_MONOTONIC, &end);
    unsigned long mutex_time = ((long long)end.tv_sec - (long long)start.tv_sec) * 1000000000LL+(end.tv_nsec - start.tv_nsec);
    waitTime[ind]+=mutex_time;
  }
  if(spin_flag)
    lock_spin(&sub_list->spin_lock);      
}

void unlock(Sublist *sub_list) {
  if (mutex_flag)       
    pthread_mutex_unlock(&sub_list->mutex_lock);

  if (spin_flag)
    unlock_spin(&sub_list->spin_lock);  
}
unsigned int hashkey(const char * key) {
  unsigned long hash = 5381;
  int c;

  while ((c=*key++))
    hash=((hash<<5)+hash)+c;
  
  return hash%num_sub_lists;
}

void mul_SortedList_insert(int ind, SortedListElement_t *element) {
 
  int list_num = hashkey(element->key);
  lock(ind,&sublists[list_num]);
  SortedList_insert(sublists[list_num].list_head,element);
  unlock(&sublists[list_num]);
}

void mul_SortedList_lookup(int ind, const char *key) {
  SortedListElement_t *ret = NULL;
  int list_num = hashkey(key);
  lock(ind,&sublists[list_num]);
  ret = SortedList_lookup(sublists[list_num].list_head, key);
  
  if (ret==NULL) {
    fprintf(stderr, "Could not find element in list\n");
    exit(2);
  }  
  int delete = SortedList_delete(ret);
  if (delete == 1) {
    fprintf(stderr, "Corrupted prev/next pointers when deleting\n");
    exit(2);
  } 
  unlock(&sublists[list_num]);
}


int mul_SortedList_length(int ind) {
  int i=0, length = 0;
  for (i=0;i<num_sub_lists;i++) {
    lock(ind,&sublists[i]);
    length += SortedList_length(sublists[i].list_head);
    unlock(&sublists[i]);
  }
  return length;
}

void *listFunction(void *input) {  
  int num = ((struct args*)input)->iteration_index;
  int ind = ((struct args*)input)->thread_index;
  int i;
  
  // insert all elements
  for(i=num;i<(num+iteration_count);i++) {
    mul_SortedList_insert(ind, &list_element[i]);
  }

  int list_length=mul_SortedList_length(ind);
  
  if (list_length < 0 ) {
    fprintf(stderr, "Unable to get the length. The list corrupted \n");
    exit(1);
  }

  for(i=num;i<(num+iteration_count);i++) {
    mul_SortedList_lookup(ind, list_element[i].key);
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
					 {"lists",required_argument, NULL, 'l'},
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

    case 'l': // lists
      num_sub_lists=atoi(optarg);
      break;
      
    default:

      fprintf(stderr, "Usage: %s [--threads=number_of_threads] [--iterations=number_of_iterations]\n", argv[0]);
      exit(1);
    }
  }

  sublists = (Sublist *)malloc(num_sub_lists*sizeof(Sublist));
  init_sublist();
  waitTime = (unsigned long*)malloc(sizeof(unsigned long*));
  // initialize a list 

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
    struct args *list_input = (struct args * ) malloc(sizeof(struct args));
    list_input->thread_index = m;
    list_input->iteration_index = index[m];
    waitTime = calloc(m,sizeof(unsigned long*));

    rc = pthread_create(&threads[m], NULL, listFunction, (void *)list_input);
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


   int total_operation = thread_count * iteration_count * 3;
   long long total_time;
   int average;

   int e=0;
   unsigned long total_wait_time = 0;
   for (;e<thread_count;e++) {
     total_wait_time+=waitTime[e];
   }
    
   unsigned long average_wait_time = 0;
   average_wait_time=total_wait_time/total_operation;

   total_time = ((long long)end_time.tv_sec - (long long)start_time.tv_sec) * 1000000000LL + (end_time.tv_nsec - start_time.tv_nsec);
 
   average = (int)(total_time/total_operation);
   gettag();
   fprintf(stdout, "%s,%d,%d,%d,%d,%lld,%d,%lu\n",tag,thread_count,iteration_count,num_sub_lists,total_operation,total_time, average,average_wait_time);
   
  free(list_element);
  free(threads);
  free(index);
  free(sublists);
  
  exit(0);
}

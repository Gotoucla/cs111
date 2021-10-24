#include "SortedList.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

static int numThreads = 1;
static int numIters = 1;
static int numElements;
const int keySize = 5;
static int mutex;
static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static int spin;
int opt_yield;
int s1 = 0;
SortedList_t* list;
SortedListElement_t* elements;

// initalize empty list
SortedList_t* listInit() {
  SortedList_t *head = malloc(sizeof(SortedList_t));
  head->next = head;
  head->prev = head;
  head->key = NULL;
  return head;
}

// create elements with random keys
void elementInit() {
  srand(time(NULL));
  for (int i = 0; i < numElements; i++) {
    char* key = (char*)malloc((keySize+1)*sizeof(char));
    for (int j = 0; j < keySize; j++) {
      key[j] = (char)(rand() % 26 + 'a');
    }
    key[keySize] = '\0';
    elements[i].key = key;
  }
}

void eachThread(void* n) {
  int index = *(int*)n;
  // each thread inserts numIters elements into list
  for (int i = index; i < (index+numIters); i++) {
    if (mutex)
      pthread_mutex_lock(&m1);
    if (spin)
      while(__sync_lock_test_and_set(&s1, 1));
    SortedList_insert(list, &elements[i]);
    if (mutex)
      pthread_mutex_unlock(&m1);
    if (spin)
      __sync_lock_release(&s1);
  }
  // check length of list
  if (mutex)
    pthread_mutex_lock(&m1);
  if (spin)
    while(__sync_lock_test_and_set(&s1, 1));
  int length = SortedList_length(list);
  if (mutex)
    pthread_mutex_unlock(&m1);
  if (spin)
    __sync_lock_release(&s1);
  if (length < 0) {
    fprintf(stderr, "Invalid length of list\n");
    exit(2);
  }
  
  // each thread looks up and deletes numIters elements
  for (int i = index; i < (index+numIters); i++) {
    if (mutex)
      pthread_mutex_lock(&m1);
    if (spin)
      while(__sync_lock_test_and_set(&s1, 1));
    SortedListElement_t* element = SortedList_lookup(list, elements[i].key);
    if (mutex)
      pthread_mutex_unlock(&m1);
    if (spin)
      __sync_lock_release(&s1);
    if (element == NULL) {
      fprintf(stderr, "Could not find element in list\n");
      exit(2);
    }
    if (mutex)
      pthread_mutex_lock(&m1);
    if (spin)
      while(__sync_lock_test_and_set(&s1, 1));
    SortedList_delete(element);
    if (mutex)
      pthread_mutex_unlock(&m1);
    if (spin)
      __sync_lock_release(&s1);
  }
}

void sighandler() {
  fprintf(stderr, "SIGSEGV caught\n");
  exit(2);
}

int main(int argc, char* argv[])
{
  signal(SIGSEGV, sighandler);
  
  static struct option long_options[] =
    {
     {"threads", required_argument, 0, 't'},
     {"iterations", required_argument, 0, 'i'},
     {"yield", required_argument, 0, 'y'},
     {"sync", required_argument, 0, 's'},
     {0, 0, 0, 0}
    };

  int c;
  pthread_t *threads;
  char tag[20];
  strcat(tag, "list-");
  char ytag[5];
  
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
	case 'y':
	  for (int i = 0; i < (int)strlen(optarg); i++) {
	    if (optarg[i] == 'i') {
	      opt_yield |= INSERT_YIELD;
	      strcat(ytag, "i");
	    }
	    else if (optarg[i] == 'd') {
	      opt_yield |= DELETE_YIELD;
	      strcat(ytag, "d");
	    }
	    else if (optarg[i] == 'l') {
	      opt_yield |= LOOKUP_YIELD;
	      strcat(ytag, "l");
	    }
	    else {
	      fprintf(stderr, "--yield: invalid argument\n");
	      exit(1);
	    }
	  }
	  break;
	case 's':
          if (strcmp(optarg, "m") == 0)
            mutex = 1;
          else if (strcmp(optarg, "s") == 0)
            spin = 1;
	  else {
	    fprintf(stderr, "--sync: invalid argument\n");
	    exit(1);
	  }
          break;
	default:
	  fprintf(stderr, "Invalid option\n");
	  exit(1);
	}
    }

  // initialize empty list
  list = listInit();

  // create elements with random keys
  numElements = numThreads * numIters;
  elements = malloc(numElements*sizeof(SortedListElement_t));
  elementInit();

  // start clock
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // create threads
  threads = (pthread_t*)malloc(numThreads*sizeof(pthread_t));
  int* index = malloc(numThreads*sizeof(int));
  for (int i = 0; i < numThreads; i++) {
    index[i] = i*numIters;
    int ret = pthread_create(&threads[i], NULL, (void*)&eachThread, &index[i]);
    if (ret < 0) {
      fprintf(stderr, "Error in creating threads\n");
      exit(2);
    }
  }

  // join threads
  for (int i = 0; i < numThreads; i++) {
    int ret = pthread_join(threads[i], NULL);
    if (ret != 0) {
      fprintf(stderr, "Error in joining threads\n");
      exit(2);
    }
  }
  
  // end clock
  clock_gettime(CLOCK_MONOTONIC, &end);
  long long time = ((long long)end.tv_sec - (long long)start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
   
  if (opt_yield == 0)
    strcat(ytag, "none");

  strcat(tag, ytag);
  
  if (mutex)
    strcat(tag, "-m\0");
  else if (spin)
    strcat(tag, "-s\0");
  else
    strcat(tag, "-none\0");
 
  int numOps = numThreads*numIters*3;
  int avgTime = (int)(time/numOps);
  printf("%s,%d,%d,1,%d,%lld,%d\n", tag, numThreads, numIters, numOps, time, avgTime);

  free(list);
  free(elements);
  free(threads);
  free(index);
  
  exit(0);
}

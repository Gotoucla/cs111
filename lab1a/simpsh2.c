// NAME: Margaret Shi
// EMAIL: margaretshi@g.ucla.edu
// ID: 704974447

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

void allocError(void* p)
{
  if (p == NULL)
    {
      free(p);
      fprintf(stderr, "Allocation Error");
      exit(1);
    }
}

int main(int argc, char* argv[])
{
  int exitStat = 0;
  int c;

  static struct option long_options[] =
    {
     {"rdonly", required_argument, 0, 'a'},
     {"wronly", required_argument, 0, 'b'},
     {"command", required_argument, 0, 'c'},
     {"verbose", no_argument, 0, 'd'},
     {0, 0, 0, 0}
    };

  // create array of file numbers
  int *fdMap = (int*)malloc(sizeof(int));
  int fdCount = 0;

  int tempopt;
  
  while (1)
    {
      c = getopt_long(argc, argv, "", long_options, NULL);

      // once there are no more options
      if (c == -1)
	break;
      
      switch(c)
	{
	case 'a':;
	  int ifd = open(optarg, O_RDONLY);
	  if (ifd < 0) {
	    fprintf(stderr, "--rdonly error with file: %s, %s\n", optarg, strerror(errno));
	    exitStat = 1;
	  }
	  fdMap = realloc(fdMap,(fdCount+1)*sizeof(int));
	  allocError(fdMap);
	  fdMap[fdCount] = ifd;
	  fdCount++;
	  break;
	case 'b':;
	  int ofd = open(optarg, O_WRONLY);
	  if (ofd < 0) {
	    fprintf(stderr, "--wronly error with file: %s, %s\n", optarg, strerror(errno));
	    exitStat = 1;
	  }
	  fdMap = realloc(fdMap,(fdCount+1)*sizeof(int));
	  allocError(fdMap);
	  fdMap[fdCount] = ofd;
	  fdCount++;
	  break;
	case 'c':;
	  // start at first arg after cmd
	  optind--;
	  tempopt = optind;
	  int in = atoi(argv[tempopt++]);
	  int out = atoi(argv[tempopt++]);
	  int err = atoi(argv[tempopt++]);

	  char* args[10];
	  int i = 0;
	  // store command + args in array                                                      
	  while (tempopt < argc && (argv[tempopt][0] != '-' || argv[tempopt][1] != '-')) {
	    // if there's a single - arg, set that to optind to avoid errors
	    if (argv[tempopt][0] == '-')
	      optind = tempopt + 1;
	    args[i++] = argv[tempopt++];
	  }
	  // mark end of array                                          
	  args[i] = NULL;

	  //int length = sizeof(args)/sizeof(int);
	  //for (i = 0; i< length; i++)
	  //fprintf(stdout,"%s\n",args[i]);
	  
	  int pid = fork();
	  if (pid < 0) {
	    exit(1);
	  }
	  else if (pid == 0) { // child
	    // file redirection for stdin
	    dup2(fdMap[in], 0);
	    close(fdMap[in]);
	    // file redirection for stdout
	    dup2(fdMap[out], 1);
	    close(fdMap[out]);
	    // file redirection for stderr
	    dup2(fdMap[err], 2);
	    close(fdMap[err]);
	   	   
	    int execret = execvp(args[0], args);
	    if (execret == -1) {
	      fprintf(stderr, "Could not execute command: %s", args[0]);
	      exitStat = 1;
	    }
	  }
	  else { // parent
	    if (in < 0 || in >= fdCount || out < 0 || out >= fdCount || err < 0 || err >= fdCount) {
	      fprintf(stderr, "Invalid file descriptor\n");
	      exitStat = 1;
	    }
	  }
	  break;
	case 'd':;
	  tempopt = optind;
	  printf("%s ", argv[tempopt]);
	  tempopt++;
	  while (tempopt < argc) {
	    printf("%s ", argv[tempopt]);
	    fflush(stdout);
	    tempopt++;
	  }
	  break;
	default:
	  exitStat = 1;
	}
    }

  exit(exitStat);
}

// NAME: Margaret Shi
// EMAIL: margaretshi@g.ucla.edu
// ID: 704974447

// change some int j=0;
// see how it works in case 17

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void allocError(void* p)
{
  if (p == NULL)
    {
      free(p);
      fprintf(stderr, "Allocation Error");
      exit(1);
    }
}

void verbose(int argc, char* argv[], int tempopt)
{
  fprintf(stdout, "%s ", argv[tempopt]);
  fflush(stdout);
  tempopt++;
  while (tempopt < argc && (argv[tempopt][0] != '-' || argv[tempopt][1] != '-')) {
    fprintf(stdout, "%s ", argv[tempopt]);
    fflush(stdout);
    tempopt++;
  }
  fprintf(stdout, "\n");
  fflush(stdout);
}

void sighandler(int signum) {
  fprintf(stderr, "%d caught\n", signum);
  exit(signum);
}

int main(int argc, char* argv[])
{
  int exitStat = 0;
  int sigStat = 0;
  int c;

  static struct option long_options[] =
    {
     {"rdonly", required_argument, 0, 'a'},
     {"wronly", required_argument, 0, 'b'},
     {"command", required_argument, 0, 'c'},
     {"verbose", no_argument, 0, 'd'},
     {"rdwr", required_argument, 0, 'e'},
     {"wait", no_argument, 0, 'f'},
     {"pipe", no_argument, 0, 'g'},
     {"append", no_argument, 0, 'h'},
     {"cloexec", no_argument, 0, 'i'},
     {"creat", no_argument, 0, 'j'},
     {"directory", no_argument, 0, 'k'},
     {"dsync", no_argument, 0, 'l'},
     {"excl", no_argument, 0, 'm'},
     {"nofollow", no_argument, 0, 'n'},
     {"nonblock", no_argument, 0, 'o'},
     {"rsync", no_argument, 0, 'p'},
     {"sync", no_argument, 0, 'q'},
     {"trunc", no_argument, 0, 'r'},
     {"close", required_argument, 0, 's'},
     {"abort", no_argument, 0, 't'},
     {"catch", required_argument, 0, 'u'},
     {"ignore", required_argument, 0, 'v'},
     {"default", required_argument, 0, 'w'},
     {"pause", no_argument, 0, 'x'},
     {0, 0, 0, 0}
    };

  // create array of file numbers to file descriptors
  int *fdMap = (int*)malloc(sizeof(int));
  int fdCount = 0; // file numbers, or index of file descriptors

  int closefd;
  int tempopt; // to keep track of optind
  int sig; // signal number
  
  int fflags = 0; // flag for file flags
  int vflag = 0; // flag for verbose option
  int cflag = 0; // flag for catch option

  int* pids = (int*)malloc(sizeof(int)); // array to hold pids of child processes                  
  int pidCount = 0;

  // reset optind
  optind = 1;
  
  while (1)
    {
      c = getopt_long(argc, argv, "", long_options, NULL);

      // once there are no more options
      if (c == -1)
	break;
      
      switch(c)
	{
	case 'a':; // rdonly
	  if (vflag == 1)
            verbose(argc, argv, optind - 2);
	  int ifd = open(optarg, O_RDONLY|fflags, 0666);
	  fflags = 0; // reset file flags
	  if (ifd < 0) {
	    fprintf(stderr, "--rdonly error with file: %s, %s\n", optarg, strerror(errno));
	    if (exitStat < 1)
	      exitStat = 1;
	  }
	  fdMap = realloc(fdMap,(fdCount+1)*sizeof(int));
	  allocError(fdMap);
	  fdMap[fdCount] = ifd;
	  fdCount++;
	  break;
	case 'b':; // wronly
	  if (vflag == 1)
	    verbose(argc, argv, optind - 2);
	  int ofd = open(optarg, O_WRONLY|fflags, 0666);
	  fflags = 0; // reset file flags
	  if (ofd < 0) {
	    fprintf(stderr, "--wronly error with file: %s, %s\n", optarg, strerror(errno));
	    if (exitStat < 1)
              exitStat = 1;
	  }
	  fdMap = realloc(fdMap,(fdCount+1)*sizeof(int));
	  allocError(fdMap);
	  fdMap[fdCount] = ofd;
	  fdCount++;
	  break;
	case 'c':; // command
	  if (vflag == 1)
            verbose(argc, argv, optind - 2);
	  // start at first arg after cmd
	  optind--;
	  tempopt = optind;
	  int in = atoi(argv[tempopt++]);
	  int out = atoi(argv[tempopt++]);
	  int err = atoi(argv[tempopt++]);

	  if (argv[tempopt][0] == '-' && argv[tempopt][1] == '-') {
	    fprintf(stderr, "--command error: missing command\n");
	    if (exitStat < 1)
              exitStat = 1;
	    break;
	  }
	  
	  char* args[10];
	  int i = 0;
	  // store command + args in array                                                      
	  while (tempopt < argc && (argv[tempopt][0] != '-' || argv[tempopt][1] != '-')) {
	    // if there's a single - arg, set optind to next option to avoid errors
	    if (argv[tempopt][0] == '-')
	      optind = tempopt + 1;
	    args[i++] = argv[tempopt++];
	  }
	  // mark end of array                                          
	  args[i] = NULL;

	  
	  int pid = fork();
	  if (pid < 0) {
	    exit(1);
	  }
	  else if (pid == 0) { // child
	    // file redirection for stdin
	    dup2(fdMap[in], 0);
	    // file redirection for stdout
	    dup2(fdMap[out], 1);
	    // file redirection for stderr
	    dup2(fdMap[err], 2);

	    int i=0;
	    // close all fds after 3
	    for (; i < fdCount; i++) {
	      close(fdMap[i]);
	    }
	    
	    int execret = execvp(args[0], args);
	    if (execret == -1) {
	      fprintf(stderr, "Could not execute command: %s\n", args[0]);
	      if (exitStat < 1)
		exitStat = 1;
	    }
	  }
	  
	  else { // parent
	    pids[pidCount] = pid;
	    pids = realloc(pids,(pidCount+1)*sizeof(int));
            allocError(pids);
            pidCount++;
	    
	    if (in < 0 || in >= fdCount || out < 0 || out >= fdCount || err < 0 || err >= fdCount) {
	      fprintf(stderr, "Invalid file descriptor\n");
	      if (exitStat < 1)
		exitStat = 1;
	    }
	   
	    else if (in == closefd || out == closefd || err == closefd) {
	      fprintf(stderr, "Cannot access closed file\n");
	      if (exitStat < 1)
		exitStat = 1;
	    }
	  }
	  optind = tempopt;
	  break;
	case 'd':; // verbose
	  vflag = 1;
	  break;
	case 'e':; // rdwr
	  if (vflag == 1)
            verbose(argc, argv, optind - 2);
	  int iofd = open(optarg, O_RDWR|fflags, 0666);
	  fflags = 0; // reset file flags
	  if (iofd < 0) {
            fprintf(stderr, "--rdwr error with file: %s, %s\n", optarg, strerror(errno)\
);
            if (exitStat < 1)
              exitStat = 1;
          }
	  fdMap = realloc(fdMap,(fdCount+1)*sizeof(int));
	  allocError(fdMap);
	  fdMap[fdCount] = iofd;
	  fdCount++;
          break;
	case 'f':; // wait
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);

	  int stat;
	  int retpid;

	  
	  while ((retpid = wait(&stat)) > 0) {
	    int i=0;
	    for (; i < pidCount; i++) {
	      if (retpid == pids[i]) {
		if (WIFSIGNALED(stat)) { // if child terminates with signal
		  int signum = WTERMSIG(stat);
		  if (sigStat < signum)
		    sigStat = signum;
		  int count = 0;
		  int j=0;
		  for (; j < optind; j++) {
		    if (strcmp(argv[j], "--command") == 0) {
		      if (count == i) {
			fprintf(stdout, "signal %d", signum);
			fflush(stdout);
			j += 4;
			while (1) {
			  if (argv[j][0] == '-' && argv[j][1] == '-') {
			    j--;
			    break;
			  }
			  else {
			    fprintf(stdout, " %s", argv[j]);
			    fflush(stdout);
			    j++;
			  }
			}
			fprintf(stdout, "\n");
			fflush(stdout);
			break;
		      }
		      else
			count++;
		    }
		  }
		}
		else if (WIFEXITED(stat)) { // if child terminates normally
		  int exitnum = WEXITSTATUS(stat);
		  if (retpid == pids[i]) {
		    if (exitStat < exitnum)
		      exitStat = exitnum;
		    int count = 0;
		    int j=0;
		    //for (; j<optind; j++) {
		    //fprintf(stdout, "%s ",argv[j]);
		    //}
		    //j=0;
		    for (; j < optind; j++) {
		      if (strcmp(argv[j], "--command") == 0) {
			if (count == i) {
			  fprintf(stdout, "exit %d", exitnum);
			  fflush(stdout);
			  j += 4;
			  while (1) {
			    if (argv[j][0] == '-' && argv[j][1] == '-') {
			      j--;
			      break;
			    }
			    else {
			      fprintf(stdout, " %s", argv[j]);
			      fflush(stdout);
			      j++;
			    }
			  }
			  fprintf(stdout, "\n");
			  fflush(stdout);
			  break;
			}
			else
			  count++;
		      }
		    }
		  }
		}
	      }
	    }
	  }    

	  break;
	case 'g':; // pipe
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);	  

	  int pfd[2];
	  if (pipe(pfd) == -1) {
	    fprintf(stderr, "Error with pipe()");
	  }
	  fdMap	= realloc(fdMap,(fdCount+2)*sizeof(int));
          allocError(fdMap);
	  fdMap[fdCount] = pfd[0]; // read end of pipe
	  fdCount++;
	  fdMap[fdCount] = pfd[1]; // write end of pipe
	  fdCount++;
	  break;
	case 'h': // append
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_APPEND;
	  break;
	case 'i': // cloexec
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_CLOEXEC;
	  break;
	case 'j': // creat
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_CREAT;
	  break;
	case 'k': // directory
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_DIRECTORY;
	  break;
	case 'l': // dsync
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_DSYNC; 
	  break;
	case 'm': // excl
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_EXCL;
	  break;
	case 'n': // nofollow
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);	  
	  fflags |= O_NOFOLLOW;
	  break;
	case 'o': // nonblock
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_NONBLOCK;
	  break;
	case 'p': // rsync
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);	  
	  fflags |= O_RSYNC;
	  break;
	case 'q': // sync
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);	  
	  fflags |= O_SYNC;
	  break;
	case 'r': // trunc
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  fflags |= O_TRUNC;
	  break;
	case 's':; // close
	  if (vflag == 1)
            verbose(argc, argv, optind - 2);
	  closefd = atoi(optarg);
	  close(fdMap[closefd]);
	  break;
	case 't': // abort
	  if (vflag == 1)
	    verbose(argc, argv, optind - 1);
	  // force segfault
	  raise(11);
	  break;
	case 'u':; // catch
	  if (vflag == 1)
	    verbose(argc, argv, optind - 2);
	  cflag = 1;
	  sig = atoi(optarg);
	  signal(sig, sighandler);
	  break;
	case 'v': // ignore
	  if (vflag == 1)
            verbose(argc, argv, optind - 2);
	  sig = atoi(optarg);
	  signal(sig, SIG_IGN);
	  break;
	case 'w': // default
	  if (vflag == 1)
            verbose(argc, argv, optind - 2);
	  sig = atoi(optarg);
	  signal(sig, SIG_DFL);
	  break;
	case 'x': // pause
	  if (vflag == 1)
            verbose(argc, argv, optind - 1);
	  pause();
	  break;
	default: // invalid option
	  exitStat = 1;
	}
    }

  free(fdMap);

  if (sigStat != 0) {
    signal(sigStat, SIG_DFL);
    raise(sigStat);
    exit(sigStat);
  }
  exit(exitStat);
}

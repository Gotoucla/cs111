
// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145

// copy of simpsh.c
// need to work more to enhance 
// simpsh.c: fail 7, case 9, 11 pass, fd works. command part can't work
// lab1a: all pass
// copy of lab1a
// begin lab1b
// 11 passed, 16 failed
// fix pipe: close all fds except 0, 1, 2 before execlp
// 12 passed, 15 failed
// fix test case 10: use the fflush
// 13 passed, 14 failed
// test case 11:  pass. The comment should be "11 caught". I wrote "catch 11" before. :|
// 15 passed, 12 failed
// test case 15: pass. I should check whether the fd is close when I enter the fd for command
// 16 passed, 11 failed
// test case 17: passed. But there is a very big bug. For some reason it passed.
// 18 passed, 9 failed
// test case 18: passed. take a lot of time. the main reason is that i close all fd before i do the wait 
// test case 19: passed. the reason is that i close all the input, output, error fds in child process.
// 21 passed, 6 failed
// copy of simpsh2.c
// test case 22: passed. The reason is that I only search for the command and get the options after that, but sometimes commands are the same. I have to diffentiate which one is which.
// 22 passed, 5 failed
// copy of simpsh3.c
// test case 23: passed. Modified signal exit. 
// 25 passed, 2 failed
// test case 26: pass. Implement the chdir option
// copy of simpsh5.c
// 


#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/types.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>

int fCount = 0;
int pid_count = 0;
int command_count = 0;
int exitFinal = 0;

void fdCheck(int fdnum) {
  if (fdnum >= fCount){
    fprintf(stderr, "Invalid fd: %d\n", fdnum);
    exitFinal = 1;
  }
}


void allocCheck(void *arg) {
  if (arg == NULL) {
    free(arg);
    fprintf(stderr, "Memory Allocation Error\n");
    exit(1);
  }
}

void verbose(int argc, char* argv[], int index)  {
  fprintf(stdout, "%s ", argv[index]);
  fflush(stdout);
  index++;
  while (index < argc && (argv[index][0]!= '-' || argv[index][1] != '-')) {
    fprintf(stdout, "%s ",argv[index]);
    fflush(stdout);
    index++;
  }
  fprintf(stdout, "\n");
  fflush(stdout);
}

//typedef void (*signal_handler)(int);
void signal_handler(int signum) {
    fprintf(stderr,"%d caught \n", signum);
    fflush(stdout);
    exit(signum);
}


int main(int argc, char * argv[]) {

  int *fdArr = (int*)malloc(sizeof(int));
  int *pid_arr = (int*)malloc(sizeof(int));
  // char **command_arr = (char**)malloc(sizeof(char*));
  char *command_arr[5] = {"1", "2", "3", "4", "5"};
  
  
  int temp = 0;
  // int pfd[2]; // for pipe
  static int vFlag = 0; // for verbose
  int opt = 0;
  int option_index = 0;
  //char *string = "r:w:c:";
  char *input_file = NULL;
  char *output_file = NULL;
  char *rdwr_file = NULL;
  int input_fd, output_fd, rdwr_fd, close_fd;
  int errorNum = 0;
  int ret = 0;
  int pr = 0;

  int sigStat = 0;
 


  int exe_status = 0;


  int flags = 0; // for open-time flags


  int seg_ret = 0; // for abort
  int signum;
  int abort_flag = 0;
  int tempopt;
  int dir_count = 0;
  char *dir=NULL;

  
 
  
  static struct option long_options[] = {
					 // File open-time flags
					 {"append", no_argument, NULL, 'A'},
					 {"cloexec", no_argument, NULL, 'o'},					
					 {"creat", no_argument, NULL, 'C'},
					 {"directory", no_argument, NULL, 'd'},
					 {"dsync", no_argument, NULL, 'D'},
					 {"excl", no_argument, NULL, 'e'},
					 {"nofollow", no_argument, NULL, 'n'},
					 {"nonblock", no_argument, NULL, 'N'},
					 {"rsync", no_argument, NULL, 's'},
					 {"sync", no_argument, NULL, 'S'},
					 {"trunc", no_argument, NULL, 't'},

					 // File-Opening options
					 {"rdonly", required_argument, NULL, 'r'},
					 {"rdwr", required_argument,  NULL,  'R'},
					 {"wronly", required_argument, NULL, 'w'},
					 {"pipe", no_argument,  NULL, 'p'},


					 //subcommand options
					 {"command", required_argument, NULL, 'c'},
					 {"wait", no_argument, NULL, 'z'},

					 // miscellaneous options
					 {"chdir",required_argument,NULL, 'L'},
					 {"close",required_argument,NULL , 'q'},
					 {"verbose", no_argument,NULL, 'v'},
					 {"abort", no_argument, NULL, 'b'},
					 {"catch", required_argument, NULL , 'h'},
					 {"ignore", required_argument, NULL, 'i'},
					 {"default", required_argument, NULL, 'u'},
					 {"pause", no_argument, NULL, 'P'},
					 
					 {0, 0, 0,  0}
  };

  
  while(1) {
    opt = getopt_long(argc, argv, "", long_options, NULL);

    if (opt == -1)
      break;

    switch (opt) {

      // open-times flags
    case 'A': // append
      //   if (vFlag)
  
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_APPEND;
      break;
      
    case 'o': // cloexec
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_CLOEXEC;
      break;

    case 'C': // creat
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_CREAT;
      break;

    case 'd': // directory
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_DIRECTORY;
      break;

    case 'D': // dsync
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_DSYNC;
      break;

    case 'e': // excl
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_EXCL;
      break;

    case 'n': // nofollow
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_NOFOLLOW;
      break;
      
    case 'N': // nonblock
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_NONBLOCK;
      break;
      
    case 's': // rsync
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_RSYNC;
      break;
      
    case 'S': // sync
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_SYNC;
      break;
      
    case 't': // trunc
      if (vFlag)
	verbose(argc, argv, optind - 1);
      flags |= O_TRUNC;
      break;

      // File-Opening options
    case 'r': //rdonly
      input_file = optarg;
      if (vFlag)
	verbose(argc, argv, optind - 2);
      input_fd = open(input_file, O_RDONLY|flags, 0666);
      flags = 0;
     
      if (input_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Unable to open the file %s\n", input_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	fflush(stdout);
	exitFinal = 1;
      }
     
      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      allocCheck(fdArr);
      
      fdArr[fCount] = input_fd;
      fCount++;	
      break;
      
    case 'R': // rdwr
      rdwr_file = optarg;
      if (vFlag)
	verbose(argc, argv, optind - 2);
      
     
      rdwr_fd = open(rdwr_file, O_RDWR |flags, 0666);
      flags = 0;
     
      if (rdwr_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Unable to open the file %s\n", rdwr_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	fflush(stdout);
	exitFinal = 1;
      }

      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      allocCheck(fdArr);
      fdArr[fCount] = rdwr_fd;
      fCount++;
      break;
            
    case 'w': //wronly
      output_file = optarg;
      
      if (vFlag)
	verbose(argc, argv, optind - 2);
      output_fd = open(output_file, O_WRONLY|flags, 0666);
      flags = 0;
      if (output_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Unable to open the file %s\n", output_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	fflush(stdout);
	exitFinal = 1;
      }

      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      allocCheck(fdArr);
      fdArr[fCount] = output_fd;
      fCount++;
      break;


    case 'p': // pipe

      if (vFlag)
	verbose(argc, argv, optind - 1);
      
      int pfd[2];
      if(pipe(pfd) == -1) {
	fprintf(stderr, "Can't create the pipe\n.");
	fflush(stdout);
	
	//	exitFinal=1;
      }
      fdArr = realloc(fdArr, (fCount+2)*sizeof(int));
      allocCheck(fdArr);
      fdArr[fCount] = pfd[0];
      fCount++;
      fdArr[fCount] = pfd[1];
      fCount++;      
      break;

      //subcommand options
    case 'c'://command
      optind--;
      tempopt = optind;
      char *command = NULL;
      int command_begin = optind;
      int in = atoi(argv[command_begin++]);
      fdCheck(in);
      if (fdArr[in] == -1) {
	fprintf(stderr, "the fd %d is closed\n", in);
	exitFinal = 1;
      }
	 
      int o = atoi(argv[command_begin++]);
      fdCheck(in);
      if (fdArr[o] == -1) {
	fprintf(stderr, "the fd %d is closed\n", o);
	exitFinal = 1;
      }
      int e = atoi(argv[command_begin++]);
      fdCheck(in);
      if (fdArr[e] == -1) {
	fprintf(stderr, "the fd %d is closed\n", e);
	exitFinal = 1;
      }
      if (argv[command_begin][0] == '-' && argv[command_begin][1] == '-') {
	
	fprintf(stderr, "missing command\n");
	fflush(stdout);
	exitFinal = 1;
      }
      else 
	command = argv[command_begin];
     
      int index = command_begin;

      int i=0;
      char *argument[10];
      
      while( index < argc && (argv[index][0] !='-' || argv[index][1] != '-')) {
	if (argv[index][0] == '-')
	  optind = index + 1;
	argument[i] = argv[index];
	i++;
	index++;
	
      }
     
      argument[i] = NULL;
      //optind += i;
      //optind += 4;

      //int length = sizeof(argument) / sizeof(int);
      if (vFlag) {
	fprintf(stdout, "--command ");
	fprintf(stdout, "%d ", in);
	fprintf(stdout, "%d ", o);
	fprintf(stdout, "%d ",e);
	fflush(stdout);
  
	//for (int d=0; d<length-2; d++) {
	//fprintf(stdout, "%s ", argument[d]);
	//fprintf(stdout, "\n");
	//}
	i = 0;
	while (argument[i] != '\0'){
	  fprintf(stdout,"%s ",argument[i]);
	  fflush(stdout);
	  i++;
	}
	fprintf(stdout,"\n");
	fflush(stdout);
	
      }
       
      ret = fork();
      if (ret < 0 ) { // fail
	fprintf(stdout, "Unable to create child process\n");
	exit(1);
      }
      else if (ret == 0) { // child process
	
	//fprintf(stdout,"command received is %s\n",command);
	//fprintf(stdout,"i am the child process. pid of process %d\n",getpid());

	
	// redirect stdin

	dup2(fdArr[in],0);
	//close(fdArr[in]);

	// redirect stdout

	dup2(fdArr[o],1);
	//close(fdArr[o]);

	// redirect stderr

	dup2(fdArr[e],2);
	//close(fdArr[e]);

	int k;
	for (k=0;k<fCount;k++) {
	  close(fdArr[k]);
	}

       	
	exe_status = execvp(command, argument);
	if (exe_status == -1) {
	  fprintf(stderr, "Fail to execute the command: %s\n", command);
	  fflush(stdout);
	  if (exitFinal < 1)
	    exitFinal = 1;
	}	
      }
      else { // parent process
	//pid_arr[pid_count] = ret;
	//command_arr[pid_count] = command;
	//fprintf(stdout,"command received is %s\n",command);
	//fprintf(stdout,"i am the parent. pid of my child process %d\n",ret);
	//fflush(stdout);
	//command_count++;
	
	
	//pid_arr = realloc(pid_arr, (pid_count+1)*sizeof(int));
	//allocCheck(pid_arr);

	//command_arr[pid_count] = malloc((pid_count+1)*sizeof(char));
	//allocCheck(command_arr);
	
     
	//pid_count++;
	pid_arr[pid_count] = ret;
	pid_arr = realloc(pid_arr, (pid_count+1)*sizeof(int));
	allocCheck(pid_arr);
	command_arr[pid_count] = command;
	//fprintf(stdout, "receive a new command. the command_count is %d \n", command_count);
	//fprintf(stdout, "receive a new command. the command put in the command arr is %s \n", command_arr[command_count]);
	//fprintf(stdout, "the process received is %d \n", ret);
	//fprintf(stdout, "the process is %d after i put in the process array \n", pid_arr[pid_count]);	

	command_count++;
	pid_count++;
	//fprintf(stdout, "pid_count=%d, command_count=%d after add 1\n", pid_count,command_count);
	//int count = 0;

	   
	//for (;count<argc;count++) {
	//fprintf(stdout, "%s ",argv[count]);
	//fflush(stdout);
	//}
	//count=0;
	//fflush(stdout);
	
      	optind = index;
      }
      break;

    case 'z': //wait
      if (vFlag)
	verbose(argc, argv, optind - 1);
 

      int ret_pid;
      int status;

      while (1) {
	ret_pid=wait(&status);	
	
	if (ret_pid == -1) {
	  //fprintf(stdout, "all the child process finish. we need to leave.\n");
	  //fflush(stdout);	  
	  break;
	}

	else if (ret_pid > 0) {
	  int l=0;
	  // get the command first 
	  for (;l<pid_count;l++) {	  
	    if (pid_arr[l] == ret_pid) { // we have found a process. we need to find the command 
	      char* command_now = command_arr[l];
	      //fprintf(stdout, "I am the command %s\n", command_now);
	      //fflush(stdout);
	    
	      // get exit code
	      if (WIFEXITED(status)) {	  
		int exit_code =  WEXITSTATUS(status);
		if (exitFinal < exit_code)
		  exitFinal = exit_code;
		fprintf(stdout,"exit %d %s ", exit_code, command_now);
	      
		// get options
		int m=0;
		int count = 0;
		// get the options
		for (;m <optind; m++) {
		  // if find the command
		  if (strcmp(argv[m],"--command") == 0) {
		    if (count == l)
		      break;		    		
		    else 
		      count ++;		  	    	    
		  }
		}

		m+=5;
		  // find next long option; then break
		while (argv[m][1] != '-' ||  argv[m][0] != '-') {
		  fprintf(stdout, "%s ", argv[m]);
		  fflush(stdout);
		  m++;
		}
		fprintf(stdout, "\n");
		fflush(stdout);
	      }
	  

	      else if (WIFSIGNALED(status)) {
		int signal_exit = WTERMSIG(status);	
		if (sigStat < signal_exit)
		  sigStat = signal_exit;
		//int count = 0;
		int m=0;
		int count = 0;
		
		fprintf(stdout,"signal %d %s ", signal_exit, command_now);
	      
				// get option	      
		// get the options
		for (;m <optind; m++) {
		  // if find the command
		  if (strcmp(argv[m],"--command") == 0) {
		    if (count == l)
		      break;		    		
		    else 
		      count ++;		  	    	    
		  }
		}


		m+=5;
		//fprintf(stdout, "%s ", argv[m][0]);
		//fprintf(stdout, "%s ", argv[m][1]);
		//fflush(stdout);
		// find next long option; then break
		while (argv[m][1] != '-' ||  argv[m][0] != '-') {
		  fprintf(stdout, "%s ", argv[m]);
		  fflush(stdout);
		  m++;
		}
	
		fprintf(stdout, "\n");
		fflush(stdout);
	      }
	    }
	  }
	}
      }
    	
      break;

      // miscellaneous options
    case 'L': //chdir
      
       if (vFlag)
	verbose(argc, argv, optind - 2);
       //fprintf(stdout, "i am in chdir \n");
       //fflush(stdout);
 
       //fprintf(stdout, "dir_count is %d \n", dir_count);
       fflush(stdout);
       if (dir_count == 0) {
	 dir = optarg;
	 dir_count++;
	 
	 //fprintf(stdout, "dir is %s \n", dir);
	 //fflush(stdout);
	 if (chdir(optarg)==-1) {
	   fprintf(stdout, "Change to the directory %s fails\n", dir);
	   fflush(stdout);
	 }
       }
	 
       
       else {
	 if (strcmp(dir, optarg) == 0) {
	   fprintf(stderr, "the same as before %s \n", dir);
	   fflush(stderr);
	   exitFinal = 1;
	 }
	 else {
	   dir = optarg;
	   //fprintf(stdout, "dir is %s \n", dir);
	   //fflush(stdout);
	   dir_count++;
	   if (chdir(optarg)==-1) {
	     fprintf(stdout, "Change to the directory %s fails\n", dir);
	     fflush(stdout);
	   }
	 }
       }
     
	
      break;
    case 'q': // close
      if (vFlag)
	verbose(argc, argv, optind - 2);
      close_fd = atoi(optarg);
      fdCheck(close_fd);
      close(fdArr[close_fd]);
      fdArr[close_fd]= -1;
      break;
      
    case 'v': // verbose
      vFlag = 1;
      
      break;
      
    case 'b': // abort
      if (vFlag)
	verbose(argc, argv, optind - 1);
      //seg_ret = raise(11);
      if (raise(SIGSEGV) != 0) {
	fprintf(stderr, "Error aborting code");
	fflush(stdout);
      }
      //abort_flag = 1;
      //if (seg_ret !=0 ) {
      //fprintf(stderr, "Can't raise a segmentation fault.\n");
      //exitFinal = 1;
      //}
      break;
      
    case 'h': // catch
      if (vFlag)
	verbose(argc, argv, optind - 2);
      signum = atoi(optarg);
      // if (abort_flag) 
      //break;
      //else {

      signal(signum,signal_handler);
      //if(signal(signum,&signal_handler) == SIG_ERR) {
      //fprintf(stderr, "Can not catch the signal\n");
      //exitFinal = 1;
      //}
	//  }
      break;
      
    case 'i': // ignore
      if (vFlag)
	verbose(argc, argv, optind - 2);
      signum = atoi(optarg);
      signal(signum,SIG_IGN);
      break;
      
    case 'u': // default
      if (vFlag)
	verbose(argc, argv, optind - 2);
      signum = atoi(optarg);
      signal(signum,SIG_DFL);
      break;
      
    case 'P': // pause
      if (vFlag)
	verbose(argc, argv, optind - 1);
      pause();
      break;
      
    default:
      
      exitFinal = 1;          
      
    }
  }
  // int z;
  //for (z=0;z<fCount;z++)
  //close(fdArr[z]);
  free(fdArr);
  free(pid_arr);
  // free(command_arr);

  if (sigStat != 0) {
    signal(sigStat, SIG_DFL);
    raise(sigStat);
    exit(sigStat);
  }
  exit(exitFinal);
}
  
  

   
    
    

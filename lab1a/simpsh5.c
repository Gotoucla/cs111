// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145
// copy of simpsh4.c
// need to work more to enhance 
// simpsh.c: fail 7, case 9, 11 pass, fd works. command part can't work
// simpsh4.c: fail 1, pass 14


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

int main(int argc, char * argv[]) {
  static int vFlag = 0;
  int opt = 0;
  int option_index = 0;
  char *string = "r:w:c:";
  char *input_file = NULL;
  char *output_file = NULL;
  int input_fd, output_fd;
  int errorNum = 0;
  int ret = 0;
  int *fdArr;


  int max = 100;
  int fCount = 0;
  int exe_status = 0;
  int exitFinal = 0;
  
  
  
  
  static struct option long_options[] = {
					 {"rdonly", required_argument, NULL, 'r'},
					 {"wronly", required_argument, NULL, 'w'},
					 {"verbose", no_argument, &vFlag, 1},
					 {"command", required_argument, NULL, 'c'},
					 {0, 0, 0,  0}
  };

  fdArr = (int*)malloc(max*sizeof(int));
  if (fdArr == NULL) {
    fprintf(stderr, "Memory Allocation error. \n");
    exit(1);
  }
  while(1) {
    opt = getopt_long(argc, argv, string, long_options, &option_index);

    if (opt == -1)
      break;

    switch (opt) {
    case 0:
      
	//fprintf(stdout,"--verbose\n" );
      //tempopt = optind;
      //fprintf(stdout,"%s ", argv[tempopt]);
      //tempopt++;
      //while (tempopt < argc) {
      //fflush(stdout);
      //tempopt++;
      //}
      break;
	
      
      
      
    case 'r':
      input_file = optarg;
      
      if (vFlag) {
	fprintf(stdout, "--rdonly ");
	fprintf(stdout, input_file);
	fprintf(stdout, " ");
      }
      input_fd = open(input_file, O_RDONLY);
      if (input_fd < 0) {
	errorNum = errno;
	
	fprintf(stderr, "Unable to open the file %s\n", input_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exitFinal = 1;
	
      }
      //fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      //if (fdArr == NULL) {
      //free(fdArr);
      //fprintf(stderr, "Memory not allocated.\n");
      //exit(1);
      //}
      fdArr[fCount] = input_fd;
      fCount++;

      if (fCount >= max) {
	max = max*2;
	fdArr=(int*)realloc(fdArr, sizeof(int)*max);
      }
      if (fdArr == NULL) {
	fprintf(stderr, "Realloc error.\n");
	exit(1);
      }
	
      
	
      break;
    case 'w':
      output_file = optarg;
      
      if (vFlag) {
	fprintf(stdout, "--wronly ");
	fprintf(stdout, output_file);
	fprintf(stdout, " ");
      }
      output_fd = open(output_file, O_WRONLY);
      if (output_fd < 0) {
	errorNum = errno;
	
	fprintf(stderr, "Unable to open the file %s\n", output_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exitFinal = 1;
	
      }

      fdArr[fCount] = output_fd;
      fCount++;
      if (fCount >= max) {
	max = max*2;
	fdArr=(int*)realloc(fdArr, sizeof(int)*max);
      }
      if (fdArr == NULL) {
	fprintf(stderr, "Realloc error.\n");
	exit(1);
      }
      

      
      break;
    case 'c':
      optind--;
      
      int command_begin = optind;
      int in = atoi(argv[command_begin++]);
      //printf("input: %i\n", in);
      int o = atoi(argv[command_begin++]);
      //printf("output: %i\n", o);
      int e = atoi(argv[command_begin++]);
      //printf("error: %i\n", e);

      char *command = argv[command_begin];
      

      int index = command_begin;

      int i=0;
      char *argument[10];

      
      while( index < argc && (argv[index][0] !='-' || argv[index][1] != '-')) {
	if (argv[index][0] == '-')
	  optind = index + 1;
	argument[i++] = argv[index++];
	//fprintf(stdout,"argument: %s\n", argument[i]);
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
	fprintf(stdout, "%s ", argv[--optind]);
  
	//for (int d=0; d<length; d++)
	//fprintf(stdout, "%s ", argument[d]);
	
      }
	

     
      //printf("The next operand to handle is: %s\n", argv[optind]);

      ret = fork();
      if (ret < 0 ) { // fail
	//fprintf(stderr, "Can't creat a child process\n");
	exitFinal=1;
      }
      else if (ret == 0) { // child process
	// redirect stdin
	//printf("the child (pid: %d) \n", (int) getpid());
	//close(0);
	dup2(fdArr[in],0);
	close(fdArr[in]);

	// redirect stdout
	//close(1);
	dup2(fdArr[o],1);
	close(fdArr[o]);

	// redirect stderr
	//close(2);
	dup2(fdArr[e],2);
	close(fdArr[e]);

	exe_status = execvp(command, argument);
	if (exe_status < 0) {
	  fprintf(stderr, "Fail to execute the command: %s\n",command);
	  exitFinal = 1;
	  
	}	
      }
      else { // parent process
	if (in >= fCount || o >= fCount || e >= fCount) {
	  //fprintf(stderr, "fd number exceed the total fd number\n");
	  exitFinal = 1;
	  
	}
		
      }   
      break;
    default:
     
      exitFinal = 1;
      
    }
  }
  
  free(fdArr);
  exit(exitFinal);
}

   
    
    

// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145

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
  pid_t ret = 0;
  int *fdArr = (int*)malloc(100*sizeof(int));
  int fCount = 0;
  int exe_status = 0;

   static struct option long_options[] = {
					 {"rdonly", required_argument, NULL, 'r'},
					 {"wronly", required_argument, NULL, 'w'},
					 {"verbose", no_argument, &vFlag, 1},
					 {"command", required_argument, NULL, 'c'},
					 {0, 0, 0,  0}
  };
   while(1) {
    opt = getopt_long(argc, argv, string, long_options, &option_index);

    if (opt == -1)
      break;

    switch(opt) {
    case 0:
      if (vFlag)
	fprintf(stdout,"--verbose\n" );
      break;

    case 'r':
      input_file = optarg;
      
      if (vFlag) {
	//write (1, "vFlag = 1\n", 10);
	write (1, "--rdonly ",9);
	write (1, input_file, strlen(input_file));
	write (1, "\n", 1);
      }
      input_fd = open(input_file, O_RDONLY);
      if (input_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Option: -%c\n", opt);
	fprintf(stderr, "Unable to open the file %s\n", input_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exit(1);
      }
      //fprintf(stdout, "input_file is: %s; input_fd: %d\n", input_file,input_fd);

      fdArr[fCount] = input_fd;
      //fprintf(stdout, "fdArr[%d]=%d\n",fCount,fdArr[fCount]);
      fCount++;
      break;

    case 'w':
      output_file = optarg;
      
      if (vFlag) {
	//write (1, "vFlag = 1\n", 10);
	write (1, "--wdonly ",9);
	write (1, output_file, strlen(output_file));
	write (1, "\n", 1);
      }
      output_fd = open(output_file, O_WRONLY);
      if (output_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Option: -%c\n", opt);
	fprintf(stderr, "Unable to open the file %s\n", output_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exit(1);
      }
      //fprintf(stdout, "output_file is: %s; output_fd: %d\n", output_file, output_fd);
      fdArr[fCount] = output_fd;
      //fprintf(stdout, "fdArr[%d]=%d\n",fCount,fdArr[fCount]);
      fCount++;
      break;

    case 'c':
      optind--;
      int command_begin = optind;
      int in = atoi(argv[command_begin++]);
      //fprintf(stdout,"input: %i\n", in);
      int o = atoi(argv[command_begin++]);
      //fprintf(stdout,"output: %i\n", o);
      int e = atoi(argv[command_begin++]);
      //fprintf(stdout,"error: %i\n", e);

      char *command = argv[command_begin++];
      //fprintf(stdout,"command: %s\n", command);

      int index = command_begin;

      int i=0;
      char *argument[10];
      argument[0]=command;
      i = 1;

      //fprintf(stdout, "index=%d, argc=%d\n", index, argc);

      if (index == argc) {
	// no argument
	for (int j=1; j<10; j++)
	  argument[j] = NULL;
      }
      else {
	// multiple argument
	while (index < argc && argv[index][0] != '-') {
	  
	  argument[i] = argv[index];
	  //fprintf(stdout, "argument:%s; i:%d; index:%d\n", argument[i], i, index);
	  i++;
	  index++;
	  
	  
	}
	argument[i] = NULL;
      }
      optind += i;
      optind += 4;
      //fprintf(stdout, "after collecting the argument, optind=%d\n", optind);

      int start = 1;
      if (vFlag) {
	//write (1, "vFlag = 1\n", 10);
	write (1, "--command ",strlen("--command "));
	while(start < i) {
	  write(1, argument[start], strlen(argument[start]));
	  write(1, " ", 1);
	  start++;
	}
	write(1, "\n", 1);
      }
		
	
      dup2(fdArr[in],0);
      close(fdArr[in]);

	// redirect stdout

      dup2(fdArr[o],1);
      close(fdArr[o]);

	// redirect stderr
      dup2(fdArr[e],2);
      close(fdArr[e]);

      ret = fork();
      if (ret < 0 ) { // fail
	
	fprintf(stderr, "Can't creat a child process.\n");
	exit(1);
      }
      else if (ret == 0) { // child process
	// redirect stdin
	//printf("the child (pid: %d) \n", (int) getpid());
	
	exe_status = execvp(command, argument);
	if (exe_status < 0) {
	  fprintf(stderr, "Fail to execute the command: %s\n", command);
	  exit(1);
	}
	
      }
      else if (ret > 0) { // parent process
	int rc_wait = wait(NULL);
	if (in >= fCount || o >= fCount || e >= fCount){
	  errorNum = errno;
	  fprintf(stderr, "Can't exceed the maximum of the fd. %s\n", strerror(errorNum));
	  exit(1);
	}
	//printf("the parent of %d (rc_wait: %d) (pid: %d)\n", ret, rc_wait, (int)getpid());
	
      }
      
      break;
    default:
      fprintf(stderr, "Invalid option %s\n", argv[option_index+1]);
      exit(1);
    }
   }
   exit(0);
}
   
  

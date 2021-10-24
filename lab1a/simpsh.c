// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145
// fail 8

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

    switch (opt) {
    case 0:
      if (vFlag)
	fprintf(stdout,"--verbose\n" );
      break;
      
      
    case 'r':
      input_file = optarg;
      //fprintf(stdout,"input file is: %s\n", input_file);
      //if (vFlag) {
      //write (1, "vFlag = 1\n", 10);
      //write (1, "--rdonly ",9);
      //write (1, input_file, strlen(input_file));
      //write (1, "\n", 1);
      //}
      input_fd = open(input_file, O_RDONLY);
      if (input_fd < 0) {
	errorNum = errno;
	//fprintf(stderr, "Option: -%c\n", opt);
	fprintf(stderr, "Unable to open the file %s\n", input_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exit(1);
      }
      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      if (fdArr == NULL) {
	free(fdArr);
	fprintf(stderr, "Allocation Error");
	exit(1);
      }
      fdArr[fCount] = input_fd;
      fCount++;
      
	
      break;
    case 'w':
      output_file = optarg;
      //fprintf(stdout,"output file is: %s\n", output_file);
      //if (vFlag) {
      //write (1, "vFlag = 1\n", 10);
      //write (1, "--wdonly ",9);
      //write (1, output_file, strlen(output_file));
      //write (1, "\n", 1);
      //}
      output_fd = open(output_file, O_WRONLY);
      if (output_fd < 0) {
	errorNum = errno;
	//fprintf(stderr, "Option: -%c\n", opt);
	fprintf(stderr, "Unable to open the file %s\n", output_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exit(1);
      }
      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      if (fdArr == NULL) {
	free(fdArr);
	fprintf(stderr, "Allocation Error");
	exit(1);
      }
      fdArr[fCount] = output_fd;
      fCount++;

      
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

      char *command = argv[command_begin++];
      //printf("command: %s\n", command);

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

     
      //printf("The next operand to handle is: %s\n", argv[optind]);

      ret = fork();
      if (ret < 0 ) { // fail
	//fprintf(stderr, "Can't creat a child process\n");
	exit(1);
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

	exe_status = execvp(argument[0], argument);
	if (exe_status < 0) {
	  fprintf(stderr, "Fail to execute the command: %s\n", command);
	  exit(1);
	}
	
	
      }
      else if (ret > 0)  { // parent process
	int rc_wait = wait(NULL);
	if (in >= fCount || o >= fCount || e >= fCount) {
	  fprintf(stderr, "Invalid fd\n");
	  exit(1);
	}
	//printf("the parent of %d (rc_wait: %d) (pid: %d)\n", ret, rc_wait, (int)getpid());
	
      }
            
      break;
    default:
      fprintf(stderr, "Invalid option %s\n", argv[option_index]);
      exit(1);      
      
    }
    //free(fdArr);
  
  }
  exit(0);
}

   
    
    

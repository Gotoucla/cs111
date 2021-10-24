// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145
// copy of simpsh.c
// need to work more to enhance 
// simpsh.c: fail 7, case 9, 11 pass, fd works. command part can't work
// all pass

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
  int *fdArr = (int*)malloc(sizeof(int));
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

  
  while(1) {
    opt = getopt_long(argc, argv, string, long_options, &option_index);

    if (opt == -1)
      break;

    switch (opt) {
    case 0:

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
      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      if (fdArr == NULL) {
	free(fdArr);
	fprintf(stderr, "Memory Allocation Error\n");
	exit(1);
      }
      fdArr[fCount] = input_fd;
      fCount++;
      
	
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
      fdArr = realloc(fdArr, (fCount+1)*sizeof(int));
      if (fdArr == NULL) {
	free(fdArr);
	fprintf(stderr, "memoryAllocation Error");
	exit(1);
      }
      fdArr[fCount] = output_fd;
      fCount++;

      
      break;
    case 'c':
      optind--;
      
      int command_begin = optind;
      int in = atoi(argv[command_begin++]);
     
      int o = atoi(argv[command_begin++]);
     
      int e = atoi(argv[command_begin++]);
      

      char *command = argv[command_begin];
      

      int index = command_begin;

      int i=0;
      char *argument[10];

      if (in >= fCount || o >= fCount || e >= fCount) {
	  fprintf(stderr, "Invalid fd\n");
	  exitFinal = 1;
      }
      
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
  
	//for (int d=0; d<length-2; d++) {
	//fprintf(stdout, "%s ", argument[d]);
	//fprintf(stdout, "\n");
	//}
	i = 0;
	while (argument[i] != '\0'){
	  fprintf(stdout,"%s ",argument[i]);
	  i++;
	}
	fprintf(stdout,"\n");
	
      }
       
      ret = fork();
      if (ret < 0 ) { // fail
	
	exit(1);
      }
      else if (ret == 0) { // child process
	// redirect stdin

	dup2(fdArr[in],0);
	close(fdArr[in]);

	// redirect stdout

	dup2(fdArr[o],1);
	close(fdArr[o]);

	// redirect stderr

	dup2(fdArr[e],2);
	close(fdArr[e]);

	exe_status = execvp(command, argument);
	if (exe_status < 0) {
	  fprintf(stderr, "Fail to execute the command: %s\n", command);
	  exitFinal = 1;
	}	
      }
      else { // parent process
      	
      }
      break;
    default:
      
      exitFinal = 1;          
      
    }
  }
  int e;
  for (e=0;e<fCount;e++)
    close(fdArr[e]);
  free(fdArr);
  exit(exitFinal);
}
  
  

   
    
    

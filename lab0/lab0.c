// NAME: Meiyi Zheng
// EMAIL: meiyizheng@yahoo.com
// ID: 605147145

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <errno.h>

void signal_handler() {
    fprintf(stderr, "Catch the segumentation fault! \n");
    exit(4);
}
    
int main(int argc, char *argv[]) {
  int opt = 0;
  int option_index = 0;
  char *string = "i:o:scd";
  int seg_flag = 0;
  int input_fd, output_fd;
  char *input_file = NULL;
  char *output_file = NULL;
  int errorNum = 0;
  char *buffer[sizeof(char)];
  ssize_t count = 0;
  int catch_flag = 0;
  int dump_flag = 0;
  
  static struct option long_options[] = {
    
     {"input", required_argument, NULL, 'i'},
     {"output",required_argument, NULL, 'o'},
     {"segfault", no_argument, NULL, 's'},
     {"catch",no_argument, NULL, 'c'},
     {"dump-core", no_argument, NULL, 'd'},
     {0,0,0,0},
  };

  
  
  while (1) {
    
    opt = getopt_long(argc,argv,string,long_options,&option_index);
    //fprintf(stderr, "argv[] after one getopt_long(): ");
    //for (i=0; i< argc; i++) {
    //fprintf(stderr, "%s, ",argv[i]);
    //}
    //fprintf(stderr, "\n");
    //fprintf(stderr, "opt: %c, optind: %d, optarg: %s\n", opt, optind, optarg);

    if (opt == -1) 
      break;

    switch (opt) {
    case 'i' :
      //fprintf(stdout, "Input file received!! %s\n", optarg);
      input_file = optarg; 
      input_fd = open(input_file, O_RDONLY);
      if (input_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Option: -%c\n", opt);
	fprintf(stderr, "Unable to open the file %s\n", input_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exit(2);
      }
      else {
	close(0);
	dup2(input_fd, 0);
	close(input_fd);
      }
      break;
    case 'o':
      //fprintf(stdout, "Output file received!! %s\n", optarg);
      output_file = optarg;
      output_fd = creat(output_file, 0666);
      if (output_fd < 0) {
	errorNum = errno;
	fprintf(stderr, "Option: -%c\n", opt);
	fprintf(stderr, "Unable to creat the file %s\n", output_file);
	fprintf(stderr, "Error number:  %s\n", strerror(errorNum));
	exit(3);
      }
      else {
	close(1);
	dup2(output_fd, 1);
	close(output_fd);
      }
      
      
      break;
    case 's':
      seg_flag = 1;
      //fprintf(stdout, "Segumentation fault!\n");
    
      break;
    case 'c':
      //fprintf(stdout, "signal(). \n");
      catch_flag = 1;
      signal(SIGSEGV, signal_handler);
      break;
    case 'd':
      dump_flag = 1;
      signal(SIGSEGV,SIG_DFL);
      break;
    default:
      
      fprintf(stderr, "Usage: lab0 [--input=input_file] [--output=output_file] [--catch] [--dump-core] [--segfault] \n");
      exit(1);
    }
  }

  if (seg_flag == 1) {
     char *p = NULL;
     *p = 'a';
  }

  if (catch_flag && !dump_flag)
    signal(SIGSEGV, signal_handler);

  if (catch_flag && dump_flag) {
    signal(SIGSEGV,SIG_DFL);
    exit(139);
  }
  
    
  
    
  if (seg_flag == 0){
    // read from input to output
    count = read(0,buffer,1);
    while(count > 0) {
      if(write(1,buffer,1)<0) {
	errorNum = errno;
	fprintf(stderr,"Unable to write to the file: %s\n", strerror(errorNum));
	exit(2);
      }
      else 
	count = read(0,buffer,1);
    }
  }
  close(0);
  close(1);
  exit(0);
}
      
      

      
      
      
      
      
      
       
		     
										   
      
    
  
    
  

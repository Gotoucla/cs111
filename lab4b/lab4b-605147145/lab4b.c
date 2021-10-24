// NAME: Meiyi Zheng
// EMAIL: meiyizheng@g.ucla.edu
// ID: 605147145

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <math.h>

#include <time.h>

#ifdef DUMMY
#define	MRAA_GPIO_IN	0
typedef int mraa_aio_context;
typedef int mraa_gpio_context;
…
int mraa_aio_read(mraa_aio_context c)    {
	return 650;
}
void mraa_aio_close(mraa_aio_context c)  {
}
…
#else
#include <mraa.h>
#include <mraa/aio.h>
#endif


#define B 4275
#define R0 100000.0


int period = 1;
int cflag = 0;
int fflag = 0;
int log_flag = 0;
FILE* log_file;
int report = 1;

mraa_gpio_context button;
mraa_aio_context temp_sensor;

struct timespec curr;
void get_current_time( struct timespec ts)
{
  if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
    fprintf(stderr,"Could not get the current time\n");
    exit(1);
  }
  //struct tm * tm;
  //tm = localtime(&(ts.tv_sec));
  //if (tm == NULL) {
  //fprintf(stderr,"Could not get the local time\n");
  // exit(1);
  //}
   
  //fprintf(stdout,“hour:%d, min:%d, second: %d\n”, tm->tm_hour, 
  //	  tm->tm_min, tm->tm_sec);
}
void shut_down() {
  struct timespec final;
  clock_gettime(CLOCK_REALTIME, &final);
  struct tm * tm;
  tm = localtime(&(final.tv_sec));
  if (tm == NULL) {
    fprintf(stderr,"Could not get the local time\n");
    exit(1);
  }
  fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
  if (log_flag)
    fprintf(log_file, "%02d:%02d:%02d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
  exit(0);  
}

void parse_command(char* command) {
  if (strcmp(command, "SCALE=F") == 0) {
    fflag = 1;
    cflag = 0;
  }
  else if (strcmp(command, "SCALE=C") == 0) {
    cflag = 1;
    fflag = 0;
  }
  else if (strncmp(command, "PERIOD=", 7) == 0) {
    period = atoi(command + 7);
  }

  else if (strcmp(command, "STOP") == 0) {
    report = 0;
  }

  else if (strcmp(command, "START") == 0) {
    report =  1;
  }
  
  else if (strcmp(command, "OFF") == 0) {
    shut_down();
  }
  else if (strncmp(command, "LOG", 3) == 0) {
  }
  else {
    fprintf(stderr, "Invalid command\n");
    exit(1);
  }
  
}

float convert_temp()
{
  int reading = mraa_aio_read(temp_sensor);
  float R = 1023.0/((float) reading) - 1.0;
  R = R0*R;
  //C is the temperature in Celcious
 
  float C = 1.0/(log(R/R0)/B + 1/298.15) - 273.15;
  //F is the temperature in Fahrenheit
  float F = (C * 9)/5 + 32;

  float temp;

  if (cflag) {
    temp= C;
  }

  else if(fflag) {
    temp= F;
  }
  return temp;
}




int main(int argc, char * argv[]) {
  int opt;
  
  static struct option long_options[] = {
    
     {"period", required_argument, NULL, 'p'},
     {"scale", required_argument,  NULL, 's'},
     {"log", required_argument, NULL, 'l'},
     {0,0,0,0},
  };

  while (1) {
    opt = getopt_long(argc,argv,"",long_options,NULL);
    
     if (opt == -1) 
       break;

     switch (opt) {
     case 'p':
       period = atoi(optarg);
       
       break;

     case 's':
       if (strcmp(optarg,"C") == 0)
	 cflag = 1;
       else if (strcmp(optarg,"F") == 0)
	 fflag = 1;
       else {
	 fprintf(stderr, "Invalid argument: scale either is F or C\n");
	 exit(1);
       }
       
       break;

     case 'l':
       log_file = fopen(optarg, "w+");
       if (log_file == NULL) {
	 fprintf(stderr, "Could not open log file\n");
	 exit(1);
       }
       log_flag = 1;
      
       break;
     default:
       fprintf(stderr, "Usage: ./lab4b --period=seconds --scale=F/C --log=filename\n");
       exit(1);
     }
  }
  
  // device is connected with address 60
  button = mraa_gpio_init(60);
  if (button == NULL) {
    fprintf(stderr, "Failed to initialize GPIO \n");
    
  }

  temp_sensor = mraa_aio_init(1);
  if (temp_sensor == NULL) {
    fprintf(stderr, "Failed to initialize AIO \n");
  }
   
  // set the button to be an input pin
  mraa_gpio_dir(button, MRAA_GPIO_IN);
  mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &shut_down, NULL);

  struct timespec prev;
  clock_gettime(CLOCK_REALTIME, &prev);
  struct tm * tm;
  tm = localtime(&(prev.tv_sec));
  
  
  int size = 0;
  char* command = malloc(sizeof(char));
  if (command == NULL) {
    fprintf(stderr, "Can not assign space\n");
    exit(1);
  }
  
  while (1) {
    
    struct pollfd fds[1]; 
    fds[0].fd = 0;
    fds[0].events = POLLIN;

    int pret = poll(fds, 1, 0);
    if (pret == -1) {
      fprintf(stderr, "The poll function fails\n");
      exit(1);
    }

  
      
    if (pret > 0) { // command ready
      if (fds[0].revents & POLLIN) {
	char buf[1];
	int ret = read(0, buf, sizeof(buf));
	if (ret < 0) {
	  fprintf(stderr, "Can not read from stdin\n");
	  exit(1);
	}

	// read till we meet a "\n"
	if (*buf == '\n') {
	  command[size] = '\0';
	  if (log_flag)
	    fprintf(log_file, "%s\n", command);
	  parse_command(command);
	  free(command);
	  command = malloc(sizeof(char));
	  size = 0;
     	  
	}
	else {
	  command[size] = *buf;
	  size++;
	  command = realloc(command,(size+1)*sizeof(char));
	  
	}
	
      }
    }

    
    //get_current_time(curr);
    struct timespec curr;
    clock_gettime(CLOCK_REALTIME, &curr);
    if (curr.tv_sec >= prev.tv_sec + period && report) {
      
      struct tm * tm2;
      tm2 = localtime(&(curr.tv_sec));
      float temp=convert_temp();
      fprintf(stdout,"%02d:%02d:%02d %.1f\n",tm2->tm_hour, tm2->tm_min, tm2->tm_sec,temp);
     
      clock_gettime(CLOCK_REALTIME, &prev);
      if (log_flag)
	fprintf(log_file,"%02d:%02d:%02d %.1f\n", tm->tm_hour,tm->tm_min, tm->tm_sec,temp);
      
    }
   
    
	    
  }
  
 

  mraa_gpio_close(button);
  mraa_aio_close(temp_sensor);
  fclose(log_file);
  exit(0);
}

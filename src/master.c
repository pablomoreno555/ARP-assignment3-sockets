#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


int spawn(const char * program, char * arg_list[])
{
  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

// Variables needed for the log file
FILE * log_file;
time_t rawtime;
struct tm * timeinfo;
char * timestamp;

// Write the current time in the log file
void write_timestamp ()
{
  time(&rawtime); timeinfo = localtime(&rawtime);
  timestamp = asctime(timeinfo); timestamp[strlen(timestamp)-1] = 0; 
  fprintf(log_file, "%s -- ", timestamp);
}


int main()
{
  // Open the log file
  log_file = fopen("log/master.log", "w");
	if (log_file == NULL) {
    perror("Error while opening the log file");
    return 1;
  }
  
  // Create a named pipe that will be used to communicate with 'processAserver', so that the master knows when the client connects,
  // since 'processB' should not be spawned until that happens
  int fd1;
  char px[80];
  char *myfifo = "/tmp/myfifo";
  mkfifo(myfifo, 0666);
  write_timestamp(); fprintf(log_file, "%s\n", "Pipe created"); fflush(log_file);

  // Variables to store the data entered by the user or received via the previous pipe
  char buff[50];
  char IP[50];
  char port[50];
  int op;

  // Prompt the user to select the execution modality
  printf("\nChoose the execution modality by entering '1', '2' or '3':\n");
  printf(" 1 -> Normal\n");
  printf(" 2 -> Client\n");
  printf(" 3 -> Server\n");
  fgets(buff, sizeof(buff), stdin);

  if (strncmp(buff, "1", 1) == 0) // normal mode -> Assignment 2
  {
    write_timestamp(); fprintf(log_file, "%s\n", "Normal mode selected"); fflush(log_file);

    char * arg_list_A[] = { "/usr/bin/konsole", "-e", "./bin/processA", NULL };
    char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };

    // Spawn 'processA' and 'processB'
    // Since processA is the one initializing the semaphores, we need to make sure that it is launched before processB.
    // That's why a small delay between both process has been added
    pid_t pid_procA = spawn("/usr/bin/konsole", arg_list_A);
    write_timestamp(); fprintf(log_file, "%s\n", "ProcessA spawned"); fflush(log_file);
    sleep(1);
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);
    write_timestamp(); fprintf(log_file, "%s\n", "ProcessB spawned"); fflush(log_file);

    int status;
    waitpid(pid_procA, &status, 0);
    waitpid(pid_procB, &status, 0);
    
    printf ("Main program exiting with status %d\n", status);
    return 0;
  }

  else if (strncmp(buff, "2", 1) == 0) // client mode
  {
    write_timestamp(); fprintf(log_file, "%s\n", "Client mode selected"); fflush(log_file);

    // Prompt the user to the enter the IP address and port number of the server
    char string_ip[50];
    char string_port[50];
    printf("Please, enter the IP ADDRESS of the server you want to connect to:\n");
    fgets(IP, sizeof(IP), stdin);
    sprintf(string_ip, "%s", IP);
    printf("Please, enter the PORT NUMBER of the server you want to connect to:\n");
    fgets(port, sizeof(port), stdin);
    sprintf(string_port, "%s", port);

    char * arg_list_Ac[] = { "/usr/bin/konsole", "-e", "./bin/processAclient", string_ip, string_port, NULL };
    char * arg_list_B[] = { "/usr/bin/konsole", "-e", "./bin/processB", NULL };

    // Spawn 'processAclient' and 'processB'
    pid_t pid_procAc = spawn("/usr/bin/konsole", arg_list_Ac);
    write_timestamp(); fprintf(log_file, "%s\n", "ProcessAclient spawned"); fflush(log_file);
    sleep(1);
    pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);
    write_timestamp(); fprintf(log_file, "%s\n", "ProcessB spawned"); fflush(log_file);

    int status;
    waitpid(pid_procAc, &status, 0);
    waitpid(pid_procB, &status, 0);

    printf("Main program exiting with status %d\n", status);
    return 0;
  }

  else if (strncmp(buff, "3", 1) == 0) // server mode
  {
    write_timestamp(); fprintf(log_file, "%s\n", "Server mode selected"); fflush(log_file);
    
    // Prompt the user to the enter the IP address and port number of the server
    char string_ip[50];
    char string_port[50];
    printf("Please, enter the IP ADDRESS of the client you want to connect to:\n");
    fgets(IP, sizeof(IP), stdin);
    sprintf(string_ip, "%s", IP);
    printf("Please, enter the PORT NUMBER of the client you want to connect to:\n");
    fgets(port, sizeof(port), stdin);
    sprintf(string_port, "%s", port);

    char *arg_list_As[] = {"/usr/bin/konsole", "-e", "./bin/processAserver", string_ip, string_port, NULL};
    char *arg_list_B[] = {"/usr/bin/konsole", "-e", "./bin/processB", NULL};

    // Spawn 'processAserver'
    pid_t pid_procAs = spawn("/usr/bin/konsole", arg_list_As);
    write_timestamp(); fprintf(log_file, "%s\n", "ProcessAserver spawned"); fflush(log_file);
    sleep(1);

    // Read the pipe to be able to know when the client connects
    fd1 = open(myfifo, O_RDONLY);
    read(fd1, px, 80);
    sscanf(px, "%d", &op);
    close(fd1); // close pipe

    // Only when the client connects, spawn 'processB'
    if (op == 1)
    {
      pid_t pid_procB = spawn("/usr/bin/konsole", arg_list_B);
      write_timestamp(); fprintf(log_file, "%s\n", "ProcessB spawned"); fflush(log_file);

      int status;
      waitpid(pid_procAs, &status, 0);
      waitpid(pid_procB, &status, 0);

      printf("Main program exiting with status %d\n", status);
      return 0;
    }
  }
}


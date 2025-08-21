#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Include header or define the IOCTL call interface and devide name

#define DEVICE_PATH "/dev/my_ioctl_device_2"
#define IOCTL_CHANGE_PARENT_PID _IOWR('a', 1, int) // Set value
#define IOCTL_KILL_CHILDREN _IOWR('a', 2, int) // Get value

//**************************************************

void sigchld_handler(int sig) {
    int status;                                          
    pid_t soldier_pid;
    while ((soldier_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("[Parent]: Soldier process %d terminated\n", soldier_pid);
        fflush(stdout);
    }
    if (soldier_pid == -1 && errno != ECHILD) {
        perror("waitpid");
    } 
}

void sigterm_handler(int sig) {
    printf("[Parent]: Control station %d exiting\n", getpid());
    fflush(stdout);
    
    exit(0);  
}

int open_driver(const char* driver_name) {

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
    }

	return fd_driver;
}

void close_driver(const char* driver_name, int fd_driver) {

    int result = close(fd_driver);
    if (result == -1) {
        perror("ERROR: could not close driver");
    }
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <sleep>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int s_d = atoi(argv[1]);

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    struct sigaction  sa_term;
    sa_term.sa_handler = sigterm_handler;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = SA_RESTART;
    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        perror("sigaction (SIGTERM)");
        exit(1);
    }   
    // printf("Nigga wat\n");
    // open ioctl driver
    int fd;
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return EXIT_FAILURE;
    }
    
    printf("Control station process %u started\n", getpid());

    sleep(s_d); // Wait for all child process until Emergency is initiated. 


    printf("[PARENT]: Emergency Emergency!\n");
    // call ioctl with pid as argument to kill all the child process and itself
    int pid=getpid();

    if (ioctl(fd, IOCTL_KILL_CHILDREN, &pid) < 0) {
        perror("IOCTL_KILL_CHILDREN failed");
        close(fd);
        return EXIT_FAILURE;
    }
	
    // close ioctl driver
    close(fd);
    return 0;

}
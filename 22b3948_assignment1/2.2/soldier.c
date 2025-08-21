#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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
        printf("Usage: %s <parent_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = atoi(argv[1]);


    // open ioctl driver
    int fd = open_driver(DEVICE_PATH);
    //printf("Pre-call : Main hu %d, mera baap %d\n",getpid(),getppid());
    // call ioctl with parent pid as argument to change the parent
    int prev_parent=getppid();
    ioctl(fd,IOCTL_CHANGE_PARENT_PID,&parent_pid);
    printf("[CHILD]: soldier %d changing its parent  from %d to %d\n",getpid(),prev_parent,getppid());
    //printf("Post-call : Main hu %d, mera baap %d\n",getpid(),getppid());

    // close ioctl driver
    close(fd);
    while(1) {
        sleep(1);
    }

	return EXIT_SUCCESS;
}



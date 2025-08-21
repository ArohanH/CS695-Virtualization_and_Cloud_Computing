#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/my_ioctl_device"

#define IOCTL_SET_PHYSICAL_ADDRESS _IOW('a', 1, struct ioctl_list_of_data)
#define IOCTL_GET_PHYSICAL_ADDRESS _IOR('a', 2, struct ioctl_data)

struct ioctl_data {
    unsigned long vaddr;   // Virtual address
    unsigned long paddr;   // Physical address
    char value;            // Value to be written
};

struct ioctl_list_of_data {
    struct ioctl_data *ioctl_data_list; // Array of ioctl_data
    unsigned long count;               // Number of entries
};

int main(int argc, char *argv[]) {
    if(argc!=2){
        fprintf(stderr, "Usage: %s <number_of_pages> <stride_in_bytes>\n",argv[0]);
        return EXIT_FAILURE;
    }
    int fd;
    int count = atoi(argv[1]);; // Size of the memory block
    unsigned char *block;
    struct ioctl_list_of_data data_list;
    struct ioctl_data *ioctl_data_array;

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device file");
        return EXIT_FAILURE;
    }

    // Allocate memory block on the heap
    block = (unsigned char *)malloc(count);
    if (!block) {
        perror("Failed to allocate memory");
        close(fd);
        return EXIT_FAILURE;
    }

    // Initialize the memory block
    printf("Initializing memory block...\n");
    for (size_t i = 0; i < count; i++) {
        block[i] = 104 + i;
        printf("Virtual Address: %p Value: %d\n", &block[i], block[i]);
    }

    // Prepare ioctl data to get physical addresses
    ioctl_data_array = (struct ioctl_data *)malloc(count * sizeof(struct ioctl_data));
    if (!ioctl_data_array) {
        perror("Failed to allocate ioctl data array");
        free(block);
        close(fd);
        return EXIT_FAILURE;
    }

    // Populate ioctl_data_array
    for (size_t i = 0; i < count; i++) {
        ioctl_data_array[i].vaddr = (unsigned long)&block[i];
    }

    // Retrieve physical addresses using ioctl
    printf("\nGetting physical addresses...\n");
    for (size_t i = 0; i < count; i++) {
        if (ioctl(fd, IOCTL_GET_PHYSICAL_ADDRESS, &ioctl_data_array[i]) < 0) {
            perror("IOCTL_GET_PHYSICAL_ADDRESS failed");
            free(block);
            free(ioctl_data_array);
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Virtual Address: %p Physical Address: %lx Value: %d\n",
               (void *)ioctl_data_array[i].vaddr, ioctl_data_array[i].paddr, block[i]);
    }

    // Update memory using physical addresses
    printf("\nUpdating memory values using physical addresses...\n");
    for (size_t i = 0; i < count; i++) {
        ioctl_data_array[i].value = 53 + i;
    }

    data_list.ioctl_data_list = ioctl_data_array;
    data_list.count = count;

    if (ioctl(fd, IOCTL_SET_PHYSICAL_ADDRESS, &data_list) < 0) {
        perror("IOCTL_SET_PHYSICAL_ADDRESS failed");
        free(block);
        free(ioctl_data_array);
        close(fd);
        return EXIT_FAILURE;
    }

    // Verify updated values
    printf("\nVerifying updated memory values...\n");
    for (size_t i = 0; i < count; i++) {
        printf("Virtual Address: %p Value: %d\n", &block[i], block[i]);
    }

    // Clean up
    free(block);
    free(ioctl_data_array);
    close(fd);
    return EXIT_SUCCESS;
}

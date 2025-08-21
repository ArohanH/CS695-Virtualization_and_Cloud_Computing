# Kernel build directory
KDIR = /lib/modules/$(shell uname -r)/build

# User-space programs
TARGETS = control_station soldier
SOURCES = $(TARGETS:=.c)

# Kernel module
KERNEL_MODULE = ioctl_module2.ko
KERNEL_OBJECTS = ioctl_module2.o ioctl_module2.mod.o ioctl_module2.mod.c

all: $(TARGETS) $(KERNEL_MODULE)

# Compile user-space programs
$(TARGETS): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Compile kernel module
$(KERNEL_MODULE): $(KERNEL_SOURCE)
	make -C $(KDIR) M=$(PWD) modules

# Clean up generated files
clean:
	rm -f $(TARGETS) $(KERNEL_OBJECTS) $(KERNEL_MODULE) Module.symvers modules.order
	make -C $(KDIR) M=$(PWD) clean

# PHONY to prevent conflict with files named 'clean' or 'all'
.PHONY: all clean

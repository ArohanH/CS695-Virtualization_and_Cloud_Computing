KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
TARGET := ioctl_driver
USER_APP := ioctl_user1

all: kbuild user

kbuild:
	@echo "Building kernel module..."
	$(MAKE) -C $(KDIR) M=$(PWD) modules

user:
	@echo "Compiling user-space application..."
	gcc -o $(USER_APP) $(USER_APP).c

clean:
	@echo "Cleaning kernel module build files..."
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	@echo "Removing user-space application binary..."
	rm -f $(USER_APP)

install: kbuild
	@echo "Installing kernel module..."
	sudo insmod $(TARGET).ko

uninstall:
	@echo "Removing kernel module..."
	sudo rmmod $(TARGET)

#!/bin/bash

# Set variables
driver="ioctl_module1.ko"
device_path="/dev/my_ioctl_device"
user_app="ioctl_user1"
kdir="/lib/modules/$(uname -r)/build"

# Ensure a count argument is provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <count>"
    exit 1
fi

count="$1"

echo "Starting spock.sh script..."

# Compile the driver
if [ -f "Kbuild" ]; then
    echo "Compiling the ioctl driver..."
    make -C $kdir M=$(pwd) || { echo "Driver compilation failed."; exit 1; }
else
    echo "Kbuild file not found. Aborting."
    exit 1
fi

# Load the ioctl driver
if sudo insmod $driver; then
    echo "Driver loaded successfully."
else
    echo "Failed to load driver. Aborting."
    exit 1
fi

# Create the device file
if [ ! -e "$device_path" ]; then
    echo "Creating device file..."
    major=$(awk '$2=="my_ioctl_device" {print $1}' /proc/devices)
    if [ -z "$major" ]; then
        echo "Unable to find major number for the device. Aborting."
        sudo rmmod $driver
        exit 1
    fi
    sudo mknod "$device_path" c "$major" 0 || { echo "Failed to create device file."; sudo rmmod $driver; exit 1; }
else
    echo "Device file already exists."
fi

# Compile the user space application
if [ -f "$user_app.c" ]; then
    echo "Compiling the user space application..."
    gcc -o $user_app $user_app.c || { echo "User space application compilation failed."; sudo rmmod $driver; rm -f $device_path; exit 1; }
else
    echo "User application source file not found. Aborting."
    sudo rmmod $driver
    rm -f $device_path
    exit 1
fi

# Run the user space application
if [ -f "$user_app" ]; then
    echo "Running the user space application with count=$count..."
    sudo ./$user_app "$count" || { echo "User space application execution failed."; }
else
    echo "User application binary not found. Aborting."
fi

# Clean up
echo "Cleaning up..."
sudo rm -f "$device_path"
sudo rmmod $driver && echo "Driver removed successfully." || echo "Failed to remove driver."

# Clean the kernel module build files
echo "Cleaning kernel module build files..."
make -C $kdir M=$(pwd) clean && echo "Kernel build files cleaned." || echo "Failed to clean kernel build files."

echo "spock.sh script completed."

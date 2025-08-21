#!/bin/bash

## Building the applications
make 

## Build your ioctl driver named chardev.c and load it here
sudo insmod ioctl_module2.ko

sudo chmod 666 /dev/my_ioctl_device_2

###############################################

# Launching the control station
./control_station 5 &
c_pid=$!
echo "Control station PID: $c_pid"

# Launching the soldiers
./soldier $c_pid  &
s_pid1=$!
echo "Soldier PID: $s_pid1"

./soldier $c_pid  &
s_pid2=$!
echo "Soldier PID: $s_pid2"

./soldier $c_pid  &
s_pid3=$!
echo "Soldier PID: $s_pid3"

./soldier $c_pid  &
s_pid4=$!
echo "Soldier PID: $s_pid4"

sleep 2
kill -9 $s_pid1
kill -9 $s_pid2

sleep 10
## Remove the driver here
sudo rmmod ioctl_module2.ko
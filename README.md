# System Call for Semaphore (Example: Reader- Writer Problem)

# File Descriptions
This readme.md will focus on hilighting what each file attached does. For further information you can evaluate the PROJECT REPORT attached.

## reader_writer_usercode.cpp

..:: This is the user-level c++ code used to show simulation. This source code also calls the avgtime_syscall which is made as a system call in the operating system. Screenshots can be seen from the report. ::..

## syscall_tester.c

..:: This code will help to run the reader writer problem simulation at kernel level which is integrated into the kernel. This basically calls the system call by its number in the syscall_64.tbl and lets it execute. :::...

## reader_writer_systemcall.c

..:: This is the kernel level c code put in the asmlinkage file used to show simulation after dmesg has been done. 

A useful command to see the dmesg output would be 

1. Open a new terminal secondary to the one where you are calling your system call using the syscall_tester.c file
2. Enter the following command:

```````````````````````````````````````````
watch -n 0.1 "dmesg | tail -n $((LINES-6))"

```````````````````````````````````````````

This will help you see the simulation. :::...

## avgtime_syscall.c

..:: This is the kernel level c code put in the asmlinkage file. This code is then made a system call called by the code reader_writer_usercode.cpp :::..
# OS_PROJECT_RWPROB

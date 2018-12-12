#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#define upID 1997
#define downID 2020

using namespace std;

struct msgbuff
{
   long mtype; // we need to choose a different mtype for different msg and receiver
   string msg; // we need to solve this string or char problem
};

/*
everything we understand about kernel till now:
ــــــــــــــــــــــــــــــــــــــــــــ
- it logs(write to a text file) all of the following:
     A)requests from processes.
     B)Itself responses to processes.
     C)disk responses.
     D)requests to disk.
- every sec it sends SIGUSR2  to all the processes and to the disk to inc their clks.
- do the following steps when it receives a msg from a process via the upstream:
     1)identify the type of the command   //should we varify the msg here or inside the process class ??
     2)check the status of the disk this is done by sending SIGUSR1 signal to the disk then disk sends a struct to kernel via upstream with mtype
        to indicate the the msg contains the # of free slots.
     3)if the # of free slots != 0 then it responses to the process with a successful command msg via downstream.
*/

class kernel(){
    public:
};
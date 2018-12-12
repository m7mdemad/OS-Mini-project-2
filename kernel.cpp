#include <ctype.h>
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

class kernel()
{
    int state;
    int command;
    key_t upStream;
    key_t downStream;

  public:
    kernel();
    void receiveProcess();
    void operate(string);
    void identify(string);
    int get_free_slots();
    void send_msg_disk(string);
    void send_state();
};

kernel::kernel()
{
    upStream = msgget(upID, IPC_CREAT | 0644);     // intialize UP stream
    downStream = msgget(downID, IPC_CREAT | 0644); // intialize DOWN stream
}

void kernel::receiveProcess()
{
    struct msgbuff process;

    int rec_val = msgrcv(upStream, &process, sizeof(process.mtext), 0, !IPC_NOWAIT); // mtype ?
    if (rec_val == -1)
        perror("Error in receive from process");

    operate(process.msg);
}

void kernel::operate(string msg)
{
    identify(msg);
    if (command == 0)
    {
        int free = get_free_slots();
        if (free != 0)
            send_msg_disk(msg);
        else{
            state = 2;
            send_state();
        }
    }
    else
        send_msg_disk(msg);
}

void kernel::identify(string msg)
{
    if (msg[0] == 'A') // assume validation inside the process class
        command = 0;
    else
        command = 1;
}

int kernel::get_free_slots()
{
    // send SIUSR1 signal
    raise(SIGUSR1);

    // receive disk msg with the # of free slots
    struct msgbuff respond;

    int rec_val = msgrcv(upStream, &respond, sizeof(respond.mtext), 0, !IPC_NOWAIT); // mtype ?
    if (rec_val == -1)
        perror("Error in receive from disk at kernel");

    return int(respond.mtext) - '0';
}

void kernel::send_msg_disk(string msg)
{
    struct msgbuff message;
    message.mtype = 0;   // what is the mtype ??
    message.mtext = msg; // should i send it as a char array or what ??

    int send_val = msgsnd(downStream, &message, sizeof(message.mtext), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in sending message to disk at kernel");
}

void kernel::send_state(){
    struct msgbuff message;
    message.mtype = 0;   // what is the mtype ??
    message.mtext = to_string(state);

    int send_val = msgsnd(downStream, &message, sizeof(message.mtext), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in sending message to disk at kernel");
}
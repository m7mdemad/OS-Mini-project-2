#include <iostream>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <queue>
using namespace std;
#define upID 1997
#define downID 2020

struct slot
{
    char mtext[64];
};

struct stream
{
    long mtype; // equal 1818 at sending number of free slots
    int clks = -1;
    char command[7];      // add or delete
    char data[64];        // number of the slot to be deleted or new setence to be added
    int state_count = -1; /* used for two things
                                1- return state:
                                    - 0: successful ADD 
                                    - 1: successful DEL
                                    - 2: unable to ADD
                                    - 3: unable to DEL
                                2- return number of free slots on calling SIGUSR1     
                            */
};

class Disk_Process
{
    struct slot slots[10];           // 10 slots each is about array of 64 char
    int free_count;                  // number of free slots
    int disk_clk;                    // disk_process clock
    key_t upStream;                  // UP stream to send data to the kernel
    key_t downStream;                // DOWN stream to receive data from the kernel
    queue<struct stream> operations; // all operations received are added to the queue on recieving them
    queue<int> time_needed;          // number of clk cycles needed for every process to be done (including latency)
    queue<struct stream> finished;   // all operations that are ready to be sent to the kernel
  public:
    Disk_Process();
    void Pick_command();
    int Add(char *);           // Add data to empty slot
    int Delete(char);          // delete data from a slot
    void Inc_clk(int);         // increment the clock
    void send_free_count(int); // send no. of free slots to the kernel
    int getFreeSlots();        // return number of free slots
    void send(long, char *);   /* send data to kernel 
                                        the long argument is for the mtype to be used 
                                        as it's different in sending free slots number)
                                        */
    void receive();            // receive data from kernel

    void addOperation(struct stream); // add new operation to the queue and cal the time needed for it
    void update_Operations();         // decrement time of first operation in the queue
};

Disk_Process::Disk_Process()
{
    for (int i = 0; i < 10; i++)
        strcpy(slots[i].mtext, ""); // intialize all slots as empty arrays
    free_count = 10;                // at the begining all slots are free
    disk_clk = 0;
    upStream = msgget(upID, IPC_CREAT | 0644);     // intialize UP stream
    downStream = msgget(downID, IPC_CREAT | 0644); // intialize DOWN stream
}

int Disk_Process::Add(char *newData)
{
    if (free_count == 0)
        return 1; // slots are full

    // find first free slot to add the newData in
    for (int i = 0; i < 10; i++)
    {
        if (slots[i].mtext == "")
        {
            strcpy(slots[i].mtext, newData); // add data
            free_count--;                    // decrement number of free slots
            break;
        }
    }
    return 0; // add done successfully
}

int Disk_Process::Delete(char slotNo)
{
    int number_of_slot = slotNo - 48; // convert the number from char to int
    if (slots[number_of_slot].mtext == "")
        return 3; // slot is already empty

    strcpy(slots[number_of_slot].mtext, ""); // delete data
    free_count++;                            // increment number of free slots
    return 2;                                // delete done successfully
}

void Disk_Process::update_Operations()
{
    if (operations.empty())
        return;

    time_needed.front() -= 1;
    if (time_needed.front() == 0)
    { // first operation time is finished
        struct stream op = operations.front();
        operations.pop(); // remove it from operations running
        time_needed.pop();
        if (op.command == "ADD")
            op.state_count = Add(op.data); // perform add
        else
            op.state_count = Delete(op.data[0]); // perform delete
        finished.push(op);                       // add to finished to send it back to the kernel
    }
}

void Disk_Process::Inc_clk(int signum)
{
    disk_clk++; // inc the clk
    update_Operations();
}

void Disk_Process::send_free_count(int signum)
{
    // set up the data to send
    struct stream to_send;
    to_send.state_count = free_count;
    to_send.mtype = 1818; // special for the intruppt

    // send free slots number to the kernel
    int send_val;
    key_t msgqid = msgget(upStream, IPC_CREAT | 0644); //0644: explicit permissions
    send_val = msgsnd(msgqid, &to_send, sizeof(to_send.data), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Error in send at disk interuppt");
}

int Disk_Process::getFreeSlots()
{
    return free_count;
}

void Disk_Process::send(long mtype_to_use, char *data_to_send)
{
    if (finished.empty())
        return;

    // set up the data to send
    struct stream to_send;
    to_send = finished.front(); // only send one per clk cycle (??)
    finished.pop();

    // send the data to the kernel
    int send_val;
    key_t msgqid = msgget(upStream, IPC_CREAT | 0644); //0644: explicit permissions
    send_val = msgsnd(msgqid, &to_send, sizeof(to_send.data), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send at disk");
}

void Disk_Process::receive()
{
    struct stream to_receive;
    int rec_val = msgrcv(downStream, &to_receive, sizeof(to_receive.data), 0, !IPC_NOWAIT);

    if (rec_val == -1)
    {
        perror("Error in receive at disk");
    }
    addOperation(to_receive); // add the new operation to the operations queue
}

void Disk_Process::addOperation(struct stream newOp)
{
    operations.push(newOp);
    int time = newOp.clks; // no. of clks needed
    if (newOp.command == "ADD")
        time += 3; // add latency
    else
        time += 1;          // delete latency
    time_needed.push(time); // push total number of clks needed to be done
}

// assumed fcfs
// send one op per clk?
// to-do: convert to pair queue
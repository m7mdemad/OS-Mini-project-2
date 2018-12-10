#include <iostream>
#include <string>

using namespace std;

#define upID  1997
#define downID 2020

struct stream{
    long mtype;
    int clks;
    char[7];
    char data[64];
};

class Disk_Process{
    string slots[10];
    int free_count;
    key_t upStream;
    key_t downStream;
public:
    Disk_Process();
    void Pick_command();
};

Disk_Process::Disk_Process(){
    free_count = 10;
    upStream = msgget(upID, IPC_CREAT|0644);
    downStream = msgget(downID, IPC_CREAT|0644);
}
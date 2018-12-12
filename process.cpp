#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#define upID 1997
#define downID 2020

using namespace std;

/* 
everything we understand till now:
ـــــــــــــــــــــــــــــــ
- read from file 
- translate it to the format to be sent to kernel via up stream
- receive SIGUSR2 signal from kernel to inc its clk
- if # of clocks incremented == time of a process send it to the kernel
- receive respond from kernel via down stream to indicate if the operation is done succussfuly or not
*/

struct msgbuff
{
   long mtype;
   string msg;
};

class process()
{
   int clk;          //internal clock for the process
   string msg;       //translated message to be sent to kernel
   key_t upStream;   // UP stream to send data to the kernel
   key_t downStream; // DOWN stream to receive data from the kernel
   string filename;

 public:
   process();
   void readFile();
   void handler(int);
   void send(string); // send msg to kernel via upstream
   void receive();    // to receive kernel respond
   void print_response(int);
};

process::process(/*string file*/)
{
   clk = 0;
   msg = '';
   filename = "test.txt"; // for now
   upStream = msgget(upID, IPC_CREAT | 0644);     // intialize UP stream
   downStream = msgget(downID, IPC_CREAT | 0644); // intialize DOWN stream
   signal(SIGUSR2, handler)
}

void process::handler(int signum)
{
   clk++;
}

void process::send(string msg)
{
   struct msgbuff message;
   message.mtype = 0;   // what is the mtype ??
   message.mtext = msg; // should i send it as a char array or what ??

   int send_val = msgsnd(upStream, &message, sizeof(message.mtext), !IPC_NOWAIT);
   if (send_val == -1)
   {
      perror("Errror in sending message to kernel");
   }
}

void process::print_response(int val)
{
   string arr[] = {"successful ADD", "successful DEL", "unable to ADD", "unable to DEL"};
   for (int i = 0; i < 4; i++)
   {
      if (val == i)
         cout << arr[i] << endl;
   }
}

void process::receive()
{
   struct msgbuff respond;

   int rec_val = msgrcv(downStream, &respond, sizeof(respond.mtext), 0, !IPC_NOWAIT); // mtype ?
   if (rec_val == -1)
      perror("Error in receive from kernel at process");

   print_response(int(respond.mtext) - '0');
}

void process::readFile()
{
   ifstream data(filename, ios::in);
   string msg;
   string line;
   int time;
   while (getline(data, line))
   {
      istringstream iss(line);
      vector<string> tokens{istream_iterator<string>{iss},
                            istream_iterator<string>{}};
      for (int i = 0; i < tokens.size(); i++)
      {
         if (i == 0)
         {
            stringstream ss(tokens[i]);
            ss >> time; // TODO: handle errors
         }
         else if (i == 1)
         {
            if (tokens[i] == "ADD" || tokens[i] == "DEL")
            {
               msg += tokens[i][0];
            }
            else
            {
               perror("ERROR: ambiguous command");
            }
         }
         else
         {
            if (abs(int(i - (tokens.size() - 1))) != 0)
               msg += tokens[i] + ' ';
            else
               msg += tokens[i];
            if (msg.length() >= 63)
            {
               perror("ERROR: The message exceeds the size limit");
            }
         }
      }
      while (time != clk){}
      send(msg);
      clk = 0;
      receive();
      /* this logic can be modified later */
   }
}
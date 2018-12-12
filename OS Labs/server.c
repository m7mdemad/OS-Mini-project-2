#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

struct msgbuff
{
	long mtype;
	char mtext[256];
};

void handler1(int signum)
{
	msgctl(777, IPC_RMID, (struct msqid_ds *)0);
	msgctl(888, IPC_RMID, (struct msqid_ds *)0);
	exit(0);
}

/* convert upper case to lower case or vise versa */
void conv(char *msg, int size)
{
	int i;
	for (i = 0; i < size; ++i)
		if (islower(msg[i]))
			msg[i] = toupper(msg[i]);
		else if (isupper(msg[i]))
			msg[i] = tolower(msg[i]);
}

int main()
{
	signal(SIGINT, handler1);
	while (1)
	{
		key_t msgqid;
		msgqid = msgget(777, IPC_CREAT | 0644); //0644: explicit permissions

		if (msgqid == -1)
		{
			perror("Error in create");
			exit(-1);
		}
		//printf("msgqid = %d\n", msgqid);

		int rec_val;
		struct msgbuff message;

		//recieve
		rec_val = msgrcv(msgqid, &message, sizeof(message.mtext), 0, !IPC_NOWAIT);

		if (rec_val == -1)
			perror("Error in receive at server side");
		else
			printf("\nMessage received: %s\n", message.mtext);

		//convert and send
		int send_val;
		msgqid = msgget(888, IPC_CREAT | 0644); //0644: explicit permissions
		conv(message.mtext, sizeof(message.mtext));
		send_val = msgsnd(msgqid, &message, sizeof(message.mtext), !IPC_NOWAIT);
		if (send_val == -1)
			perror("Errror in send at server");
	}
}

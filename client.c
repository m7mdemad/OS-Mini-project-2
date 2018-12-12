#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct msgbuff
{
	long mtype;
	char mtext[256];
};

int main()
{
	pid_t pid;
	int clientID = pid % 10000;
	//scanf("%d", clientID);
	char msg[256];
	while (1)
	{
		printf("Enter the message: ");
		scanf("%s", msg);

		key_t msgqid;

		msgqid = msgget(777, IPC_CREAT | 0644); //0644: explicit permissions
		if (msgqid == -1)
		{
			perror("Error in create");
			exit(-1);
		}
		//printf("msgqid = %d", msgqid);

		//send
		struct msgbuff message;
		message.mtype = clientID; /* arbitrary value */

		int send_val;
		strcpy(message.mtext, msg);
		send_val = msgsnd(msgqid, &message, sizeof(message.mtext), !IPC_NOWAIT);
		if (send_val == -1)
			perror("Errror in send at client side");

		//recieve
		int rec_val;
		msgqid = msgget(888, IPC_CREAT | 0644); //0644: explicit permissions
		rec_val = msgrcv(msgqid, &message, sizeof(message.mtext), clientID, !IPC_NOWAIT);

		if (rec_val == -1)
			perror("Error in receive at client side");
		else
			printf("Message received at client side: %s\n\n", message.mtext);

		msgctl(msgqid, IPC_RMID, (struct msqid_ds *)0);
	}
}

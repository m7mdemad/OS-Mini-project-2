#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
void handlerSum(int x);
void handlerAvg(int x);
	
int arr1[] = {1,0,100,29,9,6,7};
int arr2[] = {101,20,31,16,19};
int pid[2];	

int main()
{
	signal(SIGUSR1, handlerSum);
	signal(SIGUSR2, handlerAvg);

	int x;
	scanf("%d",&x);

 	int i,p;
	for(i = 0;i<2;i++){
		p = fork();
		if(p != 0){
			sleep(2);
		}
		else{
			pid[i] = getpid();
			sleep(5);
		}
	}

	if(x == 1){
		kill(pid[0], SIGUSR1);
		kill(pid[1], SIGUSR1);
		sleep(2);
	}
	else{
		kill(pid[0], SIGUSR2);
		kill(pid[1], SIGUSR2);
           	sleep(2);
	}
}

void handlerSum(int x)
{
	if(getpid() == pid[0]){
		int sum = 0;
		for (int i=0; i<7; i++) {
			sum += arr1[i];
		}
        	printf("\n Sum = %d\n",sum);
		exit(0);
	}
	else if(getpid() == pid[1]){
		int sum = 0;
		for (int i=0; i<5; i++) {
			sum += arr2[i];
		}
        	printf("\n Sum = %d\n",sum);
		exit(0);
	}
	
}

void handlerAvg(int x)
{
	if(getpid() == pid[0]){
		int avg = 0;
		for (int i = 0; i<7; i++) {
			avg += arr1[i];
		}
		avg /= 7;
        	printf("\n Avg = %d\n",avg);
		exit(0);
	}
	else if(getpid() == pid[1]){
		int avg = 0;
		for (int i = 0; i<5; i++) {
			avg += arr2[i];
		}
		avg /= 5;
        	printf("\n Avg = %d\n",avg);
		exit(0);
	}
	
}

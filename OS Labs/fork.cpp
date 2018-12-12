#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <algorithm> 
#include <sys/wait.h>

using namespace std;

int main()
{
  int pid;
  int n;
  int i;
  int stat_loc;
  cin >> n;
  for(i=0;i<n;i++){
    pid = fork();
    if (pid == -1)
      perror("error in fork");
    
    else if (pid == 0)
    { 
      printf("\nI am the child number %d, my pid = %d and my parent's pid is %d\n\n", i, getpid(), getppid());
      if(fork() == 0){
        printf("\nI am the grandchild my pid = %d and my parent's pid is %d\n\n", getpid(), getppid());
        exit(2);
      }
      pid = wait(&stat_loc);
      if(WIFEXITED(stat_loc))
        printf("\nGrandChild terminated normally with status %d\n", WEXITSTATUS(stat_loc));  
      exit(i);
    }   
    else 	
    {
      pid = wait(&stat_loc);
      if(WIFEXITED(stat_loc))
        printf("\nChild terminated normally with status %d\n", WEXITSTATUS(stat_loc));  
    }  
  }
}  

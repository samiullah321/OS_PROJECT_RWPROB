#include<stdio.h>
#include<unistd.h>
#include<sys/syscall.h>

int main(){
	long int s = syscall(337);
	printf("\n.:SYSTEM CALL VALUE:. %ld\n",s);
	return 0;
}

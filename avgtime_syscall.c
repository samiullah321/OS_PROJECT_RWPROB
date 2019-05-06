#include<linux/kernel.h>
#include<linux/syscalls.h>

SYSCALL_DEFINE3(avgtime,int, avg_time,int, nw,int, nr){

	printk("Avg time taken for RW is %d seconds\n",(avg_time/((nw+nr)*1000))/1000);
	return 0;
}

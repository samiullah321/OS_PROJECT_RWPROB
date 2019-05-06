#include <linux/init.h>
#include <linux/semaphore.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/time.h>
#include<linux/timer.h>
#include<linux/delay.h>
#include<asm/delay.h> 
#include<linux/random.h>

#define parm_reader 6 
#define parm_writer 6
#define rrand_int 1000

struct read_info{
	int id;
	int time;
};

int writer(void *i);     // writer thread
int reader(void *i);      // reader threads
int i,j,k,l,m,n;
long int avgtime,avg_time;	
int wrand_int=0;

static struct semaphore mutex;            // semaphore for writer entry into cs
static struct semaphore rwmutex;          // semaphore for shared variable read_count safety
static struct semaphore avgmutex;         // semaphore for shared variable avg_time safety
int read_count=0;       // number of readers inside cs

int writer(void * param){

	
	int id = *(int*)param;
	struct timespec rs;
	struct timespec es;
	struct timespec ls;
	

	printk("\n");
	// calculating request time
	getnstimeofday(&rs);
	printk("Request by Writer Thread %d at %.2lu:%.2lu\n",id,(rs.tv_sec/60)%60,rs.tv_sec%60);

	down(&rwmutex); // wait writer

	// calculating entry time
	
	get_random_bytes(&wrand_int,sizeof (wrand_int));
	if(wrand_int<0)
	wrand_int = -1*wrand_int;
	wrand_int = ( wrand_int % 4 ) + 1;
	wrand_int*=1000;

	getnstimeofday(&es);
	
	printk("Entered by Writer Thread %d at %.2lu:%.2lu | Will take t = %d sec to write\n",id,(es.tv_sec/60)%60,es.tv_sec%60,wrand_int/1000);   
	 
	msleep_interruptible(wrand_int);

	getnstimeofday(&ls);
	printk("Exit by Writer Thread %d at %.2lu:%.2lu\n",id,(ls.tv_sec/60)%60,ls.tv_sec%60);

	up(&rwmutex);   // signal writer

	down(&avgmutex);
	// adding waiting time to shared variable avg_time
	avg_time += es.tv_sec-rs.tv_sec;
	up(&avgmutex);   

	printk("\n");
	return 0;
}

int reader(void * param)
{
	
	struct read_info *ptr = (struct read_info*)param;
	struct timespec rs;
	struct timespec es;
	struct timespec ls;
	
	printk("\n");
	getnstimeofday(&rs);
	printk("Request by Reader Thread %d at %.2lu:%.2lu\n",ptr->id,(rs.tv_sec/60)%60,rs.tv_sec%60);


	down(&mutex);  // wait for read_count access permission
	read_count++;       // increment read count as new reader is entering 
	if(read_count==1)   // only if this is the first reader
	    down(&rwmutex); // then wait for cs permission
	up(&mutex);   // signal mutex

	getnstimeofday(&es);
	printk("Entered by Reader Thread %d at %.2lu:%.2lu | Will take t = 1 sec to read\n",ptr->id,(es.tv_sec/60)%60,es.tv_sec%60);  

	msleep_interruptible(rrand_int);

	down(&mutex);   // wait for read count access permission
	read_count--;       // decrement readcount as we are done reading
	if(read_count==0)   // only if this is the last reader
	    up(&rwmutex); // signal rwmutex

	getnstimeofday(&ls);
	printk("Exit by Reader Thread %d at %.2lu:%.2lu\n",ptr->id,(ls.tv_sec/60)%60,ls.tv_sec%60);

	up(&mutex);   

	down(&avgmutex);
	// adding waiting time to shared variable avg_time
	avg_time += es.tv_sec-rs.tv_sec;
	up(&avgmutex);

	printk("\n");
	return 0;
}

asmlinkage long sys_rwprob(void)
{

     // creating n pthreads
    static  struct task_struct *w[parm_writer],*r[parm_reader];
    int w_attr[parm_writer];
    struct read_info read[parm_reader];

    // initialising semaphores
    sema_init(&mutex,1);   
    sema_init(&rwmutex,1);
    sema_init(&avgmutex,1);

	for (j = 0; j < parm_reader; ++j) {

		get_random_bytes(&read[j].time,sizeof (read[j].time) );
		if(read[j].time<0)
		read[j].time = -1*read[j].time;
		read[j].time = ( read[j].time % 4 ) + 1;
		read[j].time*=1000;
		
		read[j].id=j+1;

                r[j]=kthread_create(reader,&read[j],"reader");
		if(r[j])
		wake_up_process(r[j]);
    }

	for (i = 0; i < parm_writer; ++i) {
        w_attr[i]=i+1;
                w[i]=kthread_create(writer,&w_attr[i],"writer");
	if(w[i])
	wake_up_process(w[i]);
    }

	for(l=0;l<parm_reader;l++)
                kthread_stop(r[l]); 
	for(k=0;k<parm_writer;k++)
                kthread_stop(w[k]);
	  
	avgtime=((avg_time)/((parm_reader+parm_writer)));
	printk("\nAvg time spent by readers and writers in critical section(sec): %ld\n",avgtime);

	
	return 0;
}

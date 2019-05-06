#include <semaphore.h>
#include <iostream>
#include <atomic>
#include <stdio.h>
#include <ctime>
#include <chrono>
#include <pthread.h>
#include <fstream>
#include <unistd.h>

using namespace std;

struct read_info{
	int id;
	int time;
};

void *writer(void *i);     // writer thread
void *reader(void *i);      // reader threads
int wrand_int;

int avg_time;

sem_t mutex;            // semaphore for writer entry into cs
sem_t rwmutex;          // semaphore for shared variable read_count safety
sem_t avgmutex;         // semaphore for shared variable avg_time safety
int read_count=0;       // number of readers inside cs


int main()
{
    // initialising semaphores
    sem_init(&mutex,0,1);   
    sem_init(&rwmutex,0,1);
    sem_init(&avgmutex,0,1);
	
    int nw,nr;
    ifstream inFile ;
    inFile.open("input.txt");    //we read from the input file
    inFile>>nr;
    inFile>>nw;
	
    srand(time(0));	
    // creating n pthreads
    pthread_t w[nw],r[nr];
    pthread_attr_t w_attr[nw],r_attr[nr];
    struct read_info read[nr];
	
	for (int i = 0; i < nw; ++i) {
		pthread_attr_init(&w_attr[i]);
		pthread_create(&w[i],&w_attr[i],writer,(void*)(intptr_t)(i+1));
	}

	 for (int i = 0; i < nr; ++i) {
		read[i].time=(rand()%4)+1;
		read[i].id=i+1;
		pthread_attr_init(&r_attr[i]);
		pthread_create(&r[i],&r_attr[i],reader,&read[i]);
	}
	
   
    // joining the threads
	 for(int i=0;i<nw;i++)
                pthread_join(w[i],NULL);
    for(int i=0;i<nr;i++)
                pthread_join(r[i],NULL);
   

    // destroying semaphores
    sem_destroy(&mutex);
    sem_destroy(&rwmutex);
    sem_destroy(&avgmutex);
	syscall(336,avg_time,nr,nw);
	printf("\nAvg time spent by readers and writers in critical section(sec): %d\n",(avg_time/((nw+nr)*1000))/1000);

    return 0;

}

void *writer(void * param){
	
   	int id = (intptr_t)param;

        // calculating request time
        auto reqTime = std::chrono::system_clock::now();      
        time_t my_time;
        time (&my_time);
        struct tm *timeinfo = localtime (&my_time);
	printf("Request by Writer Thread %d at %02d:%02d\n",id,timeinfo->tm_min,timeinfo->tm_sec);
      

        sem_wait(&rwmutex); // wait writer

        // calculating entery time
        auto enterTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
	wrand_int=(rand()%4)+1;
	printf("\tEntry by Writer Thread %d at %02d:%02d | Will take t = %d sec to write\n",id,timeinfo->tm_min,timeinfo->tm_sec,wrand_int);
	
        sleep(wrand_int);// simulate a thread executing in CS

	
        auto exitTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
	printf("\t\tExit by Writer Thread %d at %02d:%02d\n",id,timeinfo->tm_min,timeinfo->tm_sec);

        sem_post(&rwmutex);   // signal writer

        // adding waiting time to shared variable avg_time
        sem_wait(&avgmutex);
        avg_time += std::chrono::duration_cast<std::chrono::microseconds>(enterTime-reqTime).count();
        sem_post(&avgmutex);    
}

void *reader(void * param)
{
	
    	struct read_info *ptr = (struct read_info*)param;

        auto reqTime = std::chrono::system_clock::now();
        time_t my_time;
        time (&my_time);
        struct tm *timeinfo = localtime (&my_time);
	printf("Request by Reader Thread %d at %02d:%02d\n",ptr->id,timeinfo->tm_min,timeinfo->tm_sec);
        

        sem_wait(&mutex);  // wait for read_count access permission
        read_count++;       // increment read count as new reader is entering 
        if(read_count==1)   // only if this is the first reader
            sem_wait(&rwmutex); // then wait for cs permission
        sem_post(&mutex);   // signal mutex

        auto enterTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
	printf("\tEntry by Reader Thread %d at %02d:%02d | Will take t = %d sec to read\n",ptr->id,timeinfo->tm_min,timeinfo->tm_sec,ptr->time);
     
        sleep(ptr->time);// simulate a thread executing in CS

        sem_wait(&mutex);   // wait for read count access permission
        read_count--;       // decrement readcount as we are done reading
        if(read_count==0)   // only if this is the last reader
            sem_post(&rwmutex); // signal rwmutex
	

	auto exitTime = std::chrono::system_clock::now();
        time (&my_time);
        timeinfo = localtime (&my_time);
	printf("\t\tExit by Reader Thread %d at %02d:%02d\n",ptr->id,timeinfo->tm_min,timeinfo->tm_sec);
      
        sem_post(&mutex);   

        sem_wait(&avgmutex);
        avg_time += std::chrono::duration_cast<std::chrono::microseconds>(enterTime-reqTime).count();
        sem_post(&avgmutex); 
}

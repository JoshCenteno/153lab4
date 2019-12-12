#include "types.h"
#include "stat.h"
#include "user.h"
#include "uspinlock.h"

struct shm_cnt {
   struct uspinlock lock;
   int cnt;
};

int main(int argc, char *argv[])
{
int pid = 1;
int i=0;
struct shm_cnt *counter;
  pid=fork();
  sleep(1);

if(shm_open(1,(char **)&counter) == 0)
	printf(1, "ERROR OPEN");

  for(i = 0; i < 10000; i++)
    {
	 
     uacquire(&(counter->lock));
     counter->cnt++;

     urelease(&(counter->lock));

//print something because we are curious and to give a chance to switch process
     if(i%1000 == 0)
       printf(1,"Counter in %s is %d at address %x\n",pid? "Parent" : "Child", counter->cnt, counter);
	}
  
  if(pid)
     {
       printf(1,"Counter in parent is %d\n",counter->cnt);
    wait();
    } else
    printf(1,"Counter in child is %d\n\n",counter->cnt);

//shm_close: first process will just detach, next one will free up the shm_table entry (but for now not the page)
 if(shm_close(1) == 0)
   printf(1, "ERROR CLOSE");
   exit();
   return 0;
}

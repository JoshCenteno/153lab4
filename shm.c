#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
 uint sz = myproc()->sz;
 int i;
 if( id <= 0)
	return 0;
  acquire(&(shm_table.lock));
  for(i = 0; i < 64; i++){ //Case 1: Finds the page with same ID
	 if(shm_table.shm_pages[i].id == id){
		if(shm_table.shm_pages[i].refcnt <= 0 || shm_table.shm_pages[i].frame == 0){
			release(&(shm_table.lock));
			return 0;
     }    
		if(mappages(myproc()->pgdir, (char*)PGROUNDUP(sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), (PTE_W|PTE_U)) == -1){
			release(&(shm_table.lock));
			return 0;
		}	
	shm_table.shm_pages[i].refcnt++;
	*pointer = (char*)sz;
	release(&(shm_table.lock));
	return 1;	 
    }
 }
 for(i = 0; i < 64; i++){ //Case 2: Allocates first free page
	if(shm_table.shm_pages[i].id == 0){
		shm_table.shm_pages[i].frame = kalloc();
		memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
		if(mappages(myproc()->pgdir, (char*)PGROUNDUP(sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), (PTE_W|PTE_U)) == -1){
			release(&(shm_table.lock));
			return 0;
		}

		shm_table.shm_pages[i].id = id;
		if(shm_table.shm_pages[i].refcnt != 0){
			release(&(shm_table.lock));
			return 0;
		}
		shm_table.shm_pages[i].refcnt = 1;
		*pointer = (char*)sz;
		release(&(shm_table.lock));

		return 1;	
	  }
  }
  release(&(shm_table.lock)); //Case 3: No free pages Errors
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
 int i;
 if(id <= 0)
	return 0;
 acquire(&(shm_table.lock));
 //int j = -1;
 for(i = 0; i < 64; i++) {
	if(shm_table.shm_pages[i].id == id){
		if(shm_table.shm_pages[i].refcnt == 0) {
			release(&(shm_table.lock));
			return 0;
		}
		shm_table.shm_pages[i].refcnt--;	
		if(shm_table.shm_pages[i].refcnt == 0) {
			shm_table.shm_pages[i].id = 0;
			shm_table.shm_pages[i].frame = 0;
		}
		release(&(shm_table.lock));
		return 1;
	}
 }
 release(&(shm_table.lock));
 return 0;
}

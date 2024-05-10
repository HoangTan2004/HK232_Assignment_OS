
// /*
//  * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
//  */
// /*
//  * Source Code License Grant: Authors hereby grants to Licensee 
//  * a personal to use and modify the Licensed Source Code for 
//  * the sole purpose of studying during attending the course CO2018.
//  */
#define ENTRY_SIZE 16
#define VALID_OFFSET 12
#define PID_OFFSET 0
 #include "os-cfg.h"
 #ifdef CPU_TLB
// /*
//  * CPU TLB
//  * TLB module cpu/cpu-tlb.c
//  */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
pthread_mutex_t tlb_lock;
int tlb_change_all_page_tables_of(struct pcb_t *proc, struct memphy_struct *mp) {
    if (proc == NULL || mp == NULL || mp->storage == NULL) {
        return -1; // Invalid input parameters
    }

    int total_entries = mp->maxsz / ENTRY_SIZE;
    int entries_invalidated = 0;

    BYTE entry[ENTRY_SIZE];
    for (int i = 0; i < total_entries; ++i) {
        int entry_addr = i * ENTRY_SIZE;
        if (entry_addr + ENTRY_SIZE > mp->maxsz) {
            return -1; // Prevent buffer overflow
        }

        TLBMEMPHY_read(mp, entry_addr, entry);

        uint32_t entry_pid = decode_int(entry + PID_OFFSET);
        BYTE valid = entry[VALID_OFFSET];

        if (valid && entry_pid == proc->pid) {
            entry[VALID_OFFSET] = 0;
            TLBMEMPHY_write(mp, entry_addr, entry);
            entries_invalidated++;
        }
    }

    return entries_invalidated;
}


int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct *mp) {
    if (proc == NULL || mp == NULL || mp->storage == NULL) {
        return -1; // Invalid input parameters
    }

    int total_entries = mp->maxsz / ENTRY_SIZE;
    int entries_flushed = 0;

    BYTE entry[ENTRY_SIZE];
    for (int i = 0; i < total_entries; ++i) {
        int entry_addr = i * ENTRY_SIZE;
        if (entry_addr + ENTRY_SIZE > mp->maxsz) {
            return -1; // Prevent buffer overflow
        }

        TLBMEMPHY_read(mp, entry_addr, entry);

        uint32_t entry_pid = decode_int(entry + PID_OFFSET);
        BYTE valid = entry[VALID_OFFSET];

        if (valid && entry_pid == proc->pid) {
            entry[VALID_OFFSET] = 0;
            TLBMEMPHY_write(mp, entry_addr, entry);
            entries_flushed++;
        }
    }

    return entries_flushed;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index) {
    if (proc == NULL || proc->mm == NULL) {
        return -1; // Invalid process or memory management structure
    }
 pthread_mutex_lock(&tlb_lock);
    int addr, val;
    val = __alloc(proc, 0, reg_index, size, &addr);
    if (val != 0) {
       pthread_mutex_unlock(&tlb_lock);
        return val; // Memory allocation failed, propagate the error
    }

    uint32_t num_pages = (size + PAGING_PAGESZ - 1) / PAGING_PAGESZ;
    uint32_t start_page = PAGING_PGN(addr);

    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t pgnum = start_page + i;
        uint32_t *pte = &proc->mm->pgd[pgnum];

        if (pte && (*pte & PAGING_PTE_PRESENT_MASK)) {
            uint32_t frame_number = PAGING_FPN(*pte);
            BYTE frame_in_tlb;
            int tlb_result = tlb_cache_read(proc->tlb, proc->pid, pgnum, &frame_in_tlb);

            if (tlb_result != 0 || frame_in_tlb != frame_number) {
                tlb_cache_write(proc->tlb, proc->pid, pgnum, (BYTE)frame_number);
            }
        }
    }
 pthread_mutex_unlock(&tlb_lock);
    return 0; // Success
}
/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index) {
    if (proc == NULL || proc->mm == NULL || reg_index >= PAGING_MAX_SYMTBL_SZ) {
        return -1; // Invalid input check
    }
 pthread_mutex_lock(&tlb_lock);
    struct vm_rg_struct *region = &proc->mm->symrgtbl[reg_index];
    if (region->rg_start == 0 && region->rg_end == 0) {
       pthread_mutex_unlock(&tlb_lock);
        return -1; // Check for uninitialized or already freed region
    }

    __free(proc, 0, reg_index);

    uint32_t start_page = PAGING_PGN(region->rg_start);
    int tmp = region->rg_end - 1;
    uint32_t end_page = PAGING_PGN(tmp);
    BYTE dummy_frame_number = 0xFF;

    for (uint32_t page_number = start_page; page_number <= end_page; ++page_number) {
        BYTE current_frame;
        if (tlb_cache_read(proc->tlb, proc->pid, page_number, &current_frame) == 0) {
            tlb_cache_write(proc->tlb, proc->pid, page_number, dummy_frame_number);
        }
    }
 pthread_mutex_unlock(&tlb_lock);
    return 0; // Success
}



/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  //done 
   pthread_mutex_lock(&tlb_lock);
  BYTE data;
  int frmnum = -1;
  BYTE check = 0; 
  int addr = proc->regs[source] + offset;
  //getting the page number 
  int pgn = PAGING_PGN(addr);

  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  /* frmnum is return value of tlb_cache_read/write value*/
  frmnum = tlb_cache_read(proc->tlb,proc->pid,pgn,&check);

  //check if the address is exists on RAM or not! 
  if(check == -1) {
      //if not exists -> get PAGE!, if not exists -> ERROR!
      if(pg_getpage(proc->mm,pgn,&frmnum,proc) != 0){ //Need page is loaded into RAM  
       pthread_mutex_unlock(&tlb_lock);
        return -1; /* invalid page access */
      }
  } 
	
#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
   //if hit 
  if(frmnum > 0) { 
      //get the physic pos 
      int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + offset;
      //read the TLB memphy, if there is no data -> error 
      if(TLBMEMPHY_read(proc->mram,phyaddr,&data) == -1) {
         pthread_mutex_unlock(&tlb_lock);
        return -1;  
      }
  }  
  //if Miss 
  else{  
        /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
        /* by using tlb_cache_read()/tlb_cache_write()*/ 
        __read(proc, 0, source, offset, &data);
        //get the PTE from the table 
          uint32_t pte = proc->mm->pgd[pgn];  
        //perform writting on cache 
          if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) {
             pthread_mutex_unlock(&tlb_lock);
            return -1; /*cannot write~*/
          }
  } 

  //get it into the process register! 
  proc->regs[destination] = (uint32_t)data; 
 pthread_mutex_unlock(&tlb_lock);
  return 0; 
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
   pthread_mutex_lock(&tlb_lock);
  int val;
  int frmnum = -1;
  BYTE check = 0; 

  int addr = proc->regs[destination] + offset;
  //getting the page number and offset 
    int pgn = PAGING_PGN(addr);
    int off = PAGING_OFFST(addr);
  //getting the PTE 
  //uint32_t pte = proc->mm->pgd[pgn];  
  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/
  frmnum = tlb_cache_read(proc->tlb,proc->pid,pgn,&check);

  if(check == -1) {
      //if not exists -> get PAGE!, if not exists -> ERROR!
      if(pg_getpage(proc->mm,pgn,&frmnum,proc) != 0) {//Need page is loaded into RAM  
       pthread_mutex_unlock(&tlb_lock);
        return -1; /* invalid page access */
      }
  } 

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  // do if hit
  if(frmnum >= 0) {
      //get physical address
      int phyaddr = (frmnum << PAGING_ADDR_FPN_LOBIT) + off;
      //perform write 
      val = MEMPHY_write(proc->mram,phyaddr,data); 
  }

  // do if miss   
  else { 
      /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
      /* by using tlb_cache_read()/tlb_cache_write()*/
      val = __write(proc, 0, destination, offset, data);
      uint32_t pte = proc->mm->pgd[pgn];  
      //perform writting on cache 
        if(tlb_cache_write(proc->tlb,proc->pid,pgn,pte) == -1) {
           pthread_mutex_unlock(&tlb_lock);
            return -1; /*cannot write~*/
        }
  }
   pthread_mutex_unlock(&tlb_lock);
  return val;
}
uint32_t decode_int(const BYTE *src)
{
   uint32_t value;
   memcpy(&value, src, sizeof(uint32_t));
   return value;
}
#endif


/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef MM_TLB
/*
 * Memory physical based TLB Cache
 * TLB cache module tlb/tlbcache.c
 *
 * TLB cache is physically memory phy
 * supports random access 
 * and runs at high speed
 */


#include "mm.h"
#include <stdlib.h>

#define init_tlbcache(mp,sz,...) init_memphy(mp, sz, (1, ##__VA_ARGS__))
#define CACHE_LINE_SIZE 6
/*
 *  tlb_cache_read read TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */

// Read TLB cache device
int tlb_cache_read(struct memphy_struct *mp, int pid, int pgnum, BYTE *data) {
    if (mp == NULL || data == NULL)
        return -1;

    int cache_size = mp->maxsz / CACHE_LINE_SIZE;
    int tlbnb = pgnum % cache_size;

    int plb_pid = mp->storage[tlbnb];
    int plb_pgnum = (mp->storage[tlbnb + 1] << 8) | mp->storage[tlbnb + 2];

    if (pid == plb_pid && pgnum == plb_pgnum) {
        int plb_pte = (mp->storage[tlbnb + 3] << 16) | (mp->storage[tlbnb + 4] << 8) | mp->storage[tlbnb + 5];
        if (!PAGING_PAGE_PRESENT(plb_pte)) {
            *data = -1;
            return -1;
        }
        *data = PAGING_FPN(plb_pte);
        return 0;
    }
    return -1;
}
/*
 *  tlb_cache_write write TLB cache device
 *  @mp: memphy struct
 *  @pid: process id
 *  @pgnum: page number
 *  @value: obtained value
 */
int tlb_cache_write(struct memphy_struct *mp, int pid, int pgnum, int pte) {
    if (mp == NULL || pgnum < 0)
        return -1;

    int cache_size = mp->maxsz / CACHE_LINE_SIZE;
    int tlbnb = pgnum % cache_size;

    mp->storage[tlbnb] = pid;
    mp->storage[tlbnb + 1] = (pgnum >> 8) & 0xFF;
    mp->storage[tlbnb + 2] = pgnum & 0xFF;
    mp->storage[tlbnb + 3] = (pte >> 16) & 0xFF;
    mp->storage[tlbnb + 4] = (pte >> 8) & 0xFF;
    mp->storage[tlbnb + 5] = pte & 0xFF;

    return 0;
}

/*
 *  TLBMEMPHY_read natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @value: obtained value 
 */
int TLBMEMPHY_read(struct memphy_struct * mp, int addr, BYTE *value)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   *value = mp->storage[addr];

   return 0;
}


/*
 *  TLBMEMPHY_write natively supports MEMPHY device interfaces
 *  @mp: memphy struct
 *  @addr: address
 *  @data: written data
 */
int TLBMEMPHY_write(struct memphy_struct * mp, int addr, BYTE* data)
{
   if (mp == NULL)
     return -1;

   /* TLB cached is random access by native */
   mp->storage[addr] = *data;
   return 0;
}

/*
 *  TLBMEMPHY_format natively supports MEMPHY device interfaces, use for debug
 *  @mp: memphy struct
 */

int TLBMEMPHY_dump(struct memphy_struct * mp)
{
   /*TODO dump memphy contnt mp->storage 
    *     for tracing the memory content
    */
    if (mp == NULL)
        return -1;
   MEMPHY_dump(mp); 

   return 0;
}


/*
 *  Init TLBMEMPHY struct
 */
int init_tlbmemphy(struct memphy_struct *mp, int max_size) {
    if (mp == NULL || max_size <= 0)
        return -1;

    mp->storage = (BYTE *)malloc(max_size * sizeof(BYTE));
    if (mp->storage == NULL)
        return -1;

    mp->maxsz = max_size;
    mp->rdmflg = 1;

    return 0;
}

//#endif
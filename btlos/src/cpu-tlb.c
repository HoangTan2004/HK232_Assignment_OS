/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee
 * a personal to use and modify the Licensed Source Code for
 * the sole purpose of studying during attending the course CO2018.
 */
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */

#include "mm.h"
#ifdef CPU_TLB
// #include "cpu-tlbcache.h"
#include <stdlib.h>
#include <stdio.h>

// BEGIN Support functions
// Function to handle if TLB Miss
int handle_page_fault(struct pcb_t *proc, uint32_t virtual_address)
{

  uint32_t page_index = PAGING_PGN(virtual_address);
  if (page_index >= PAGING_MAX_PGN)
  {
    return -1; // Ensure page index is within bounds
  }

  uint32_t *pte = &proc->mm->pgd[page_index];

  if (!(*pte & PAGING_PTE_PRESENT_MASK))
  {
    struct framephy_struct *frames;

    // Error handling if no frames available
    if (alloc_pages_range(proc, 1, &frames) != 0)
    {
      return -1;
    }

    int frame_number = frames->fpn;
    init_pte(pte, 1, frame_number, 0, 0, 0, 0);
    vmap_page_range(proc, virtual_address, 1, frames, NULL);

    // Update TLB if necessary
    tlb_cache_write(proc->tlb, proc->pid, page_index, frame_number);
  }

  // Page fault handled successfully
  return 0;
}

// END Support functions

int tlb_change_all_page_tables_of(struct pcb_t *proc, struct memphy_struct *mp)
{
  if (proc == NULL || mp == NULL || mp->storage == NULL)
  {
    return -1; // Invalid input parameters
  }

  int total_entries = mp->maxsz / ENTRY_SIZE; // Calculate the number of entries
  int entries_invalidated = 0;

  BYTE entry[ENTRY_SIZE];
  // Iterate over all TLB entries
  for (int i = 0; i < total_entries; ++i)
  {
    int entry_addr = i * ENTRY_SIZE;
    if (entry_addr + ENTRY_SIZE > mp->maxsz)
    {
      return -1; // Prevent buffer overflow
    }

    // Read the entry from the TLB storage
    TLBMEMPHY_read(mp, entry_addr, entry);

    uint32_t entry_pid = decode_int(entry + PID_OFFSET);
    BYTE valid = entry[VALID_OFFSET];

    // Check if the entry belongs to the process and is valid
    if (valid && entry_pid == proc->pid)
    {
      // Invalidate the entry by setting the valid bit to 0
      entry[VALID_OFFSET] = 0;                // Mark as invalid
      TLBMEMPHY_write(mp, entry_addr, entry); // Write back the updated entry
      entries_invalidated++;
    }
  }

  return entries_invalidated; // Return the count of invalidated entries
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct *mp)
{
  if (proc == NULL || mp == NULL || mp->storage == NULL)
  {
    return -1; // Invalid input parameters
  }

  int total_entries = mp->maxsz / ENTRY_SIZE; // Calculate the number of entries
  int entries_flushed = 0;                    // Counter for how many entries are flushed

  BYTE entry[ENTRY_SIZE];
  for (int i = 0; i < total_entries; ++i)
  {
    int entry_addr = i * ENTRY_SIZE;
    if (entry_addr + ENTRY_SIZE > mp->maxsz)
    {
      return -1; // Prevent buffer overflow
    }

    // Read the entry from the TLB storage
    TLBMEMPHY_read(mp, entry_addr, entry);

    uint32_t entry_pid = decode_int(entry + PID_OFFSET);
    BYTE valid = entry[VALID_OFFSET];

    // Check if the entry is valid and belongs to the given process
    if (valid && entry_pid == proc->pid)
    {
      // Invalidate the entry by setting the valid bit to 0
      entry[VALID_OFFSET] = 0;                // Mark as invalid
      TLBMEMPHY_write(mp, entry_addr, entry); // Write back the updated entry
      entries_flushed++;
    }
  }

  return entries_flushed; // Return the number of entries flushed
}

/* tlballoc - CPU TLB-based allocate a region memory
 *  @proc:  Process executing the instruction
 *  @size: allocated size
 *  @reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  if (proc == NULL || proc->mm == NULL)
  {
    return -1; // Invalid process or memory management structure
  }

  int addr, val;

  // Allocate virtual memory region
  val = __alloc(proc, 0, reg_index, size, &addr);
  if (val != 0)
  {
    return val; // Memory allocation failed, propagate the error
  }

  // Calculate the number of pages and the start page number
  uint32_t num_pages = (size + PAGING_PAGESZ - 1) / PAGING_PAGESZ;
  uint32_t start_page = PAGING_PGN(addr);

  // Update TLB for each page in the newly allocated region
  for (uint32_t i = 0; i < num_pages; i++)
  {
    uint32_t pgnum = start_page + i;
    uint32_t *pte = &proc->mm->pgd[pgnum]; // Direct access to the page table entry

    if (pte && (*pte & PAGING_PTE_PRESENT_MASK))
    {
      uint32_t frame_number = PAGING_FPN(*pte);

      // Check TLB for existing entry and update if necessary
      BYTE frame_in_tlb;
      int tlb_result = tlb_cache_read(proc->tlb, proc->pid, pgnum, &frame_in_tlb);

      // If TLB miss or frame number mismatch, update TLB
      if (tlb_result != 0 || frame_in_tlb != frame_number)
      {
        tlb_cache_write(proc->tlb, proc->pid, pgnum, (BYTE)frame_number);
      }
    }
  }

  return 0; // Success
}

/*  pgfree - CPU TLB-based free a region memory
 *  @proc: Process executing the instruction
 *  @size: allocated size
 *  @reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  if (proc == NULL || proc->mm == NULL || reg_index >= PAGING_MAX_SYMTBL_SZ)
  {
    return -1; // Invalid input check
  }

  // Get region details
  struct vm_rg_struct *region = &proc->mm->symrgtbl[reg_index];
  if (region->rg_start == 0 && region->rg_end == 0)
  {
    return -1; // Check for uninitialized or already freed region
  }

  // Free the memory region
  __free(proc, 0, reg_index);

  // Invalidate TLB entries for all pages in the region
  uint32_t start_page = PAGING_PGN(region->rg_start);
  uint32_t end_page = PAGING_PGN(region->rg_end - 1); // inclusive end
  BYTE dummy_frame_number = 0xFF;                     // Use a dummy frame number to denote an invalid entry

  for (uint32_t page_number = start_page; page_number <= end_page; ++page_number)
  {
    BYTE current_frame;
    if (tlb_cache_read(proc->tlb, proc->pid, page_number, &current_frame) == 0)
    {
      // Invalidate the entry
      tlb_cache_write(proc->tlb, proc->pid, page_number, dummy_frame_number);
    }
  }

  return 0; // Success
}

/* tlbread - CPU TLB-based read a region memory
 * @proc: Process executing the instruction
 * @source: index of source register
 * @offset: source address = [source] + [offset]
 * @destination: destination storage
 */
int tlbread(struct pcb_t *proc, uint32_t source, uint32_t offset, uint32_t *destination)
{
  BYTE data;
  BYTE frmnum = -1; // Initialize to an invalid frame number

  // Calculate the virtual address from the base address stored in regs[source] plus the offset
  uint32_t virtual_address = proc->regs[source] + offset;
  uint32_t page_number = PAGING_PGN(virtual_address); // Compute the page number, assuming PAGE_SIZE defined

  // Attempt to read from TLB
  if (tlb_cache_read(proc->tlb, proc->pid, page_number, (BYTE *)&frmnum) != 0)
  {
    // TLB miss, handle the page fault
    if (handle_page_fault(proc, virtual_address) != 0)
    {
      return -1; // Handle error if page fault handling fails
    }

    // After handling the page fault, attempt to read the frame number again
    if (tlb_cache_read(proc->tlb, proc->pid, page_number, (BYTE *)&frmnum) != 0)
    {
      return -1; // Still failing after handling page fault
    }
  }

#ifdef IODUMP
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", source, offset);
  else
    printf("TLB miss at read region=%d offset=%d\n", source, offset);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); // Print the entire page table.
#endif
  MEMPHY_dump(proc->mram);
#endif

  // If the frame number is valid, perform the memory read
  int val = __read(proc, 0, source, offset, &data);
  if (val != 0)
  {
    return val; // Error reading memory
  }

  // Store the read data to the destination register
  *((uint32_t *)destination) = (uint32_t)data;

  // Optionally update the TLB with the frame number of the recently accessed page
  tlb_cache_write(proc->tlb, proc->pid, page_number, frmnum);

  return val;
}

/* tlbwrite - CPU TLB-based write a region memory
 * @proc: Process executing the instruction
 * @data: data to be wrttien into memory
 * @destination: index of destination register
 * @offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t *proc, BYTE data, uint32_t destination, uint32_t offset)
{
  int val;
  BYTE frmnum = -1; // Initialize frame number as invalid

  if (!proc || !proc->tlb || destination >= sizeof(proc->regs) / sizeof(proc->regs[0]))
  {
    return -1;
  }

  // Calculate the virtual address using the destination register and offset
  uint32_t virtual_address = proc->regs[destination] + offset;
  uint32_t page_number = PAGING_PGN(virtual_address);

  // Try to retrieve the frame number from the TLB
  if (tlb_cache_read(proc->tlb, proc->pid, page_number, (BYTE *)&frmnum) != 0)
  {
    // TLB miss, handle the page fault
    if (handle_page_fault(proc, virtual_address) != 0)
    {
      return -1; // Handle error if page fault handling fails
    }

    // After handling the page fault, attempt to read the frame number again
    if (tlb_cache_read(proc->tlb, proc->pid, page_number, (BYTE *)&frmnum) != 0)
    {
      return -1; // Still failing after handling page fault
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
  print_pgtbl(proc, 0, -1); // Print the entire page table.
#endif
  MEMPHY_dump(proc->mram); // Dump physical memory.
#endif

  // If the frame number is valid, perform the memory write
  int physical_address = (frmnum * PAGING_PAGESZ) + (virtual_address % PAGING_PAGESZ);
  val = __write(proc, 0, destination, offset, data);

  if (val == 0)
  {
    // Only update TLB on successful write
    tlb_cache_write(proc->tlb, proc->pid, page_number, (BYTE)frmnum);
  }

  return val;
}

#endif

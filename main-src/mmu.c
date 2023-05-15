#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/* The frame table pointer. You will set this up in system_init. */
fte_t *frame_table;

/**
 * In this problem, you will initialize the frame_table pointer. The frame table will
 * be located at physical address 0 in our simulated memory. You should zero out the 
 * entries in the frame table, in case for any reason physical memory is not clean.
 */
void system_init(void) {
    memset(mem, 0, PAGE_SIZE);
    frame_table = (fte_t *)mem;
    frame_table->protected = 1;
}

/**
 * Takes an input virtual address and performs a memory operation.
 * 
 * @param addr virtual address to be translated
 * @param access 'r' if the access is a read, 'w' if a write
 * @param data If the access is a write, one byte of data to written to our memory.
 *             Otherwise NULL for read accesses.
 */
uint8_t mem_access(vaddr_t addr, char access, uint8_t data) {
    stats.accesses++;
    
    vpn_t vpn = vaddr_vpn(addr);
    uint16_t offset = vaddr_offset(addr);
    uint8_t *page_mem_start = mem + PTBR*PAGE_SIZE;
    pte_t *page_table = vpn + (pte_t *)page_mem_start;
    if (page_table->valid == 0) {
        page_fault(addr);
    }
    frame_table[page_table->pfn].referenced = 1;
    // if(frame_table[page_table->pfn].ref_count < 255) {
    //     frame_table[page_table->pfn].ref_count++;
    // }
    paddr_t physical = (paddr_t)(page_table->pfn << OFFSET_LEN) | offset;

    /* Either read or write the data to the physical address
       depending on 'rw' */
    if (access == 'r') {
        return mem[physical];
    } else {
        page_table->dirty = 1;
        mem[physical] = data;
        return mem[physical];
    }

    return 0;
}
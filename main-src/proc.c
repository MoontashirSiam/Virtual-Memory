#include "proc.h"
#include "mmu.h"
#include "pagesim.h"
#include "va_splitting.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * This function gets called every time a new process is created.
 * You will need to allocate a frame for the process's page table using the
 * free_frame function. Then, you will need update both the frame table and
 * the process's PCB. 
 * 
 * @param proc pointer to process that is being initialized 
 */
void proc_init(pcb_t *proc) {
    pfn_t victim_pfn = free_frame();
    uint8_t *page_mem_start = mem + victim_pfn*PAGE_SIZE;
    memset(page_mem_start, 0, PAGE_SIZE);

    fte_t *frame_table_entry = frame_table + victim_pfn;
    frame_table_entry->process = proc;
    frame_table_entry->mapped = 1;
    frame_table_entry->protected = 1;
    
    proc->saved_ptbr = victim_pfn;

}

/**
 * Switches the currently running process to the process referenced by the proc 
 * argument.
 * 
 * Every process has its own page table, as you allocated in proc_init. You will
 * need to tell the processor to use the new process's page table.
 * 
 * @param proc pointer to process to become the currently running process.
 */
void context_switch(pcb_t *proc) {
    PTBR = proc->saved_ptbr;
}

/**
 * When a process exits, you need to free any pages previously occupied by the
 * process.
 */
void proc_cleanup(pcb_t *proc) {
    for (size_t i = 0; i < NUM_PAGES; i++) {
        pfn_t process_pfn = proc->saved_ptbr;
        pte_t *page = (pte_t *)(mem + process_pfn * PAGE_SIZE) + i;
        if(page->valid) {
            page->valid = 0;
            frame_table[page->pfn].mapped = 0;
            frame_table[page->pfn].referenced = 0;
            frame_table[page->pfn].ref_count = 0;
        }
        // Check if process has swapped any pages to disk
        if(swap_exists(page)) {
            swap_free(page);
        }
    }

    // Clear frame table
    fte_t *frame_table_entry = frame_table + proc->saved_ptbr;
    frame_table_entry->mapped = 0;
    frame_table_entry->protected = 0;
}

#pragma GCC diagnostic pop

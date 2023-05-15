#include "mmu.h"
#include "pagesim.h"
#include "swapops.h"
#include "stats.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/**
 * Page fault handler.
 * 
 * When the CPU encounters an invalid address mapping in a page table, it invokes the 
 * OS via this handler. Your job is to put a mapping in place so that the translation 
 * can succeed.
 * 
 * @param addr virtual address in the page that needs to be mapped into main memory.
 */
void page_fault(vaddr_t addr) {
   stats.page_faults++;
   
   vpn_t vpn = vaddr_vpn(addr);
   pte_t *pagetable = (pte_t *)(mem + PTBR * PAGE_SIZE) + vpn;
   
   pfn_t victim = free_frame();
   pagetable->dirty = 0;
   pagetable->pfn = victim;
   pagetable->valid = 1;

   fte_t *frame_table_entry = frame_table + pagetable->pfn;
   frame_table_entry->mapped = 1;
   frame_table_entry->protected = 0;
   frame_table_entry->referenced = 1;
   frame_table_entry->ref_count = 0;
   frame_table_entry->vpn = vpn;
   frame_table_entry->process = current_process;

   if(swap_exists(pagetable)){
      swap_read(pagetable, (uint8_t *)(mem + victim*PAGE_SIZE));
   } else {
      memset((uint8_t *)(mem + victim*PAGE_SIZE), 0, PAGE_SIZE);
   }
}

#pragma GCC diagnostic pop

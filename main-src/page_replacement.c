#include "types.h"
#include "pagesim.h"
#include "mmu.h"
#include "swapops.h"
#include "stats.h"
#include "util.h"

pfn_t select_victim_frame(void);

pfn_t last_evicted = 0;

/**
 * Make a free frame for the system to use. You call the select_victim_frame() method
 * to identify an "available" frame in the system (already given). You will need to 
 * check to see if this frame is already mapped in, and if it is, you need to evict it.
 * 
 * @return victim_pfn: a phycial frame number to a free frame be used by other functions.
 */
pfn_t free_frame(void) {
    pfn_t victim_pfn = select_victim_frame();
    
    if (frame_table[victim_pfn].mapped) {
        frame_table[victim_pfn].mapped = 0;
        vpn_t vpn = frame_table[victim_pfn].vpn;
        pfn_t process_pfn = frame_table[victim_pfn].process->saved_ptbr;
        pte_t *pagetable = (pte_t *)(mem + process_pfn * PAGE_SIZE) + vpn;
        pagetable->valid = 0;
        if (pagetable->dirty) {
            swap_write(pagetable, mem + victim_pfn * PAGE_SIZE);
            stats.writebacks++;
            pagetable->dirty = 0;
        }
    }

    return victim_pfn;
}



/**
 * MAKE SURE YOU COMPLETE daemon_update() BELOW BEFORE DOING APPROX_LRU
 * 
 * Finds a free physical frame. If none are available, uses either a
 * randomized, approximate LRU, or clocksweep algorithm to find a used frame for
 * eviction.
 * 
 * @return The physical frame number of a victim frame.
 */
pfn_t select_victim_frame() {
    /* See if there are any free frames first */
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (size_t i = 0; i < num_entries; i++) {
        if (!frame_table[i].protected && !frame_table[i].mapped) {
            return i;
        }
    }

    if (replacement == RANDOM) {
        /* Play Russian Roulette to decide which frame to evict */
        pfn_t unprotected_found = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                unprotected_found = i;
                if (prng_rand() % 2) {
                    return i;
                }
            }
        }
        /* If no victim found yet take the last unprotected frame
           seen */
        if (unprotected_found < NUM_FRAMES) {
            return unprotected_found;
        }


    } else if (replacement == APPROX_LRU) {
        pfn_t least_recently_used = NUM_FRAMES;
        for (pfn_t i = 0; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                if (least_recently_used == NUM_FRAMES) {
                    least_recently_used = i;
                } else if(frame_table[i].ref_count < frame_table[least_recently_used].ref_count) {
                    least_recently_used = i;
                }
            }
        }
        if (least_recently_used < NUM_FRAMES) {

            return least_recently_used;
        }

    } else if (replacement == CLOCKSWEEP) {
        for (pfn_t i = last_evicted; i < num_entries; i++) {
            if (!frame_table[i].protected) {
                if(frame_table[i].referenced) {
                    frame_table[i].referenced = 0;
                } else {
                    last_evicted = (i + 1) % num_entries;
                    return i;
                }
            }
            i = (i + 1) % num_entries - 1;
        }
        pfn_t victim = last_evicted;
        last_evicted = (last_evicted + 1) % num_entries;
        return victim; 
    }

    panic("System ran out of memory\n");
    exit(1);
}

/**
 * Updates the associated variables for the Approximate LRU,
 * called every time the simulator daemon wakes up.
*/
void daemon_update(void) {
    size_t num_entries = MEM_SIZE / PAGE_SIZE;
    for (pfn_t i = 0; i < num_entries; i++) {
        frame_table[i].ref_count = (frame_table[i].referenced << 7 | frame_table[i].ref_count >> 1);
        if (frame_table[i].referenced) {
            frame_table[i].referenced = 0;
        }
    }
}


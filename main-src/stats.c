#include "stats.h"

/* The stats. See the definition in stats.h. */
stats_t stats;

/**
 * Calulate the total average time it takes for an access
 */
void compute_stats() {
    stats.amat = (MEMORY_ACCESS_TIME * stats.accesses + DISK_PAGE_READ_TIME * stats.page_faults + DISK_PAGE_WRITE_TIME * stats.writebacks) / (double) stats.accesses;
}

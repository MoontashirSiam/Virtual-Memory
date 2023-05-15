#pragma once

#include "swap.h"
#include "types.h"
#include "pagesim.h"
#include "va_splitting.h"

/**
 * Process functions.
 */

void proc_init(pcb_t *proc);
void context_switch(pcb_t *proc);
void proc_cleanup(pcb_t *proc);

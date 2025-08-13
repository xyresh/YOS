#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define TOTAL_MEMORY_MB 128
#define TOTAL_PAGES ((TOTAL_MEMORY_MB * 1024 * 1024) / PAGE_SIZE)

void pmm_init();
void* alloc_page();
void free_page(void* addr);
uint32_t pmm_get_free_count();

#endif
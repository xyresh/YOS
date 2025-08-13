#include "pmm.h"
#include <stddef.h>
#include <stdint.h>

#define BITMAP_BASE 0x400000  
static uint8_t* bitmap = (uint8_t*)BITMAP_BASE;

static inline void set_bit(uint32_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(uint32_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int test_bit(uint32_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}

void pmm_init() {
    for (uint32_t i = 0; i < TOTAL_PAGES; i++) set_bit(i);

    uint32_t first_free_page = (BITMAP_BASE + 4096 + PAGE_SIZE - 1) / PAGE_SIZE;
    for (uint32_t i = first_free_page; i < TOTAL_PAGES; i++) clear_bit(i);
}

// Allocate a free 4KB page, return its physical address
void* alloc_page() {
    for (uint32_t i = 0; i < TOTAL_PAGES; i++) {
        if (!test_bit(i)) {
            set_bit(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL; // Out of memory
}

// Free a previously allocated 4KB page
void free_page(void* addr) {
    uint32_t page = ((uint32_t)addr) / PAGE_SIZE;
    clear_bit(page);
}

uint32_t pmm_get_free_count() {
    uint32_t count = 0;
    for (uint32_t i = 0; i < TOTAL_PAGES; i++) {
        if (!test_bit(i)) count++;
    }
    return count;
}
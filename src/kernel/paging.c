#include "paging.h"
#include "pmm.h"
#include "io.h"
#include <stdint.h>
#include <stddef.h>

#define PAGE_ENTRIES 1024

static uint32_t* page_directory = (uint32_t*)0x500000;
static uint32_t* first_page_table = (uint32_t*)0x501000;

void load_page_directory(uint32_t* pd) {
    __asm__ __volatile__("mov %0, %%cr3":: "r"(pd));
}

void enable_paging() {
    uint32_t cr0;
    __asm__ __volatile__(
        "mov %%cr0, %0\n"
        "or $0x80000000, %0\n"
        "mov %0, %%cr0"
        : "=r"(cr0)
        :
        : "memory"
    );
}

void paging_init() {
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        page_directory[i] = 0x00000002; 
        first_page_table[i] = 0x00000002;
    }

    // Identity map 0x00000000 - 0x003FFFFF (4MB)
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;

    load_page_directory(page_directory);
    enable_paging();
}

// Map a single page (4KB)
void map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    uint32_t* pt;

    // Get or create page table
    if (page_directory[pd_index] & PAGE_PRESENT) {
        pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    } else {
        pt = (uint32_t*)alloc_page();
        for (int i = 0; i < PAGE_ENTRIES; i++) pt[i] = 0;
        page_directory[pd_index] = ((uint32_t)pt) | flags | PAGE_PRESENT | PAGE_RW;
    }

    pt[pt_index] = (paddr & 0xFFFFF000) | (flags & 0xFFF) | PAGE_PRESENT;
}

// Unmap a single page
void unmap_page(uint32_t vaddr) {
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    if (page_directory[pd_index] & PAGE_PRESENT) {
        uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
        pt[pt_index] = 0;
    }
}

void* get_phys_addr(uint32_t vaddr) {
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = (vaddr >> 12) & 0x3FF;
    if (!(page_directory[pd_index] & PAGE_PRESENT)) return NULL;
    uint32_t* pt = (uint32_t*)(page_directory[pd_index] & 0xFFFFF000);
    if (!(pt[pt_index] & PAGE_PRESENT)) return NULL;
    return (void*)((pt[pt_index] & 0xFFFFF000) | (vaddr & 0xFFF));
}
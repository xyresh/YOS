#include "heap.h"
#include <stddef.h>

#define HEAP_START 0x300000
#define HEAP_SIZE  (1024 * 1024)  // 1MB heap

typedef struct heap_block {
    uint32_t size;
    struct heap_block* next;
    int free;
} heap_block_t;

#define BLOCK_SIZE sizeof(heap_block_t)

static uint8_t* heap = (uint8_t*)HEAP_START;
static heap_block_t* free_list = NULL;

static void heap_init() {
    if (!free_list) {
        free_list = (heap_block_t*)heap;
        free_list->size = HEAP_SIZE - BLOCK_SIZE;
        free_list->next = NULL;
        free_list->free = 1;
    }
}

void* kmalloc(uint32_t size) {
    heap_init();
    heap_block_t* curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size > size + BLOCK_SIZE) {
                // Split block
                heap_block_t* next = (heap_block_t*)((uint8_t*)curr + BLOCK_SIZE + size);
                next->size = curr->size - size - BLOCK_SIZE;
                next->next = curr->next;
                next->free = 1;
                curr->size = size;
                curr->next = next;
            }
            curr->free = 0;
            return (void*)((uint8_t*)curr + BLOCK_SIZE);
        }
        curr = curr->next;
    }
    return NULL;
}

void kfree(void* ptr) {
    if (!ptr) return;
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - BLOCK_SIZE);
    block->free = 1;
    // Coalesce adjacent free blocks
    heap_block_t* curr = free_list;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += BLOCK_SIZE + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}
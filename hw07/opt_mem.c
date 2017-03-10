
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include "nu_mem.h"

/*
Start with the HW06 allocator.

Add a second free list - cell_stack. This is treated as a stack - all you can do is push or pop from the front.

For malloc, if request size = sizeof(icell):
    Pop from cell_stack, use that.
    If cell_stack is empty, allocate as usual from the original free list / new chunk.

For free, if size = sizeof(icell):
    Push the block to cell_stack.
    Else, free as usual.
*/

typedef struct nu_free_cell {
    int64_t              size;
    struct nu_free_cell* next;
} nu_free_cell;

typedef struct icell {
    int64_t              size;
    struct icell*        next;
} icell;

// You should update these counters on memory allocation / deallocation events.
// These counters should only go up, and should provide totals for the entire
// execution of the program.
static int64_t nu_malloc_count  = 0; // How many times has malloc returned a block.
static int64_t nu_malloc_bytes  = 0; // How many bytes have been allocated total
static int64_t nu_free_count    = 0; // How many times has free recovered a block.
static int64_t nu_free_bytes    = 0; // How many bytes have been recovered total.
static int64_t nu_malloc_chunks = 0; // How many chunks have been mmapped?
static int64_t nu_free_chunks   = 0; // How many chunks have been munmapped?

// one of: 1024, 65536, 1048576
static const int64_t CHUNK_SIZE = 65536;
static const int64_t CELL_SIZE  = (int64_t)sizeof(nu_free_cell);

static nu_free_cell* nu_free_list = 0;
static nu_free_cell* cell_stack = 0;

void* nu_malloc(size_t usize);
void nu_free(void* addr);
nu_free_cell* make_cell2();
void* pop_cell_stack();
void free_list_insert(nu_free_cell* cell);

/* Allocate memory of size usize. 
 * Returns address of allocated memory block.
 * size == sizeof(icell) - run optimized allocator.
 * size != sizeof(icell) - (void*)hw06_malloc(size).
 */
void* nu_malloc(size_t size) {
    // check if size is equal to the icell size
    if (size == sizeof(icell)) {
      return pop_cell_stack();
    }
    // otherwise use regular allocator
    else {
        return (void*)hw06_malloc(size);
    }
}

/* Free memory for object located at the input address
 * == sizeof(icell) - specialized deallocator.
 * != sizeof(icell) - hwo6_free(ptr).
 */
void nu_free(void* ptr) {
    nu_free_cell* cell = (nu_free_cell*)(ptr - sizeof(int64_t));
    int64_t size = *((int64_t*) cell);

    if (size == sizeof(icell)) {
        cell->size = size;
        free_list_insert(cell);
    }
    else {
        hw06_free(ptr);
    }
}

// Make a new cell, return pointer to it
// Redefined from make_cell() (in hw06_mem.c) because of
//   strange incompatability issues
nu_free_cell*
make_cell2() {
    nu_free_cell* new_cell;
    void* addr = mmap(0, CHUNK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    nu_malloc_chunks += 1;
    
    new_cell = (nu_free_cell*)addr;
    new_cell->size = CHUNK_SIZE;
    new_cell->next = 0;
    
    return new_cell;
}

// pop_cell_stack: Optimized allocator for list_sum_ints.c.
void* pop_cell_stack() {
    int64_t size = sizeof(icell) + sizeof(int64_t);

    nu_free_cell* pp = cell_stack; // pointer to cell_stack
    nu_free_cell* cell = 0;        // new cell

    if (cell_stack != NULL) {      // if cell_stack is not empty
        if (pp->size >= size) {    // if first cell is large enough
            cell = pp;
            cell_stack = pp->next;
        }
    } else if (cell == NULL){      // otherwise make a new list
        cell = make_cell2();
    }

    // return unused portion to free list
    int64_t rest_size = cell->size - size;
    if (rest_size > CELL_SIZE) {
        void* addr = (void*) cell;
        nu_free_cell* rest = (nu_free_cell*) (addr + size);
        rest->size = rest_size;
        free_list_insert(rest);
    } else {
        size += rest_size;
    }

    *((int64_t*) cell) = size;
    return ((void*) cell) + sizeof(int64_t);
}

/* Insert a new free cell into cell_stack. */
void free_list_insert(nu_free_cell* cell) {
    cell->next = cell_stack;
    cell_stack = cell;
}
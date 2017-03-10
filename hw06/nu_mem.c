/* Bailey Kane
 * February 25, 2017
 * hw06
 * Supplemented with code produced by Prof. Nat Tuck during CS3650 lecture.
 */

#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>

#include "nu_mem.h"

/* Basic strategy:
 *  - Linked list of memory blocks.
 *  - global ptr to head of the list.
 */

/* Allocate:
 *   1. calc real size = requested + sizeof(size)
 *   2. > 64k ?
 *     a. just mmap it, return
 *   3. < 64k ?
 *     a. Find big enough block on free list
 *     b. if found, save any be enough remaining, return ptr to block
 *     c. if no large enough block found, mmap new 64k block, add to free list,
 *          goto 3.a. again
 */

/* free(ptr):
 *   1. recover size(before ptr)
 *   2. > 64k ? munmap
 *   3. < 64k ?
 *     a. stick block on free list
 *       i. cons
 *       ii. OR insert in memory order
 *     b. check for coalescing
 *
 * coalescing - find any adjacent blocks on free list, combine into single block
 *   - doubly linked list helps
 */

typedef struct nu_free_cell {
    int64_t              size;
    struct nu_free_cell* next;
} nu_free_cell;

static const int64_t CHUNK_SIZE = 65536;
static const int64_t CELL_SIZE  = (int64_t)sizeof(nu_free_cell);

static nu_free_cell* nu_free_list = 0;

// You should update these counters on memory allocation / deallocation events.
// These counters should only go up, and should provide totals for the entire
// execution of the program.
static int64_t nu_malloc_count  = 0; // How many times has malloc returned a block.
static int64_t nu_malloc_bytes  = 0; // How many bytes have been allocated total
static int64_t nu_free_count    = 0; // How many times has free recovered a block.
static int64_t nu_free_bytes    = 0; // How many bytes have been recovered total.
static int64_t nu_malloc_chunks = 0; // How many chunks have been mmapped?
static int64_t nu_free_chunks   = 0; // How many chunks have been munmapped?

int64_t nu_free_list_length();
void nu_mem_print_stats();
void* nu_malloc(size_t usize);
void nu_free(void* addr);
void nu_free_list_insert(nu_free_cell* cell);
void nu_free_list_coalesce();
nu_free_cell* free_list_get_cell(int64_t req_size);
nu_free_cell* make_cell();
void print_free_list();

int64_t
nu_free_list_length()
{
    nu_free_cell* pp = nu_free_list;
    int64_t length = 0;
    
    // Traverse through list, incrementing length
    while(pp != NULL) {
        pp = pp->next;
        length++;
    }
    
    return length;
}

void
nu_mem_print_stats()
{
    fprintf(stderr, "\n== nu_mem stats ==\n");
    fprintf(stderr, "malloc count: %ld\n", nu_malloc_count);
    fprintf(stderr, "malloc bytes: %ld\n", nu_malloc_bytes);
    fprintf(stderr, "free count: %ld\n", nu_free_count);
    fprintf(stderr, "free bytes: %ld\n", nu_free_bytes);
    fprintf(stderr, "malloc chunks: %ld\n", nu_malloc_chunks);
    fprintf(stderr, "free chunks: %ld\n", nu_free_chunks);
    fprintf(stderr, "free list length: %ld\n", nu_free_list_length());
}

/* Allocate memory of size usize. 
 * Returns address of allocated memory block.
 */
void*
nu_malloc(size_t usize)
{
    // printf("Entered nu_malloc\n");
    // Allocate small blocks of memory by allocating 64k chunks
    // and then satisfying multiple requests from that.
    //
    // Allocate large blocks (>= 64k) of memory directly with
    // mmap.
    
    int64_t size = (int64_t) usize;
    
    //space for size, requested + sizeof(size)
    int64_t alloc_size = size + sizeof(int64_t);
    
    // space for free cell when returned to list
    if (alloc_size < CELL_SIZE) {
        alloc_size = CELL_SIZE;
    }
    
    // increment counters
    nu_malloc_count += 1;
    nu_malloc_bytes += alloc_size;
    
    // printf("Checking large allocation\n");
    // Large allocations
    if (alloc_size > CHUNK_SIZE) {
        void* addr = mmap(0, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        *((int64_t*)addr) = alloc_size;
        nu_malloc_chunks += 1;
        return addr + sizeof(int64_t);
    }
    // printf("Not large. Tryna get small allocation\n");
    
    nu_free_cell* cell = free_list_get_cell(alloc_size);
    if (!cell) {
        // printf("free_list_get_cell returned false\n");
        cell = make_cell();
    }
    
    // Return unused portion to free list.
    int64_t rest_size = cell->size - alloc_size;
    // printf("Returning unused %i bytes to free list\n", rest_size);
    if (rest_size >= CELL_SIZE) {
        void* addr = (void*) cell;
        nu_free_cell* rest = (nu_free_cell*) (addr + alloc_size);
        rest->size = rest_size;
        nu_free_list_insert(rest);
    } else {
        alloc_size += rest_size;
    }
    
    *((int64_t*)cell) = alloc_size;
    return ((void*)cell) + sizeof(int64_t);
}

/* Free memory for object located at the input address
 * Large (> CHUNK_SIZE) - munmap
 * Small (<= CHUNK_SIZE) - put in free list
 */
void
nu_free(void* addr)
{
    // Free small blocks by saving them for reuse.
    //   - Stick together adjacent small blocks into bigger blocks.
    //   - Advanced: If too many full chunks have been freed (> 4 maybe?)
    //     return some of them with munmap.
    // Free large blocks with munmap.
    
    nu_free_cell* cell = (nu_free_cell*)(addr - sizeof(int64_t));
    int64_t size = *((int64_t*) cell);
    
    if (size > CHUNK_SIZE) {            // if large, free with munmap
        munmap((void*) cell, size);
        nu_free_chunks += 1;
    } else {                            // if small, put in free list
        cell->size = size;
        nu_free_list_insert(cell);
    }
    
    nu_free_count += 1;
    nu_free_bytes += size;
}

/* Insert a *new* free cell into nu_free_list. */
void nu_free_list_insert(nu_free_cell* cell) {
    if (nu_free_list == 0 || ((uint64_t) nu_free_list) > ((uint64_t) cell)) {
        cell->next = nu_free_list;
        nu_free_list = cell;
        return;
    }
    
    nu_free_cell* pp = nu_free_list;
    
    while (pp->next != 0 && ((uint64_t)pp->next) < ((uint64_t) cell)) {
        pp = pp->next;
    }
    
    cell->next = pp->next;
    pp->next = cell;
    
    nu_free_list_coalesce();
}

/* Merge adjacent free cells in memory. */
void nu_free_list_coalesce() {
    nu_free_cell* pp = nu_free_list;
    int free_chunk = 0;
    
    while (pp != 0 && pp->next != 0) {
        if (((uint64_t) pp) + pp->size == ((uint64_t) pp->next)) {
            pp->size += pp->next->size;
            pp->next = pp->next->next;
        }
        
        pp = pp->next;
    }
}

// Check if nu_free_list has enough space for a cell of req_size
//   If yes, return pointer to it. If not, return NULL.
nu_free_cell*
free_list_get_cell(int64_t req_size) {
    
    if (nu_free_list != 0) { // If nu_free_list has cells in it
        nu_free_cell* pp = nu_free_list;
        
        if (pp->size >= req_size) { // Check if first node is big enough
            nu_free_list = pp->next;
            return pp;
        } else { // Check if any node is big enough
            while (pp->next != 0) {
                if (pp->next->size >= req_size) {
                    nu_free_cell* cell = pp->next;
                    cell->size = pp->next->size;
                    pp->next = pp->next->next;
                    return cell;
                }
                pp = pp->next;
            }
        }
    }
    return NULL;
}

// Make a new cell, return pointer to it
nu_free_cell*
make_cell() {
    // printf("makin' a new cell\n");
    nu_free_cell* new_cell;
    void* addr = mmap(0, CHUNK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    nu_malloc_chunks += 1;
    
    new_cell = (nu_free_cell*)addr;
    new_cell->size = CHUNK_SIZE;
    new_cell->next = 0;
    
    return new_cell;
}

void
print_free_list() {
    nu_free_cell* pp = (nu_free_cell*) nu_free_list;
    
    while (pp != 0) {
        // printf("addr: %i, size: %i\n", pp, pp->size);
        pp = pp->next;
    }
}
# Assignment
Write a memory allocator. This means implementing "malloc" and "free" functions.

## Details
The "malloc" and "free" functions are the point of the assignment, so they can't be used - directly or indirectly (e.g. strdup) - in the solution.

Memory is requested from the operating system with mmap. The allocator maintains a "free list" of memory blocks available for allocation. The allocator handles two cases, differentiating between large and small allocations:

1. For large allocations (>= 65536 bytes = 64k), request the memory directly with mmap.
2. For small allocations (< 64k):
  1. If there's a big enough block on the free list, use that. Return any excess portion of the block to the free list.
  2. If there isn't a big enough block on the free list, allocate a new 64k block with mmap. Return any excess to the free list.

Freeing memory handles both cases. Large allocations go back to the system with munmap, small allocations go on the free list. Blocks on the free list that are adjacent in memory are coalesced - joined together into one larger block.

There are event tracking counters, along with a status printing function. These counters are updated accurately as described in the comments.

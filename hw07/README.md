# Assignment
* Write an optimized memory allocator for list_sum_ints.
* Benchmark different allocators with varying chunk sizes.
* Document your results.

## Details
* hw06_mem.c - Allocator from hw06.
* opt_mem.c - This is the new memory allocator optimized for the list_sum_ints program. The test runs faster with this allocator than the hw06 allocator - without using the system malloc / free. The custom allocator still requests memory from the system with mmap in (default 64k) chunks.

In addition, two allocators are tested with three different chunk sizes (+ the system allocator) on the machine. The three chunk sizes to test are: 1k, 64k, and 1024k (where 1k =1024B).

The "report.txt" file contains the following information:

1. Your name
2. The CPU, RAM, and OS of the machine you did the tets on.
3. A table, showing the runtime for each of the 7 allocators: 6 {allocator, chunk size} combinations + the system allocator in sys_mem.c.
4. At least two paragraphs explaining how and why varying the chunk size effects performance of the allocators.

## Strategy for opt_mem.c

1. Start with the hw06 allocator.
2. Add a second free list - cell_stack. This is treated as a stack - all you can do is push or pop from the front.
3. For malloc, if request size = sizeof(icell):
  1. Pop from cell_stack, use that.
  2. If cell_stack is empty, allocate as usual from the original free list / new chunk.
4. For free, if size = sizeof(icell):
  1. Push the block to cell_stack.
  2. Else, free as usual.

This means that all frees and all reuses of icells will be O(1) with no coalescing to worry about.

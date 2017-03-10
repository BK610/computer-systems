#ifndef NU_MEM_H
#define NU_MEM_H

#include <stdint.h>

void* nu_malloc(size_t size);
void  nu_free(void* ptr);
void  nu_mem_print_stats();
void  nu_print_free_list();

#endif

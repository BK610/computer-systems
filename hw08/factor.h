#ifndef FACTOR_H
#define FACTOR_H

#include <stdint.h>

#include "int128.h"
#include "ivec.h"

typedef struct factor_job {
    int128_t number;
    ivec*    factors;
} factor_job;

factor_job* make_job(int128_t nn);
void free_job(factor_job* job);

void submit_job(factor_job* job);
factor_job* get_result();

void* run_jobs();

ivec* factor(int128_t xx);

void factor_init(pthread_t threads[], int num_threads);
void factor_cleanup();

#endif

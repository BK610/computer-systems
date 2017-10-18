#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h> // include threads!

#include "int128.h"
#include "factor.h"
#include "queue.h"
#include "ivec.h"

int
main(int argc, char* argv[])
{
    int total_jobs = 0;
    int64_t ii;
    int rv;

    if (argc != 4) {
        printf("Usage:\n");
        printf("  ./main threads start count\n");
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int128_t start = atoh(argv[2]);
    int64_t  count = atol(argv[3]);

    pthread_t threads[num_threads];
    factor_init(threads, num_threads);
    
    for (ii = 0; ii < count; ++ii) {
        factor_job* job = make_job(start + ii);
        submit_job(job);
        
        if (ii == (count - 1)) {
            input_close();
        }
    }
    
    int64_t oks = 0;

    factor_job* job;
    while ((job = get_result())) {
        total_jobs++;
        print_int128(job->number);
        printf(": ");
        print_ivec(job->factors);

        ivec* ys = job->factors;
        
        int128_t prod = 1;
        for (ii = 0; ii < ys->len; ++ii) {
            prod *= ys->data[ii];
        }

        if (prod == job->number) {
            ++oks;
        }
        else {
            fprintf(stderr, "Warning: bad factorization");
        }

        free_job(job);

        if(total_jobs == count) {
            output_close();
            int jj;
            for(jj = 0; jj < num_threads; jj++) {
                factor_signal();
            }
        }
    }

    printf("Factored %ld / %ld numbers.\n", oks, count);
    
    factor_cleanup(num_threads, threads);

    return 0;
}

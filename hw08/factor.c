
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <pthread.h> // include threads!

#include "queue.h"
#include "factor.h"

// FIXME: Shared mutable data
static queue* iqueue;
static queue* oqueue;
static int total_jobs = 0;
static int complete_jobs = 0;

void
factor_init(pthread_t threads[], int num_threads)
{
    if (iqueue == 0) iqueue = make_queue();
    if (oqueue == 0) oqueue = make_queue();

    int ii = 0;
    while (ii < num_threads) {
        int rv = pthread_create(&(threads[ii]), NULL, run_jobs, NULL);
        assert(rv == 0);
        
        ii++;
    }
}

void
factor_cleanup(int num_threads, pthread_t* threads)
{
    // printf("cleaning!\nFree queues\n");
    free_queue(iqueue);
    iqueue = 0;
    free_queue(oqueue);
    oqueue = 0;

    // printf("joining threads\n");    
    int ii;
    for (ii = 0; ii < num_threads; ii++) {
        // printf("joining %d at %u\n", ii, &threads[ii]);
        int rv = pthread_join(threads[ii], NULL);
        assert(rv == 0);
    }
    // printf(("Joined %d threads!\n", num_threads));
}

factor_job* 
make_job(int128_t nn)
{
    factor_job* job = malloc(sizeof(factor_job));
    job->number  = nn;
    job->factors = 0;
    return job;
}

void 
free_job(factor_job* job)
{
    // printf("tryna free job!\n");
    if (job->factors) {
        free_ivec(job->factors);
    }
    free(job);
}

void 
submit_job(factor_job* job)
{
    queue_put(iqueue, job);
}

void
input_close()
{
    pthread_mutex_lock(&iqueue->mutex);
    iqueue->closed = 1;
    pthread_mutex_unlock(&iqueue->mutex);
}

void
output_close()
{
    pthread_mutex_lock(&oqueue->mutex);
    oqueue->closed = 1;
    pthread_mutex_unlock(&oqueue->mutex);
}

void
factor_signal()
{
    // printf("signaling\n");
    // pthread_mutex_lock(&oqueue->mutex);
    // pthread_cond_signal(&oqueue->not_empty);
    // pthread_mutex_unlock(&oqueue->mutex);
    
    pthread_mutex_lock(&iqueue->mutex);
    pthread_cond_signal(&iqueue->not_empty);
    pthread_mutex_unlock(&iqueue->mutex);

}

factor_job* 
get_result()
{
    if(oqueue->closed) { return 0; }
    
    return queue_get(oqueue);
}

static
int128_t
isqrt(int128_t xx)
{
    double yy = ceil(sqrt((double)xx));
    return (int128_t) yy;
}

ivec*
factor(int128_t xx)
{
    ivec* ys = make_ivec();

    while (xx % 2 == 0) {
        ivec_push(ys, 2);
        xx /= 2;
    }

    int128_t ii;
    for (ii = 3; ii <= isqrt(xx); ii += 2) {
        int128_t x1 = xx / ii;
        if (x1 * ii == xx) {
            ivec_push(ys, ii);
            xx = x1;
            ii = 1;
        }
    }

    ivec_push(ys, xx);

    return ys;
}

void*
run_jobs()
{
    factor_job* job;

    // FIXME: We should block on an empty queue waiting for more work.
    //        We can still use null job as the "end" signal.
    while ((job = queue_get(iqueue))) {
        job->factors = factor(job->number);
        queue_put(oqueue, job);
    }

    return 0;
}

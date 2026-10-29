#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "carlo.h"

typedef struct {
    unsigned int seed;
    unsigned long long rounds;
    unsigned long long success;
} task;

void *worker(void *arg){
    task *t = (task *)arg;
    unsigned long long s = carlo_alg(t->seed, t->rounds);
    t->success = s;
    return NULL;
}

int main(int argc, char *argv[]){
    if (argc < 3){
        fprintf(stderr, "Usage: %s <rounds> <threads> [seed]\n", argv[0]);
        return 1;
    }

    char *endp;
    unsigned long long rounds;
    unsigned int max_threads, seed;

    rounds = strtoull(argv[1], &endp, 10);
    if (*endp != '\0' || rounds == 0){
        fprintf(stderr, "Bad rounds: %s\n", argv[1]);
        return 1;   
    }

    max_threads = (unsigned int)strtoul(argv[2], &endp, 10);
    if (*endp != '\0' || max_threads == 0){
        fprintf(stderr, "Bad threads: %s\n", argv[2]);
        return 1;
    }

    if (argc >= 4){
        seed = (unsigned int)strtoul(argv[3], &endp, 10);
        if (*endp != '\0'){
            fprintf(stderr, "Bad seed: %s\n", argv[3]);
            return 1;
        }
    } else {
        seed = (unsigned int)time(NULL);
    }
    if (max_threads > rounds) {
        max_threads = (unsigned int)rounds;
    }
    pthread_t threads[max_threads];
    task tasks[max_threads];

    for (unsigned int i = 0; i < max_threads; i++){
        tasks[i].rounds = rounds / max_threads;
        if (i < rounds % max_threads){
            tasks[i].rounds++;
        }
        tasks[i].seed = seed + i;
        tasks[i].success = 0;
    }

    struct timespec start, end;
    unsigned int created = 0;
    int th = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (unsigned int i = 0; i < max_threads; i++){
        th = pthread_create(&threads[i], NULL, worker, &tasks[i]);
            if (th != 0){
                fprintf(stderr, "pthread_create failed: %s\n", strerror(th));
                break;
            }
        created++;
    }

    for (unsigned int i = 0; i < created; i++){
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (th != 0){
        return 1;
    }

    unsigned long long total = 0;
    for (unsigned int i = 0; i < max_threads; i++){
        total += tasks[i].success;
    }
    double prob = (double)total / rounds;

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Probability: %f\nTime: %.3f\n", prob, time_taken);
    return 0;
}
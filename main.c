#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "carlo.h"
#include <limits.h>
#include <errno.h>


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
    if (argc != 3 && argc != 4){
        fprintf(stderr, "Usage: %s <rounds> <threads> [seed]\n", argv[0]);
        return 1;
    }

    char *endp;
    unsigned long long rounds;
    unsigned int max_threads, seed;

    errno = 0;
    rounds = strtoull(argv[1], &endp, 10);

    if (argv[1][0] == '-' || endp == argv[1] || *endp != '\0' || rounds == 0 || errno == ERANGE) {
        fprintf(stderr, "Bad rounds: %s\n", argv[1]);
        return 1;
    }

    errno = 0;
    unsigned long long temp = strtoull(argv[2], &endp, 10);

    if (argv[2][0] == '-' || endp == argv[2] || *endp != '\0' || temp == 0 || errno == ERANGE || temp > UINT_MAX) {
        fprintf(stderr, "Bad threads: %s\n", argv[2]);
        return 1;
    }

    max_threads = (unsigned int)temp;

    if (argc == 4){
        errno = 0;
        temp = strtoul(argv[3], &endp, 10);
        if (argv[3][0] == '-' || endp == argv[3] || *endp != '\0' || errno == ERANGE || temp > UINT_MAX){
            fprintf(stderr, "Bad seed: %s\n", argv[3]);
            return 1;
        }
        seed = (unsigned int)temp;
    } else {
        seed = (unsigned int)time(NULL);
    }


    if (max_threads > rounds) {
        max_threads = (unsigned int)rounds;
    }

    pthread_t *threads = malloc(max_threads * sizeof(pthread_t));
    task *tasks = malloc(max_threads * sizeof(task));

    if (threads == NULL || tasks == NULL) {
        free(threads);
        free(tasks);
        return 1;
    }

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
    int create_error = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (unsigned int i = 0; i < max_threads; i++){
        th = pthread_create(&threads[i], NULL, worker, &tasks[i]);
            if (th != 0){
                fprintf(stderr, "pthread_create failed: %s\n", strerror(th));
                create_error = 1;
                break;
            }
        created++;
    }

    for (unsigned int i = 0; i < created; i++){
        th = pthread_join(threads[i], NULL);
         if (th != 0){
            fprintf(stderr, "pthread_join failed: %s\n", strerror(th));
            break;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    if (th != 0 || create_error == 1){
        free(threads);
        free(tasks);
        return 1;
    }

    unsigned long long total = 0;
    for (unsigned int i = 0; i < max_threads; i++){
        total += tasks[i].success;
    }
    double prob = (double)total / rounds;

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Probability: %f\nTime: %.3f\n", prob, time_taken);
    free(threads);
    free(tasks);
    return 0;
}
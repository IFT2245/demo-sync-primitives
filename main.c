/**
 * --------------------------------------------
 * Small mutex + condvars example.
 * This example is built so the mix of condvars + chores to do reflects the ideas behind a semaphore.
 * I prefer to teach condvars because of the versatility and their general usefulness.
 * Mutexes are required in the case of condvars and are "the" synchronisation primitive.
 * --------------------------------------------
 * Usage:
 * --------------------------------------------
 * ```
 * gcc main.c -lpthread
 * ./a.out [mut | cond]
 * ```
 * --------------------------------------------
 *
 * --------------------------------------------
 * Samuel Yvon
 * <samuel.yvon@umontreal.ca>
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

typedef void *(start_func)(void *);

pthread_mutex_t *mut;
pthread_cond_t *cond;

volatile int chores_to_do = 0;


void *locked_thread(void *arg) {
    printf("Help! I am locked!\n");
    pthread_mutex_lock(mut);
    printf("I gained the lock, which means I am free!\n");
    pthread_mutex_unlock(mut);
}

void *rebelious_locked_thread(void *arg) {
    printf("Help! I am locked!\n");
    printf("I am devious and I am going to unlock myself and lock again:\n");
    pthread_mutex_unlock(mut);
    pthread_mutex_lock(mut);
    printf("I gained the lock, which means I am free!\n");
    pthread_mutex_unlock(mut);
}

void *chores_doer(void *arg) {
    int id = (int) arg;
    printf("[%d] Hi, I do chores!\n", id);

    bool do_stuff = true;

    while (do_stuff) {
        pthread_mutex_lock(mut);
        while (0 == chores_to_do) {
            printf("[%d] No chores, Napping...\n", id);
            pthread_cond_wait(cond, mut);
            printf("[%d] Waking up from nap!\n", id);
        }

        do_stuff &= chores_to_do >= 0;

        if (do_stuff) {
            chores_to_do--;
            printf("[%d] %d chores left.\n", id, chores_to_do);
        } else {
            printf("[%d] No more for today? Bye bye!.\n", id);
        }

        pthread_mutex_unlock(mut);

        if (do_stuff) {
            printf("[%d] Doing some work! BRB!\n", id);
            sleep(2);
            printf("[%d] Done!\n", id);
        }
    }
}

pthread_t thread_easy_create(start_func start) {
    pthread_t tid;
    pthread_create(&tid, NULL, start, NULL);
    return tid;
}

pthread_t thread_id_create(start_func start, int id) {
    pthread_t tid;
    pthread_create(&tid, NULL, start, (void *) id);
    return tid;
}

void init(void) {
    mut = malloc(sizeof(pthread_mutex_t));
    cond = malloc(sizeof(pthread_cond_t));

    pthread_mutex_init(mut, NULL);
    pthread_cond_init(cond, NULL);
}

void cleanup(void) {
    free(mut);
    free(cond);
}

void mutexes(void) {
    printf("Enter anything to unlock:");

    pthread_mutex_lock(mut);
    pthread_t t1 = thread_easy_create(locked_thread);

    getc(stdin);

    pthread_mutex_unlock(mut);
    pthread_join(t1, NULL);
}

void condvars(void) {
    pthread_t doer1 = thread_id_create(chores_doer, 1);
    pthread_t doer2 = thread_id_create(chores_doer, 2);

    bool send_more_chores = true;
    while (send_more_chores) {
        int new_chore_count = 0;
        printf("How many chores?");
        scanf(" %d", &new_chore_count);
        printf("Sending out %d chores\n", new_chore_count);

        send_more_chores &= new_chore_count >= 0;

        pthread_mutex_lock(mut);
        chores_to_do += new_chore_count;

        if (chores_to_do > 0) {
            pthread_cond_broadcast(cond);
//            pthread_cond_signal(cond);
        }

        if (!send_more_chores) {
            pthread_cond_broadcast(cond);
        }
        pthread_mutex_unlock(mut);
    }

    pthread_join(doer1, NULL);
    pthread_join(doer2, NULL);
}

int main(int argc, char **argv) {
    init();

    int status = EXIT_SUCCESS;

    if (1 == argc) {
        printf("Missing argument operation (mut or cond)\n");
        status = EXIT_FAILURE;
    } else if (0 == strcmp(argv[1], "mut")) {
        mutexes();
    } else if (0 == strcmp(argv[1], "cond")) {
        condvars();
    }

    cleanup();

    return status;
}

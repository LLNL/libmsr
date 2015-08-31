/// Highlander... because there can be only one.

#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>

static int amHighlander=-1;

static sem_t* semk;
static sem_t* seml;

/// Determines/initializes the process highlander status
int highlander() {
    if(amHighlander != -1) {
        return amHighlander;
    }

    semk = sem_open("/power_wrapperK", O_CREAT | O_EXCL, 0600, 0);
    if(semk == NULL || semk == SEM_FAILED) {
        semk = sem_open("/power_wrapperK", O_CREAT, 0600, 0);
        seml = sem_open("/power_wrapperL", O_CREAT, 0600, 0);
        sem_post(semk);
        int v;
        sem_getvalue(semk,&v);
        amHighlander = 0;
        return 0;
    }
    seml = sem_open("/power_wrapperL", O_CREAT, 0600, 0);
    amHighlander = 1;
    return 1;
}

/// Causes the highlander to wait until all foes have called wait
int highlander_wait() {
    if(amHighlander<0) {
        return 1;
    }
    if(amHighlander) {
        int v;
        sem_getvalue(semk, &v);
        while(v>1) {
            sem_wait(seml);
        }

        sem_close(semk);
        sem_close(seml);
        sem_unlink("/power_wrapperK");
        sem_unlink("/power_wrapperL");

        return 0;
    } else {
        sem_wait(semk);
        sem_post(seml);
        sem_close(semk);
        sem_close(seml);
        return 0;
    }
    return 1;
}

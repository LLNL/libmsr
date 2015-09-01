/*! \file ghighres.c
 * \brief high resolution sleep function
 */

#include <time.h>
#include <sys/select.h>

struct mstimer {
    unsigned long startms; // when we started tracking the timer
    uint step;     // which time is the next interval
    uint interval; // how many ms between firings
    unsigned long nextms;  // when does the timer expire next
};

// Get a number of millis from realtime clock
/*
unsigned long now_rt_ms() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    unsigned long sec = t.tv_sec*1000;
    unsigned long msec = (t.tv_nsec+500000)/1000000;
    return sec+msec;
}
*/
// Get a number of millis from a monotonic clock
unsigned long now_ms() {
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    unsigned long sec = t.tv_sec*1000;
    unsigned long msec = (t.tv_nsec+500000)/1000000;
    return sec+msec;
}

// Use a select to sleep a given number of millis
void sleep_ms(long ms) {
    struct timeval i;
    i.tv_sec = ms/1000;
    i.tv_usec = (ms%1000)*1000;
    select(0, NULL, NULL, NULL, &i);
}

// Initialize a msTimer
void init_msTimer(struct mstimer* t, int ms_interval) {
    t->step=1;
    t->interval = ms_interval;
    t->startms = now_ms();
    t->nextms  = t->startms + t->step*t->interval;
}

// Initialize a msTimer
void init_sync_msTimer(struct mstimer* t, int ms_interval, unsigned long start) {
    t->step=1;
    t->interval = ms_interval;
    t->startms = start;
    t->nextms  = t->startms + t->step*t->interval;
}

// sleep until timer time has elapsed
int timer_sleep(struct mstimer* t) {
    unsigned long now = now_ms();
    if(now >= t->nextms) {
        int cadd = 0;
        while(t->nextms <= now) {
            cadd++;
            t->step++;
            t->nextms = t->startms + t->step * t->interval;
        }
        return cadd; // we slipped this many intervals
    }
    sleep_ms(t->nextms - now);
    t->step++;
    t->nextms = t->startms + t->step * t->interval;
    return 0;
}

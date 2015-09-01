#include <stdint.h>

#ifdef HAVE_LIBMSR1
#include <msr/msr_core.h>
#include <msr/msr_counters.h>
static uint64_t last_inst[NUM_THREADS];
static uint64_t last_core[NUM_THREADS];
static uint64_t last_ref[NUM_THREADS];
static struct ctr_data c0, c1, c2;

int init_data() {
    int i;
    uint64_t inst[NUM_THREADS], core[NUM_THREADS], ref[NUM_THREADS];
    for(i=0; i<NUM_THREADS; i++) {
        c0.enable[i] = c1.enable[i] = c2.enable[i] = 1;
        c0.ring_level[i] = c1.ring_level[i] = c2.ring_level[i] = 3; // usr+os
        c0.anyThread[i] = c1.anyThread[i] = c2.anyThread[i] = 1;
        c0.pmi[i] = c1.pmi[i] = c2.pmi[i] = 0;

        last_inst[i]=0;
        last_core[i]=0;
        last_ref[i]=0;
    }
    // set up the MSRs so that data is collected
    set_fixed_ctr_ctrl( &c0, &c1, &c2 );
    // perform a read to init for deltas
    read_data(inst, core, ref);
    return 0;
}

int read_data(uint64_t* inst, uint64_t* core, uint64_t* ref) {
    int i;
    get_fixed_ctr_values(&c0, &c1, &c2);
    for(i=0;i<NUM_THREADS;i++) {
        //if(inst != NULL) {
            if(last_inst[i]>c0.value[i]) {
                inst[i] = c0.value[i] + 0xFFFFFFFFFFFF - last_inst[i];
            } else {
                inst[i] = c0.value[i] - last_inst[i];
            }
            last_inst[i] = c0.value[i];
        //}
        //if(core != NULL) {
            if(last_core[i]>c1.value[i]) {
                core[i] = c1.value[i] + 0xFFFFFFFFFFFF - last_core[i];
            } else {
                core[i] = c1.value[i]-last_core[i];
            }
            last_core[i] = c1.value[i];
        //}
        //if(ref != NULL) {
            ref[i] = c2.value[i]-last_ref[i];
            last_ref[i] = c2.value[i];
        //}
    }
    return 0;
}
#else
int init_data() {
    return 0;
}
#endif

void take_measurement() {
#ifdef HAVE_LIBMSR1
    uint64_t inst[NUM_THREADS], core[NUM_THREADS], ref[NUM_THREADS];
#endif
    uint64_t instr0=0, instr1=0;
    uint64_t core0 =0, core1 =0;
    int i;
    double rapl_data[8];
    pthread_mutex_lock(&mlock);
    
    // RAPL reads
    read_rapl_energy_and_power(rapl_data);

#ifdef HAVE_LIBMSR1
    // counter reads
    read_data(inst,core,ref);
    for(i=0;i<8;i++) {
        instr0 += inst[i];
        core0  += core[i];
    }
    for(i=8;i<16;i++) {
        instr1 += inst[i];
        core1  += core[i];
    }
#endif

    total_joules += rapl_data[0] + rapl_data[1];
    limit_joules += rapl_data[2] + rapl_data[3];
    if(max_watts < rapl_data[4]) { max_watts = rapl_data[4]; }
    if(max_watts < rapl_data[5]) { max_watts = rapl_data[5]; }
    if(min_watts > rapl_data[4]) { min_watts = rapl_data[4]; }
    if(min_watts > rapl_data[5]) { min_watts = rapl_data[5]; }
    fprintf(logfile, "time:%ld 0joules:%lf 1joules:%lf 0limwatts:%lf 1limwatts:%lf instr0:%lu instr1:%lu core0:%lu core1:%lu\n",now_ms(),rapl_data[0],rapl_data[1],rapl_data[6],rapl_data[7],instr0,instr1,core0,core1);
    pthread_mutex_unlock(&mlock);
}

void* power_measurement(void* arg) {
    struct mstimer timer;
    // according to the Intel docs, the counter wraps a most once per second
    // 100 ms should be short enough to always get good information
    init_msTimer(&timer, 100);
    init_data();
    read_rapl_init();
    start = now_ms();

    timer_sleep(&timer);
    while(running) {
        take_measurement();
        timer_sleep(&timer);
    }
}

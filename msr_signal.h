/* msr_signal.h
 *
 * T Patki
 * Signal Handling Functionality
 */
#include <stdint.h>
#include <signal.h>
#ifndef MSR_SIGNAL_H
#define MSR_SIGNAL_H

void handle_sig(int signum);
void register_sig();
void restore_defaults();
#endif  // MSR_SIGNAL_H



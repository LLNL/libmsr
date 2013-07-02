/* msrThermal.h
 *
 * 
*/
#ifndef MSR_THERMAL_H
#define MSR_THERMAL_H
#include <stdio.h>

struct msr_temp_target{
	uint64_t raw;
	uint64_t temp_target;	//Read only (probably the TCC Activation Temp)
};

struct misc_enable{
	uint64_t raw;
	int fast_string_enable;			// Read/Write (thread scope)	
	int auto_TCC_enable;				// Read/Write 
	int performance_monitoring;			// Read	(thread scope)
	int branch_trace_storage_unavail;		// Read only (thread scope)
	int precise_event_based_sampling_unavail;	// Read Only (thread scope)
	int TM2_enable;					// Read/Write 
	int enhanced_Intel_SpeedStep_Tech_enable;	// Read/Write (Package scope)
	int enable_monitor_fsm;				// Read/Write (thread scope)
	int limit_CPUID_maxval;				// Read/Write (thread scope)
	int xTPR_message_disable;			// Read/Write (thread scope)
	int XD_bit_disable;				// Read/Write (thread scope)
	int turbo_mode_disable;				// Read/Write (package scope)
};

struct clock_mod{
	uint64_t raw;

// There is a bit at 0 that can be used for Extended On-Demand Clock Modulation Duty Cycle
// It is added with the bits 3:1. When used, the granularity of clock modulation duty cycle
// is increased to 6.25% as opposed to 12.5%
// To enable this, must have CPUID.06H:EAX[Bit 5] = 1
// I am not sure how to check that because otherwise bit 0 is reserved

	int duty_cycle;		// 3 binary digits
				// 0-7 in decimal
				//
				// Value	Duty Cycle
				//   0		 Reserved
				//   1		 12.5% (default)
				//   2		 25.0%
				//   3		 37.5%
				//   4		 50.0%
				//   5		 63.5%
				//   6		 75.0%
				//   7		 87.5%

	int duty_cycle_enable;	// Read/Write
};

struct therm_stat{
	uint64_t raw;
	int status;			//Read only

	int status_log;			//Read and Write (Sticky bit)  
					//Cleared by software to 0 or RESET

	int PROCHOT_or_FORCEPR_event;	//Read only

	int PROCHOT_or_FORCEPR_log;	//Read and Write

	int crit_temp_status;		//Read only

	int crit_temp_log;		//Read and Write
					//Sticky bit

	int therm_thresh1_status;	//Read only

	int therm_thresh1_log;		//Read and Write 
					//Sticky bit

	int therm_thresh2_status;	//Read only

	int therm_thresh2_log;		//Read and Write
					//Sticky bit

	int power_limit_status;		//Read only

	int power_notification_log;	//Read and Write
					//Sticky bit

	int readout;			//Read only

	int resolution_deg_celsius;	//Read only

	int readout_valid;		//Read only
};

struct therm_interrupt{
	uint64_t raw;
	//Below all read and write
	int high_temp_enable;	
	int low_temp_enable;
	int PROCHOT_enable;
	int FORCEPR_enable;
	int crit_temp_enable;
	int thresh1_val;
	int thresh1_enable;
	int thresh2_val;
	int thresh2_enable;
	int pwr_limit_notification_enable;
};

struct pkg_therm_stat{
	uint64_t raw;
	int status;		//Read only

	int status_log;		//Read or Write Sticky bit (default: clear) 
				//Cleared by software to 0 or reset

	int PROCHOT_event;	//Read only

	int PROCHOT_log;	//Read or Write
				//Sticky bit

	int crit_temp_status;	//Read only
	int crit_temp_log;	//Read or write
				//Sticky bit

	int therm_thresh1_status;	//Read only
	int therm_thresh1_log;		//Read or write
					//Sticky bit

	int therm_thresh2_status;	//Read only
	int therm_thresh2_log;		//Read or write
					//Sticky bit

	int power_limit_status;		//Read only
	int power_notification_log;	//Read or write
					//Sticky bit

	int readout;			//Read only
};

struct pkg_therm_interrupt{
	uint64_t raw;
	//All read and write below
	int high_temp_enable;	
	int low_temp_enable;	
	int PROCHOT_enable;	
	int crit_temp_enable;	
	int thresh1_val;	
	int thresh1_enable;	
	int thresh2_val;	
	int thresh2_enable;
	int pwr_limit_notification_enable;
};

void dump_msr_temp_target(struct msr_temp_target *s);
void get_msr_temp_target(int socket, int core, struct msr_temp_target *s);
void dump_misc_enable(struct misc_enable *s);
void get_misc_enable(int package, struct misc_enable *s);
void set_misc_enable(int package, struct misc_enable *s);
void dump_clock_mod(struct clock_mod *s);
void get_clock_mod(int socket, int core, struct clock_mod *s);
void set_clock_mod(int socket, int core, struct clock_mod *s);
void dump_therm_stat(int socket, int core, struct therm_stat * s);
void dump_therm_interrupt(int socket, int core, struct therm_interrupt *s);
void dump_pkg_therm_stat(int package, struct pkg_therm_stat * s);
void dump_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);
void get_therm_stat(int socket, int core, struct therm_stat *s);
void get_therm_interrupt(int socket, int core, struct therm_interrupt *s);
void get_pkg_therm_stat(int package, struct pkg_therm_stat *s);
void set_therm_stat(int socket, int core, struct therm_stat *s);
void set_therm_interrupt(int socket, int core, struct therm_interrupt *s);
void set_pkg_therm_stat(int package, struct pkg_therm_stat *s);
void get_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);
void set_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);
void Human_Interface_set_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);

void dump_core_temp(int socket, int core, struct therm_stat *s);

#endif

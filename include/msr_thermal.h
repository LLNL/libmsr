/* msr_thermal.h
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
void get_temp_target(int socket, struct msr_temp_target *s);
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

void dump_core_temp(int socket, int core, struct therm_stat *s);
void dump_thermal_terse_label();
void dump_thermal_terse();

#endif

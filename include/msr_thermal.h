/* msr_thermal.h
 *
 * Low-level msr interface.
 *
 * Copyright (c) 2013-2015, Lawrence Livermore National Security, LLC.  
 * Produced at the Lawrence Livermore National Laboratory  
 * Written by Barry Rountree, rountree@llnl.gov
 *            Scott Walker,   walker91@llnl.gov
 *            Kathleen Shoga, shoga1@llnl.gov
 *
 * All rights reserved. 
 * 
 * This file is part of libmsr.
 * 
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 * 
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along
 * with libmsr.  If not, see <http://www.gnu.org/licenses/>. 
 */

#ifndef MSR_THERMAL_H
#define MSR_THERMAL_H
#include <stdio.h>

// TODO: used num_sockets
struct msr_temp_target{				//Scope is "unique" for Sandy Bridge
						//Assuming that it is by socket
	uint64_t ** raw;
	uint64_t * temp_target;	//Read only (probably the TCC Activation Temp)
};

// TODO: used num_cores
struct therm_stat{			//Scope is "core" for Sandy Bridge
	uint64_t ** raw;
	int * status;				//Read only

	int * status_log;			//Read and Write (Sticky bit)
							//Cleared by software to 0 or RESET

	int * PROCHOT_or_FORCEPR_event;	//Read only

	int * PROCHOT_or_FORCEPR_log;		//Read and Write

	int * crit_temp_status;		//Read only

	int * crit_temp_log;			//Read and Write
							//Sticky bit

	int * therm_thresh1_status;		//Read only

	int * therm_thresh1_log;		//Read and Write
							//Sticky bit

	int * therm_thresh2_status;		//Read only

	int * therm_thresh2_log;		//Read and Write
							//Sticky bit

	int * power_limit_status;		//Read only

	int * power_notification_log;		//Read and Write
							//Sticky bit

	int * readout;				//Read only

	int * resolution_deg_celsius;		//Read only

	int * readout_valid;			//Read only
};

// TODO: used num_cores
struct therm_interrupt{			//Core scope
	uint64_t ** raw;
	//Below all read and write
	int * high_temp_enable;
	int * low_temp_enable;
	int * PROCHOT_enable;
	int * FORCEPR_enable;
	int * crit_temp_enable;
	int * thresh1_val;
	int * thresh1_enable;
	int * thresh2_val;
	int * thresh2_enable;
	int * pwr_limit_notification_enable;
};

// TODO: used num_sockets
struct pkg_therm_stat{			//Package (socket) scope
	uint64_t ** raw;
	int * status;		//Read only

	int * status_log;		//Read or Write Sticky bit (default: clear)
						//Cleared by software to 0 or reset

	int * PROCHOT_event;		//Read only

	int * PROCHOT_log;		//Read or Write
						//Sticky bit

	int * crit_temp_status;	//Read only
	int * crit_temp_log;		//Read or write
						//Sticky bit

	int * therm_thresh1_status;	//Read only
	int * therm_thresh1_log;	//Read or write
						//Sticky bit

	int * therm_thresh2_status;	//Read only
	int * therm_thresh2_log;	//Read or write
						//Sticky bit

	int * power_limit_status;	//Read only
	int * power_notification_log; //Read or write
						//Sticky bit

	int * readout;		//Read only
};

// TODO: used num_sockets
struct pkg_therm_interrupt{			//Package(socket) scope
	uint64_t ** raw;
	//All read and write below
	int * high_temp_enable;
	int * low_temp_enable;
	int * PROCHOT_enable;
	int * crit_temp_enable;
	int * thresh1_val;
	int * thresh1_enable;
	int * thresh2_val;
	int * thresh2_enable;
	int * pwr_limit_notification_enable;
};

#ifdef __cplusplus
extern "C" {
#endif

void dump_msr_temp_target();
void get_temp_target(struct msr_temp_target *s);

void dump_therm_stat(struct therm_stat * s);
void dump_therm_interrupt(struct therm_interrupt *s);
void dump_pkg_therm_stat(struct pkg_therm_stat * s);
void dump_pkg_therm_interrupt(struct pkg_therm_interrupt *s);

void get_therm_stat(struct therm_stat *s);
void get_therm_interrupt(struct therm_interrupt *s);
void get_pkg_therm_stat(struct pkg_therm_stat *s);
void get_pkg_therm_interrupt(struct pkg_therm_interrupt *s);

void set_therm_stat(struct therm_stat *s);
void set_therm_interrupt(struct therm_interrupt *s);
void set_pkg_therm_stat(struct pkg_therm_stat *s);
void set_pkg_therm_interrupt(struct pkg_therm_interrupt *s);

void dump_thermal_terse_label(FILE *w);
void dump_thermal_terse(FILE *w);
void dump_thermal_verbose_label(FILE *w);
void dump_thermal_verbose(FILE *w);

#ifdef __cplusplus
}
#endif

#endif // MSR_THERMAL_H

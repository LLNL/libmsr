/* msr_thermal.h
 *
 * Copyright (c) 2011, 2012, 2013, 2014 by Lawrence Livermore National Security, LLC. LLNL-CODE-645430 
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by Kathleen Shoga and Barry Rountree (shoga1|rountree@llnl.gov).
 * All rights reserved.
 *
 * This file is part of libmsr.
 *
 * libmsr is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation, either 
 * version 3 of the License, or (at your option) any
 * later version.
 *
 * libmsr is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along
 * with libmsr. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department
 * of Energy's Lawrence Livermore National Laboratory. Office of 
 * Science, under Award number DE-AC52-07NA27344.
 *
 */

#ifndef MSR_THERMAL_H
#define MSR_THERMAL_H
#include <stdio.h>

struct msr_temp_target{				//Scope is "unique" for Sandy Bridge
						//Assuming that it is by socket
	uint64_t raw[NUM_SOCKETS];
	uint64_t temp_target[NUM_SOCKETS];	//Read only (probably the TCC Activation Temp)
};


struct therm_stat{			//Scope is "core" for Sandy Bridge
	uint64_t raw[NUM_CORES];
	int status[NUM_CORES];				//Read only

	int status_log[NUM_CORES];			//Read and Write (Sticky bit)  
							//Cleared by software to 0 or RESET

	int PROCHOT_or_FORCEPR_event[NUM_CORES];	//Read only

	int PROCHOT_or_FORCEPR_log[NUM_CORES];		//Read and Write

	int crit_temp_status[NUM_CORES];		//Read only

	int crit_temp_log[NUM_CORES];			//Read and Write
							//Sticky bit

	int therm_thresh1_status[NUM_CORES];		//Read only

	int therm_thresh1_log[NUM_CORES];		//Read and Write 
							//Sticky bit

	int therm_thresh2_status[NUM_CORES];		//Read only

	int therm_thresh2_log[NUM_CORES];		//Read and Write
							//Sticky bit

	int power_limit_status[NUM_CORES];		//Read only

	int power_notification_log[NUM_CORES];		//Read and Write
							//Sticky bit

	int readout[NUM_CORES];				//Read only

	int resolution_deg_celsius[NUM_CORES];		//Read only

	int readout_valid[NUM_CORES];			//Read only
};

struct therm_interrupt{			//Core scope
	uint64_t raw[NUM_CORES];
	//Below all read and write
	int high_temp_enable[NUM_CORES];	
	int low_temp_enable[NUM_CORES];
	int PROCHOT_enable[NUM_CORES];
	int FORCEPR_enable[NUM_CORES];
	int crit_temp_enable[NUM_CORES];
	int thresh1_val[NUM_CORES];
	int thresh1_enable[NUM_CORES];
	int thresh2_val[NUM_CORES];
	int thresh2_enable[NUM_CORES];
	int pwr_limit_notification_enable[NUM_CORES];
};

struct pkg_therm_stat{			//Package (socket) scope
	uint64_t raw[NUM_SOCKETS];
	int status[NUM_SOCKETS];		//Read only

	int status_log[NUM_SOCKETS];		//Read or Write Sticky bit (default: clear) 
						//Cleared by software to 0 or reset

	int PROCHOT_event[NUM_SOCKETS];		//Read only

	int PROCHOT_log[NUM_SOCKETS];		//Read or Write
						//Sticky bit

	int crit_temp_status[NUM_SOCKETS];	//Read only
	int crit_temp_log[NUM_SOCKETS];		//Read or write
						//Sticky bit

	int therm_thresh1_status[NUM_SOCKETS];	//Read only
	int therm_thresh1_log[NUM_SOCKETS];	//Read or write
						//Sticky bit

	int therm_thresh2_status[NUM_SOCKETS];	//Read only
	int therm_thresh2_log[NUM_SOCKETS];	//Read or write
						//Sticky bit

	int power_limit_status[NUM_SOCKETS];	//Read only
	int power_notification_log[NUM_SOCKETS];//Read or write
						//Sticky bit

	int readout[NUM_SOCKETS];		//Read only
};

struct pkg_therm_interrupt{			//Package(socket) scope
	uint64_t raw[NUM_SOCKETS];
	//All read and write below
	int high_temp_enable[NUM_SOCKETS];	
	int low_temp_enable[NUM_SOCKETS];	
	int PROCHOT_enable[NUM_SOCKETS];	
	int crit_temp_enable[NUM_SOCKETS];	
	int thresh1_val[NUM_SOCKETS];	
	int thresh1_enable[NUM_SOCKETS];	
	int thresh2_val[NUM_SOCKETS];	
	int thresh2_enable[NUM_SOCKETS];
	int pwr_limit_notification_enable[NUM_SOCKETS];
};

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

#endif

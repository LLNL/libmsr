/* msrThermal.h
 *
 * 
*/

#include <stdio.h>

void dump_msr_temp_target(struct msr_temp_target *s);
void get_msr_temp_target(int socket, int core, struct msr_temp_target *s);
void dump_misc_enable(struct misc_enable *s);
void get_misc_enable(int package, struct misc_enable *s);
void set_misc_enable(int package, struct misc_enable *s);
void dump_clock_mod(struct clock_mod *s);
void get_clock_mod(int socket, int core, struct clock_mod *s);
void set_clock_mod(int socket, int core, struct clock_mod *s);
void dump_therm_stat(struct therm_stat * s);
void dump_therm_interrupt(struct therm_interrupt *s);
void dump_pkg_therm_stat(struct pkg_therm_stat * s);
void dump_pkg_therm_interrupt(struct pkg_therm_interrupt *s);
void get_therm_stat(int socket, int core, struct therm_stat *s);
void get_therm_interrupt(int socket, int core, struct therm_interrupt *s);
void get_pkg_therm_stat(int package, struct pkg_therm_stat *s);
void set_therm_stat(int socket, int core, struct therm_stat *s);
void set_therm_interrupt(int socket, int core, struct therm_interrupt *s);
void set_pkg_therm_stat(int package, struct pkg_therm_stat *s);
void get_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);
void set_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);
void Human_Interface_set_pkg_therm_interrupt(int package, struct pkg_therm_interrupt *s);




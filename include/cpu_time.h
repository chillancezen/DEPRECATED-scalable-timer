#ifndef _CPU_TIME
#define _CPU_TIME
#include <stdint.h>
#include <inttypes.h>

#define CPU_FREQUENCY_HZ 2800072000
#define CYCLES_PER_SECOND CPU_FREQUENCY_HZ
#define CYCLES_PER_MICRO_SECOND ((CYCLES_PER_SECOND)/1000)

uint64_t timer_read_tsc(void);

#define cycle_to_secs(cycles)	((cycles)/CYCLES_PER_SECOND) 
#define cycle_to_micro_secs(cycles) ((cycles)/CYCLES_PER_MICRO_SECOND) 


#define secs_to_cycles(sec) ((sec)*CYCLES_PER_SECOND)
#define micro_secs_to_cycles(msecs) ((msecs)*CYCLES_PER_MICRO_SECOND)

#endif

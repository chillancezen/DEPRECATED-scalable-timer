#include <cpu_time.h>


/*imported from DPDK eal*/
uint64_t timer_read_tsc(void)
{
        union {
                uint64_t tsc_64;
                struct {
                        uint32_t lo_32;
                        uint32_t hi_32;
                };
        } tsc;
		
        asm volatile("rdtsc" :
                     "=a" (tsc.lo_32),
                     "=d" (tsc.hi_32));
        return tsc.tsc_64;
}



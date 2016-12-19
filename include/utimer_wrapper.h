
#ifndef _UTIMER_WRAPPER
#define _UTIMER_WRAPPER
#include <ustack_timer.h>
#include <signal.h>   
#include <time.h> 
#include <unistd.h> 

#define LINUX_BASE_TIEMR_MSEC 100 /*100 mili-seconds*/

struct timer_wrapper{
	int is_running;
	timer_t linux_base_timer;
	struct u_timer_stub * stub;
};
int initialize_timer_wrapper(struct timer_wrapper* wrapper);
int uninitialize_timer_wrapper(struct timer_wrapper *wrapper);
#endif
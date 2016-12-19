#ifndef _USTACK_TIMER
#define _USTACK_TIMER
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <cpu_time.h>
#include <assert.h>
#define TIMER_SCHEDULE_FLAG_ONCE 0x0
#define TIMER_SCHEDULE_FLAG_LOOP 0x1

#define MAX_USTIMER_PER_STUB 1024

struct u_timer{
	struct u_timer_stub *stub;/*reference to stub struct*/
	
	uint64_t init_time;
	uint64_t iterate_time;
	uint64_t next_expiry_time;
	void *arg;
	int (*handler)(void *arg);
	int (*delete)(struct u_timer *timer);/*optional*/
	int is_to_del;/*useful for delayed deletion*/
	uint64_t schedul_flag:1;
	int is_deleted;
};

struct u_timer_stub{
	pthread_spinlock_t guard;
	int nr_timers;
	struct u_timer * timer_array[MAX_USTIMER_PER_STUB];/*this is a min heap*/
};

#define alloc_u_timer() ({\
	struct u_timer *timer=malloc(sizeof(struct u_timer));\
	if(timer) \
		memset(timer,0x0,sizeof(struct u_timer)); \
	timer;\
})

#define alloc_u_timer_stub() ({\
	struct u_timer_stub *stub=malloc(sizeof(struct u_timer_stub));\
	if(stub) {\
		memset(stub,0x0,sizeof(struct u_timer_stub)); \
		pthread_spin_init(&stub->guard,0);\
	}\
	stub;\
})
int initialize_timer(struct u_timer* timer,
		int run_once,
		int initial_msec,
		int interval_msec,
		int (*handler)(void *),
		int (*delete)(struct u_timer *),
		void *arg);

int install_timer(struct u_timer_stub *stub,struct u_timer *timer);
struct u_timer* peek_timer_at_heap_top(struct u_timer_stub *stub);
struct u_timer* fetch_timer_at_heap_top(struct u_timer_stub *stub);
int  uninstall_timer(struct u_timer *timer);
int uninstall_timer_sync(struct u_timer_stub *stub,struct u_timer* timer);


#define install_timer_mt(stub,timer) ({\
	int rc; \
	assert(0==pthread_spin_lock(&(stub)->guard)); \
	rc=install_timer((stub),(timer)); \
	assert(0==pthread_spin_unlock(&(stub)->guard)); \
	rc;\
})

#define uninstall_timer_mt(stub,timer) ({\
	int rc; \
	assert(0==pthread_spin_lock(&(stub)->guard)); \
	rc=uninstall_timer((timer)); \
	assert(0==pthread_spin_unlock(&(stub)->guard)); \
	rc;\
}) 

#define uninstall_timer_sync_mt(stub,timer) ({\
	int rc; \
	assert(0==pthread_spin_lock(&(stub)->guard)); \
	rc=uninstall_timer_sync((stub),(timer)); \
	assert(0==pthread_spin_unlock(&(stub)->guard)); \
	rc;\
}) 

#define peek_timer_at_heap_top_mt(stub) ({\
	struct u_timer *timer;\
	assert(0==pthread_spin_lock(&(stub)->guard)); \
	timer=peek_timer_at_heap_top((stub));\
	assert(0==pthread_spin_unlock(&(stub)->guard)); \
	timer;\
})

#define fetch_timer_at_heap_top_mt(stub) ({\
	struct u_timer *timer;\
	assert(0==pthread_spin_lock(&(stub)->guard)); \
	timer=fetch_timer_at_heap_top((stub));\
	assert(0==pthread_spin_unlock(&(stub)->guard)); \
	timer;\
})
#endif

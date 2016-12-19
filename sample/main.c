#include <stdio.h>
#include <cpu_time.h>
#include <ustack_timer.h>
#include <utimer_wrapper.h>

#define swap_two_node(stub,node1,node2) {\
	struct u_timer *timer_tmp;\
	timer_tmp=(stub)->timer_array[(node1)];\
	(stub)->timer_array[(node1)]=(stub)->timer_array[(node2)];\
	(stub)->timer_array[(node2)]=timer_tmp;\
}

int demo_handler(void * arg)
{
	printf("%d\n:",(int)(uint64_t)arg);
	return 0;
}
int demo_delete(struct u_timer* timer)
{

	free(timer);

	return 0;
}
int main()
{
	#if 0
	uint64_t start=timer_read_tsc();

	sleep(2);
	uint64_t end=timer_read_tsc();

	uint64_t diff=end-start;
	
	printf("%"PRIu64"\n",cycle_to_micro_secs(diff));
	
	struct u_timer * timer;
	struct u_timer_stub * stub=alloc_u_timer_stub();

	timer=alloc_u_timer();
	timer->next_expiry_time=21;
	install_timer(stub,timer);

	timer=alloc_u_timer();
	timer->next_expiry_time=232;
	install_timer(stub,timer);

	timer=alloc_u_timer();
	timer->next_expiry_time=3;
	install_timer(stub,timer);

	timer=alloc_u_timer();
	timer->next_expiry_time=16;
	install_timer(stub,timer);

	timer=alloc_u_timer();
	timer->next_expiry_time=9;
	install_timer(stub,timer);
		

	timer=alloc_u_timer();
	timer->next_expiry_time=22;
	install_timer(stub,timer);

	timer=alloc_u_timer();
	timer->next_expiry_time=1;
	install_timer_mt(stub,timer);

	
	uninstall_timer_sync_mt(stub,timer);
	
	
	while(timer=fetch_timer_at_heap_top_mt(stub)){
		printf("%d\n",timer->next_expiry_time);
	}
	#endif

	#if 0
	int rc;
	struct timer_wrapper wrapper;
	initialize_timer_wrapper(&wrapper);

	struct u_timer * timer=alloc_u_timer();
	initialize_timer(timer,0,2000,1000,demo_handler,NULL,timer);

	rc=install_timer_mt(wrapper.stub,timer);
	
	getchar();
	rc=uninstall_timer_sync_mt(timer->stub,timer);
	printf("%d\n",rc);
	getchar();
	#endif
	int unsucessful_cnt=0;
	int idx=0;
	int rc;
	#define MAX_WRAPPER 255
	#define TEST_TIMER_CNT (1024*4)
	int wrapper_index=0;
	
	struct timer_wrapper wrapper[MAX_WRAPPER];
	struct u_timer *timer;
	memset(wrapper,0x0,sizeof(wrapper));
	for(idx=0;idx<MAX_WRAPPER;idx++){
		rc=initialize_timer_wrapper(&wrapper[idx]);
		if(rc)
			printf("init wrapper:%d failure\n",idx);
	}
	for(idx=0;idx<TEST_TIMER_CNT;idx++){
		timer=alloc_u_timer();
		initialize_timer(timer,0,5000,1000+idx%10,demo_handler,demo_delete,(void*)(uint64_t)idx);
		
		rc=install_timer_mt(wrapper[wrapper_index].stub,timer);
		if(rc){
			wrapper_index++;
			rc=install_timer_mt(wrapper[wrapper_index].stub,timer);
			
			unsucessful_cnt++;
			
		}
	}
	printf("%d\n",unsucessful_cnt);
	getchar();
	return 0;
}

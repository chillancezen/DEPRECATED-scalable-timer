#include <utimer_wrapper.h>
#include <string.h>

static void timer_wrapper_handler(union sigval v)
{
	struct timer_wrapper *wrapper=v.sival_ptr;
	struct u_timer * timer_tmp;
	int idx=0;
	int rc;
	uint64_t curent_tsc;
	int nr_loop=wrapper->stub->nr_timers;
	#if 0
	for(idx=0;idx<nr_loop;idx++){
		pthread_spin_lock(&wrapper->stub->guard);
		timer_tmp=fetch_timer_at_heap_top(wrapper->stub);
		if(!timer_tmp)/*there is no pending timer in the list*/
			goto term_flag;
		
		curent_tsc=timer_read_tsc();
		if(timer_tmp->next_expiry_time>curent_tsc)/*not one timer expired*/
			goto term_flag;
		
		if(timer_tmp->is_to_del){
			if(timer_tmp->delete)
				timer_tmp->delete(timer_tmp);
			goto norm_loop;
		}
		
		/*execute registered handler  */
		timer_tmp->handler(timer_tmp->arg);
		switch(timer_tmp->schedul_flag)
		{
			case TIMER_SCHEDULE_FLAG_ONCE:
				if(timer_tmp->delete)
					timer_tmp->delete(timer_tmp);
				break;
			case TIMER_SCHEDULE_FLAG_LOOP:
				timer_tmp->next_expiry_time=timer_read_tsc()+timer_tmp->iterate_time;
				rc=install_timer(wrapper->stub,timer_tmp);
				if(rc){
					printf("install_timer fails");
					exit(0);
				}
				break;
		}
		norm_loop:
		pthread_spin_unlock(&wrapper->stub->guard);
		continue;
		
		term_flag:
		pthread_spin_unlock(&wrapper->stub->guard);
		break;
	}
	
	#else
	for(idx=0;idx<nr_loop;idx++){
		timer_tmp=peek_timer_at_heap_top_mt(wrapper->stub);
		if(!timer_tmp)/*there is pending timer in the list*/
			break;
		curent_tsc=timer_read_tsc();
		if(timer_tmp->next_expiry_time>curent_tsc)/*not one timer expired*/
			break;

		if(timer_tmp->is_to_del){
			timer_tmp=fetch_timer_at_heap_top_mt(wrapper->stub);
			
			if(timer_tmp->delete)
				timer_tmp->delete(timer_tmp);/*if possible ,timer_tmp->is_deleted=1; is executed in delete callback*/
			else 
				timer_tmp->is_deleted=1;/*also ,it could be set here*/
			
			continue;
		}
		/*execute registered handler  */
		timer_tmp->handler(timer_tmp->arg);
		
		switch(timer_tmp->schedul_flag)
		{
			case TIMER_SCHEDULE_FLAG_ONCE:
				uninstall_timer_mt(wrapper->stub,timer_tmp);
				break;
				#if 0
				timer_tmp=fetch_timer_at_heap_top_mt(wrapper->stub);
				if(timer_tmp->delete)
					timer_tmp->delete(timer_tmp);
				break;
				#endif
			case TIMER_SCHEDULE_FLAG_LOOP:
				pthread_spin_lock(&wrapper->stub->guard);
				timer_tmp=fetch_timer_at_heap_top(wrapper->stub);
				timer_tmp->next_expiry_time=timer_read_tsc()+timer_tmp->iterate_time;
				install_timer(wrapper->stub,timer_tmp);
				pthread_spin_unlock(&wrapper->stub->guard);
				break;
		}
	}
	#endif
}

int initialize_timer_wrapper(struct timer_wrapper* wrapper)
{
	int ret;
	struct sigevent evp;
	struct itimerspec ts;
	wrapper->stub=alloc_u_timer_stub();
	wrapper->is_running=0;
	memset(&evp, 0, sizeof (evp));
	evp.sigev_value.sival_ptr=(void*)wrapper;    
	evp.sigev_notify = SIGEV_THREAD;    
	evp.sigev_notify_function = timer_wrapper_handler;
	ret = timer_create(CLOCK_REALTIME, &evp, &wrapper->linux_base_timer);
	if(ret)
		return ret;
	ts.it_interval.tv_sec =LINUX_BASE_TIEMR_MSEC/1000;      
	ts.it_interval.tv_nsec =LINUX_BASE_TIEMR_MSEC%1000;      
	ts.it_value.tv_sec =LINUX_BASE_TIEMR_MSEC/1000;      
	ts.it_value.tv_nsec =LINUX_BASE_TIEMR_MSEC%1000;
	ret = timer_settime(wrapper->linux_base_timer, TIMER_ABSTIME, &ts, NULL);   
	if(ret)        
		return ret;
	wrapper->is_running=1;
	return 0;
}
int uninitialize_timer_wrapper(struct timer_wrapper *wrapper)
{
	if(wrapper->is_running)
		timer_delete(wrapper->linux_base_timer);
	wrapper->is_running=0;
	if(wrapper->stub)
		free(wrapper->stub);
	wrapper->stub=NULL;
	return 0;
}

/*use api offer by ustack_timer.c to bind or unbind a timer to or from wrapper*/




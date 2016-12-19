#include <ustack_timer.h>

#define parent_of_node(node)  (((node)-1)/2)
#define left_child_of_node(node) ((node)*2+1)
#define right_child_of_node(node) ((node)*2+2)



#define swap_two_node(stub,node1,node2) {\
	struct u_timer *timer_tmp;\
	timer_tmp=(stub)->timer_array[(node1)];\
	(stub)->timer_array[(node1)]=(stub)->timer_array[(node2)];\
	(stub)->timer_array[(node2)]=timer_tmp;\
}

int initialize_timer(struct u_timer* timer,
	int run_once,
	int initial_msec,
	int interval_msec,
	int (*handler)(void *),
	int (*delete)(struct u_timer *),
	void *arg)
{
	memset(timer,0x0,sizeof(struct u_timer));
	timer->arg=arg;
	timer->handler=handler;
	timer->delete=delete;
	timer->is_to_del=0;
	timer->schedul_flag=run_once?TIMER_SCHEDULE_FLAG_ONCE:TIMER_SCHEDULE_FLAG_LOOP;
	timer->init_time=micro_secs_to_cycles(initial_msec);
	timer->iterate_time=micro_secs_to_cycles(interval_msec);
	timer->next_expiry_time=timer_read_tsc()+timer->init_time;
	return 0;
}
static int _adjust_node_once(struct u_timer_stub *stub,int node_index,int *is_swaped,int *swaped_node)
/*swaped_node is -1 indicate no nodes swaped*/
{
	int node_left_child=left_child_of_node(node_index);
	int node_right_child=right_child_of_node(node_index);
	int target_node;
	struct u_timer *parent;
	struct u_timer *left_child;
	struct u_timer *right_child;
	struct u_timer *target_child;
	
	if((node_index>=stub->nr_timers)||(node_left_child>=stub->nr_timers))/*check whether it's a leaf node already*/
		return -1;
	
	*is_swaped=0;
	parent=stub->timer_array[node_index];
	
	if(node_right_child>=stub->nr_timers){/*there is no right child node*/
		
		left_child=stub->timer_array[node_left_child];
		if(parent->next_expiry_time>left_child->next_expiry_time){
			swap_two_node(stub,node_index,node_left_child);
			*is_swaped=1;
			*swaped_node=node_left_child;
		}
	}else{
		left_child=stub->timer_array[node_left_child];
		right_child=stub->timer_array[node_right_child];
		target_node=(left_child->next_expiry_time<right_child->next_expiry_time)?
			node_left_child:
			node_right_child;
		target_child=stub->timer_array[target_node];
		if(parent->next_expiry_time>target_child->next_expiry_time){
			swap_two_node(stub,node_index,target_node);
			*is_swaped=1;
			*swaped_node=target_node;
		}
	}
	return 0;
}
static int _adjust_node_branch(struct u_timer_stub*stub,int node_index)
{
	int rc;
	int target_node=node_index;
	int is_swaped;
	int swaped_node;
	do{
		rc=_adjust_node_once(stub,target_node,&is_swaped,&swaped_node);
		if(rc)/*already leaf node*/
			break;
		if(!is_swaped)
			break;/*already sorted*/
		target_node=swaped_node;
	}while(target_node<stub->nr_timers);
	return 0;
}
static int _adjust_node_tree(struct u_timer_stub *stub)
{
	if(stub->nr_timers<=1)/*empty tree or only one node in it*/
		return 0;
	int target_node=stub->nr_timers-1;
	for(target_node=parent_of_node(target_node);target_node>=0;target_node--)
		_adjust_node_branch(stub,target_node);
	return 0;
}
int install_timer(struct u_timer_stub *stub,struct u_timer *timer)
{
	int idx;
	int target_node;
	int is_swapped;
	int swaped_node;
	int rc;
	timer->is_deleted=0;
	timer->stub=NULL;
	if(stub->nr_timers==MAX_USTIMER_PER_STUB)
		return -1;
	for(idx=0;idx<stub->nr_timers;idx++)
		if(stub->timer_array[idx]==timer)
			return -2;
	/*1.insert timer into last available slot*/	
	timer->stub=stub;
	stub->timer_array[stub->nr_timers]=timer;
	stub->nr_timers++;
	
	/*2.adjust the whole heap to sorted state*/
	target_node=stub->nr_timers-1;
	if(target_node==0)/*root node already*/
		return 0;
	
	do{
		target_node=parent_of_node(target_node);
		rc=_adjust_node_once(stub,target_node,&is_swapped,&swaped_node);
		if(!rc&&!is_swapped)
			break;
	}while(target_node);
	return 0;
}
struct u_timer* peek_timer_at_heap_top(struct u_timer_stub *stub)
{
	if(!stub->nr_timers)
		return NULL;
	return stub->timer_array[0];
}

struct u_timer* fetch_timer_at_heap_top(struct u_timer_stub *stub)
{
	struct u_timer* timer=NULL;
	if(!stub->nr_timers)
		return NULL;
	timer=stub->timer_array[0];

	stub->timer_array[0]=stub->timer_array[stub->nr_timers-1];
	stub->nr_timers--;
	_adjust_node_branch(stub,0);
	return timer;

}
int  uninstall_timer(struct u_timer *timer)
{
	timer->is_to_del=1;
	return 0;
}
int uninstall_timer_sync(struct u_timer_stub *stub,struct u_timer* timer)
{
	int target_node=-1;
	int idx=0;
	for(idx=0;idx<stub->nr_timers;idx++)
		if(stub->timer_array[idx]==timer){
			target_node=idx;
			break;
		}
	if(target_node==-1)/*timer not found in registery*/
		return -1;
	
	stub->timer_array[target_node]=stub->timer_array[stub->nr_timers-1];
	stub->nr_timers--;
	_adjust_node_tree(stub);
	if(timer->delete)
		timer->delete(timer);
	return 0;
}

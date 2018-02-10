/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
 
 
unsigned long ticks_cpu;
#define DEFAULT_QUANTUM 300
struct list_head freequeue;
struct list_head readyqueue;

void init_stats(struct task_struct *t) {
	t->st.user_ticks = 0;
	t->st.system_ticks = 0;
	t->st.blocked_ticks = 0; 
	t->st.ready_ticks = 0; 
	t->st.elapsed_total_ticks = get_ticks();
	t->st.total_trans = 0; 
	t->st.remaining_ticks = get_ticks();

}

union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task"))); 
  
  //nuestro task es de 12. el primero y
  // el ultimo no tiene permiso de escritura nuesto vector es de 10 posiciones
union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

//#if 0  //el compilador pasa de esto
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  //return list_entry( l, struct task_struct, list);
  return (struct task_struct*)((unsigned long)l&0xfffff000);
  //devolver task_struct de l, alineado a 4KB= 4096Bytes ---> 0x1000
}
//#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	printk("\nSoy proceso Idle\n");
	while(1)
	{
	
	}
}

struct task_struct* idle_task; 
struct task_struct* init_task;
void init_idle (void)
{
	/*Get an available task*/
	idle_task = list_head_to_task_struct(list_first(&freequeue));
	list_del(&idle_task->list); 
	idle_task->PID = 0;
	/*Init Directory*/
	allocate_DIR(idle_task); //init dir-pages_baseAaddr
	//cpu_idle();
	
	idle_task->quantum = DEFAULT_QUANTUM;
    
	union task_union* idle_task_union = (union task_union*)idle_task; //task struck + pila de sistema del proceso
	
	/*Address of the code that it will execute*/
	idle_task_union->stack[KERNEL_STACK_SIZE - 1] = (unsigned long) &cpu_idle; //dirección que se ejecutara 
    /*Assign register ebp for task_switch*/
    idle_task_union->stack[KERNEL_STACK_SIZE - 2] = 0; //valor inicial a asignar al registro ebp
    /*esp_kernel <--- 0(Top of stack) */
    idle_task->kernel_esp = (unsigned int) &(idle_task_union->stack[KERNEL_STACK_SIZE - 2]);
    
    init_stats(idle_task);

}

void init_task1(void)
{	
	init_task = list_head_to_task_struct(list_first(&freequeue));
	list_del(&init_task->list);
	
	init_task->PID = 1;
	init_task->state = ST_RUN;
	init_task->quantum = DEFAULT_QUANTUM;
	ticks_cpu = DEFAULT_QUANTUM;

	/*init dir_pages_baseAaddr*/
	allocate_DIR(init_task);
	/*init address space*/
	set_user_pages(init_task);
	
	/*Update TSS gor system stack*/
	union task_union* init_task_union = (union task_union*) init_task; 
	tss.esp0 = (unsigned long) &init_task_union->stack[KERNEL_STACK_SIZE];
	
	/*Set Directory process current*/
	set_cr3(get_DIR(init_task));
	ticks_cpu = DEFAULT_QUANTUM;
	
	init_stats(init_task);
	 
}

void inner_task_switch(union task_union*new)
{
	
	
	
	struct task_struct* current_task = current();
	struct task_struct* new_task = (struct task_struct*) new;
	
	new_task->st.ready_ticks += get_ticks();
	
	/*Update TSS, point to the new_task system stack*/
	tss.esp0 = (unsigned long) &new->stack[KERNEL_STACK_SIZE];
	/*current directory*/
	set_cr3(get_DIR(new_task));
	//cambio contexto ejecucion
	
	/*current.kernel_esp <- &ebp*/
	__asm__ __volatile__("movl %%ebp, %0"
	:"=r"(current_task->kernel_esp));
    /*%esp <- new.kernel_esp*/
	__asm__ __volatile__("movl %0, %%esp"
	::"r"(new_task->kernel_esp));
	//cambio contexto mem
	
	/*Restore %ebp*/
	__asm__ __volatile__("popl %ebp\n"
	"ret \n");
	
	current_task->st.elapsed_total_ticks += get_ticks();
	
}

void task_switch(union task_union*new)
{
	/*We have to save %esi,%ebx,%edi*/
	__asm__ __volatile__ (
    "pushl %esi\n"
    "pushl %ebx\n"
    "pushl %edi\n"
   );
  
  inner_task_switch(new);

  __asm__ __volatile__ (
    "popl %edi\n"
    "popl %ebx\n"
    "popl %esi\n"
	);
	
}




void init_freequeue(){
	INIT_LIST_HEAD(&freequeue);
	int i;
	for(i = 0; i < NR_TASKS;++i) list_add_tail(&task[i].task.list,&freequeue); // Al ser una cola se añade al final
}
void init_readyqueue(){
	INIT_LIST_HEAD(&readyqueue);
}


void init_sched(){
	init_freequeue();
	init_readyqueue();
		
}

struct task_struct* current()
{
  int ret_value;
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}


//Process scheduling
//create a Round Robin policy.Implement this functions.

void update_sched_data_rr() {
	--ticks_cpu;
	}

/*returns : 1 if it is necessary to change the current process 
 * 0 otherwise*/
int needs_sched_rr(){
	if ((ticks_cpu==0)) return 1;
	else return 0;
}


void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue){
	if (t->state != ST_RUN) list_del(&(t->list)); //remove to assign
	
	if (dst_queue == NULL) t->state = ST_RUN;
	else { //new estate is not running
		//modify state of task_struct
		if (dst_queue!=&readyqueue) t->state=ST_BLOCKED;
		else t->state=ST_READY;
		list_add_tail(&(t->list), dst_queue);  //insert the process to new state queue
	}
}

/*select the next process to execute, first in the readyqueue*/ 
void sched_next_rr() {
	struct task_struct* new;
	
	if (!list_empty(&readyqueue)) {
		new = list_head_to_task_struct(list_first(&readyqueue));
		list_del(list_first(&readyqueue));
	}
	else new=idle_task;
	new->state=ST_RUN;
	/*trampolin to execute new process*/
	
	
	
	task_switch((union task_union*)new);
	ticks_cpu = get_quantum(new);
	
}

int get_quantum(struct task_struct *t) {
	return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum) {
	t->quantum = new_quantum;
}

//Round robin policy
void schedule() {
	update_sched_data_rr();
	if (needs_sched_rr() == 1) {
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
	}	
}





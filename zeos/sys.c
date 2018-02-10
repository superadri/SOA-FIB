/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

int PID = 1000;

int newPID() {
	return PID +1;
}

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork() {
	return 0;
}

int sys_fork()
{
  
	if(list_empty(&freequeue)) return ENOMEM;
	else {
		struct task_struct  *son_task = list_head_to_task_struct(list_first(&freequeue));
		list_del(list_first(&freequeue));
		
		/*Copy de parent task_union to the child*/
		copy_data((union task_union*)current(),(union task_union*)son_task,sizeof(union task_union));
		
		/*Init Directory, dir_pages_baseAddr*/
		allocate_DIR(son_task);
		/*Searching free physical pages*/
		int pag;
		unsigned long frames[NUM_PAG_DATA];
		for (pag=0;pag<NUM_PAG_DATA;pag++){
		frames[pag] = alloc_frame();
		if (frames[pag] == -1){
			int j;
			for (j = 0; j <= pag; ++j) free_frame(frames[j]);
			return ENOMEM;
			}
		}

		
		page_table_entry* current_pt = get_PT(current());
		page_table_entry* new_pt = get_PT(son_task);
		/*assign new frames to child */
		for (pag = 0; pag < NUM_PAG_DATA; ++pag) {
			set_ss_pag(new_pt,PAG_LOG_INIT_DATA+pag,frames[pag]);
		}
		/*assign Code to child (shared all process)*/
		for (pag=0; pag<NUM_PAG_CODE; pag++) {
			set_ss_pag(new_pt, PAG_LOG_INIT_CODE+pag, get_frame(current_pt, PAG_LOG_INIT_CODE+pag));
		}
		/*assign Kernel to child (shared all process)*/
		for (pag = 0; pag < NUM_PAG_KERNEL; ++pag) {
			set_ss_pag(new_pt, pag, get_frame(current_pt, pag));
		}
		/*copy Data+Stack to child in the new Frames*/
		int i = 0;
		unsigned int address_o;
		unsigned int address_d;
		for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++) {
			set_ss_pag(current_pt, pag+NUM_PAG_DATA, frames[i]); //paginas
			
			address_o =pag*PAGE_SIZE; 
			address_d =(pag+NUM_PAG_DATA)*PAGE_SIZE;
			copy_data((void*)(address_o), (void*)(address_d), PAGE_SIZE); //direccion logica
			
			del_ss_pag(current_pt, pag+NUM_PAG_DATA); //paginas
			++i;
		}
		/*Flush TLB*/
		set_cr3(get_DIR(current())); 


		son_task->PID = newPID();
		union task_union* new_union = (union task_union*)son_task;


		/*16 pos to contx hw & sw, 17 -> @handler,18->@ret_from_fork,19->0 to ebp task_switch*/
		new_union->stack[KERNEL_STACK_SIZE-19] = 0;
		new_union->stack[KERNEL_STACK_SIZE-18] = (unsigned int)ret_from_fork;
		son_task->kernel_esp = (unsigned long *)&new_union->stack[KERNEL_STACK_SIZE-19];
		
		init_stats(&(son_task));
		/*add readyqueue*/
		list_add_tail(&(new_union->task.list), &readyqueue);

		return new_union->task.PID;

	}
}


void sys_exit()
{  	
	/*physical memory*/
	int pag;
	for (pag=0; pag<NUM_PAG_DATA; pag++)	{
		free_frame(get_frame(get_PT(current()), PAG_LOG_INIT_DATA+pag));
		del_ss_pag(get_PT(current()), PAG_LOG_INIT_DATA+pag);
	}
	/*task_struct*/
	list_add_tail(&(current()->list), &freequeue);
	sched_next_rr();
}

int sys_write(int fd,char *buffer,int size){ // copy_data
	//check parametres
	//Buffer not null
	//size > 0
	int retorno = check_fd(fd,ESCRIPTURA);
	if (retorno != 0) return retorno;
	else if (size <= 0 || buffer == NULL) return -EINVAL;
	
	char buff[size];
	retorno = copy_from_user(buffer, buff, size);
	if (retorno != 0) return retorno;
	
	retorno = sys_write_console(buff,size);
	return retorno;
	
}


int sys_gettime() {
	return get_zeos_ticks();
}
int sys_get_stats(int pid, struct stats *st) {
	if (pid < 0) return -1;
	else {
		int i;
		for (i =0; i < NR_TASKS; ++i){
			if (task[i].task.PID == pid){
				int retorno = copy_from_user(&(task[i].task.st), st, sizeof(struct stats));
				if (retorno != 0) return retorno;
				return 0;
			}
		}
	}
	
}





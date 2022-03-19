/*
 * sched.c - initializes struct for task 0 and task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return (struct task_struct*) ((unsigned int)l&0xFFFFF000);
}

extern TSS tss;
extern struct list_head blocked;
void writeMSR(int msr_num, long int value);
void return_gate(Word ds, Word ss, DWord esp, Word cs, DWord eip);
void inner_task_switch_asm(union task_union *new);
int main(void);

struct list_head freequeue;
struct list_head readyqueue;
struct task_struct *idle_task;

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

	while(1)
	{
	;
	}
}

void init_idle (void)
{
  struct list_head *free_list = list_first(&freequeue);
  list_del(free_list);

  struct task_struct *free_struct = list_head_to_task_struct(free_list);
  free_struct->PID = 0;

  allocate_DIR(free_struct);

  union task_union *free_union = (union task_union *) free_struct;
  free_union->stack[KERNEL_STACK_SIZE - 2] = 0; //ebp
  free_union->stack[KERNEL_STACK_SIZE - 1] = (long unsigned int) cpu_idle;

  free_struct->kernel_esp = (int *) &free_union->stack[KERNEL_STACK_SIZE - 2];

  idle_task = free_struct;
}

void init_task1(void)
{
  struct list_head *free_list = list_first(&freequeue);
  list_del(free_list);

  struct task_struct *free_struct = list_head_to_task_struct(free_list);
  free_struct->PID = 1;

  allocate_DIR(free_struct);
  set_user_pages(free_struct);

  union task_union *free_union = (union task_union *) free_struct;

  tss.esp0 = (DWord) KERNEL_ESP(free_union);
  writeMSR(0x175, (long int) KERNEL_ESP(free_union));

  page_table_entry* dir = get_DIR(free_struct);
  set_cr3(dir);
}


void init_sched()
{
  INIT_LIST_HEAD(&freequeue);
  list_add(&task[0].task.list, &freequeue);
  for (int i = 1; i < NR_TASKS; ++i)
    list_add(&task[i].task.list, &task[i - 1].task.list);

  INIT_LIST_HEAD(&readyqueue);
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0XFFFFF000);
}

void inner_task_switch(union task_union *new) {
  tss.esp0 = (DWord) KERNEL_ESP(new);
  writeMSR(0x175, (long int) KERNEL_ESP(new));

  struct task_struct *new_struct = (struct task_struct *) new;
  page_table_entry *dir = get_DIR(new_struct);
  set_cr3(dir);

  inner_task_switch_asm(new);
}

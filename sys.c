/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; 
  if (permissions!=ESCRIPTURA) return -EACCES; 
  return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS; 
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int get_first_free_page(page_table_entry *parent_PT) {
  int pag = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA;
  while (pag < TOTAL_PAGES && get_frame(parent_PT, pag) != 0)
    ++pag;

  if (pag == TOTAL_PAGES)
    return -EAGAIN;

  return pag;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);

  int new_ph_pag, pag, i;
  page_table_entry *parent_PT = get_PT(current());

  int temp_pag = get_first_free_page(parent_PT);

  /* Allocate pages for DATA+STACK */
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);

      /* Return error */
      return -EAGAIN;
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, temp_pag, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)(temp_pag<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, temp_pag);
    /* Deny access to the child's memory space */
    set_cr3(get_DIR(current()));
  }

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  uchild->task.callback_function = NULL;
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;
	
	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{  
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }
  
  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);
  
  current()->PID=-1;
  
  /* Restarts execution of the next process */
  sched_next_rr();
}

extern struct Buffer key_buffer;
char *read_char(struct Buffer *buffer);

int sys_get_key(char *c) {
  if (!access_ok(VERIFY_WRITE, c, sizeof(char)))
    return -EFAULT;
  char *d = read_char(&key_buffer);
  if (d == NULL)
    return -EAGAIN;
  copy_to_user(d, c, sizeof(char));
  return 0;
}

char *sys_get_screen() {
  page_table_entry *process_PT = get_PT(current());
  int pag = get_first_free_page(process_PT);
  int new_ph_pag = alloc_frame();
  set_ss_pag(process_PT, pag, new_ph_pag);
  return (char *) (pag << 12);
}

int sys_remove_screen(char *s) {
  page_table_entry *process_PT = get_PT(current());
  int pag = (int) s >> 12;
  if (pag >= TOTAL_PAGES || pag < NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA
    || get_frame(process_PT, pag) == 0)
    return -EFAULT;
  free_frame(get_frame(process_PT, pag));
  del_ss_pag(process_PT, pag);
  set_cr3(get_DIR(current()));
  return 0;
}

int sys_set_screen_callback(void (*callback_function)(char*)) {
  if (current()->callback_function != NULL &&
    !access_ok(VERIFY_READ, callback_function, sizeof(callback_function)))
    return -EINVAL;
  current()->callback_function = callback_function;
  return 0;
}

void sys_return_from_callback() {
  union task_union *current_union = (union task_union *) current();
  char (*screen)[80][2] = (char (*)[80][2]) current()->screen;
  DWord *stack = (DWord *) ((current_union->stack[KERNEL_STACK_SIZE - 2]) & 0xfffff000);

  copy_data(current()->registers, &current_union->stack[KERNEL_STACK_SIZE - 16], sizeof(current()->registers));

  for (int i = 0; i < 25; ++i)
    for (int j = 0; j < 80; ++j)
      printc_xy_colour(j, i, screen[i][j][0], screen[i][j][1]);

  sys_remove_screen((char *) screen);
  sys_remove_screen((char *) stack);
  current()->screen = NULL;
}

void (*screen_callback_wrapper)(void (void (*)(char *),char *));

int sys_system_info_screen_callback_wrapper(void (*wrapper)(void (void (*)(char *),char *))) {
  if (!access_ok(VERIFY_READ, wrapper, sizeof(wrapper)))
    return -EINVAL;
  screen_callback_wrapper = wrapper;
  return 0;
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

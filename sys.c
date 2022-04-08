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

int last_PID;
int zeos_ticks;
void schedule(void);
void sys_fork_asm(struct task_struct *new);
extern struct list_head freequeue;
extern struct list_head readyqueue;

int check_fd(int fd, int permissions) {
  if (fd != 1)
    return -EBADF;
  if (permissions != ESCRIPTURA)
    return -EACCES;

  return 0;
}

int sys_ni_syscall() {
  return -ENOSYS;
}

int sys_getpid() {
  return current()->PID;
}

int sys_fork() {
  if (list_empty(&freequeue))
    return -ENOMEM;

  struct list_head *new_list = list_first(&freequeue);
  list_del(new_list);

  struct task_struct *new_struct = list_head_to_task_struct(new_list);
  copy_data(current(), new_struct, sizeof(union task_union));

  allocate_DIR(new_struct);

  int new_frame[NUM_PAG_DATA];
  int frame = 0;
  for (int i = 0; i < NUM_PAG_DATA; ++i) {
    frame = alloc_frame();

    if (frame < 0) {
      list_add(new_list, &freequeue);
      return -ENOMEM;
    }

    new_frame[i] = frame;
  }

  page_table_entry *current_PT = get_PT(current());
  page_table_entry *new_PT =  get_PT(new_struct);
  for (int i = 1; i < NUM_PAG_KERNEL; ++i) {
    unsigned int frame = get_frame(current_PT, i);
    set_ss_pag(new_PT, i, frame);
  }
  for (int i = PAG_LOG_INIT_CODE; i < (PAG_LOG_INIT_CODE + NUM_PAG_CODE); ++i) {
    unsigned int frame = get_frame(current_PT, i);
    set_ss_pag(new_PT, i, frame);
  }
  for (int i = PAG_LOG_INIT_DATA; i < (PAG_LOG_INIT_DATA + NUM_PAG_DATA); ++i) {
    set_ss_pag(current_PT, i + NUM_PAG_DATA, new_frame[i - PAG_LOG_INIT_DATA]);

    set_ss_pag(new_PT, i, new_frame[i - PAG_LOG_INIT_DATA]);
    copy_data((void *) (i*PAGE_SIZE), (void *) ((i + NUM_PAG_DATA)*PAGE_SIZE), PAGE_SIZE);
    del_ss_pag(current_PT, i + NUM_PAG_DATA);
  }

  page_table_entry *dir = get_DIR(current());
  set_cr3(dir);

  new_struct->PID = ++last_PID;
  INIT_LIST_HEAD(new_list);
  new_struct->pstats.user_ticks = 0;
  new_struct->pstats.system_ticks = 0;
  new_struct->pstats.blocked_ticks = 0;
  new_struct->pstats.ready_ticks = 0;
  new_struct->pstats.elapsed_total_ticks = get_ticks();
  new_struct->pstats.total_trans = 0;
  new_struct->pstats.remaining_ticks = 0;

  sys_fork_asm(new_struct);

  list_add(new_list, &readyqueue);

  return new_struct->PID;
}

int ret_from_fork() {
  return 0;
}

void sys_exit() {
  list_add(&current()->list, &freequeue);

  page_table_entry *current_PT = get_PT(current());
  for (int i = PAG_LOG_INIT_DATA; i < (PAG_LOG_INIT_DATA + NUM_PAG_DATA); ++i) {
    int frame = get_frame(current_PT, i);
    free_frame(frame);
  }

  schedule();
}

int sys_write(int fd, char * buffer, int size) {
  int error = check_fd(fd, ESCRIPTURA);
  if (error != 0)
    return error;
  if (buffer == NULL)
    return -EFAULT;
  if (size < 0)
    return -EINVAL;

  char buff[size];
  copy_from_user(buffer, buff, size);

  return sys_write_console(buffer, size);
}

int sys_gettime() {
  return zeos_ticks;
}

//TODO
int sys_get_stats(int pid, struct stats *st) {
  if (st == NULL)
    return -EFAULT;

  if (current()->PID == pid) {
    copy_to_user(&current()->pstats, st, sizeof(struct stats));
    return 0;
  }

  struct list_head *element;
  list_for_each(element, &readyqueue) {
    struct task_struct *to_check = list_head_to_task_struct(element);
    if (to_check->PID == pid) {
      copy_to_user(&to_check->pstats, st, sizeof(struct stats));
      return 0;
    }
  }

  return -EINVAL;
}

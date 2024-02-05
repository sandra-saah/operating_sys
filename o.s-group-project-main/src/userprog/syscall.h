#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "lib/user/syscall.h"
#include "threads/synch.h"
#include "devices/shutdown.h"

void syscall_init (void);
void sys_halt(void);
void sys_exit(int status);

int sys_open(const char *file);
bool sys_create(const char *file,unsigned initial_size);
bool sys_remove (const char *file);
int sys_read (int fd, void *buffer, unsigned size);
int sys_write(int fd, void *buffer, unsigned size);
void sys_close(int fd);
void sys_close_file(int fd);
void sys_exit(int status);
off_t sys_fileSize(int fd);
unsigned sys_tell (int fd);
void sys_seek(int fd, unsigned position);
int sys_set_file(struct file *fn);
struct file* sys_get_file (int filedes);

#endif /* userprog/syscall.h */

#include <stdio.h>
#include <stdlib.h>
#include <syscall-nr.h>
#include "lib/debug.h"
#include "lib/user/syscall.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "filesys/off_t.h"
#include "devices/input.h"
#include "devices/shutdown.h"
#include "lib/kernel/list.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "userprog/process.h"


#define PHYS_BASE 0xc0000000
#define EXIT_ERROR -1

struct file *get_file (int filedes);

struct process_file{
    int fd;
    struct file *file;
    struct list_elem elem;
};

static void syscall_handler(struct intr_frame *);
void sys_halt(void);
void sys_exit(int status);

int sys_open(const char *file);
bool sys_create(const char *file,unsigned initial_size);
bool sys_remove (const char *file);
int sys_read (int fd, void *buffer, unsigned size);
int sys_write(int fd, void *buffer, unsigned size);
void sys_close(int fd);
void sys_exit(int status);
off_t sys_fileSize(int fd);
unsigned sys_tell (int fd);
void sys_seek(int fd, unsigned position);


int set_file(struct file *fn);
struct file* get_file (int filedes);



/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

bool validate_pointer(void *vaddr){ // take a pointer and check it's in userspace
	
	if (get_user((uint8_t *) vaddr) == -1)  // use the get_user function to verify
		return false; // invalid pointer
     
	return true; //valid pointer
}

bool validate_buffer(void *buffer, size_t length)  // checking each value in buffer is a valid pointer
{
	size_t i;
	char *temp_buffer = (char *)buffer; 

	for (i = 0; i < length; i++)
	{
		if (validate_pointer(temp_buffer + i) == false) // checking each pointer
		{
			return false; // found invalid pointer
		}

	return true;  // all valid

	}
}



static struct semaphore fileSema; 

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  struct thread *current = thread_current();
  list_init(&current->file_list);//Initializing the file_list for the file names
  sema_init (&fileSema, 1);// initiate semma 
}



static void
syscall_handler (struct intr_frame *f)
{
  int syscall_code = *(int *) f -> esp; // gets stack pointer from interrupt frame  | A.S
  switch (syscall_code)
  {
  case SYS_HALT:  // Implemented by Adam S
    printf("SYS_HALT Executing...\n");
    sys_halt();
    break;
  

  case SYS_EXIT:  // Implemented by Adam S
    printf("SYS_EXIT Executing...\n");
    int exit_status = *(int *) f -> esp + 1;  // getting exit status from stack
    sys_exit(exit_status);
    break;
  
  case SYS_EXEC:  // 2
    printf("SYS_EXEC Executing...\n");

      break;
    

  case SYS_WAIT: // 3
    printf("SYS_WAIT Executing...\n");
      break;
    

  case SYS_CREATE:  // 4
    printf("SYS_CREATE Executing...\n");
      break;
    

    case SYS_REMOVE:  // 5
    printf("SYS_REMOVE Executing...\n");

    break;
    

  case SYS_OPEN:  // 6
    printf("SYS_OPEN Executing...\n");
    uint32_t file_name = *(char*) f -> esp + 1;
    sys_open(file_name);
      break;
    

  case SYS_FILESIZE:  // 7
    printf("SYS_FILESIZE Executing...\n");
      break;
    

  case SYS_READ:  // 8
    printf("SYS_READ Executing...\n");
      break;
    
  case SYS_WRITE:  // 9    
    printf("SYS_WRITE Executing...\n");
    int fd = *((int*)f->esp + 1); // file descriptor
    void* buffer = (void*)(*((int*)f->esp + 2));  // buffer of values
    unsigned size = *((unsigned*)f->esp + 3); // size of buffer

    // off_t bytes_written = sys_write(fd, *buffer, size);  // This line is commented out as the function doesn't work

    break;


  case SYS_SEEK:  // 10
    printf("SYS_SEEK Executing...\n");
      break;

    
  case SYS_TELL:  // 11
    printf("SYS_TELL Executing...\n");
      break;
    

  case SYS_CLOSE:  // 12
    printf("SYS_CLOSE Executing...\n");
      break;
  
  
  default:
    break;
  
  thread_exit ();
  }
}

struct file *get_file (int fd) /*find files that matche the fd . S.s */
{
    struct thread *current = thread_current();
    struct process_file *file_to_process;
    struct list_elem *list;
    //Loops from the start of file_list to the end getting each element.
    for(list = list_begin(&current->file_list); list != list_end(&current->file_list); list = list_next(list)){
        //Gets the next entry in the file_list
        file_to_process = list_entry(list, struct process_file, elem);
        //Compare both files descriptors if matche
        if(file_to_process->fd == fd){
            return file_to_process->file;
        }
    }
    //Return nothing if file was not found
    return NULL;
}

  void sys_halt(void) // A.S
  {
    shutdown_power_off(); // “Terminates Pintos by calling 'shutdown_power_off'"  
  }

  void sys_exit(int status)
  {
    struct thread *cur = thread_current(); // get the current running thread
    cur->exit_status = status; // set its exit status to the new status
    thread_exit();
  }

    int sys_open(const char *file) // file name passed
  {
    //Goes and fetches the file
    struct file *file_open = filesys_open(file); 

    //File cound not be found
    if(!file_open){ // if the file cannot be found 
        return EXIT_ERROR;  // return -1 error
    }

    sema_down(&fileSema); //wait
   // int fd = set_file(file_open);  // CHECK THIS WE NEED TO REMOVE IT IF IT DOESNT WORK
    int fd = 4;
    sema_up(&fileSema); //post
     return fd;
  }

  bool sys_create(const char *file, unsigned initial_size){ /*Create file S.s */
    bool isCreated = false;

    if (file == NULL){ // return false if filename is empty
        return false;
    }else{
        sema_down(&fileSema);//wait
        isCreated = filesys_create(file, initial_size);
        sema_up(&fileSema);//post
	
        return isCreated;
    }
    return isCreated;
 }


int sys_write(int fd, void *buffer, unsigned size)
{
  
  if (validate_buffer(buffer, size) == false) // validate the buffer
  {
    return EXIT_ERROR; // return -1 if invalid
  }

  // get the file relating to the file descriptor
  struct file *file_to_write = get_file(fd);


  sema_down(&fileSema);// protect file resource while being written to
  file_write(file_to_write, buffer, size);
  sema_up(&fileSema);// release file resource back

  off_t bytes_written = size; // bytes written should be equal to buffer size
   
  return bytes_written;
}

void sys_close(int fd){ /* implemented by S.S*/
    struct thread *current = thread_current();//Gets the currently running thread
    struct process_file *file_to_process;
    struct list_elem *list;

    for (list = list_begin(&current->file_list); list != list_end(&current->file_list); list = list_next(list))//Loop through file_list and gets each element in the list
    {
        file_to_process = list_entry(list, struct process_file, elem);//Goes and fetches that file name from the process_file structure
        if (fd == file_to_process->fd)
        {
            file_close(file_to_process->file);//Will close that file
        }
    }





#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/malloc.h"
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "process.h"
#include "pagedir.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/shutdown.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);
void isValidAddress(const void*);
void closeAllFiles(struct list* files);
void closeFile(struct list* files, int fd);
struct struct_file* list_search(struct list* files, int fd);

extern bool running;

struct struct_file {
	struct file* fptr;
	int fd;
	struct list_elem elem;
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int *stack_ptr = f->esp;

	switch (*stack_ptr)
	{
    
		case SYS_HALT:
		shutdown_power_off();
		break;

    case SYS_OPEN:
    {
		//isValidAddress((void *)(stack_ptr+1));
		struct file* fptr = filesys_open ((char *)*(stack_ptr+1));

		if (fptr!=NULL) {
      struct struct_file *file = malloc(sizeof(*file));
			file->fptr = fptr;
			file->fd = thread_current()->fd_count;
			thread_current()->fd_count++;
			list_push_back (&thread_current()->files, &file->elem);
			f->eax = file->fd;
    }
    else
			f->eax = -1;
		break;
    }

    case SYS_EXEC:
		isValidAddress((void *)(stack_ptr+1));
		f->eax = process_execute((char *)*(stack_ptr+1));
		break;

    case SYS_FILESIZE:
    isValidAddress((void *)(stack_ptr+1));
		f->eax = file_length (list_search(&thread_current()->files, *(stack_ptr+1))->fptr);
		break;

 		case SYS_EXIT:
		isValidAddress((void *)(stack_ptr+1));
		exit_process(*(stack_ptr+1));
		break;

    case SYS_CREATE:
    //hex_dump(*(stack_ptr+1),*(stack_ptr+1),16,true);
    //printf("%s", (char *) *(stack_ptr+1));
    isValidAddress((void *)(stack_ptr+1));
		f->eax = filesys_create((char *) *(stack_ptr+1),*(stack_ptr+2));
		break;

		case SYS_SEEK:
		isValidAddress((void *)(stack_ptr+2));
		file_seek(list_search(&thread_current()->files, *(stack_ptr+1))->fptr,*(stack_ptr+2));
		break;

		case SYS_TELL:
		isValidAddress((void *)(stack_ptr+1));
		f->eax = file_tell(list_search(&thread_current()->files, *(stack_ptr+1))->fptr);
		break;

    case SYS_READ:
		if(*(stack_ptr+1)==0)
		{
			int i;
      uint8_t* buffer = *(stack_ptr+2);
			for(i=0;i<*(stack_ptr+3);i++)
				buffer[i] = input_getc();
			f->eax = *(stack_ptr+3);
		}
		else
		{
			struct struct_file* fptr = list_search(&thread_current()->files, *(stack_ptr+1));
			if(fptr==NULL)
				f->eax=-1;
			else
				f->eax = file_read_at (fptr->fptr, (void *)*(stack_ptr+2), *(stack_ptr+3),0);
		}

		case SYS_WAIT:
		isValidAddress((void *)(stack_ptr+1));
		f->eax = process_wait(*(stack_ptr+1));
		break;

		case SYS_WRITE:
    isValidAddress((void *)(stack_ptr+1));
		if(*(stack_ptr+1)==1) {
			putbuf((char *)*(stack_ptr+2),*(stack_ptr+3));
      f->eax = *(stack_ptr + 3);
		} else {
      struct struct_file* fptr = list_search(&thread_current()->files, *(stack_ptr+1));
			if(fptr==NULL)
				f->eax = -1;
			else
				f->eax = file_write_at (fptr->fptr, (void *)*(stack_ptr + 2), *(stack_ptr + 3),0);
    }
		break;

    case SYS_REMOVE:
		isValidAddress((void *)*(stack_ptr+1));
		if(!filesys_remove((char *)*(stack_ptr+1)))
			f->eax = false;
		else
			f->eax = true;
		break;

    case SYS_CLOSE:
		closeFile(&thread_current()->files,*(stack_ptr+1));
		break;

		default:
		printf("No match\n");
	}
}

void isValidAddress(const void *vaddr)
{
	if (!is_user_vaddr(vaddr) || pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) {
    exit_process(-1);
  }
}

void closeFile(struct list* files, int fd) {
 	struct list_elem *e;
  for (e = list_begin (files); e != list_end (files);
      e = list_next (e))
  {
    struct struct_file *f = list_entry (e, struct struct_file, elem);
    if(f->fd == fd)
    {
      file_close(f->fptr);
      list_remove(e);
    }
  }
}

void closeAllFiles(struct list* files) {
 	struct list_elem *e;
  for (e = list_begin (files); e != list_end (files);
      e = list_next (e))
  {
    struct struct_file *f = list_entry (e, struct struct_file, elem);
    
    file_close(f->fptr);
    list_remove(e);
  }
} 

struct struct_file* list_search(struct list* files, int fd) {
 	struct list_elem *e;
  for (e = list_begin (files); e != list_end (files);
      e = list_next (e))
  {
    struct struct_file *f = list_entry (e, struct struct_file, elem);
    if(f->fd == fd)
      return f;
  }
   return NULL;
}

void exit_process(int status) {
	struct list_elem *e;

	for (e = list_begin (&thread_current()->parent->child_processes); e != list_end (&thread_current()->parent->child_processes);
				e = list_next (e))
		{
			struct child *f = list_entry (e, struct child, elem);
			if(f->tid == thread_current()->tid)
			{
				f->used = true;
				f->error_code = status;
			}
		}


	thread_current()->error_code = status;

	if(thread_current()->parent->waiting_on_child == thread_current()->tid)
		sema_up(&thread_current()->parent->child_sema);

	thread_exit();
}
#include "threads/synch.h"
#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void exit_process (int);
int execute_process(char*);
void closeAllFiles(struct list*);
void isValidAddress(const void*);
void isInitialAddressValid(int *stack_ptr);
void closeFile(struct list* files, int fd);
struct struct_file* list_search(struct list* files, int fd);

#endif /* userprog/syscall.h */

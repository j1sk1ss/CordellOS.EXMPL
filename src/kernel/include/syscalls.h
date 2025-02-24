#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <fslib.h>

#include "ip.h"
#include "vfs.h"
#include "vga.h"
#include "isr.h"
#include "vesa.h"
#include "time.h"
#include "kstdio.h"
#include "tasking.h"
#include "rtl8139.h"
#include "keyboard.h"
#include "datetime.h"
#include "allocator.h"


// Screen managment through kprint 
#define SYS_SCROLL             47
#define SYS_READ_ELF           4
#define SYS_GET_KEY_KEYBOARD   5
#define SYS_AREAD_KEYBOARD     46

// System managment (memory, tasks, vars)
#define SYS_WRITE       64
#define SYS_TIME        6
#define SYS_MALLOC      7
#define SYS_PAGE_MALLOC 35
#define SYS_FREE        8
#define SYS_PAGE_FREE   34
#define SYS_KERN_PANIC  51
#define SYS_GET_PID     52
#define SYS_ADD_VAR     53
#define SYS_SET_VAR     54
#define SYS_GET_VAR     55
#define SYS_DEL_VAR     56
#define SYS_EXST_VAR    57
#define SYS_SWITCH_USER 60
#define SYS_GET_TICKS   3

// FS managment
#define SYS_READ_FILE          9
#define SYS_READ_FILE_STP      59
#define SYS_READ_FILE_OFF      33
#define SYS_READ_FILE_OFF_STP  58
#define SYS_WRITE_FILE         10
#define SYS_WRITE_FILE_OFF     50
#define SYS_OPENDIR            67 // LAST
#define SYS_LSDIR              11
#define SYS_CONTENT_STAT       65
#define SYS_OPEN_CONTENT       30
#define SYS_CLOSE_CONTENT      66
#define SYS_EXECUTE_FILE       12 // FREE
#define SYS_CEXISTS            15
#define SYS_FCREATE            16
#define SYS_DIRCREATE          17
#define SYS_CDELETE            18
#define SYS_CHANGE_META        25
#define SYS_GET_FS_INFO        45

// Multitasking 
#define SYS_START_PROCESS 26
#define SYS_KILL_PROCESS  27

// Graphics
#define SYS_PPUT_PIXEL       28
#define SYS_VPUT_PIXEL       37
#define SYS_GET_PIXEL        29
#define SYS_GET_RESOLUTION_X 31
#define SYS_GET_RESOLUTION_Y 32
#define SYS_FBUFFER_SWIPE    36
#define SYS_GET_FBUFFER_SIZE 62
#define SYS_GET_FBUFFER_BPP  63

// Networking
#define SYS_SEND_ETH_PACKET 38
#define SYS_GET_ETH_PACKETS 39
#define SYS_SET_IP          41
#define SYS_GET_IP          42
#define SYS_GET_MAC         43

// Disk manager
#define SYS_SWITCH_DISK 40 // WIP

// Hardware
#define SYS_RESTART 44


void i386_syscalls_init();
void syscall(struct Registers* regs);

#endif
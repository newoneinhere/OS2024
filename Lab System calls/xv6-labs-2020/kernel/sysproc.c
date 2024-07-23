#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int mask;
  
  //argint(int n, int *ip)
  //获取第一个参数（0 表示第一个参数）的整数值，并将其存储在 mask 中。
  //如果获取参数失败，argint 会返回一个负值（通常是 -1），表示错误。
  // 取 a0 寄存器中的值返回给 mask( kernel/syscall.c 中 syscall 那个函数设定的)
  if(argint(0, &mask) < 0)
    return -1;
  
  //myproc() 是一个函数，返回指向当前进程结构的指针。
  //myproc()->mask 获取当前进程的 mask 成员。
  myproc()->mask = mask;
  return 0;
}

uint64
sys_sysinfo(void)
{
  uint64 addr;//用户虚拟地址
  struct sysinfo info;
  struct proc *p = myproc();
  
  //用于获取系统调用的参数。如果获取失败，则返回 -1 表示错误。
  if (argaddr(0, &addr) < 0)
	  return -1;

  //获取剩余的字节数
  info.freemem = free_mem();

  //调用 nproc() 函数，获取系统中非 UNUSED 状态的进程数量，并将结果存储在 info.nproc 中。
  info.nproc = nproc();

  //使用 copyout 函数将 info 结构体的数据从内核空间复制到用户空间。
  //p->pagetable：当前进程的页表
  //如果复制失败，则返回 -1 表示错误。
  if (copyout(p->pagetable, addr, (char *)&info, sizeof(info)) < 0)
    return -1;
  
  return 0;
}




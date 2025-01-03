#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"
uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

  argint(0, &pid);
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

uint64 sys_sysinfo(void) {
    struct sysinfo info; // Tạo struct để lưu thông tin hệ thống
    uint64 addr;

    // Get the user-space pointer to the sysinfo struct
    argaddr(0, &addr);

    // Fill the sysinfo struct
    info.freemem = getfreemem();
    info.nproc = getnproc();

    // Copy the struct back to user space
    if (copyout(myproc()->pagetable, addr, (char*)&info, sizeof(info)) < 0)
        return -1;

    return 0;
}

//triển khai hàm đó ở đây
uint64 sys_trace(void) {
    int mask; // Bit mask quyết định systemm call nào được theo dõi
    // Lấy tham số từ user space
    argint(0, &mask); // Lấy mask từ user space sang kernel space
    if (mask < 0)
      return -1;
    // Lưu giá trị mask vào struct proc của tiến trình hiện tại
    struct proc *p = myproc();
    p->trace_mask = mask;
    return 0;
}  

//argint(0, &mask);
// a7 = 22: Đặt số syscall vào thanh ghi a7 (được đọc trong kernel).
// ecall: Nhảy vào kernel để gọi hàm xử lý sys_trace().
// Tham số truyền qua thanh ghi:
// a0: Tham số MASK = 32 được lưu vào thanh ghi này.

// Sau khi viết xong sys_trace này rồi thì chúng ta sửa lại định dạng in log của syscall này ở bên file syscall.c
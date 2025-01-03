#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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

// Trong kernel/sysproc.c, đã được định nghĩa sẵn sys_pgaccess(). Ta tiến hành cài đặt. Theo 
// như đề bài hàm này sẽ nhận vào ba tham số từ người dùng nhập vào: 
//  Một là, địa chỉ ảo bắt đầu trang của người dùng: uint64 start_va 
//  Hai là, số lượng trang cần kiểm tra: int num_pages 
//  Ba là, địa chỉ người dùng để lưu kết quả: uint64 user_address 

int 
sys_pgaccess(void) {
    uint64 start_va;
    int num_pages;
    uint64 user_address;

    argaddr(0, &start_va);   
    argint(1, &num_pages);     
    argaddr(2, &user_address);   

    if (num_pages <= 0) {
        return -1;
    }

    if (user_address >= MAXVA || user_address % 4 != 0) { 
        return -1;
    }
    
    struct proc *my_proc = myproc();
    int bitmask = 0;
    

    for (int i = 0; i < num_pages; i++) {
        uint64 next_address = start_va + i * PGSIZE;
         // mục đích của hàm này là tìm một PTE tương 
        // ứng với địa chỉ logic cung cấp (va). Ta sẽ sử dụng hàm này để có thể tìm đến một PTE và
        // kiểm tra cờ PTA_V và PTE_A của nó. Nếu cờ PTE_V và PTE_A đều được set, ta sẽ set bit tương ứng
        pte_t *pte = walk(my_proc->pagetable, next_address, 0);     
        if(pte == 0) {
            continue;
        }

        if ((*pte & PTE_V) && (*pte & PTE_A)) {
            bitmask |= (1 << i);
            *pte &= ~PTE_A; // Nếu không xóa cờ PTE_A, trạng thái "truy cập" của trang sẽ giữ nguyên mãi mãi sau lần truy cập đầu tiên.
            // Không thể phân biệt giữa lần truy cập cũ và lần truy cập mới khi gọi lại pgaccess().
            //Trạng thái sẽ trở nên không chính xác vì không thể phát hiện được lần truy cập tiếp theo.
        }
    }

    return copyout(my_proc->pagetable, user_address, (char *)&bitmask, sizeof(bitmask));
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

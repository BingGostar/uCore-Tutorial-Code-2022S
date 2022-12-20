#include "proc.h"
#include "defs.h"
#include "loader.h"
#include "trap.h"

struct proc pool[NPROC];
char kstack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char ustack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char trapframe[NPROC][PAGE_SIZE];

extern char boot_stack_top[];
struct proc *current_proc;
struct proc idle;

int threadid()
{
	return curr_proc()->pid;
}

struct proc *curr_proc()
{
	return current_proc;
}

// initialize the proc table at boot time.
void proc_init(void)
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		p->state = UNUSED;
		p->kstack = (uint64)kstack[p - pool];
		p->ustack = (uint64)ustack[p - pool];
		p->trapframe = (struct trapframe *)trapframe[p - pool];
		/*
		* LAB1: you may need to initialize your new fields of proc here
		*/
	}
	idle.kstack = (uint64)boot_stack_top;
	idle.pid = 0;
	current_proc = &idle;
}

int allocpid()
{
	static int PID = 1;
	return PID++;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel.
// If there are no free procs, or a memory allocation fails, return 0.
struct proc *allocproc(void)
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		if (p->state == UNUSED) {
			goto found;
		}
	}
	return 0;

found:
	p->pid = allocpid();
	p->state = USED;
	memset(&p->context, 0, sizeof(p->context));
	memset(p->trapframe, 0, PAGE_SIZE);
	memset((void *)p->kstack, 0, PAGE_SIZE);// 分配栈指针，p->kstack 到 p->kstack + pagesize 
	p->context.ra = (uint64)usertrapret;	// 设置ra。swth执行完，ret返回，进入usertrapret，此函数是返回用户空间前的准备
	p->context.sp = p->kstack + PAGE_SIZE;  // 为什么要加pagesize, 因为栈指针从高到低走，所以先加上
	return p;
}

// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void)
{
	struct proc *p;
	for (;;) {
		for (p = pool; p < &pool[NPROC]; p++) {
			if (p->state == RUNNABLE) {
				/*
				* LAB1: you may need to init proc start time here
				*/
				p->state = RUNNING;
				current_proc = p;
				swtch(&idle.context, &p->context);
			}
		}
	}
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
	struct proc *p = curr_proc();
	if (p->state == RUNNING)
		panic("sched running");
	swtch(&p->context, &idle.context);
	// swtch 在汇编中是：
	// 80200b24:	00000097          	auipc	ra,0x0
    // 80200b28:	3a6080e7          	jalr	934(ra) # 80200eca <swtch>
	// jalr 会将当前pc+4存入ra中(即jalr的下一条指令，也就是swtch()的下一条语句)，
	// 然后跳转到swtch函数swtch最后会执行ret，即跳到最新任务的ra执行
}

// Give up the CPU for one scheduling round.
void yield(void)
{
	current_proc->state = RUNNABLE;
	sched();
}

// Exit the current process.
void exit(int code)
{
	struct proc *p = curr_proc();
	infof("proc %d exit with %d", p->pid, code);
	p->state = UNUSED;
	finished();
	sched();
}

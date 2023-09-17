/* fork.c - fork */

#include <xinu.h>

#define MAIN_PID 4

/*------------------------------------------------------------------------
 *  fork  -  Fork a process on x86
 *------------------------------------------------------------------------
 */
pid32 fork()
{
	uint32		savsp, *pushsp;
	intmask 	mask;    	 /* Interrupt mask		*/
	pid32		pid;		 /* Stores new process id	*/
	pid32 		parent_pid;  /* Stores parent process id */
	struct	    procent	*prptr;		/* Pointer to proc. table entry */
    struct	    procent *parent_prptr; /* Pointer to parent process in PCB */
	uint32 		*parent_baseptr;
	uint32 		*parent_ptr;
	uint32 		register_val;
	int32		i;
	uint32		*saddr;		/* Stack address		*/
	uint32 		ready_status;     /* Stores result of setting process to ready */


	parent_pid = getpid();
	parent_prptr = &proctab[parent_pid];

	mask = disable();
	
	if (((pid=newpid()) == SYSERR) ||
	    ((saddr = (uint32 *)getstk(parent_prptr->prstklen)) == (uint32 *)SYSERR)) 
	{

		kprintf("My new PID is %d. This is my parent %d \n", pid, parent_pid);

		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];

	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = parent_prptr->prprio;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = parent_prptr->prstklen;
	prptr->prname[PNMLEN-1] = NULLCH;
	for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=parent_prptr->prname[i])!=NULLCH; i++)
		;	
	prptr->prsem = -1;
	prptr->prparent = getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	if ((parent_pid == MAIN_PID) ||          /* Child of main is a user process */		
		(parent_prptr->user_process == TRUE))  /* Child of a user process is a user process */
	{
		prptr->user_process = TRUE;
	}

	parent_baseptr = (uint32 *) parent_prptr->prstkbase;
	parent_ptr = parent_baseptr;

	/* This copies the parent stack up until the EBP */
	while (*parent_ptr != (uint32) parent_prptr->prstkbase)
	{
		*saddr = *parent_ptr;
		--saddr;
		--parent_ptr;
	}

	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
								/* interrupts enabled		*/

	*--saddr = NPROC;		/* %eax */
	asm("movl %%ecx, %0" : "=r"(register_val));
	*--saddr = register_val;			/* %ecx */
	asm("movl %%edx, %0" : "=r"(register_val));
	*--saddr = register_val;			/* %edx */
	asm("movl %%ebx, %0" : "=r"(register_val));
	*--saddr = register_val;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	asm("movl %%esi, %0" : "=r"(register_val));
	*--saddr = register_val;			/* %esi */
	asm("movl %%edi, %0" : "=r"(register_val));
	*--saddr = register_val;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);

	restore(mask);
	ready_status = ready(pid);

	if (ready_status != OK)
	{
		return SYSERR;
	}
	else
	{
		return pid;
	}
}

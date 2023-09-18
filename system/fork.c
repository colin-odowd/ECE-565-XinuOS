/* fork.c - fork */

#include <xinu.h>

local uint32 get_eip();
local void printstack(pid32 pid);

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
	uint32 		*parent_ptr;
	uint32 		register_val;
	int32		i;
	uint32		*saddr;		/* Stack address		*/
	uint32 		*frame_pointer; 

	parent_pid = getpid();

	mask = disable();
	
	if (((pid=newpid()) == SYSERR) ||
	    ((saddr = (uint32 *)getstk(parent_prptr->prstklen)) == (uint32 *)SYSERR)) 
	{
		restore(mask);
		return SYSERR;
	}

	prcount++;
	prptr = &proctab[pid];

	/* Initialize process table entry for new process */
	prptr->prstate = PR_READY;	/* Initial state is suspended	*/
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

	parent_ptr = (uint32 *) parent_prptr->prstkbase;
	frame_pointer =  parent_ptr;
	savsp = (uint32) saddr;

	asm("movl %%ebp, %0" : "=r"(register_val));

	/* This copies the parent stack up until the final stack frame  */
	while (parent_ptr > register_val)
	{
		/* When the dereference of pointer equals frame pointer address, EBP pointer found */
		while ((*parent_ptr) != (uint32)frame_pointer)
		{
			*saddr = *parent_ptr;
			--saddr;
			--parent_ptr;
		}
		*saddr = savsp; 			 /* Save child frame base pointer instead of parent */
		savsp = (uint32) saddr;		 /* Update child frame base pointer address */
		frame_pointer = parent_ptr;	 /* Update the frame pointer address we are looking for */
		--saddr;					
		--parent_ptr;
	}	

	/* Set function return address to the ESI and then return to top of stack */
	register_val = get_eip();
	*saddr = register_val;
	*--saddr = savsp;

	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
								/* interrupts enabled		*/

	*--saddr = NPROC;					/* %eax */
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

	insert(pid, readylist, prptr->prprio);
	restore(mask);
	return(pid);
}

uint32 get_eip() 
{
    uint32 eip_val;
    __asm__ volatile(
        "movl (%%esp), %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(eip_val)  
        :
        : "%eax"         
    );
    return eip_val;
}

void printstack(pid32 pid)
{
	unsigned long *p = (unsigned long *)proctab[pid].prstkbase;
	unsigned long   *esp, *ebp;

	asm("movl %%esp, %0\n" :"=r"(esp));
	asm("movl %%ebp, %0\n" :"=r"(ebp));

	kprintf("stackbase = 0x%08x\n",p);
	kprintf("ESP=0x%08x, EBP=0x%08x\n", esp, ebp);
	kprintf("ADDRESS\t\tVALUE\n");

	while(p >= esp){
		kprintf("0x%08x\t0x%08x\n", p, *p);
		p--;
	}
} 
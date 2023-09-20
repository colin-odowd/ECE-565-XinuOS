/* fork.c - fork */

#include <xinu.h>


#define MAIN_PID 4

/*------------------------------------------------------------------------
 *  fork  -  Fork a process on x86
 *------------------------------------------------------------------------
 */
pid32 fork()
{
	uint32 		*ebp;
	uint32 		ecx, edx, ebx, esi, edi;
	uint32		savsp, *pushsp;
	intmask 	mask;    	 /* Interrupt mask		*/
	pid32		pid;		 /* Stores new process id	*/
	pid32 		parent_pid;  /* Stores parent process id */
	struct	    procent	*prptr;		/* Pointer to proc. table entry */
    struct	    procent *parent_prptr; /* Pointer to parent process in PCB */
	uint32 		*parent_ptr;
	int32		i;
	uint32		*saddr;		/* Stack address		*/
	uint32 		*frame_pointer; 

	asm("movl %%ebp, %0" : "=r"(ebp));
	asm("movl %%ecx, %0" : "=r"(ecx));
	asm("movl %%edx, %0" : "=r"(edx));
	asm("movl %%ebx, %0" : "=r"(ebx));
	asm("movl %%esi, %0" : "=r"(esi));
	asm("movl %%edi, %0" : "=r"(edi));

	parent_pid = getpid();
	parent_prptr = &proctab[parent_pid];

	/* Visualize the stack of the parent. Remember that this */
	/* includes a stack frame for stacktrace() function call */
	//kprintf("\n\nParent Stacktrace: \n");
	//stacktrace(parent_pid);
	//kprintf("\n\n\n");

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
	prptr->prstate = PR_READY;	
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

	/* This copies the parent stack up until to the frame pointer of the first frame stack */   
	while ((uint32)parent_ptr > *ebp)
	{
		/* When the dereference parent stack pointer equals a frame pointer address, make new frame pointers */
		while ((*parent_ptr) != (uint32)frame_pointer)
		{
			*saddr = *parent_ptr;
			--saddr;
			--parent_ptr;
		}
		*saddr = savsp; 			 /* Save child frame pointer instead of parent */
		savsp = (uint32) saddr;		 /* Update child frame pointer address */
		frame_pointer = parent_ptr;	 /* Update the frame pointer address we are looking for */
		--saddr;					
		--parent_ptr;
	}	

	while (parent_ptr > ebp)
	{
		*saddr = *parent_ptr;
		--saddr;
		--parent_ptr;
	}

	*saddr = savsp;

	savsp = (uint32) saddr;		/* Start of frame for ctxsw	*/
	*--saddr = 0x00000200;		/* New process runs with	*/
								/* interrupts enabled		*/

	*--saddr = NPROC;		/* %eax */
	*--saddr = ecx;			/* %ecx */
	*--saddr = edx;			/* %edx */
	*--saddr = ebx;			/* %ebx */
	*--saddr = 0;			/* %esp; value filled in below	*/
	pushsp = saddr;			/* Remember this location	*/
	*--saddr = savsp;		/* %ebp (while finishing ctxsw)	*/
	*--saddr = esi;			/* %esi */
	*--saddr = edi;			/* %edi */
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);

	/* Visualize the stack of the child */
	// kprintf("\n\nChild Stacktrace: \n");
	// stacktrace(pid);
	// kprintf("\n\n\n");

	insert(pid, readylist, prptr->prprio);
	restore(mask);
	return(pid);
}

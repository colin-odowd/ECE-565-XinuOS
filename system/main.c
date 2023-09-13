/*  main.c  - main */

#include <xinu.h>

#define CHILD_PROCESSES 15

process child_process()
{
    while(1)
    {
        sleepms(1);
    }
}

void print_processes()
{
    int32 i;
	int32 j;

    for (i = 0; i < NPROC; i++) {

        if (proctab[i].prstate != PR_FREE)
        {
            kprintf("Parent: %d, Children: ", i);            
            for (j = 0; j < NPROC; j++) 
            {   
                if (proctab[j].prparent == i)
                {
                    kprintf("%d ", j);
                }
            }
            kprintf("\n");
        }
    }
}


process	main(void)
{
    int32 i;
    pid32 child_pid;
	pid32 main_pid;
	pid32 shpid;		/* Shell process ID */

	printf("\n\n");

	/* Create a local file system on the RAM disk */

	lfscreate(RAM0, 40, 20480);

	/* Run the Xinu shell */

	recvclr();
	resume(shpid = create(shell, 8192, 50, "shell", 1, CONSOLE));

    for (i = 0; i <= CHILD_PROCESSES; i++)
    {
        kprintf("Creating Child PID: %d\n", getpid()+i);
        child_pid = create((void *)child_process, 1024, 50, "child_process", 0);
        proctab[child_pid].user_process = TRUE;
        resume(child_pid);
    }

    /* Print active processes before termination */
    print_processes();

    /* Kill first user process after main */
	main_pid = getpid();
    kill(main_pid+1);

    /* Print active processes after termination */
    print_processes();

    return OK;
}


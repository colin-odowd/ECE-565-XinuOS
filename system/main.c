/*  main.c  - main */

#include <xinu.h>

#define CHILD_PROCESSES 15
#define CREATION_LOOP 3

void run_forever()
{
    while (1) 
    {
        sleepms(1);
    }
}

process child_process()
{
    int32 i;
    int32 child_pid;

    for (i = 0; i < CREATION_LOOP; i++)
    {
        child_pid = create((void *)run_forever, 1024, 50, "child_process", 0);
        proctab[child_pid].user_process = TRUE;
        resume(child_pid);
    }
    
    run_forever();
    return OK;
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
                if ((proctab[j].prparent == i) &&
                    (proctab[j].prstate != PR_FREE))
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

	printf("\n\n");

    for (i = 0; i <= CHILD_PROCESSES; i++)
    {
        child_pid = create((void *)child_process, 1024, 50, "child_process", 0);
        proctab[child_pid].user_process = TRUE;
        resume(child_pid);
    }

    sleepms(1000);

    /* Print active processes before termination */
    print_processes();

    /* Kill first user process after main */
	main_pid = getpid();
    kill(main_pid+1);

    /* Print active processes after termination */
    print_processes();

    return OK;
}


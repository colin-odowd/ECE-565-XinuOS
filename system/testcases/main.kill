/*  main.c  - main */

#include <xinu.h>

#define TESTCASE1
#define TESTCASE2

#define CHILD_PROCESSES 5
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

    kprintf("Creating Children From PID: %d\n", getpid());
    for (i = 0; i < CREATION_LOOP; i++)
    {
        child_pid = create((void *)run_forever, 1024, INITPRIO, "child_process", 0);
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
	pid32 main_pid;
    pri16 main_prio;
    pid32 child_pids[CHILD_PROCESSES + 1];

	printf("\n\n");

    main_pid = getpid();
    main_prio = proctab[main_pid].prprio; 

    for (i = 0; i <= CHILD_PROCESSES; i++)
    {
        kprintf("Creating Children From Main \n");
        child_pids[i] = create((void *)child_process, 1024, main_prio+1, "child_process", 0);
        proctab[child_pids[i]].user_process = TRUE;
    }

    for (i = 0; i <= CHILD_PROCESSES; i++)
    {
        resume(child_pids[i]);
    }

    /* Print active processes before termination */
    print_processes();

#ifdef TESTCASE1
    /* Kill first child processes of main */
    kill(child_pids[0]);

    /* Print active processes after termination */
    print_processes();

    sleepms(1000);
#endif

#ifdef TESTCASE2
    /* Kill third child processes of main */
    kill(child_pids[3]);

    /* Print active processes after termination */
    print_processes();

    sleepms(1000);
#endif

    return OK;
}


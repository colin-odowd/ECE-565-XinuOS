/* newpid.c - newpid */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  newpid  -  Obtain a new (free) process ID
 *------------------------------------------------------------------------
 */
pid32	newpid(void)
{
	uint32	i;					/* Iterate through all processes*/
	static	pid32 nextpid = 1;	/* Position in table to try or	*/
								/*   one beyond end of table	*/

	/* Check all NPROC slots */

	for (i = 0; i < NPROC; i++) {
		nextpid %= NPROC;	/* Wrap around to beginning */
		if (proctab[nextpid].prstate == PR_FREE) {
			return nextpid++;
		} else {
			nextpid++;
		}
	}
	return (pid32) SYSERR;
}

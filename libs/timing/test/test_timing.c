/******************************************************************************
 *
 *  Title:        timing.c
 *
 *****************************************************************************/

#include <stdio.h>

#include "timing.h"


/*------------------------------------------------------------------------------
 * int main()
 * Tests timing functions.
 *----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	/// Declare variables.
	TIMING t1;
	TIMING t2;
	float p1 = 1.0;
	float p2 = 0.001;
	int num_hits = 0;
	int quitter = 0;

	/// Initialize timers.
	timing_set_timer(&t1);
	timing_set_timer(&t2);

	while(!quitter) {
		if(timing_check_elapsed(&t1, p1)) {
			quitter = 1;
		}
		if(timing_check_elapsed(&t2, p2)) {
			timing_set_timer(&t2);
			num_hits++;
		}
	}

	printf("MAIN: Hit %f timer %d times in %f seconds.\n", p2, num_hits, p1);

	return 0;
}

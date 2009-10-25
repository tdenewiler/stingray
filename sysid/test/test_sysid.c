/******************************************************************************
 *
 *  Title:        test_sysid.c
 *
 *****************************************************************************/

#include <stdio.h>

#include "sysid.h"


/*------------------------------------------------------------------------------
 * int main()
 * Tests sysid functions.
 *----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
	/// Declare variables.
	int status = 0;
	int num_steps = 10;
	float min = -150.;
	float max = 44.8;
	float step_seq[num_steps];

	status = sysid_get_step_seq(step_seq, max, min, num_steps);

	for (int ii = 0; ii < num_steps; ii++) {
		printf("MAIN: Step %d = %f\n", ii, step_seq[ii]);
	}

	printf("MAIN: Got %d step inputs.\n", status);

	return 0;
}

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
	int step_size = 10;
	int prb_size = 10;
	float min = -150.;
	float max = 44.8;
	float step_seq[step_size];
	float prb_seq[prb_size];

	/// Testing step inputs.
	printf("Testing step inputs.\n");
	status = sysid_get_step_seq(step_seq, min, max, step_size);
	for (int ii = 0; ii < step_size; ii++) {
		printf("MAIN: Step %d = %f\n", ii, step_seq[ii]);
	}
	printf("MAIN: Got %d step inputs.\n", status);
	
	/// Testing psuedo random binary inputs.
	printf("Testing pseudo random binary inputs.\n");
	status = sysid_get_prb_seq(prb_seq, min, max, prb_size);
	for (int ii = 0; ii < prb_size; ii++) {
		printf("MAIN: Step %d = %f\n", ii, prb_seq[ii]);
	}
	printf("MAIN: Got %d prb inputs.\n", status);

	return 0;
}

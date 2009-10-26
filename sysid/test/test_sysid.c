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
	MSG_DATA msg;

	/// Initialize variables.
	memset(&msg, '0', sizeof(MSG_DATA));

	/// Fill msg.pitch target and status with dummy variables for testing.
	float pitch_tol = 5.;
	float pitch_range = 70. - (-70.);
	msg.status.data.pitch = 5.2;
	msg.target.data.pitch = 5.0;

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

	/// Testing steady-state check.
	printf("MAIN: Testing steady-state check.\n");
	status = sysid_check_ss(msg.status.data.pitch, msg.target.data.pitch, pitch_range, pitch_tol);
	if (status)
		printf("MAIN: Steady-state reached.\n");
	else
		printf("MAIN: Steady-state not reached.\n");

	return 0;
}

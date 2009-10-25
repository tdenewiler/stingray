/******************************************************************************
 *
 *  Title:        sysid.c
 *
 *****************************************************************************/

#include "sysid.h"

/*------------------------------------------------------------------------------
 * int sysid_get_step_seq(float **step_seq, float max, float min, int size)
 * Generates a sequence of step inputs.
 *----------------------------------------------------------------------------*/

int sysid_get_step_seq(float *step_seq, float max, float min, int size)
{
	/// Declare variables.
	int numinputs = 0;
	float input = min;

	/// Set the first array value to the min value.
	*step_seq = min;
	*step_seq++;
	numinputs++;

	/// Check if there are any more array inputs.
	if (size > 1) {
		/// Add values to the array for everything BUT min and max.
		for (int ii = 1; ii < size - 1; ii++) {
			input += (max - min) / (size - 1);
			*step_seq = input;
			numinputs++;
			*step_seq++;
		}
	}

	/// Check if there is one more array input to fill.
	if (size - numinputs > 0) {
		/// Set the last array value to the max value.
		*step_seq = max;
		numinputs++;
	}

	return numinputs;
} /* end sysid_get_step_seq() */

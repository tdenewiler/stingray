/*------------------------------------------------------------------------------
 *
 *  Title:        sysid.c
 *
 *----------------------------------------------------------------------------*/

#include "sysid.h"

/*------------------------------------------------------------------------------
 * int sysid_get_step_seq()
 * Generates a sequence of step inputs.
 *----------------------------------------------------------------------------*/

int sysid_get_step_seq(float *step_seq, float min, float max, int size)
{
	/// Declare variables.
	int numinputs = 0;
	float input = min;

	/// Set the first array value to the min value.
	if (size > 0) {
		*step_seq = min;
		*step_seq++;
		numinputs++;
	}

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


/*------------------------------------------------------------------------------
 * int sysid_get_prb_seq()
 * Generates a sequence of step inputs.
 *----------------------------------------------------------------------------*/

int sysid_get_prb_seq(float *step_seq, float min, float max, int size)
{
	/// Declare variables.
	int numinputs = 0;
	float range = max - min;
    struct timeb seed;

	/// Get the current system time and use as a seed for the random number generator.
	ftime(&seed);
	srand(seed.millitm);

	/// Check if there are any array inputs.
	if (size > 0) {
		/// Add random values to the array.
		for (int ii = 0; ii < size; ii++) {
			*step_seq = min + (rand() / (float)RAND_MAX) * range;
			numinputs++;
			*step_seq++;
		}
	}

	return numinputs;
} /* end sysid_get_prb_seq() */


/*------------------------------------------------------------------------------
 * int sysid_log()
 * Logs data.
 *----------------------------------------------------------------------------*/

int sysid_log(MSG_DATA *msg, FILE *fd)
{
	/// Declare variables.
    struct timeval ctime;
    struct tm ct;
    char write_time[128] = {0};

	/// Get a timestamp and use for filename.
	gettimeofday( &ctime, NULL );
	ct = *( localtime ((const time_t*) &ctime.tv_sec) );
	strftime( write_time + strlen(write_time), sizeof(write_time), "20%y%m%d_%H%M%S", &ct);
	snprintf( write_time + strlen(write_time),
			strlen(write_time), ".%.03ld", ctime.tv_usec );

	/// Write the data to file.
	fprintf(fd, "%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
		write_time,
		msg->mstrain.data.pitch, msg->mstrain.data.roll, msg->mstrain.data.yaw,
		msg->status.data.depth, msg->mstrain.data.ang_rate[0], msg->mstrain.data.ang_rate[1],
		msg->mstrain.data.ang_rate[2], msg->mstrain.data.accel[0], msg->mstrain.data.accel[1],
		msg->mstrain.data.accel[2], msg->mstrain.data.mag[0], msg->mstrain.data.mag[1],
		msg->mstrain.data.mag[2], msg->target.data.fx, msg->target.data.fy,
		msg->target.data.speed, msg->target.data.pitch, msg->target.data.roll,
		msg->target.data.yaw, msg->target.data.depth);

	return 1;
} /* end sysid_log() */


/*------------------------------------------------------------------------------
 * int sysid_log_init()
 * Logs data.
 *----------------------------------------------------------------------------*/

int sysid_log_init(FILE *fd)
{
	/// Write an informative header to the first line of the log file.
	fprintf(fd, "Time Pitch Roll Yaw Depth AngRate0 AngRate1 AngRate2 Accel0"
		" Accel1 Accel2 Mag0 Mag1 Mag2 Fx Fy Speed DPitch DRoll DYaw DDepth\n");

	return 1;
} /* end sysid_log_init() */


/*------------------------------------------------------------------------------
 * int sysid_check_ss()
 * Checks to see if a state variable has reached some percentage of its target
 * value which we consider to be steady-state.
 *----------------------------------------------------------------------------*/

int sysid_check_ss(float state, float target, float range, float tol)
{
	/// Check difference against tolerance. Note that tol is a percentage so / 100.
	if (fabsf(state - target) < (range * tol / 100.) )
		return 1;

	return 0;
} /* end sysid_log_init() */

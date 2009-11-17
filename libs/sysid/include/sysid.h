/**
 *  \file sysid.h
 *  \brief Functions for system identification techniques and data logging.
 */

#ifndef SYSID_H
#define SYSID_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#include "msgtypes.h"

/******************************
 *
 * #defines
 *
 *****************************/

#ifndef SYSID_RETURN_VALS
#define SYSID_SUCCESS			1
#define SYSID_ERROR			0
#endif /* RETURN_VALS */


/******************************
 *
 * Data types
 *
 *****************************/


/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Generates a sequence of step inputs.
//! \param step_seq Array to fill with input values.
//! \param min The minimum value for the step sequence to go to.
//! \param max The maximum value for the step sequence to go to.
//! \param size The number of inputs to create.
//! \return 1 on success, 0 on failure.
int sysid_get_step_seq(float *step_seq, float min, float max, int size);

//! Generates a sequence of pseudo random binary inputs.
//! \param prb_seq Array to fill with input values.
//! \param min The minimum value for the step sequence to go to.
//! \param max The maximum value for the step sequence to go to.
//! \param size The number of inputs to create.
//! \return 1 on success, 0 on failure.
int sysid_get_prb_seq(float *prb_seq, float min, float max, int size);

//! Logs data.
//! \param msg Message variable with target and status information.
//! \param fd File descriptor for the log file.
void sysid_log(MSG_DATA *msg, FILE *fd);

//! Initializes the data log with a header as the first line.
//! \param fd File descriptor for the log file.
//! \return 1 on success, 0 on failure.
int sysid_log_init(FILE *fd);

//! Checks to see if a state variable has reached some percentage of its target
//! value which we consider to be steady-state.
//! \param state The state variable to check.
//! \param target The target value that the state variable is trying to achieve.
//! \param range The range that the target values can take for the state variable.
//! \param tol The percentage that is considered close enough to the target value.
//! \return 1 on true, 0 on false.
int sysid_check_ss(float state, float target, float range, float tol);


#endif /* SYSID_H */

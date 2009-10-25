/**
 *  \file sysid.h
 *  \brief Functions for system identification techniques and data logging.
 */

#include <stdio.h>
#include <math.h>

#ifndef _SYSID_H_
#define _SYSID_H_

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

#ifndef _SYSID_
#define _SYSID_
/*! System identification struct. */
typedef struct _SYSID {
	int tmp;	//!< Placeholder
} SYSID;
#endif /* _SYSID_ */


/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Generates a sequence of step inputs.
//! \param step_seq Array to fill with input values.
//! \param max The maximum value for the step sequence to go to.
//! \param min The minimum value for the step sequence to go to.
//! \param size The number of inputs to create.
//! \return 1 on success, 0 on failure.
int sysid_get_step_seq(float *step_seq, float max, float min, int size);


#endif /* _SYSID_H_ */

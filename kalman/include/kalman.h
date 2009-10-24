/**
 *  \file kalman.h
 *  \brief  Kalman filter algorithm for position estimation.
 */

#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <unistd.h>
#include <cv.h>
#include <cvaux.h>
#include <cxcore.h>
#include <highgui.h>

/******************************
**
** #defines
**
******************************/

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */


/******************************
**
** Function prototypes
**
******************************/

//! Function Sets up all variables for a Kalman filter for position estimation.
//! \return Whether initialization is successful (1=success, 0=failure).
int init_kalman();

//! Function Cleans up variables.
void close_kalman();

//! Function Updates the kalman filter.
//! \param dt Time in seconds since last update.
//! \param depth Depth in meters from the surface.
//! \param ang Angles of rotation about each axis.
//! \param accel Acceleration along each axis.
//! \param ang_rate Rate of rotation about each axis.
void kalman_update( float dt, float depth, float ang[3], float accel[3], float ang_rate[3] );

//! Function Prints some state information as a test.
void kalman_print_test();

//! Function Prints out the internal transition matrix (F).
void kalman_print_transition_matrix();

//! Function Prints out the internal state (PRE/POST).
//! \param p Specifies which version PRE=0 and POST=1.
void kalman_print_state( int p );

//! Function Gets the current location estimation.
//! \param loc Structure to put the point into.
void kalman_get_location( CvPoint3D32f &loc );


#endif /* _KALMAN_H_ */

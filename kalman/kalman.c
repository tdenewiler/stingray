/******************************************************************************
 *
 *  Title:        kalman.c
 *
 *  Description:  Kalman filter algorithm for position estimation. Check out 
 *  http://www.edinburghrobotics.com/docs/tutorial_drivers_virtual.html for 
 *  more detailed info. Another great reference is O'Reilly's Learning OpenCV 
 *  book on Kalman Filters.  I reference the equations for the kalman filter 
 *  based on this book.
 * 
 *  NOTE: All matrices are row vectors which means (m X n) is columns by rows.
 *
 *****************************************************************************/

#include <unistd.h>
#include <cv.h>
#include <cvaux.h>
#include <cxcore.h>
#include <highgui.h>

#include "kalman.h"

/* Helper variables. */
CvRandState		rng;			// Random number generator for multiple purposes
int bPrint;						// Flag to print when updating.
int printCount;					// Threshold used for printing when updating.

/* Core variables. */
CvKalman		*kf;			// The actual Kalman filter
CvMat			*x_k;			// Current state estimation
CvMat			*u_k;			// Current Controls
CvMat			*z_k;			// Current measurements


/******************************************************************************
 *
 * Title:       init_kalman()
 *
 * Description: Sets up all variables for a Kalman filter for position estimation.
 *
 * Input:       None.
 *
 * Output:      Instance of kalman class.
 *
 *****************************************************************************/
int init_kalman()
{
	/* Initialize helper variables. */
	int n = 9;					// x, y, z, v_x, v_y, v_z, a_x, a_y, a_z 
	int m = 3;					// accel[x,y,z]
	int c = 0;					//
	bPrint = 1;
	printCount = 100 + 1;
	cvRandInit( &rng, 0, 1, -1, CV_RAND_UNI );	// random number generator
	
	/* Initialize the actual Kalman filter. */
	kf = cvCreateKalman( n, m, c );
	
	//////////////////////////////
	// Prepare variables for state equation: x_k = F * x_k-1 + B * u_k + w_k
	//////////////////////////////
	 
	/* Initialize the state - x_k. */
	/* It contains x, y, z, v_x, v_y, v_z, a_x, a_y, a_z (n X 1). */
	x_k = cvCreateMat( n, 1, CV_32FC1 );
	cvRandSetRange( &rng, 0, 1, 0 );
	rng.disttype = CV_RAND_NORMAL;
	cvRand( &rng, x_k );
	
	/* Initialize transition matrix - F - to relate how states interact (n X n). */
	float dt = 0.0; // no time has passed at start up
	float v = dt;
	float a = .5 * dt * dt;
	const float F[] = { 
		1, 0, 0, v, 0, 0, a, 0, 0,	// x_new = x + v_x*dt + .5*a_x*dt*dt
		0, 1, 0, 0, v, 0, 0, a, 0,	// y_new = y + v_y*dt + .5*a_y*dt*dt
		0, 0, 1, 0, 0, v, 0, 0, a,	// z_new = z + v_z*dt + .5*a_z*dt*dt
		0, 0, 0, 1, 0, 0, v, 0, 0,	// v_x_new = v_x + a_x*dt
		0, 0, 0, 0, 1, 0, 0, v, 0,	// v_y_new = v_y + a_y*dt
		0, 0, 0, 0, 0, 1, 0, 0, v,	// v_z_new = v_z + a_z*dt
		0, 0, 0, 0, 0, 0, 1, 0, 0,  // a_x_new = a_x
		0, 0, 0, 0, 0, 0, 0, 1, 0,  // a_y_new = a_y
		0, 0, 0, 0, 0, 0, 0, 0, 1  	// a_z_new = a_z
	};
	memcpy( kf->transition_matrix->data.fl, F, sizeof(F) );
	
	/* Initialize the controls (c X 1). */
	// u_k =
	
	/* Initialize control matrix - B - to relate controls to state (n X c). */
	// const float B[] = {
	
	//////////////////////////////
	// Prepare variables for measurement equation: z_k = H_k * x_k + v_k
	//////////////////////////////
	
	/* Initialize measurements (accel[x,y,z]). */
	z_k = cvCreateMat( m, 1, CV_32FC1 );
	cvZero(z_k);
	
	/* Initialize internal parameter - H - for relating state
	 * to measurement (m X n). */
	const float H[] = {
		0, 0, 0, 0,  0,  0,  1,  0,  0,
		0, 0, 0, 0,  0,  0,  0,  1,  0,
		0, 0, 0, 0,  0,  0,  0,  0,  1
	//  x  y  z  v_x v_y v_z a_x a_y a_z
	};
	memcpy( kf->measurement_matrix->data.fl, H, sizeof(H) );
	
	
	//////////////////////////////
	// Prepare covariance matrices
	//////////////////////////////
	
	/* Initialize covariance internal parameters. */
	cvSetIdentity( kf->process_noise_cov, cvRealScalar(1e-5) );		// (Q)
	cvSetIdentity( kf->measurement_noise_cov, cvRealScalar(1e-1) );	// (R)
	cvSetIdentity( kf->error_cov_post, cvRealScalar(1));			// (P_k)

	
	//////////////////////////////
	// Set initial internal state
	//////////////////////////////
	
	/* Set initial state to be at point (0,0,0) with no movement. */
	kf->state_post->data.fl[0] = (float)0;	// x
	kf->state_post->data.fl[1] = (float)0;	// y
	kf->state_post->data.fl[2] = (float)0;	// z
	kf->state_post->data.fl[3] = (float)0;	// v_x
	kf->state_post->data.fl[4] = (float)0;	// v_y
	kf->state_post->data.fl[5] = (float)0;	// v_z
	kf->state_post->data.fl[6] = (float)0; 	// a_x
	kf->state_post->data.fl[7] = (float)0; 	// a_y
	kf->state_post->data.fl[8] = (float)0; 	// a_z

	/* Initialization successful. */
	return 1;
}


/******************************************************************************
 *
 * Title:       close_kalman()
 *
 * Description: Cleans up variables.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/
void close_kalman()
{
	/* Destroy state, controls, process noise, measurements, 
	 * and measurement noise matricies. */
	cvReleaseMat( &x_k );
	cvReleaseMat( &u_k );
	cvReleaseMat( &z_k );
	
	/* Destroy the actual Kalman filter. */
	cvReleaseKalman( &kf );
}


/******************************************************************************
 *
 * Title:       kalman_update( float dt, float depth, float ang[3], 
 * 								float accel[3], float ang_rate[3] )
 *
 * Description: Prints out some data to show things are working.
 *
 * Input:       dt: Time in seconds since last update.
 * 				depth: Depth in meters from the surface.
 * 				ang: Angles of rotation about each axis.
 * 				accel: Acceleration along each axis.
 * 				ang_rate: Rate of rotation about each axis.
 *
 * Output:      None.
 *
 *****************************************************************************/
void kalman_update( float dt, float depth, float ang[3], 
					float accel[3], float ang_rate[3] )
{
	/* Print data if flags allow. */
	if ( bPrint > 0 && bPrint > printCount ) {
		printf( "\nInput: dt=%f accel=[%f,%f,%f]\n",
			dt, accel[0], accel[1], accel[2] );
		printf( "         depth=%f ang=[%f,%f,%f] ang_rate=[%f,%f,%f]\n",
			depth, ang[0], ang[1], ang[2],
			ang_rate[0], ang_rate[1], ang_rate[2] );
			
		printf( "Start   " );
		kalman_print_state( 0 );
		printf( "Start   " );
		kalman_print_state( 1 );
	}
	
	/* Predict the state. */
	cvKalmanPredict( kf, 0 );
	
	/* Print data if flags allow. */
	if ( bPrint > 0 && bPrint > printCount ) {
		printf( "Predict " );
		kalman_print_state( 0 );
		printf( "Predict " );
		kalman_print_state( 1 );
	}
	
	/* Set the measurement data. */
	z_k->data.fl[0] = accel[0];
	z_k->data.fl[1] = accel[1];
	z_k->data.fl[2] = accel[2];
	
	/* Correct the state. */
	cvKalmanCorrect( kf, z_k );

	/* Print data if flags allow. */
	if ( bPrint > 0 && bPrint > printCount ) {
		printf( "Correct " );
		kalman_print_state( 0 );
		printf( "Correct " );
		kalman_print_state( 1 );
		
		printf( "\nMeasurement: accel = ( %f, %f, %f )\n",
		z_k->data.fl[0], z_k->data.fl[1], z_k->data.fl[2] );
	}

	/* Fix transition matrix - F - to relate how states interact (n X n).
	 * Based on x_t = x_t-1 + v_x_t-1*dt + .5*a_x_t*dt*dt and
	 * v_x_t = v_x_t-1 + a_x_t*dt. */
	float v = dt;
	float a = .5 * dt * dt;
	const float F[] = { 
		1, 0, 0, v, 0, 0, a, 0, 0,	// x_new = x + v_x*dt + .5*a_x*dt*dt
		0, 1, 0, 0, v, 0, 0, a, 0,	// y_new = y + v_y*dt + .5*a_y*dt*dt
		0, 0, 1, 0, 0, v, 0, 0, a,	// z_new = z + v_z*dt + .5*a_z*dt*dt
		0, 0, 0, 1, 0, 0, v, 0, 0,	// v_x_new = v_x + a_x*dt
		0, 0, 0, 0, 1, 0, 0, v, 0,	// v_y_new = v_y + a_y*dt
		0, 0, 0, 0, 0, 1, 0, 0, v,	// v_z_new = v_z + a_z*dt
		0, 0, 0, 0, 0, 0, 1, 0, 0,  // a_x_new = a_x
		0, 0, 0, 0, 0, 0, 0, 1, 0,  // a_y_new = a_y
		0, 0, 0, 0, 0, 0, 0, 0, 1  	// a_z_new = a_z
	}; 
	memcpy( kf->transition_matrix->data.fl, F, sizeof(F) );
	
	/* Print data if flags allow. */
	if ( bPrint > 0 && bPrint > printCount ) {
		kalman_print_transition_matrix();
	}
	
	/* Update the print flags. */
	if ( bPrint > printCount ) {
		bPrint = 1;
	} else if ( bPrint > 0 ) {
		bPrint++;
	}
}


/******************************************************************************
 *
 * Title:       kalman_print_test()
 *
 * Description: Prints out some data to show things are working.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/
void kalman_print_test()
{
	printf( "-------------- KALMAN START UP -------------\n" );
	
	/* Print the transition matrix F. */
	kalman_print_transition_matrix();
	
	printf( "\n" );
	
	/* Print the PRE state data. */
	kalman_print_state( 0 );
	
	printf( "\n" );
	
	/* Print the POST state data. */
	kalman_print_state( 1 );

	printf( "--------------------------------------------\n" );
}


/******************************************************************************
 *
 * Title:       kalman_print_transition_matrix()
 *
 * Description: Prints out the internal transition matrix (F).
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/
void kalman_print_transition_matrix()
{
	int n = kf->DP;
	printf( "F = [ " );
	for ( int i = 0; i < n; i++ ) {
		for ( int j = 0; j < n; j++ ) {
			printf( "%.2f ", kf->transition_matrix->data.fl[ i*n+j ] );
		}
		if ( i < n-1 ) {
			printf( "\n      " );
		}
	}
	printf( "]\n" );
}


/******************************************************************************
 *
 * Title:       kalman_print_state()
 *
 * Description: Prints out the internal state (PRE/POST).
 *
 * Input:       p: Specifies which version PRE=0 and POST=1.
 *
 * Output:      None.
 *
 *****************************************************************************/
void kalman_print_state( int p )
{
	int n = kf->DP;
	if ( p == 0 ) {
		/* Print the PRE state data. */
		printf( "PRE  state = [ " );
		for ( int i = 0; i < n; i++ ) {
			printf( "%f", kf->state_pre->data.fl[i] );
			if ( i < n-1 ) {
				printf( ", " );
			}
		}
		printf( " ]\n" );
	} else if ( p == 1 ) {
		/* Print the POST state data. */
		printf( "POST state = [ " );
		for ( int i = 0; i < n; i++ ) {
			printf( "%f", kf->state_post->data.fl[i] );
			if ( i < n-1 ) {
				printf( ", " );
			}
		}
		printf( " ]\n" );
	}
}


/******************************************************************************
 *
 * Title:       kalman_get_location( CvPoint3D32f &loc )
 *
 * Description: Gets the current location estimation.
 *
 * Input:       loc: Structure to put the point into.
 *
 * Output:      None.
 *
 *****************************************************************************/
void kalman_get_location( CvPoint3D32f &loc )
{
	/* Set the current internal state to this point. */
	loc.x = kf->state_post->data.fl[0];	// x
	loc.y = kf->state_post->data.fl[1];	// y
	loc.z = kf->state_post->data.fl[2];	// z
}

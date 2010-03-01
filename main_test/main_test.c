#include <stdio.h>
#include <math.h>
#include <termios.h>
#include <unistd.h>     // for close()
#include <stdlib.h>     // for calloc() and free()
#include <signal.h>     // for signal handles (ctr-c handler)
#include <sys/time.h>   // for time functions
#include <sys/wait.h>
#include <time.h>       // for time functions
#include <string.h>

#include "labjack.h"
#include "serial.h"
#include "microstrain.h"
#include "pololu.h"
#include "logframe.h"

// sets the base frequency for the timer scheduler DO NOT CHANGE -AM
#define TIMER_PERIOD 50000 // in nanoseconds (20 Hz)

// threshold for detecting water (V)
#define WATER_THRESHOLD 2.0 // in volts

// for helping me format the screen
#define SCREEN_FORMAT_STRING "\n\n\n\n\n\n\n\n\n\n"

/* logframe - Define your own custom error codes. */
#define DEFAULT_LOG_FILE_NAME "main_test.log"
#define PID_LOG_FILE_NAME "pid.csv"
#define ERROR_NO_POLOLU 100
#define ERROR_NO_MICROSTRAIN 101
#define ERROR_NO_LABJACK 102
#define ERROR_WATER_LEAK 103
#define ERROR_IO 104
#define ERROR_GENERAL 105

// pid defines
#define BOUND_ROLL_TORQUE 80
#define BOUND_PITCH_TORQUE 80
#define BOUND_YAW_TORQUE 80
#define BOUND_VERTICAL_THRUST 80
#define BOUND_TOTAL_VERTICAL_THRUST 80
#define BOUND_ROLL_INTEGRAL 30
#define BOUND_PITCH_INTEGRAL 35
#define BOUND_YAW_INTEGRAL 30
#define BOUND_PRESSURE_INTEGRAL 50


typedef struct {
	float battery1;
	float battery2;
	float pressure;
	float water;
} labjack_struct;

typedef struct {
	float ref;    // target
	float kp;
	float kd;
	float ki;
	float c;    // current value
	float e;    // error
	float de;   // differentiated error
	float se;   // integrated error
} pid_data_struct;

typedef struct {

	pid_data_struct roll;
	pid_data_struct pitch;
	pid_data_struct yaw;
	pid_data_struct pressure;
	float c_period;   // last period in us
	// add more stuff here
} pid_struct;

// GLOBAL VARIABLES
// NOTE that these are all used by signal handlers
// sig_atomic_t is a atomic type that can be used to prevent problems
// sig_atomic_t should be equivalent to an int on this machine
// there is NO atomic float though so some kind of safety mechanism needs to
// be thought up -AM 12/4/08
// ALSO, just because a read or write to a sig_atomic_t should be atomic does
// NOT mean that an expression involving the variable is atomic
int imu_fd = -1;
int pololu_fd = -1;

// targets for pid_iteration
float target_roll = 0.0;
float target_pitch = 0.0;
float target_yaw = 0.0;
float target_pressure = 0.0;

// global buffer for front and bottom cameras
char* front_buffer = NULL;
char* bottom_buffer = NULL;

// global buffer for log file
/* Define your buffer(s), buffers are where log messages will be written to. */
logf_buffer* main_test_log_file;
/* Define your main logging instance. */
logf_instance* default_log;

// pid logging buffer
logf_buffer* pid_log_file;
logf_instance* pid_log;

// SAFETY MECHANISM
// This program relies on the programmer not accessing any global variables while the timer is in action.
// The proper way to interrupt the timer is shown in sigint_handler.
// Essentially, while the timer is running the timer is the "main" and everything else is silent.
// While the timer is stopped the actual main runs or some other asynchronous function.
// This should only happen during initialization and during shutdown.
// The way to exit the program is to press Ctrl-C.  Ctrl-C generates a SIGINT which calls sigint_handler.
// I need to make a way to "idle" the program as well.
// DO NOT MESS WITH GLOBAL VARIABLES EXCEPT DURING INITIALIZATION PHASE AND INSIDE OF TIMER_HANDLER
// OR ONE OF THE FUNCTIONS IT CALLS.  PID_ITERATION CAN ACCESS GLOBAL VARIABLES.
// -AM


// FUNCTION PROTOTYPES

// logging library functions, not really used yet
void call_me_if_threshold_reached( void );
void call_me_if_error( void );


// handles exit() but the original exit() code is called afterward automatically
void exit_handler( void );

// handles ctrl-c
void sigint_handler( int sig );

// handles timer signal
void timer_handler( int signum );
// installs the timer
void install_timer();
// removes the timer
void remove_timer();

void pid_init();
void pid_iteration( pid_struct *pid );

void waitForKey();

// some custom math functions
float absolute_value( float x );
float sign_value( float x );
float bound_integral( float value, float gain, float bound );

// handles water leaks
void water_leak_handler();

void updateLabjack( labjack_struct *current );

// this is sort of an experiment
int sample_video_frames() ;



int main()
{


	// Setup exit procedure
	void ( *exit_ptr )( void );
	exit_ptr = exit_handler;
	atexit( exit_ptr );

	// Setup control-c handler
	// This handler handles the ctrl-c signal.
	// Now when ctrl-c is pushed the program will set all servos and motors to neutral before exiting.

	struct sigaction sigint_action;
	sigint_action.sa_handler = sigint_handler;
	// SIG_RESTART causes problems below
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );


	// we needed a title -AM
	printf( "\n************ STINGRAY MAIN TEST ************\n" );
	printf( "Please ensure that the kill switch is open.\n" );
	waitForKey();

	// Setup the logging, Thanks Robert -AM
	/* Initialize your main logging instance. */
	default_log = logf_init();
	/* Initialize buffer(s) */
	main_test_log_file = logf_open_file(
	                         default_log, /* our log instance */
	                         DEFAULT_LOG_FILE_NAME, /* filename */
	                         /* we'll only log the fatal and error messages, drop everything else */
	                         /* we'll not print to stderr */
	                         LOGF_FATAL | LOGF_ERROR | LOGF_DEBUG | LOGF_INFO,
	                         20 /* write to disk after 10 messages , set to 0 if you want to manually control flushes.
                                                      You might want to do this in a time-critical loop. Delay flushes until later. */
	                     );
	/* Note that these calls will still trigger the thresholds and error callbacks regardless of whether they are filtered out
	by the buffer or not! */
	//logf_add_threshold( default_log, /* our log instance */
	//                    BAD_DATA_ERROR, /* this threshold is for our bad data (user-defined) */
	//                    1, /* If this error occurs 1 times, call our function */
	//                   call_me_if_threshold_reached /* call this one */
	//                    );
	/* this will call the function when an error is encountered. Note that the function cannot be that of a threshold. */
	//logf_set_error_calls( default_log, NULL, call_me_if_error, NULL, NULL, NULL);
	// First log entry
	//logf_info(default_log, main_test_log_file, "main_test init\n");
	logf_flush( main_test_log_file, 1 );

	// setup pid log file
	pid_log = logf_init();
	pid_log_file = logf_open_file( pid_log, PID_LOG_FILE_NAME, LOGF_INFO, 20 );


	// sample video frame here for kicks
	// sample_video_frames();

	// LABJACK INITIALIZATION
	// if the labjack was initialized

	if ( init_labjack() ) {
		query_labjack();
		printf( "Labjack is initialized...\n" );
		logf_debug( default_log, main_test_log_file, "Labjack initialized" );
	}
	else {
		printf( "No Labjack, quitting...\n" );
		logf_fatal_error( default_log, main_test_log_file, ERROR_NO_LABJACK, "No Labjack connected" );
		exit( 1 );
	}

	// IMU INITIALIZATION
	// baud rate must be 38400 unless imu is reconfigured
	int imu_serial = -1;

	printf( "Trying IMU at /dev/ttyS0 with 38400.\n" );

	imu_fd = mstrain_setup( "/dev/ttyS0", 38400 );

	if ( imu_fd < 0 ) {
		printf( "No IMU, quitting...\n" );
		logf_fatal_error( default_log, main_test_log_file, ERROR_NO_MICROSTRAIN, "No IMU connected" );
		exit( 1 );
	}

	mstrain_serial_number( imu_fd, &imu_serial );

	printf( "IMU initialized, imu serial # is: %d.\n", imu_serial );
	logf_debug( default_log, main_test_log_file, "IMU initialized, imu serial#, %d", imu_serial );
	// END IMU INITIALIZATION

	// POLOLU INITIALIZATION
	printf( "Trying Pololu at /dev/ttyUSB0 with 9600.\n" );
	pololu_fd = pololuSetup( "/dev/ttyUSB0", 9600 );
	// pololuSetup initializes all of the Pololu channels

	if ( pololu_fd < 0 ) {
		printf( "No Pololu, quitting...\n" );
		logf_fatal_error( default_log, main_test_log_file, ERROR_NO_POLOLU, "No Pololu connected" );
		exit( 1 );
	}

	printf( "Pololu Initialized.\n" );

	logf_debug( default_log, main_test_log_file, "Pololu initialized" );
	// END POLOLU INITIALIZATION

	printf( "\nThe Stingray is initialized.  You may close the kill switch now.\n" );
	printf( "Please wait for the motors to arm before continuing.\n" );
	waitForKey();

	// Get the targetRoll and targetPitch for the PID
	pid_init();

	// start the timer (20Hz)
	install_timer();

	// DO NOTHING HERE. DO NOT ACCESS GLOBAL VARIABLES.
	// TIMER_HANDLER IS NOW THE MAIN
	// IT EXECUTES EVERY 50 ms
	// -AM

	while ( 1 );

// scorpion calibration code
// am -need to make calibration programs
	//pololuSetPosition7Bit(pololu_fd,0,127);
	//pololuSetPosition7Bit(pololu_fd,3,127);
	//waitForKey();
	//pololuSetPosition7Bit(pololu_fd,0,63);
	//pololuSetPosition7Bit(pololu_fd,3,63);
	//waitForKey();




	exit( 0 );
} /* end main() */




//----------------------- FUNCTIONS -------------------------------


// PID FUNCTIONS

// SETS TARGETS
// COULD BECOME AN ARGUMENT TO PID_ITERATION
void pid_init()
{
	float accel[3];
	float ang_rate[3];
	float roll, pitch, yaw;
	//waitForKey();


	// lets try a loop
	// WE WILL READ FROM THE IMU UNTIL WE GET A GOOD VALUE
	// Should only loop through once but we will see. -am
	int count = 0;
	int r1 = -1;

	while ( ( r1 != IMU_LENGTH_31 ) && ( count <= 10 ) ) {
		r1 = mstrain_euler_vectors( imu_fd, &pitch, &roll, &yaw, accel, ang_rate );
		// check result

		if ( r1 != IMU_LENGTH_31 ) {
			fprintf( stderr, "DEBUG: imu_euler_vectors failed in pid_init, r1=%d\n", r1 );
			logf_error( default_log, main_test_log_file, ERROR_IO, "imu_euler_vectors failed, status, %d", r1 );
			count++;
		}
	}

	// if count>=10 we failed to read the imu for some reason
	if ( count > 10 ) {
		fprintf( stderr, "DEBUG: failed to initialize the pid." );
		logf_fatal_error( default_log, main_test_log_file, ERROR_IO, "Failed to initialize the PID" );
		exit( 1 );
	}

	// set the global targets
	target_roll = 0.0; //roll;

	target_yaw = 0.0; //yaw;

	// to fix the whole +-180 pitch problem
	if ( pitch < 0 ) pitch = pitch + 360.0;

	target_pitch = 190.0; //pitch;

	// lets even set a target_pressure
	// need to add checks to this -AM 12/22/08
	query_labjack();

	target_pressure = getBatteryVoltage( AIN_2 );



	printf( "PID is initialized, target_roll = %f, target_pitch = %f.\n", target_roll, target_pitch );

	logf_info( default_log, main_test_log_file, "PID is initialized, target_roll, %f, target_pitch, %f", target_roll, target_pitch );

	//waitForKey();
}

// THIS IS ONE PID ITERATION
void pid_iteration( pid_struct *pid )
{

	// results for controlVertical and controlVoiths
	float vertical_thrust = 0;
	float roll_torque = 0;
	float pitch_torque = 0;
	float voith_thrust = 0;
	float thrust_angle = 0;
	float thrust = 80;
	float yaw_torque = 0;

	//float kp_pressure=4500.0;
	//float kp_pressure=0.0;
	//float t_pressure=0.47;
	int r1 = -1;
	int r2 = -1;

	// PID EQUATIONS HERE
	roll_torque = ( pid->roll.kp ) * ( pid->roll.e ) + ( pid->roll.kd ) * ( pid->roll.de ) + ( pid->roll.ki ) * ( pid->roll.se );

	pitch_torque = ( pid->pitch.kp ) * ( pid->pitch.e ) + ( pid->pitch.kd ) * ( pid->pitch.de ) + ( pid->pitch.ki ) * ( pid->pitch.se );

	yaw_torque = ( pid->yaw.kp ) * ( pid->yaw.e ) + ( pid->yaw.kd ) * ( pid->yaw.de ) + ( pid->yaw.ki ) * ( pid->yaw.se );

	vertical_thrust = ( pid->pressure.kp ) * ( pid->pressure.e ) + ( pid->pressure.kd ) * ( pid->pressure.de ) + ( pid->pressure.ki ) * ( pid->pressure.se );

	// check bounds

	if ( absolute_value( roll_torque ) > BOUND_ROLL_TORQUE ) {
		logf_error( default_log, main_test_log_file, ERROR_GENERAL, "roll_torque out of bounds, roll_torque, %f", roll_torque );
		roll_torque = sign_value( roll_torque ) * BOUND_ROLL_TORQUE;
	}

	if ( absolute_value( pitch_torque ) > BOUND_PITCH_TORQUE ) {
		logf_error( default_log, main_test_log_file, ERROR_GENERAL, "pitch_torque out of bounds, pitch_torque, %f", pitch_torque );
		pitch_torque = sign_value( pitch_torque ) * BOUND_PITCH_TORQUE;
	}

	if ( absolute_value( yaw_torque ) > BOUND_YAW_TORQUE ) {
		logf_error( default_log, main_test_log_file, ERROR_GENERAL, "yaw_torque out of bounds, yaw_torque, %f", yaw_torque );
		yaw_torque = sign_value( yaw_torque ) * BOUND_YAW_TORQUE;
	}

	if ( absolute_value( vertical_thrust ) > BOUND_VERTICAL_THRUST ) {
		logf_error( default_log, main_test_log_file, ERROR_GENERAL, "vertical_thrust out of bounds, vertical_thrust, %f", vertical_thrust );
		vertical_thrust = sign_value( vertical_thrust ) * BOUND_VERTICAL_THRUST;
	}

	// need to check another bound here, abs(vertical_thrust)+abs(roll_torque)>BOUND_TOTAL_VERTICAL_THRUST
	// the sum of the absolute values of the two = 100 for full thrust on one both motors
	// we are going to bound that to BOUND_TOTAL_VERTICAL_THRUST
	if ( ( absolute_value( vertical_thrust ) + absolute_value( roll_torque ) ) > BOUND_TOTAL_VERTICAL_THRUST ) {
		logf_error( default_log, main_test_log_file, ERROR_GENERAL, "vertical_thrust out of bounds, vertical_thrust, %f", vertical_thrust );
		// bound by limiting the vertical thrust -am
		// without limiting: if roll_torque is 80 and vertical_thrust is 80 you would get 80% on one motor and 0% on another
		// with limiting: on vertical thrust you would get -80% on one motor and +80% on the other
		// we could split the difference and take 1/2 of the difference out of each
		vertical_thrust = BOUND_TOTAL_VERTICAL_THRUST - absolute_value( roll_torque );
	}


	// debug message to screen
	printf( "IMU Results\nRoll: %f Pitch: %f Yaw: %f\n", pid->roll.c, pid->pitch.c, pid->yaw.c );

	printf( "dRoll: %f dPitch: %f dYaw: %f\n", pid->roll.de, pid->pitch.de, pid->yaw.de );

	printf( "PID Results\ntarget roll: %f roll_torque: %f\ntarget pitch: %f pitch_torque: %f\ntarget yaw: %f yaw_torque: %f\ntarget pressure: %f current_pressure: %f vertical_thrust: %f\n",
	        pid->roll.ref, roll_torque, pid->pitch.ref, pitch_torque, pid->yaw.ref, yaw_torque, pid->pressure.ref, pid->pressure.c, vertical_thrust );

	// CONTROL VERTICAL
	r1 = controlVertical( pololu_fd, vertical_thrust, roll_torque, pitch_torque );

	if ( !r1 ) {
		fprintf( stderr, "DEBUG: controlVertical failed, r1=%d\n", r1 );
		logf_error( default_log, main_test_log_file, ERROR_IO, "controlVertical failed, status, %d", r1 );
	}

	// CONTROL CONTROL VOITHS
	r2 = controlVoiths( pololu_fd, voith_thrust, thrust_angle, thrust, yaw_torque );

	if ( !r2 ) {
		fprintf( stderr, "DEBUG: controlVertical failed, r2=%d\n", r2 );
		logf_error( default_log, main_test_log_file, ERROR_IO, "controlVertical failed, status, %d", r1 );
	}

	// FOR TESTING
	// controlVoiths(pololu_fd, 40, 0, 80, 0);


}







// EXIT PROGRAM FUNCTIONS

// redirects ctrl-c (SIGINT) to exit(1)
void sigint_handler( int sig )
{
	printf( "SIGINT Received...\n" );
	exit( 0 );
}

void exit_handler( void )
{
	printf( "Main Test is now exiting...\n" );

	remove_timer();

	// sleep a little to make sure nothing is happening (100 ms)
	usleep( 200000 );

	// this closes the global file descriptors

	if ( pololu_fd >= 0 ) {
		printf( "Closing Pololu...\n" );

		pololuInitializeChannels( pololu_fd );
		// need a long sleep here for some reason that I don't understand -AM
		usleep( 200000 );
		close( pololu_fd );
	}

	if ( imu_fd >= 0 ) {
		printf( "Closing IMU...\n" );
		close( imu_fd );
	}

	// Close the labjack
	// cleanup the connection
	printf( "Closing Labjack...\n" );

	close_labjack();


	printf( "Closing log file...\n" );

	logf_debug( default_log, main_test_log_file, "main_test finished" );

	logf_cleanup( default_log );

	logf_cleanup( pid_log );

	printf( "Please remember to open the kill switch.\n" );
}




// TIMER FUNCTIONS
void install_timer()
{
	// We are going to schedule a function to execute at specific intervals using signals
	// Timer data structures

	struct sigaction sa;

	struct itimerval timer;

	// Install timer_handler as the signal handler for SIGVTALRM.
	memset( &sa, 0, sizeof( sa ) );
	sa.sa_handler = &timer_handler;
	sigaction( SIGALRM, &sa, NULL );
	/* Configure the timer to expire after 50 msec... */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = TIMER_PERIOD;
	/* ... and every 50 msec after that. */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = TIMER_PERIOD;
	/* Start a virtual timer. It counts down whenever this process is
	executing. */
	setitimer( ITIMER_REAL, &timer, NULL );
	printf( "Timer installed...\n" );
}

void remove_timer()
{
	// This is just the opposite of installing the timer
	// Timer data structures

	struct sigaction sa;

	struct itimerval timer;

	// Remove timer_handler as the signal handler for SIGVTALRM.
	memset( &sa, 0, sizeof( sa ) );
	sa.sa_handler = SIG_DFL;
	sigaction( SIGALRM, &sa, NULL );
	/* Configure the timer to expire after 0 msec... */
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 0;
	/* ... and every 0 msec after that. */
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	/* Save the settings */
	setitimer( ITIMER_REAL, &timer, NULL );
	printf( "Timer removed...\n" );
}

void timer_handler( int signum )
{

	// this is the "beginning" for timing purposes
	// static because I want to also know actual time of timer period (we aren't in real time OS)
	// I think it needs to be initialized carefully -am
	static int schedule_number = 0;

	static struct timeval start_time = {0, 0};

	static struct timeval stop_time = {0, 0};

	static struct timeval duration_time = {0, 0};
	static double period = 0.0;  // in microseconds
	static double duration = 0.0; // in microseconds
	static double time_margin = 0.0; // in microseconds

	// labjack struct is initialized here
	// for now we just set them to realistic values
	// I think it needs to be initialized carefully -am
	// {bat1, bat2, pressure, water} in volts
	static labjack_struct labjack_status = {0.0, 0.0, 0.0, 0.0};

	// PID struct is initialized here
	// for now we just set them to temporary values
	// I think it needs to be initialized carefully -am
	// gains from first water test 12/21/08 -am
	//  { t_roll, kp_roll, kd_roll, t_pitch, kp_pitch, kd_pitch, t_pressure, kp_pressure, ki_pressure, current_pressure, current_period }
	// {    0.0,    -2.0,    -0.5,     0.0,      2.0,     0.0,       0.47,      4500.0,         0.0,              0.0,            0.0 };
	static pid_struct pid_data = {{0}, {0}, {0}, {0}, 0.0}; // all zero to begin with
	// see further down for initialization


	// figure out period, approximately
	// We need to track more than one period to get a better approximation.
	gettimeofday( &start_time, NULL );
	period = duration + ( start_time.tv_sec - stop_time.tv_sec ) * 1000000 + ( start_time.tv_usec - stop_time.tv_usec );
	// check period to track down bug

	if ( period < 0 ) {
		logf_debug( default_log, main_test_log_file, "NEGATIVE PERIOD, start, %f, %f, stop, %f, %f, period, %f", start_time.tv_sec, start_time.tv_usec, stop_time.tv_sec, stop_time.tv_usec );
		// this at least shouldn't harm our calculations too much -am
		// now it only happens on the first iteration!
		period = 0.0;
	}

	// prints a bunch of newlines
	printf( SCREEN_FORMAT_STRING );

	printf( "\nSIGALRM received, period = %f us.\n", period );

	// schedule number decides what actions are taken
	// 0-99 is range of schedule number
	// translates to 20Hz-1/5Hz
	// vary the numbers on the right
	// try not to have too many things happening at once
	// Schedule
	// SN % 100 == 0 -> 1/5Hz
	// SN % 50 == 1 -> 1/2.5Hz
	// SN % 20 == 2 -> 1Hz
	// SN % 10 == 3 -> 2Hz
	// SN % 5 == 4 -> 4Hz
	// pid_iteration 20Hz
	// read_labjack 4Hz
	// write_log 1/5Hz or 1/2.5 Hz


	// insert scheduled code here
	// generally slowest at the top, fastest at the bottom

	// 1/5 Hz Sample Video Frames to front.ppm and bottom.ppm
	if ( schedule_number % 100 == 0 ) {
		sample_video_frames();
		logf_debug( default_log, main_test_log_file, "sampled video frames", period );
	}

	// ********* SAMPLE SENSORS 4 Hz ********
	if ( schedule_number % 5 == 4 ) {
		updateLabjack( &labjack_status );
		logf_info( default_log, main_test_log_file, "period, %f, AIN0, %f, AIN1, %f, AIN2, %f, AIN3, %f", period, labjack_status.battery1, labjack_status.battery2, labjack_status.pressure, labjack_status.water );

		// check for a leak here

		if ( labjack_status.water > WATER_THRESHOLD ) {
			water_leak_handler();
		}

		// check battery voltages here
		// INSERT CODE
		//printf("Labjack: AIN0 = %f, AIN1 = %f, AIN2 = %f, AIN3 = %f\n",labjack_status.battery1,labjack_status.battery2,labjack_status.pressure,labjack_status.water);
	}

	// *********** PID 20 Hz ****************

	// SAMPLE IMU FIRST
	// update targets
	// read from the imu, this could be moved to out to timer_handler
	float accel[3];

	float ang_rate[3];

	int r1 = mstrain_euler_vectors( imu_fd, &pid_data.pitch.c, &pid_data.roll.c, &pid_data.yaw.c, accel, ang_rate );

	if ( r1 != IMU_LENGTH_31 ) {

		fprintf( stderr, "DEBUG: imu_euler_vectors failed, r1=%d, PID SKIPPED!\n", r1 );
		logf_error( default_log, main_test_log_file, ERROR_IO, "imu_euler_vectors failed, status, %d", r1 );

	}
	else {

		// IMU reading is good so do the PID

		// fix the whole +-180 problem with pitch
		if ( pid_data.pitch.c < 0 ) pid_data.pitch.c = pid_data.pitch.c + 360.0;

		// get the targets from the globals but could also be assigned constants here
		// fill up the pid struct with updated values
		// INSERT PID CONSTANTS HERE FOR NOW
		pid_data.roll.ref = target_roll;

		pid_data.roll.kp = 8.0;  //4.0

		pid_data.roll.kd = -60.0; //15.0*1.2/2.25; //-40.0; //1.0;

		pid_data.roll.ki = 4.0; //15.0*0.6*2.25/8.0; //0.75;

		pid_data.roll.e = pid_data.roll.c - pid_data.roll.ref;

		pid_data.roll.de = ang_rate[1];

		pid_data.roll.se = pid_data.roll.se + pid_data.roll.e * period / 1000000;

		pid_data.roll.se = bound_integral( pid_data.roll.se, pid_data.roll.ki, BOUND_ROLL_INTEGRAL );

		pid_data.pitch.ref = target_pitch;

		pid_data.pitch.kp = -4.0;

		pid_data.pitch.kd = -40.0;

		pid_data.pitch.ki = -1.0;

		pid_data.pitch.e = pid_data.pitch.c - pid_data.pitch.ref;

		pid_data.pitch.de = ang_rate[0];

		pid_data.pitch.se = pid_data.pitch.se + pid_data.pitch.e * period / 1000000;

		pid_data.pitch.se = bound_integral( pid_data.pitch.se, pid_data.pitch.ki, BOUND_PITCH_INTEGRAL );

		//printf("Integral term: %f\n",pid_data.pitch.se);
		pid_data.yaw.ref = target_yaw;

		pid_data.yaw.kp = 0.0;

		pid_data.yaw.kd = 0.0;

		pid_data.yaw.ki = 0.0;

		pid_data.yaw.e = pid_data.yaw.c - pid_data.yaw.ref;

		pid_data.yaw.de = ang_rate[2];

		pid_data.yaw.se = pid_data.yaw.se + pid_data.yaw.e * period / 1000000;

		pid_data.yaw.se = bound_integral( pid_data.yaw.se, pid_data.yaw.ki, BOUND_YAW_INTEGRAL );

		//pid_data.pressure.ref=0.47;
		pid_data.pressure.ref = target_pressure;

		pid_data.pressure.kp = 0.0; //-4500.0;

		pid_data.pressure.kd = 0.0;

		pid_data.pressure.ki = 0.0; //-1000.0;

		pid_data.pressure.e = pid_data.pressure.c - pid_data.pressure.ref;

		pid_data.pressure.de = 0.0; // implement later

		pid_data.pressure.se = pid_data.pressure.se + pid_data.pressure.e * period / 1000000;

		pid_data.pressure.c = labjack_status.pressure;

		pid_data.pressure.se = bound_integral( pid_data.pressure.se, pid_data.pressure.ki, BOUND_PRESSURE_INTEGRAL );

		pid_data.c_period = period / 1000000;   // in seconds

		pid_iteration( &pid_data );

		// log pid at 4 Hz
		if ( schedule_number % 5 == 3 ) {
			logf_info( pid_log, pid_log_file, "eRoll, %f, ePitch, %f, eYaw, %f, ePressure, %f", pid_data.roll.e, pid_data.pitch.e, pid_data.yaw.e, pid_data.pressure.e );
		}
	}

	// ********** END SCHEDULED CODE **********

	schedule_number = ( schedule_number + 1 ) % 100;

	// I used this line to test a worst case scenario where the timer_handler takes 1 second to run! AM
	// sleep(1);

	// this is the "end" for timing purposes
	// duration is approximate duration of the "custom code" or pid_iteration
	// time margin is the margin of time remaining in our timer period
	// time margin<0 means we need to slow down our PID timer (BAD)
	// NOTE that once timer_handler is executed SIGALRMS will be masked until timer_handler is done
	// That means that if our timer period is too short some of the iterations will be skipped
	// This will divide our pid frequency
	gettimeofday( &stop_time, NULL );

	duration_time.tv_sec = stop_time.tv_sec - start_time.tv_sec;

	duration_time.tv_usec = stop_time.tv_usec - start_time.tv_usec;

	duration = duration_time.tv_sec * 1000000 + duration_time.tv_usec;

	time_margin = TIMER_PERIOD - duration;

	printf( "Timer handler is finished, duration = %f us, time margin = %f us.\n", duration, time_margin );

}

// Utility Functions
void waitForKey()
{
	printf( "Press Enter to continue...\n" );
	getchar();
	return;
}

void updateLabjack( labjack_struct *current )
{
	query_labjack();
	current->battery1 = getBatteryVoltage( AIN_0 );
	current->battery2 = getBatteryVoltage( AIN_1 );
	current->pressure = getBatteryVoltage( AIN_2 );
	current->water = getBatteryVoltage( AIN_3 );
}

void water_leak_handler()
{
	// does not query the labjack only uses current value
	logf_fatal_error( default_log, main_test_log_file, ERROR_WATER_LEAK, "WATER LEAK DETECTED, AIN3, %f", getBatteryVoltage( AIN_3 ) );
	printf( "\nWARNING\nWATER LEAK!!!\n" );
	exit( 1 );
}

// Logging functions
/* Define our custom error functions */
void call_me_if_error( void )
{
	// printf("Logging Error!\n");
	// don't do anything
}

void call_me_if_threshold_reached( void )
{

	printf( "Logging threshold tripped!\n" );

	// you must manually log the errors here!
	logf_info(
	    default_log,
	    main_test_log_file,
	    "THRESHOLD TRIPPED"
	);
	// threshold is automatically reset
}

float absolute_value( float x )
{
	if ( x < 0 ) return -x;

	return x;
}

float sign_value( float x )
{
	if ( x < 0.0 ) {
		return -1.0;
	}
	else {
		return 1.0;
	}
}

float bound_integral( float value, float gain, float bound )
{
	if ( absolute_value( value*gain ) > bound ) {
		return sign_value( value )*absolute_value( bound / gain );
	}
	else {
		return value;
	}
}

// magic happens here
int sample_video_frames()
{
	int l;
	time_t now;
	now = time( NULL );
	char front_buffer[100] = "pics/";
	char bottom_buffer[100] = "pics/";
	strcat( front_buffer, ctime( &now ) );
	l = ( int )strlen( front_buffer );
	front_buffer[l-1] = 0;
	strcat( front_buffer, " front.ppm" );
	strcat( bottom_buffer, ctime( &now ) );
	l = ( int )strlen( bottom_buffer );
	bottom_buffer[l-1] = 0;
	strcat( bottom_buffer, " bottom.ppm" );

	int child_status_front;
	char* arg_list_front[] = {"v4lctl", "-c", "/dev/video0", "snap", "ppm", "640x400", NULL, NULL};
	char* arg_list_bottom[] = {"v4lctl", "-c", "/dev/video1", "snap", "ppm", "640x400", NULL, NULL};
	arg_list_front[6] = front_buffer;
	arg_list_bottom[6] = bottom_buffer;

	pid_t child_pid_front;
	child_pid_front = fork();

	if ( child_pid_front != 0 ) {
		//parent
	} else {

		// child
		execvp( "v4lctl", arg_list_front );
		fprintf( stderr, "DEBUG: streamer failed to execute\n" );
		abort();
	}

	wait( &child_status_front );

	int child_status_bottom;
	pid_t child_pid_bottom;
	child_pid_bottom = fork();

	if ( child_pid_bottom != 0 ) {
		//parent
	} else {

		// child
		execvp( "v4lctl", arg_list_bottom );
		fprintf( stderr, "DEBUG: streamer failed to execute\n" );
		abort();
	}

	wait( &child_status_bottom );

	//front_buffer=read_file("/pics/front.grey");
	//bottom_buffer=read_file("/pics/bottom.grey");

	return 1;
}


char* read_file( char *name )
{
	FILE *file;
	char *buffer;
	unsigned long fileLen;

	//Open file
	file = fopen( name, "rb" );

	if ( !file ) {
		fprintf( stderr, "DEBUG: read_file, unable to open file %s", name );
		return NULL;
	}

	//Get file length
	fseek( file, 0, SEEK_END );

	fileLen = ftell( file );

	fseek( file, 0, SEEK_SET );

	//Allocate memory
	buffer = ( char * )malloc( fileLen + 1 );

	if ( !buffer ) {
		fprintf( stderr, "DEBUG: read_file, memory error!" );
		fclose( file );
		return NULL;
	}

	//Read file contents into buffer
	fread( buffer, fileLen, 1, file );

	fclose( file );

	//Do what ever with buffer
	return buffer;

	//free(buffer);
}


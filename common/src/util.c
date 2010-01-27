/******************************************************************************
 *
 *  Title:        util.c
 *
 *  Description:  Utility functions.
 *
 *****************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "util.h"


/******************************************************************************
 *
 * Title:       int util_calc_dt(int *time1s, int *time1ms,
 *                           		int *time2s, int *time2ms)
 *
 * Description: Calculate the difference between two times.
 *
 * Input:       time1s: First time [seconds].
 *              time1ms: First time [microseconds].
 *              time2s: Second time [seconds].
 *              time2ms: Second time [microseconds].
 *
 * Output:      dt: The time difference in microseconds.
 *
 *****************************************************************************/

int util_calc_dt(int *time1s, int *time1ms, int *time2s, int *time2ms)
{
	int dt;
	int time1;
	int time2;

	time1 = (*time1s * 1000000) + *time1ms;
	time2 = (*time2s * 1000000) + *time2ms;
	dt = time1 - time2;

	return dt;
} /* end util_calc_dt() */


/******************************************************************************
 *
 * Title:       void util_print_help()
 *
 * Description: Prints instructions on program usage.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/

void util_print_help(int app)
{
	switch (app) {
		case STINGRAY:
			printf("PRINT_HELP: To specify a configuration file use the "
			        "command\n'./uuv -c <filename>'.\nUse './uuv -h' for more "
			        "help.\n"
			     );

			break;

		case GUI:
			printf("PRINT_HELP: To specify a configuration file use the "
			        "command\n'./client -c <filename>'.\nUse './client -h' for "
			        "more help.\n"
			     );

			break;
	}

	printf("\n");
} /* end print_help() */


/******************************************************************************
 *
 * Title:       void util_print_config_help()
 *
 * Description: Prints instructions on configuration options.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 *****************************************************************************/

void util_print_config_help()
{
	printf("PRINT_CONFIG_HELP:\n"
	        "Conifuration file values can be set for:\n"
	        "client ip ....... IP address of client computer.\n"
	        "client port ..... Port of client computer.\n"
	        "server ip ....... IP address of server computer.\n"
	        "server port ..... Port of server computer.\n"
	        "capture radius .. Used to determine if a waypoint has been\n"
	        "                  reached in [m].\n"
	        "window height ... Height of GUI window [pixels].\n"
	        "window width .... Width of GUI window [pixels].\n"
	        "debug level ..... Changes the verbosity of the debug messages\n"
	        "                  printed to screen during operation.\n"
	        "enable api ...... UUV code will send data to clients when\n"
	        "                  requested.\nUse 0 or 1.\n"
	        "api clients ..... Maximum number of clients that can request\n"
	        "                  data\n"
	        "                  from the UUV using the API commands.\n"
	        "enable logs ..... UUV will log sensor and command data to a\n"
	        "                  file.\n"
	        "                  Use 0 or 1.\n"
	        "enable imu ...... UUV will use the IMU.\n"
	        "                  Use 0 or 1.\n"
	        "imu port ........ The serial port for the IMU.\n"
	        "                  For example, /dev/ttyUSB0\n"
	        "imu baud ........ Serial port baud rate for the IMU.\n"
	        "imu stab ........ Sets whether to get instantaneous or gyro\n"
	        "                  stabilized data from the IMU.\n"
	        "                  Use 0 for instantaneous or 1 for gyro\n"
	        "                  stabilized.\n"
	        "enable ssc ...... UUV will use serial servo controller.\n"
	        "                  Use 0 or 1.\n"
	        "ssc port ........ The serial port for the SSC.\n"
	        "                  For example, /dev/ttyUSB1\n"
	        "ssc baud ........ Serial port baud rate for the SSC.\n"
	        "kp yaw .......... Proportional gain term for the yaw PID\n"
	        "                  controller.\n"
	        "ki yaw .......... Integral gain term for the yaw PID controller.\n"
	        "kd yaw .......... Derviative gain term for the yaw PID\n"
	        "                  controller.\n"
	        "pid dt .......... Time step for the PID controller.\n"
	     );

	printf("\n");
} /* end util_print_config_help() */


/******************************************************************************
 *
 * Title:       float util_sign_value(float value)
 *
 * Description: Determines sign of value.
 *
 * Input:       value: Value to check sign of.
 *
 * Output:      Either 1 or -1 depeding on sign.
 *
 *****************************************************************************/

float util_sign_value(float value)
{
	if(value < 0.0) {
		return -1.0;
	}
	else {
		return 1.0;
	}
} /* end util_sign_value() */


/******************************************************************************
 *
 * Title:       float util_fequals(float value1 , float value2)
 *
 * Description: Determines whether or not two floats are equal
 *
 * Input:       value1: First value to compare.
 * 				value2: Second value to compare
 *
 * Output:      TRUE or FALSE depending on equality or not respectively.
 *
 *****************************************************************************/

float util_fequals(float value1 , float value2)
{
	if(fabsf(value1 - value2) < UTIL_FEQUALS_EPSILON)
		return TRUE;

	return FALSE;

} /* end util_sign_value() */

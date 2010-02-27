/******************************************************************************
 * file: battery_sensor.c
 * desc:
 * date: nov 21, 2008
 * auth: chris boynton <bboynton@ucsd.edu>
 *****************************************************************************/

#include "labjack.h"

extern float ain0;
extern float ain1;
extern float ain2;
extern float ain3;

float getBatteryVoltage( uint batteryNum )
{
	float batteryVoltage = 0.0f;

	switch ( batteryNum ) {

		case AIN_0:
			return ain0;
			break;

		case AIN_1:
			return ain1;
			break;

		case AIN_2:
			return ain2;
			break;

		case AIN_3:
			return ain3;
			break;

		default:
			break;
	}

	return batteryVoltage;
}


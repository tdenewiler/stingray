/**
 *  \file labjack.h
 *  \brief handles analog and digital I/O with the LabJack device
 */

#ifndef _LABJACK_H_
#define _LABJACK_H_

#ifndef LABJACK_VALS
#define LABJACK_VALS
#define BATTERY_1 0
#define BATTERY_2 1
#define AIN_0 0 //!< Motor battery.
#define AIN_1 1 //!< Electronics battery.
#define AIN_2 2 //!< Depth (pressure) sensor.
#define AIN_3 3 //!< Water leak sensor.
#endif /* LABJACK_VALS */

#ifndef _LABJACK_DATA_
#define _LABJACK_DATA_

typedef struct _LABJACK_DATA {
	float battery1;
	float battery2;
	float pressure;
	float water;
} LABJACK_DATA;

#endif /* _LABJACK_DATA_ */


typedef unsigned int uint;

int init_labjack();
int query_labjack();
void close_labjack();

float getBatteryVoltage( uint batteryNum );
float getDepth();
void setupLabjackLog( char *filePathName );
void logLabjack();
int checkWaterSensor();

#endif /* _LABJACK_H_ */

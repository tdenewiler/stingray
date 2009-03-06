#include <stdio.h>
#include <unistd.h>
#include "labjack.h"

#define WATER_THRESHOLD 2.0

int main()
{
	printf( "Labjack Test Program\n" );

	// if the labjack was initialized

	if ( init_labjack() ) {
		printf( "Labjack is initialized.\n" );
		// output the voltages of the batteries

		while ( 1 ) {
			// request an update from the labjack
			query_labjack();
			printf( "\nAIN 0: %f (V)\n", getBatteryVoltage( AIN_0 ) );
			printf( "AIN 1: %f (V)\n", getBatteryVoltage( AIN_1 ) );
			printf( "AIN 3: %f (V)\n", getBatteryVoltage( AIN_2 ) );
			printf( "AIN 4: %f (V)\n", getBatteryVoltage( AIN_3 ) );

			if ( getBatteryVoltage( AIN_3 ) > WATER_THRESHOLD ) {
				printf( "\nWARNING WATER DETECTED!!!\n" );
			}

			sleep( 1 );
		}
	}

	// cleanup the connection
	close_labjack();

	return 0;
} /* end main() */


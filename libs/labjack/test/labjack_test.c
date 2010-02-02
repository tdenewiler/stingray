#include <stdio.h>
#include <unistd.h>
#include "labjack.h"

#define WATER_THRESHOLD 2.0

int main()
{
	printf("MAIN: Labjack Test Program\n");

	/// Initialize the labjack.
	if (init_labjack()) {
		printf("MAIN: Labjack is initialized.\n");
		/// Output the voltages of the batteries.
		while (1) {
			/// Request an update from the labjack.
			query_labjack();
			printf("MAIN: AIN 0 %f (V)\n", getBatteryVoltage(AIN_0));
			printf("MAIN: AIN 1 %f (V)\n", getBatteryVoltage(AIN_1));
			printf("MAIN: AIN 3 %f (V)\n", getBatteryVoltage(AIN_2));
			printf("MAIN: AIN 4 %f (V)\n", getBatteryVoltage(AIN_3));

			/// Check for a water leak.
			if (getBatteryVoltage(AIN_3) > WATER_THRESHOLD) {
				printf("MAIN: Warning!!! Water IS detected.\n");
			}
			else {
				printf("MAIN: Water not detected.\n");
			}

			sleep(1);
		}
	}
	else {
		printf("MAIN: WARNING!!! Labjack not initialized.\n");
	}

	/// Cleanup the connection.
	close_labjack();

	return 0;
} /* end main() */


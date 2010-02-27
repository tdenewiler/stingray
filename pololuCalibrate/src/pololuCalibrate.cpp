/******************************************************************************
 *
 *  Title:        pololuCalibrate.c
 *
 *  Description:  Main program for calibrating the Pololu motor controller.
 * 				  Sets the neutral position for the controllers to 63 for the
 * 				  Electronic Servo Controllers. The number 63 was found by
 * 				  127 / 2 = 63. Channels 0 and 3 are the motors for the Voiths.
 *
 *****************************************************************************/


#include <stdio.h>
#include <string.h>
#include "pololu.h"
#include "labjack.h"


typedef struct {
	float battery1;
	float battery2;
	float pressure;
	float water;
} labjack_struct;


void waitForKey() {
	printf("Press a key when ready...\n");
	getchar();
}


void updateLabjack(labjack_struct *current)
{
	query_labjack();
	current->battery1 = getBatteryVoltage(AIN_0);
	current->battery2 = getBatteryVoltage(AIN_1);
	current->pressure = getBatteryVoltage(AIN_2);
	current->water = getBatteryVoltage(AIN_3);
}


int main(void)
{
	/* Create variables. */
	int pololu_fd = -1;
	char pololu_port[25];
	strncpy(pololu_port, "/dev/ttyUSB0", 25);
	labjack_struct lj;

	// this is all divided into steps
	/*
	printf("Pololu Calibrate\n");
	printf("We are going to do the voiths first then the thrusters.\n");
	printf("Start with the Stingray out of the water and the battery box open.\n");
	printf("Close any conflicting programs and open kill switch.\n");
	waitForKey();
	*/
	// Set up the Pololu port.
	pololu_fd = pololu_setup(pololu_port, 57600);
	// Initialize all the channels.
	//rintf("Initializing all channels.\n");
	//pololuInitializeChannels(pololu_fd);

	/*
	// Send full throttle commands to channels 0 and 3.
	printf("Sending full throttle commands.\n");
	pololuSetPosition7Bit(pololu_fd,0,127);
	pololuSetPosition7Bit(pololu_fd,3,127);

	//printf("Reconnect the single red wire to the MIDDLE pin in the 7th row of the Pololu.\n");
	printf("Now close the kill switch.\n");
	printf("Wait for 2 beeps that signify the full throttle position is set before you press a key.\n");
	printf("You must press a key to continue within 10 seconds of the 2 beeps.\n");
	waitForKey();

	// Set the desired neutral position for channels 0 and 3.
	printf("Sending neutral throttle commands.\n");
	pololuSetPosition7Bit(pololu_fd,0,63);
	pololuSetPosition7Bit(pololu_fd,3,63);

	printf("Wait for 4 beeps that signify the neutral thottle position is set before you press a key.\n");
	printf("You must press a key to continue within 10 seconds of the 2 beeps.\n");
	waitForKey();
	*/


	// This isn't working perfectly yet - AM
	/*
	printf("We are done calibrating the voiths.  Now we calibrate the thrusters.\n");

	printf("Open the kill switch.\n");
	printf("Disconnect the single red wire attached to middle pin in the 7th row of the Pololu.\n");
	waitForKey();

	printf("Initializing all channels.\n");
	pololuInitializeChannels(pololu_fd);

	printf("Now close the kill switch.\n");
	waitForKey();

	printf("Sending full throttle commands.\n");

	pololuSetPosition7Bit(pololu_fd,7,127);
	waitForKey();
	pololuSetPosition7Bit(pololu_fd,8,127);
	waitForKey();
	pololuSetPosition7Bit(pololu_fd,9,127);
	waitForKey();

	printf("Reconnect the single red wire to the MIDDLE pin in the 7th row of the Pololu.\n");
	printf("Wait for multiple tones that signify the full throttle position is set before you press a key.\n");
	printf("You must move quickly for this to work.\n");
	waitForKey();

	printf("Sending full reverse throttle commands.\n");
	pololuSetPosition7Bit(pololu_fd,7,0);
	pololuSetPosition7Bit(pololu_fd,8,0);
	pololuSetPosition7Bit(pololu_fd,9,0);

	printf("Wait for multiple tones that signify the full reverse throttle position is set before you press a key.\n");
	printf("You must move quickly for this to work.\n");
	waitForKey();

	printf("Sending neutral throttle commands.\n");
	pololuSetPosition7Bit(pololu_fd,7,63);
	pololuSetPosition7Bit(pololu_fd,8,63);
	pololuSetPosition7Bit(pololu_fd,9,63);

	printf("Wait for multiple tones followed by two arming tones that signify the neutral throttle position is set before you press a key.\n");
	printf("You must move quickly for this to work.\n");
	waitForKey();
	*/

	// test the vertical thrusters
	//printf("Open the kill switch.\n");
	//printf("Disconnect the single red wire attached to middle pin in the 7th row of the Pololu.\n");
	//waitForKey();

	/*
	printf("Initializing all channels.\n");
	pololuInitializeChannels(pololu_fd);

	printf("Now close the kill switch.\n");
	waitForKey();


	printf("Vertical thrusters up.\n");
	pololuSetPosition7Bit(pololu_fd, 7, 70);
	pololuSetPosition7Bit(pololu_fd, 8, 70);
	pololuSetPosition7Bit(pololu_fd, 10, 70);
	waitForKey();

	printf("Vertical thrusters down.\n");
	pololuSetPosition7Bit(pololu_fd, 7, 56);
	pololuSetPosition7Bit(pololu_fd, 8, 56);
	pololuSetPosition7Bit(pololu_fd, 10, 56);
	waitForKey();

	printf("Vertical thrusters neutral.\n");
	pololuSetPosition7Bit(pololu_fd, 7, 63);
	pololuSetPosition7Bit(pololu_fd, 8, 63);
	pololuSetPosition7Bit(pololu_fd, 10, 63);
	waitForKey();
	*/

	init_labjack();

	while (1) {
	updateLabjack(&lj);
	printf("lJ: %f, %f, %f, %f\n", lj.battery1, lj.battery2, lj.pressure, lj.water);
	waitForKey();
}

	return 0;
} /* end main() */

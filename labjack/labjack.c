/******************************************************************************
 * file: labjack.c
 * desc: handles analog and digital I/O with the LabJack device
 * date: nov 21, 2008
 * auth: chris boynton <bboynton@ucsd.edu>
 *****************************************************************************/

#include <unistd.h>
#include <termios.h>
#include "labjack.h"
#include "u3.h"

HANDLE hDevice;
u3CalibrationInfo caliInfo;

// voltages returned from the labjack
float ain0;
float ain1;
float ain2;
float ain3;

int config_labjack();

// initialize the labjack device
// returns 0 for initialization failure, 1 for success
int init_labjack()
{
	if ( ( hDevice = openUSBConnection( -1 ) ) == NULL )
		return 0;

	// how is the labjack pre-configured?
	if ( getCalibrationInfo( hDevice, &caliInfo ) < 0 ) {
		return 0;
	}

	// configure the labjack
	if ( config_labjack() != 0 ) {
		return 0;
	}

	// labjack was successfully initialized
	return 1;
}

// handles cleanup of the labjack and its connection
void close_labjack()
{
	// close the USB connection
	closeUSBConnection( hDevice );

	return;
}

//Sends a ConfigIO low-level command that configures the FIOs, DAC, Timers and
//Counters for this example
int config_labjack()
{
	uint8 sendBuff[12], recBuff[12];
	const char *fName = "config_labjack()";
	uint16 checksumTotal;
	int sendChars, recChars;
	uint8 timerCounterConfig, fioAnalog;

	//timerCounterConfig = 74;
	//fioAnalog = 3;
	timerCounterConfig = 64;
	fioAnalog = 255;

	sendBuff[1] = ( uint8 )( 0xF8 );  //Command byte
	sendBuff[2] = ( uint8 )( 0x03 );  //Number of data words
	sendBuff[3] = ( uint8 )( 0x0B );  //Extended command number

	sendBuff[6] = 5;  //Writemask : Setting writemask for timerCounterConfig (bit 0)
	//and FIOAnalog (bit 2)

	sendBuff[7] = 0;  //Reserved
	sendBuff[8] = timerCounterConfig;  //TimerCounterConfig
	sendBuff[9] = 0;  //DAC1 enable : not enabling, though could already be enabled.
	//If U3 hardware version 1.30, DAC1 is always enabled.
	sendBuff[10] = fioAnalog; //FIOAnalog
	sendBuff[11] = 0; //EIOAnalog : Not setting anything
	extendedChecksum( sendBuff, 12 );

	//Sending command to U3
	if ( ( sendChars = LJUSB_BulkWrite( hDevice, U3_PIPE_EP1_OUT, sendBuff, 12 ) ) < 12 ) {
		if ( sendChars == 0 )
			printf( "%s: write failed\n", fName );
		else
			printf( "%s: did not write all of the buffer\n", fName );

		return -1;
	}

	//Reading response from U3
	if ( ( recChars = LJUSB_BulkRead( hDevice, U3_PIPE_EP1_IN, recBuff, 12 ) ) < 12 ) {
		if ( recChars == 0 )
			printf( "%s: read failed\n", fName );
		else
			printf( "%s: did not read all of the buffer\n", fName );

		return -1;
	}

	checksumTotal = extendedChecksum16( recBuff, 12 );

	if ( ( uint8 )( ( checksumTotal / 256 ) & 0xff ) != recBuff[5] ) {
		printf( "%s: read buffer has bad checksum16(MSB)\n", fName );
		return -1;
	}
	if ( ( uint8 )( checksumTotal & 0xff ) != recBuff[4] ) {
		printf( "%s: read buffer has bad checksum16(LBS)\n", fName );
		return -1;
	}
	if ( extendedChecksum8( recBuff ) != recBuff[0] ) {
		printf( "%s: read buffer has bad checksum8\n", fName );
		return -1;
	}
	if ( recBuff[1] != ( uint8 )( 0xF8 ) || recBuff[2] != ( uint8 )( 0x03 ) || recBuff[3] != ( uint8 )( 0x0B ) ) {
		printf( "%s: read buffer has wrong command bytes\n", fName );
		return -1;
	}
	if ( recBuff[6] != 0 ) {
		printf( "%s: read buffer received errorcode %d\n", fName, recBuff[6] );
		return -1;
	}
	if ( recBuff[8] != timerCounterConfig ) {
		printf( "%s: TimerCounterConfig did not get set correctly\n", fName );
		return -1;
	}
	if ( recBuff[10] != fioAnalog && recBuff[10] != ( fioAnalog | ( 0x0F ) ) ) {
		printf( "%s: FIOAnalog(%d) did not set correctly\n", fName, recBuff[10] );
		return -1;
	}

	return 0;
}

//Calls a Feedback low-level call to read AIN0, AIN1, FIO7, Counter1(FIO6)
//and temperature.  Will work with U3 hardware versions 1.30 HV.
int query_labjack()
{
	uint8 sendBuff[20], recBuff[18];
	const char *fName = "query_labjack()";
	int sendChars, recChars;
	uint16 checksumTotal;
	double voltage;

	sendBuff[1] = ( uint8 )( 0xF8 );  //Command byte
	sendBuff[2] = 7;             // even number of data words (.5 word for echo(0), 6.5
	//words for IOTypes, a word is 2 bytes)
	sendBuff[3] = ( uint8 )( 0x00 );  //Extended command number

	sendBuff[6] = 0;	// echo begin

	sendBuff[7] = 1;    //IOType is AIN
	sendBuff[8] = 64;    //Positive channel (bits 0-4) is 0, LongSettling (bit 6)
	//is not set and QuickSample (bit 7) is not set
	sendBuff[9] = 30;   //Negative channel is 30 (for differential Special Range)

	sendBuff[10] = 1;   //IOType is AIN
	sendBuff[11] = 65;   //Positive channel (bits 0-4) is 1, LongSettling (bit 6)
	//is not set and QuickSample (bit 7) is not set
	sendBuff[12] = 30;  //Negative channel is 30 (for differential Special Range)

	sendBuff[13] = 1;   //IOType is AIN
	sendBuff[14] = 66;   //Positive channel (bits 0-4) is 3, LongSettling (bit 6)
	//is not set and QuickSample (bit 7) is not set
	sendBuff[15] = 30;   //Negative channel is 30 (for differential Special Range)

	sendBuff[16] = 1;   //IOType is AIN
	sendBuff[17] = 67;   //Positive channel (bits 0-4) is 4, LongSettling (bit 6)
	//is not set and QuickSample (bit 7) is not set
	sendBuff[18] = 30;   //Negative channel is 30 (for differential Special Range)

	sendBuff[19] = 0;	// echo end

	//sendBuff[19] = 10;  //IOType is BitStateRead
	//sendBuff[20] = 7;   //IO number is 7 (FIO7)

	//sendBuff[21] = 55;  //IOType is Counter1
	//sendBuff[22] = 0;   //Reset (bit 0) is not set

	//sendBuff[23] = 1;   //IOType is AIN
	//sendBuff[24] = 30;  //Positive channel is 30 (temp sensor)
	//sendBuff[25] = 31;  //Negative channel is 31 (SE)

	extendedChecksum( sendBuff, 20 );

	//Sending command to U3
	if ( ( sendChars = LJUSB_BulkWrite( hDevice, U3_PIPE_EP1_OUT, sendBuff, 20 ) ) < 20 ) {
		if ( sendChars == 0 )
			printf( "%s: write failed\n", fName );
		else
			printf( "%s: did not write all of the buffer\n", fName );

		return -1;
	}

	//Reading response from U3 (9 + data)
	if ( ( recChars = LJUSB_BulkRead( hDevice, U3_PIPE_EP1_IN, recBuff, 18 ) ) < 18 ) {
		if ( recChars == 0 ) {
			printf( "%s: read failed\n", fName );
			printf( "%s: got response of size %d containing |%s|\n", fName, recChars, recBuff );
			return -1;
		}
		else
			printf( "%s: did not read all of the expected buffer\n", fName );
	}

	if ( recChars < 10 ) {
		printf( "%s: response is not large enough\n", fName );
		return -1;
	}

	checksumTotal = extendedChecksum16( recBuff, recChars );

	if ( ( uint8 )( ( checksumTotal / 256 ) & 0xff ) != recBuff[5] ) {
		printf( "%s: read buffer has bad checksum16(MSB)\n", fName );
		return -1;
	}

	if ( ( uint8 )( checksumTotal & 0xff ) != recBuff[4] ) {
		printf( "%s: read buffer has bad checksum16(LBS)\n", fName );
		return -1;
	}

	if ( extendedChecksum8( recBuff ) != recBuff[0] ) {
		printf( "%s: read buffer has bad checksum8\n", fName );
		return -1;
	}

	if ( recBuff[1] != ( uint8 )( 0xF8 ) ||  recBuff[3] != ( uint8 )( 0x00 ) ) {
		printf( "%s: read buffer has wrong command bytes \n", fName );
		return -1;
	}

	if ( recBuff[6] != 0 ) {
		printf( "%s: received errorcode %d for frame %d ", fName, recBuff[6], recBuff[7] );

		switch ( recBuff[7] ) {

			case 1:
				printf( "(AIN0(SE))\n" );
				break;

			case 2:
				printf( "(AIN1(SE))\n" );
				break;

			case 3:
				printf( "(AIN2(SE))\n" );
				break;

			case 4:
				printf( "(AIN3(SE))\n" );
				break;

			case 5:
				printf( "(BitStateRead for FIO7)\n" );
				break;

			case 6:
				printf( "(Counter1)\n" );
				break;

			case 7:
				printf( "(Temp. Sensor\n" );
				break;

			default:
				printf( "(Unknown)\n" );
				break;
		}

		return -1;
	}

	binaryToCalibratedAnalogVoltage_hw130( &caliInfo, 0, 32, recBuff[9]  + recBuff[10]*256, &voltage );

	ain0 = voltage;
	binaryToCalibratedAnalogVoltage_hw130( &caliInfo, 1, 32, recBuff[11] + recBuff[12]*256, &voltage );
	ain1 = voltage;
	binaryToCalibratedAnalogVoltage_hw130( &caliInfo, 2, 32, recBuff[13] + recBuff[14]*256, &voltage );
	ain2 = voltage;
	binaryToCalibratedAnalogVoltage_hw130( &caliInfo, 3, 32, recBuff[15] + recBuff[16]*256, &voltage );
	ain3 = voltage;

	return 1;
}

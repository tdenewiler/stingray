/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

#define BOATNAMESTRLEN 64
#define MAXWAYPOINTS	128


#define NUMCMDS	12
#define CMDNAMESTRLEN 32

#ifdef NEED_CMDNAMES
	static char cmdnames[NUMCMDS][CMDNAMESTRLEN] = {
		"NULL",
		"halt",
		"waypoint",
		"pause",
		"resume",
		"interrupt",
		"uninterrupt",
		"manual",
		"calibrate",
		"stationkeep",
		"freedrift",
		"headinghold",
	};
#endif

	typedef enum {
		CMD_NULL,
		CMD_HALT,
		CMD_WPT,
		CMD_PAUSE,
		CMD_RESUME,
		CMD_INTERRUPT,
		CMD_UNINTERRUPT,
		CMD_MANUAL,
		CMD_CALIBRATE,
		CMD_STATIONKEEP,
		CMD_FREEDRIFT,
		CMD_HEADINGHOLD,
	}CMDTYPE;


	typedef struct {

		// boat identification
		int nPlatformID;
		char sPlatformName[BOATNAMESTRLEN];

		// command
		int nCmdID;
		char sCmdName[CMDNAMESTRLEN];

		// waypoints
		int nWaypointCount;
		WAYPT Waypoints[MAXWAYPOINTS];

		// manual controls
		int nRudder;
		int nThrottle;

		// heading control
		int nHeading;

	}COMMAND;



	COMMAND command_InitCommand();

	void command_PrintCommand( COMMAND c );
	void command_WriteString( char *outstr, int size, COMMAND cmd );

	int command_CompareCommand( COMMAND c1, COMMAND c2 );

#ifdef __cplusplus
}

#endif

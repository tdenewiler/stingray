/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	/*******************************/

//#define MODENAMESTRLEN 128
#define PLATFORMNAMESTRLEN 64
#define TIMESTRLEN 64
#define UNITSTRLEN 32

#define MAXWPTS 128


#define NUMSCIDAT	11
#define SCIDATNAMESTRLEN 32

#ifdef NEED_SCINAMES
	static char scidatnames[NUMSCIDAT][SCIDATNAMESTRLEN] = {
		"Humidity",
		"Air Temp",
		"Water Temp",
		"Windspeed",
		"Wind Angle",
		"Air Pressure",
		"Salinity",
		"Fluorescence",
		"Bathymetry",
		"Current Speed",
		"Current Angle"
	};
#endif

	typedef enum {
		SCIDAT_HUMIDITY,
		SCIDAT_AIRTEMP,
		SCIDAT_WATERTEMP,
		SCIDAT_WINDSPEED,
		SCIDAT_WINDANGLE,
		SCIDAT_AIRPRESSURE,
		SCIDAT_SALINITY,
		SCIDAT_FLUORESCENCE,
		SCIDAT_BATHYMETRY,
		SCIDAT_CURRENTSPEED,
		SCIDAT_CURRENTANGLE
	}SCIDATTYPE;




	/*******************************/

	typedef struct {

		double dValue;

		char sUnits[UNITSTRLEN];

		char sTime[TIMESTRLEN];

		POSITION position;


	}SCI_DBL;


	/*******************************/

	typedef struct {

		//platform identification
		int    nPlatformID;
		char   sPlatformName[PLATFORMNAMESTRLEN];
		int    nPlatformType;
		char   sPlatformTypeName[PLATTYPESTRLEN];

		//engineering telemetry
		int    nPlatformMode;
		char   sPlatformModeName[MODENAMESTRLEN];

		int	 nCommsLatency;

		char   sTime[TIMESTRLEN];	// date string
		long int nTime; 	// seconds since epoch

		double dLonDeg;
		double dLatDeg;
		double dAltMeters;

		double dSpeed_mps;			// groundspeed (GPS / net motion)
		double dDirectionDeg;		// direction of groundspeed (GPS / net motion)
		double dWaterspeed_mps;		// speed of boat relative to water

		double dHdgDeg;
		double dPitchDeg;
		double dRollDeg;

		double dBattVolts;

		//science telemetry
		SCI_DBL dHumidPct;
		SCI_DBL dAirTempDegC;
		SCI_DBL dWaterTempDegC;

		SCI_DBL dWindSpeed_mps;
		SCI_DBL dWindAngleDeg;
		SCI_DBL dAirPress_mbar;

		SCI_DBL dSalinity_psu;
		SCI_DBL dFluorescence_V;

		SCI_DBL dBathymetry;

		SCI_DBL dCurrentSpeedMPS;
		SCI_DBL dCurrentAngleDeg;

		// waypoints
		int    nWaypointCount;
		int	nCurrWaypt;
		WAYPT CurrWpt;
		WAYPT Waypoints[MAXWPTS];

	} STATUS;

	enum COMMSLATENCY {
		COMMS_LATENCY_GOOD = 0,
		COMMS_LATENCY_SLOW,
		COMMS_LATENCY_BAD
	};

	/*******************************/
	STATUS  status_InitStatus();
	SCI_DBL status_InitSciDbl();
	void    status_PrintSciDbl( SCI_DBL d );
	void    status_PrintStatus( STATUS s );
	int     status_compareStatus( STATUS s1, STATUS s2 );
//STATUS  status_ReadFile(char *filespec);
//void    status_WriteFile(char *filespec,STATUS s);
	int status_compareEngStatus( STATUS s1, STATUS s2 );
	int status_compareSciDbl( SCI_DBL s1, SCI_DBL s2 );

	POSITION status_PositionFromStatus( STATUS s );
	WAYPT status_WaypointFromStatus( STATUS s );
	int status_VerifyStatus( STATUS s );
	void status_SetTimeString( char *s, int size, struct tm *time );

#ifdef __cplusplus
}

#endif

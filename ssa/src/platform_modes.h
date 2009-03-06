/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

#define NUMMODES	13
#define MODENAMESTRLEN 32

#ifdef NEED_MODENAMES
	static char modenames[NUMMODES][MODENAMESTRLEN] = {
		"Mode 0",
		"Mode 1",
		"Awaiting Instructions",
		"In Transit",
		"Paused",
		"Interrupt",
		"Free Drift",
		"Station Keep",
		"Manual",
		"Heading Hold",
		"Waypoint",
		"Calibrate",
		"Mode 12"
	};
#endif

	typedef enum {
		MODE0 = 0,
		MODE1,
		MODE_AWAITINSTR,
		MODE_TRANSIT,
		MODE_PAUSE,
		MODE_INTERRUPT,
		MODE_FREEDRIFT,
		MODE_STATIONKEEP,
		MODE_MANUAL,
		MODE_HEADINGHOLD,
		MODE_WAYPOINT,
		MODE_CALIBRATE,
		MODE12
	}MODETYPE;


#ifdef __cplusplus
}

#endif

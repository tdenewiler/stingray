/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	/***********************************************************

	  RobotDatFileFormat.h

	  Steve Stancliff

	  v1.0

	  2/24/2007

	  Formats for writing telemetry to the "robot.dat" file

	*************************************************************/


#define FORMAT_LAT "lat,%16.13f\n"

#define FORMAT_LON "lon,%16.13f\n"

#define FORMAT_ALT "alt,%16.13f\n"

#define FORMAT_HDG "hdg,%16.13f\n"

#define FORMAT_PITCH "pitch,%16.13f\n"

#define FORMAT_ROLL "roll,%16.13f\n"

#define FORMAT_BATT "batt,%16.13f\n"

#define FORMAT_SPEED "speed,%16.13f\n"

#define FORMAT_HUMID "humid,%16.13f\n"

#define FORMAT_AIRTEMP "airtemp,%16.13f\n"

#define FORMAT_WATERTEMP "watertemp,%16.13f\n"

#define FORMAT_WINDSPEED "windspeed,%16.13f\n"

#define FORMAT_WINDANGLE "windangle,%16.13f\n"

#define FORMAT_AIRPRESS "airpressure,%16.13f\n"

#define FORMAT_SALINITY "salinity,%16.13f\n"

#define FORMAT_CHLORO "chlorophyll,%16.13f\n"

#define FORMAT_NUMWPT "numwaypts,%d\n"

#define FORMAT_WPTLAT "waypoint_lat,%16.13f\n"

#define FORMAT_WPTLON "waypoint_lon,%16.13f\n"

#define FORMAT_ASFMODE "asf_mode,%d\n"

#define FORMAT_ASFMODENAME "asf_modename,%s\n"

#define FORMAT_COMMSLATENCY "comms_latency,%d\n"

#define FORMAT_WATERSPEED "waterspeed,%16.13f\n"

#define FORMAT_CURRENTSPEED "current_speed,%16.13f\n"

#define FORMAT_CURRENTANGLE "current_angle,%16.13f\n"

#ifdef __cplusplus
}

#endif

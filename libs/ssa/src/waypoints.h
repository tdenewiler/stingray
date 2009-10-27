/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

//#define WPARRAYSIZE	16

	typedef struct {
		double dLatMps;
		double dLonMps;

	} VELOCITY;

	typedef struct {
		double dLatDeg;    // Latitude  (degrees)
		double dLonDeg;    // Longitude (degrees)
		double dAltMeters; // Altitude  (meters)
	} COORDINATE;

	typedef struct {
		double dLatDeg;
		double dLonDeg;
		double dSpeedMps;
		double dCloseEnoughM;
	} WAYPT;

	typedef struct {
		double dLatDeg;
		double dLonDeg;
	} POSITION;

	typedef struct wpnode {
		WAYPT wp;

		struct wpnode *prev;

		struct wpnode *next;
	}WPNODE;

	void waypoints_WriteFile( char *filespec, int count, WAYPT *wp );
	void waypoints_ReadFile( char *filespec, int wpcount, WAYPT *wp );

	POSITION waypoints_PositionInit();
	WAYPT waypoints_WaypointInit();

	int waypoints_WaypointsCompare( WAYPT w1, WAYPT w2 );

	void waypoints_WaypointsPrint( WAYPT *wp, int count );
	void waypoints_WptPrint( WAYPT wpt, int num );

	void waypoints_PositionPrint( POSITION p );
	void waypoints_PositionPrintWithString( char *s, POSITION p );
	void waypoints_WaypointPrint( WAYPT p );
	void waypoints_WaypointPrintWithString( char *s, WAYPT p );

	double waypoints_PositionCalcDist( POSITION wp1, POSITION wp2 );
	POSITION waypoints_PositionRotate( POSITION p1, POSITION center, float a );
	POSITION waypoints_PositionAdd( POSITION p1, POSITION p2 );
	POSITION waypoints_PositionSubtract( POSITION p1, POSITION p2 );

	double waypoints_DistanceFromWaypoint( WAYPT s, WAYPT w );
	double waypoints_HeadingFromWaypoint( WAYPT s, WAYPT w );



#ifdef __cplusplus
}

#endif

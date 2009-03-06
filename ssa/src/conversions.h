/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

// earth's radius in meters
#define EARTHRADIUS (6371000.)

#define RADIANS(x)	(x*M_PI/180.)
#define DEGREES(x)	(x*180./M_PI)


// convert meters to long. degrees at 37.9 lat
#define METERSTOLONDEG(x) (x / 87952.)
// convert meters to lat. degrees at 37.9 lat
#define METERSTOLATDEG(x) (x / 110995.)

// convert meters to long. degrees at 37.9 lat
#define LONDEGTOMETERS(x) (x * 87952. )

// convert meters to lat. degrees at 37.9 lat
#define LATDEGTOMETERS(x) (x * 110995.)

#define SQ(x) (x*x)
#define MOD(y,x) 		((y) - (x)*floor((y)/(x)))


#ifdef __cplusplus
}

#endif

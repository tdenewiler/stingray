/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct timespec TIMESPEC;

	double delay_deltatime( struct timeb start, struct timeb end );
	TIMESPEC delay_setdelay( float period );
	void delay_sleep( TIMESPEC *delay );

#ifdef __cplusplus
}

#endif

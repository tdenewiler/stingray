# include "../../../src/common/pid.c"

/* float pid_subtract_angles( float ang1, float ang2 ); */

int main()
{
	float current[]   = { 179 , 90 , 200 , 136 , 251 };
	float reference[] = { 0   , 60 , 176 , 150 , 250 };
	float angle;
	int   n = 5;
	int i;
	
	for( i = 0 ; i < n ; i++ )
	{
		angle = pid_subtract_angles( current[i] , reference[i] );
		printf( "c:%.2f -> r:%.2f = %.2f\n" , current[i] , reference[i] , angle );
	}
	
	
	
		
	return 0;
}

/******************************************************************************
 *
 * Title:       void pid_subtract_angles( float ang1, float ang2 )
 *
 * Description: Calculates the difference between two angles.
 *
 * Input:		ang1: Angle 1.
 * 				ang2: Angle 2.
 *
 * Output:      diff: Difference between angles.
 *
 *****************************************************************************/
/*
float pid_subtract_angles( float ang1, float ang2 )
{
	float e = 0.0;

	if(ang1 == ang2 ) {
		return e;
	}
	if( ang1 < ang2 ) {
		if( (ang2 - ang1) < 180 ) {
			e = ang1 - ang2;
		}
		else {
			e = 360 - ang2 + ang1;
		}
	}
	else {
		if( (ang1 - ang2) < 180 ) {
			e = ang1 - ang2;
		}
		else {
			e = -360 + ang1 -ang2;
		}
	}

	return e;
} *//* end pid_subtract_angles() */

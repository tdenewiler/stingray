/**
 *  \file util.h
 *  \brief Utility functions.
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include "pololu.h"
#include "labjack.h"


/******************************
**
** #defines
**
******************************/

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef STINGRAY
#define STINGRAY 1
#endif /* STINGRAY */

#ifndef GUI
#define GUI 2
#endif /* GUI */

#define UTIL_FEQUALS_EPSILON 0.00001


/******************************
**
** Data types
**
******************************/



/******************************
**
** Function prototypes
**
******************************/

//! Prints a help message to the user.
//!
void util_print_help( );

//! Prints the valid variables for use in the configuration file.
//!
void util_print_config_help( );

//! Calculate the difference between two time values.
//! Returns the time difference in microseconds.
//! \param time1s First time [seconds].
//! \param time1ms First time [microseconds].
//! \param time2s Second time [seconds].
//! \param time2ms Second time [microseconds].
//!  \return The time difference in microseconds.
int util_calc_dt( int *time1s, int *time1ms, int *time2s, int *time2ms );

//! Determines sign of value.
//! \param value Value to determine sign of.
//! \return Either 1 or -1 depeding on sign.
float util_sign_value( float value );

//! Determines whether or not two floats are equal
//! \param value1 first value for equality check
//! \param value2 second value for equality check
//! \return Either TRUE for equality, FALSE otherwise
float util_fequals( float value1 , float value2 );

#endif /* _UTIL_H_ */

/*!
 * \file logframe.c
 *
 * \author Robert Chen
 * \date Dec-15-08
 *
 * Logging library implementation.
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "include/hashtable.h"
#include "include/hashtable_itr.h"
#include "include/logframe.h"

#define TRUE 1
#define FALSE 0




// Utility functions
/******************************************************************************
 *
 * Title:               static unsigned int logf_hash_thresh(void* key)
 *
 * Description:   Internal function for hash table implementation. Given a threshold object
 *                         it converts it into a hash value for the hash table. Which is, in this case,
 *                         the user-defined error code. There is no pointer checking here.
 *
 * Input:              key: the threshold object.
 *
 * Output:           The value of evt_error_code for the threshold object
 *
 * Globals:           None.
 *
 *****************************************************************************/
static unsigned int logf_hash_thresh( void* key )
{
	logf_threshold *thresh = ( logf_threshold * ) key;

	/* We'll return the pointer value as the hash value */
	return ( unsigned int ) thresh->evt_error_code;
}

/******************************************************************************
 *
 * Title:               static unsigned int logf_cmp_thresh(void* key1, void* key2)
 *
 * Description:   Internal function for hash table implementation. Given a threshold object,
 *                         it compares the two and returns TRUE or FALSE depending on whether they
 *                         are the same. There is no pointer checking here.
 *
 * Input:              key1: a threshold object.
 *                         key2: another threshold object to be compared
 *
 * Output:           The value of evt_error_code for the threshold object
 *
 * Globals:           None.
 *
 *****************************************************************************/
static int logf_cmp_thresh( void* key1, void* key2 )
{
	logf_threshold *thresh1, *thresh2;

	/* is this really necessary? Whatever... */
	thresh1 = ( logf_threshold* ) key1;
	thresh2 = ( logf_threshold* ) key2;

	return ( thresh1->evt_error_code ==
	         thresh2->evt_error_code ) ?
	       TRUE : FALSE;
}

/******************************************************************************
 *
 * Title:               static unsigned int logf_hash_buf(void* key)
 *
 * Description:   Internal function for hash table implementation. Given a buffer object
 *                         it converts it into a hash value for the hash table. Which is, in this case,
 *                         the pointer value to the file stream.
 *
 * Input:              key: the buffer object.
 *
 * Output:           The pointer value of the buffer's file stream.
 *
 * Globals:           None.
 *
 *****************************************************************************/
static unsigned int logf_hash_buf( void* key )
{
	logf_buffer* buffer;
	buffer = ( logf_buffer* ) key;
	//printf("call to hash fn: %u \n", (unsigned int)buffer->stream);
	/* We'll return the pointer value as the hash value */
	/* 64-bit portability */
#ifdef __64BIT__
	return ( unsigned int )( ( long )( buffer->stream ) << 32 );
#else
	return ( unsigned int ) buffer->stream;
#endif
}

/******************************************************************************
 *
 * Title:               static unsigned int logf_cmp_buf(void* key1, void* key2)
 *
 * Description:   Internal function for hash table implementation. Given a threshold object,
 *                         it compares the two and returns TRUE or FALSE depending on whether they
 *                         are the same. There is no pointer checking here.
 *
 * Input:              key1: a buffer object.
 *                         key2: another buffer object to be compared
 *
 * Output:           TRUE or FALSE depending on whether the two file stream pointers are the
 *                         same.
 *
 * Globals:           None.
 *
 *****************************************************************************/
static int logf_cmp_buf( void* key1, void* key2 )
{
	logf_buffer *buffer1, *buffer2;
	buffer1 = ( logf_buffer* ) key1;
	buffer2 = ( logf_buffer* ) key2;

	return ( buffer1->stream == buffer2->stream ) ?
	       TRUE : FALSE;
}

/******************************************************************************
 *
 * Title:               int update_threshold(logf_instance* instance, int evt_error_code)
 *
 * Description:   Internal function for logging implementation. This function updates the
 *                         threshold for the given error code by incrementing the hits counter. You
 *                         should not use this function directly even though there is pointer checking.
 *
 * Input:             instance: an instance object
 *                        evt_error_code: User-defined error code.
 *
 * Output:          TRUE or FALSE depending on whether operation was successful.
 *
 * Globals:           None.
 *
 *****************************************************************************/
int update_threshold( logf_instance* instance,
                      int evt_error_code )
{
	/* create new threshold for comparison and other purposes */
	logf_threshold *temp, *retvalue;
	temp = ( logf_threshold* ) malloc( sizeof( logf_threshold ) );

	/* For comparison, as the hashing function needs the error code */
	temp->evt_error_code = evt_error_code;

	/* Sanity checks */

	if ( instance == NULL ) {
		free( temp );
		return LOGF_API_INST_ERR;
	}

	/* check to see if threshold exists */
	retvalue = ( logf_threshold* )hashtable_search(
	               instance->thresholds,
	               temp
	           );

	if ( retvalue == NULL ) {
		free( temp );
		return 0;
	}

	/* just in case */
	if ( retvalue->threshold_func == NULL ) {
		free( temp );
		return 0;
	}

	/* increment number of hits for this threshold */
	retvalue->threshold_hits ++;

	/* check to see if action needs to be taken */
	if ( retvalue->threshold_hits >=
	        retvalue->threshold_limit ) {
		/* reset threshold */
		retvalue->threshold_hits = 0;

		/* call user defined function */
		retvalue->threshold_func();

		free( temp );
		return 1;
	}

	free( temp );

	return 1;
}



/******************************************************************************
 *
 * Title:              logf_event* logf_ new_event()
 *
 * Description: This function is used to create a new event
 *
 * Input:            buffer: the buffer it is written to
 *                        timestamp: the timestamp of when this event occurred.
 *                        error_code: User defined location/event code.
 *                        char* message: event message
 *
 * Output:          TRUE or FALSE
 *
 * Globals:         None.
 *
 *****************************************************************************/
logf_event* logf_new_event( logf_buffer* buffer,
                            unsigned int timestamp,
                            int error_code,
                            char* message )
{

	logf_event* new_event = ( logf_event * ) malloc( sizeof( logf_event ) );

	/* set all variables */
	new_event->evt_timestamp = timestamp;
	new_event->evt_error_code = error_code;
	strcpy( new_event->evt_msg, message );


	return new_event;

}

/******************************************************************************
 *
 * Title:               int logf_print_error(logf_instance* instance,
 *                                                               logf_buffer* buffer,
 *                                                               int evt_error_code,
 *                                                               int type,
 *                                                               char* format,
 *                                                               va_list args);
 *
 * Description:   Internal function for logging implementation. This function is called
 *                         by all the error functions to log to buffer and update threshold.
*                          You shouldn't try to use this function directly.
 *
 * Input:             instance: an instance object
 *                        buffer: which buffer are we printing to?
 *                        evt_error_code: User-defined error code.
 *                        type: what type of error are we printing?
 *                        format: the message and printf formatting
 *                         args: variable arguments
 *
 * Output:          TRUE or FALSE depending on whether operation was successful.
 *
 * Globals:           None.
 *
 *****************************************************************************/
int logf_print_error( logf_instance* instance,
                      logf_buffer* buffer,
                      int evt_error_code,
                      int type,
                      char* format,
                      va_list args )
{

	char buff[LOGF_MAX_MSG_LEN];
	char tempBuffer[LOGF_MAX_MSG_LEN];

	int retval = 0;
	void* stream = NULL;
	logf_event* newEvent;

	struct timeval tv;
	time_t curtime;

	char timeBuffer [255];

	gettimeofday( &tv, NULL );
	curtime = tv.tv_sec;

	/* Get current time */
	strftime( timeBuffer, 255, "%H:%M:%S", localtime( &curtime ) );
	sprintf( timeBuffer, "%s.%03ld", timeBuffer, tv.tv_usec / 1000 );


	/* Sanity checks */

	if ( instance == NULL )
		return LOGF_API_INST_ERR;

	if ( buffer == NULL || buffer->stream == NULL )
		return LOGF_API_BUFF_ERR;


	/* print arguments to buffer */
	vsprintf( buff, format, args );

	/* write to file stream first */
	stream = buffer->stream;

	/* print out appropriate messages according to our filter */
	if ( type == LOGF_FATAL ) {
		/* increment hits */
		instance->fatal_error_hits ++;


		if ( ( buffer->logging_level & LOGF_FATAL ) ) {
			/* prepare string */
			sprintf( tempBuffer, "%s, FATAL, %d, %s\n",
			         timeBuffer,
			         evt_error_code,
			         buff );
			/* create new event object */
			newEvent = logf_new_event( buffer,
			                           clock() * CLOCKS_PER_SEC,
			                           evt_error_code,
			                           tempBuffer
			                         );

			/* I could move the following function into new event, but meh */
			buffer->events[buffer->event_counter] =  newEvent;
			buffer->event_counter ++;


			/* print to stderr for now */

			if ( buffer->logging_level & LOGF_STDERR ) {
				retval = fprintf( stderr, newEvent->evt_msg );
			}
		}

		/* call user-defined function */
		if ( instance->call_fatal_error != NULL ) instance->call_fatal_error();

	}

	if ( type == LOGF_ERROR ) {
		/* increment hits */
		instance->error_hits ++;


		if ( ( buffer->logging_level & LOGF_ERROR ) ) {

			/* prepare string */
			sprintf( tempBuffer, "%s, ERROR, %d, %s\n",
			         timeBuffer,
			         evt_error_code,
			         buff );
			/* create new event object */
			newEvent = logf_new_event( buffer,
			                           clock() * CLOCKS_PER_SEC,
			                           evt_error_code,
			                           tempBuffer
			                         );

			/* I could move the following function into new event, but meh */
			buffer->events[buffer->event_counter] =  newEvent;
			buffer->event_counter ++;

			if ( buffer->logging_level & LOGF_STDERR ) {
				retval = fprintf( stderr, newEvent->evt_msg );
			}
		}

		/* call user-defined function */
		if ( instance->call_error != NULL ) instance->call_error();
	}

	if ( type == LOGF_WARN ) {
		/* increment hits */
		instance->warn_hits ++;


		if ( ( buffer->logging_level & LOGF_WARN ) ) {

			/* prepare string */
			sprintf( tempBuffer, "%s, WARN, %d, %s\n",
			         timeBuffer,
			         evt_error_code,
			         buff );
			/* create new event object */
			newEvent = logf_new_event( buffer,
			                           clock() * CLOCKS_PER_SEC,
			                           evt_error_code,
			                           tempBuffer
			                         );

			/* I could move the following function into new event, but meh */
			buffer->events[buffer->event_counter] =  newEvent;
			buffer->event_counter ++;

			if ( buffer->logging_level & LOGF_STDERR ) {
				retval = fprintf( stderr, newEvent->evt_msg );
			}
		}

		/* call user-defined function */
		if ( instance->call_warn != NULL ) instance->call_warn();
	}

	if ( type == LOGF_INFO ) {
		/* increment hits */
		instance->info_hits ++;

		if ( ( buffer->logging_level & LOGF_INFO ) ) {

			/* prepare string */
			sprintf( tempBuffer, "%s, INFO, %d, %s\n",
			         timeBuffer,
			         evt_error_code,
			         buff );
			/* create new event object */
			newEvent = logf_new_event( buffer,
			                           clock() * CLOCKS_PER_SEC,
			                           evt_error_code,
			                           tempBuffer
			                         );

			/* I could move the following function into new event, but meh */
			buffer->events[buffer->event_counter] =  newEvent;
			buffer->event_counter ++;

			if ( buffer->logging_level & LOGF_STDERR ) {
				retval = fprintf( stderr, newEvent->evt_msg );
			}
		}

		/* call user-defined function */
		if ( instance->call_info != NULL ) instance->call_info();

	}

	if ( type == LOGF_DEBUG ) {
		/* increment hits */
		instance->debug_hits ++;


		if ( ( buffer->logging_level & LOGF_DEBUG ) ) {

			/* prepare string */
			sprintf( tempBuffer, "%s, DEBUG, %d, %s\n",
			         timeBuffer,
			         evt_error_code,
			         buff );
			/* create new event object */
			newEvent = logf_new_event( buffer,
			                           clock() * CLOCKS_PER_SEC,
			                           evt_error_code,
			                           tempBuffer
			                         );

			/* I could move the following function into new event, but meh */
			buffer->events[buffer->event_counter] =  newEvent;
			buffer->event_counter ++;

			if ( buffer->logging_level & LOGF_STDERR ) {
				retval = fprintf( stderr, newEvent->evt_msg );
			}
		}

		/* call user-defined function */
		if ( instance->call_debug != NULL ) instance->call_debug();
	}

	/* update thresholds*/
	update_threshold( instance, evt_error_code );

	/* do we need to flush? */
	/* User will manually flush if count is 0 */

	if ( buffer->buf_size != 0 && ( buffer->count < buffer->buf_size ) ) {

		buffer->count ++;

	}
	else if ( buffer->buf_size != 0 && (
	              ( buffer->count >= buffer->buf_size ) || ( buffer->event_counter >= LOGF_MAX_EVENTS )
	          ) ) {

		logf_flush( buffer, 1 );
	}


	va_end( args );

	return retval;

}

/******************************************************************************
 *
 * Title:              logf_instance* logf_init(void)
 *
 * Description:  This function initializes the logging library and creates a logging instance.
 *
 * Input:             None
 *
 * Output:          Pointer to a new logging instance.
 *
 * Globals:          None.
 *
 *****************************************************************************/
logf_instance* logf_init( void )
{

	void* buffer;
	logf_instance* this_instance;

	/* allocate memory for new logf_instance */
	buffer = malloc( sizeof( logf_instance ) );
	this_instance = ( logf_instance * ) buffer;

	/* initialize variables */
	this_instance->fatal_error_hits = this_instance->error_hits =
	                                      this_instance->warn_hits = this_instance->info_hits =
	                                                                     this_instance->debug_hits = 0;

	this_instance->call_fatal_error = this_instance->call_error =
	                                      this_instance->call_warn = this_instance->call_info =
	                                                                     this_instance->call_debug = NULL;

	/* intialize hash tables */
	this_instance->buffers = create_hashtable( LOGF_MAX_BUFFERS,
	                         logf_hash_buf,
	                         logf_cmp_buf );

	this_instance->thresholds = create_hashtable( LOGF_MAX_THRESHOLDS,
	                            logf_hash_thresh,
	                            logf_cmp_thresh );

	/* return pointer to the stack */
	return this_instance;

}

/******************************************************************************
 *
 * Title:              logf_buffer* logf_open_file (logf_instance *instance,
 *                                                                            char* filename,
 *                                                                            int logging_level)
 *
 * Description:  This function initializes a file buffer for use.
 *
 * Input:            instance: the logging instance
 *                       filename: the name of file
 *                       logging_level: a filter for this particular buffer
 *
 * Output:          Pointer to a new buffer instance.
 *
 * Globals:          None.
 *
 *****************************************************************************/
logf_buffer* logf_open_file( logf_instance *instance, char* filename,
                             int logging_level, int size )
{
	/* ugly, I know */

	struct timeval tv;
	time_t curtime;

	char timeBuffer [255];

	gettimeofday( &tv, NULL );
	curtime = tv.tv_sec;

	/* Get current time */
	strftime( timeBuffer, 255, "%H:%M:%S", localtime( &curtime ) );
	sprintf( timeBuffer, "%s.%03ld", timeBuffer, tv.tv_usec / 1000 );

	/* allocate our new buffer */
	logf_buffer* newBuffer = ( logf_buffer * ) malloc( sizeof( logf_buffer ) );
	newBuffer->count = newBuffer->event_counter = 0;

	/* open the file name, and check */
	newBuffer->stream = fopen( filename, "a" );

	if ( newBuffer->stream == NULL ) {
		return NULL;
	}

	/* initialize logging level */
	newBuffer->logging_level = logging_level;

	/* intialize size restriction */
	newBuffer->buf_size = size;

	/* insert into the hash table of buffers in our main instance struct */
	hashtable_insert( instance->buffers, newBuffer, newBuffer );

	/* announce our creation */
	if ( logging_level & LOGF_STDERR )
		fprintf( stderr, "%s, INFO, None, LOGGING BUFFER INITIALIZED: \"%s\"\n",
		         timeBuffer,  filename );

	fprintf( ( FILE* )newBuffer->stream,
	         "%s, INFO, None, LOGGING BUFFER INITIALIZED: \"%s\"\n",
	         timeBuffer,  filename );

	return newBuffer;
}


/******************************************************************************
 *
 * Title:             int logf_fatal_error ( logf_instance* instance, logf_buffer* buffer,
 *                                                             int evt_error_code, char* format, ... )
 *
 * Description:  This function is called for fatal errors. Chainloads off to logf_print_error.
 *
 * Input:            instance: the logging instance
 *                       buffer: target buffer
 *                       evt_error_code: error code associated with this event
 *                       format: the format for the output message.
 *                       ...: stuff to be printed
 *
 * Output:          TRUE or FALSE depending on success.
 *
 * Globals:          None.
 *
 *****************************************************************************/
int logf_fatal_error( logf_instance* instance, logf_buffer* buffer,
                      int evt_error_code, char* format, ... )
{
	/* variable argument magic */
	va_list args;
	va_start( args, format );

	/* pass off to function to handle */
	return logf_print_error( instance, buffer, evt_error_code,
	                         LOGF_FATAL, format, args );

}

/******************************************************************************
 *
 * Title:             int logf_error ( logf_instance* instance, logf_buffer* buffer,
 *                                                             int evt_error_code, char* format, ... )
 *
 * Description:  This function is called for errors. Chainloads off to logf_print_error.
 *
 * Input:            instance: the logging instance
 *                       buffer: target buffer
 *                       evt_error_code: error code associated with this event
 *                       format: the format for the output message.
 *                       ...: stuff to be printed
 *
 * Output:          TRUE or FALSE depending on success.
 *
 * Globals:          None.
 *
 *****************************************************************************/
int logf_error( logf_instance* instance, logf_buffer* buffer,
                int evt_error_code, char* format, ... )
{
	/* variable argument magic */
	va_list args;
	va_start( args, format );

	/* pass off to function to handle */
	return logf_print_error( instance, buffer, evt_error_code,
	                         LOGF_ERROR, format, args );


}

/******************************************************************************
 *
 * Title:             int logf_warn ( logf_instance* instance, logf_buffer* buffer,
 *                                                             int evt_error_code, char* format, ... )
 *
 * Description:  This function is called for warnings. Chainloads off to logf_print_error.
 *
 * Input:            instance: the logging instance
 *                       buffer: target buffer
 *                       format: the format for the output message.
 *                       ...: stuff to be printed
 *
 * Output:          TRUE or FALSE depending on success.
 *
 * Globals:          None.
 *
 *****************************************************************************/
int logf_warn( logf_instance* instance, logf_buffer* buffer,
               char* format, ... )
{
	/* variable argument magic */
	va_list args;
	va_start( args, format );

	/* pass off to function to handle */
	return logf_print_error( instance, buffer, 0,
	                         LOGF_WARN, format, args );


}

/******************************************************************************
 *
 * Title:             int logf_info( logf_instance* instance, logf_buffer* buffer,
 *                                                             int evt_error_code, char* format, ... )
 *
 * Description:  This function is called for info logging Chainloads off to logf_print_error.
 *
 * Input:            instance: the logging instance
 *                       buffer: target buffer
 *                       format: the format for the output message.
 *                       ...: stuff to be printed
 *
 * Output:          TRUE or FALSE depending on success.
 *
 * Globals:          None.
 *
 *****************************************************************************/
int logf_info( logf_instance* instance, logf_buffer* buffer,
               char* format, ... )
{
	/* variable argument magic */
	va_list args;
	va_start( args, format );

	/* pass off to function to handle */
	return logf_print_error( instance, buffer, 0,
	                         LOGF_INFO, format, args );


}

/******************************************************************************
 *
 * Title:              int logf_debug ( logf_instance* instance, logf_buffer* buffer,
 *                                                             int evt_error_code, char* format, ... )
 *
 * Description:  This function is used for debug messages Chainloads off to logf_print_error.
 *
 * Input:            instance: the logging instance
 *                       buffer: target buffer
 *                       evt_error_code: error code associated with this event
 *                       format: the format for the output message.
 *                       ...: stuff to be printed
 *
 * Output:          TRUE or FALSE depending on success.
 *
 * Globals:          None.
 *
 *****************************************************************************/
int logf_debug( logf_instance* instance, logf_buffer* buffer,
                char* format, ... )
{

	/* variable argument magic */
	va_list args;
	va_start( args, format );

	/* pass off to function to handle */
	return logf_print_error( instance, buffer, 0,
	                         LOGF_DEBUG, format, args );


}


/******************************************************************************
 *
 * Title:              int logf_add_threshold( logf_instance* instance,
 *                                                                    int evt_error_code,
 *                                                                    int threshold,
 *                                                                    user_call udef )
 *
 * Description:  This function is used to add a new threshold.
 *
 * Input:            instance: the logging instance
 *                       evt_error_code: error code associated with this threshold
 *                       udef: user defined function
 *
 * Output:         TRUE or FALSE on whether the operation was successful.
 *
 * Globals:         None.
 *
 *****************************************************************************/
int logf_add_threshold( logf_instance* instance,
                        int evt_error_code,
                        int threshold,
                        user_call udef )
{

	/* create new threshold for comparison and other purposes */
	logf_threshold* temp;
	temp = ( logf_threshold* ) malloc( sizeof( logf_threshold ) );

	/* For comparison, as the hashing function needs the error code */
	temp->evt_error_code = evt_error_code;

	/* Sanity checks */

	if ( instance == NULL ) {
		free( temp );
		return LOGF_API_INST_ERR;
	}

	if ( udef == NULL ) {
		free( temp );
		return 0;
	}

	/* check to see if threshold exists */
	if ( hashtable_search( instance->thresholds, temp ) ) {
		free( temp );
		return 0;
	}

	/* nope, prepare a new threshold */
	temp->evt_error_code = evt_error_code;

	temp->threshold_limit = threshold;

	temp->threshold_hits = 0;

	temp->threshold_func = udef;

	/* insert it */
	return hashtable_insert( instance->thresholds, temp, temp );

}

/******************************************************************************
 *
 * Title:              int logf_remove_threshold( logf_instance* instance,
 *                                                                    int evt_error_code
 *                                                                     )
 *
 * Description:  This function is used to remove a threshold.
 *                        Searches hash table for logf_threshold object
 *                        with specified error code and "removes" it.
 *
 * Input:            instance: the logging instance
 *                       evt_error_code: error code associated with this threshold
 *
 * Output:         TRUE or FALSE on whether the operation was successful.
 *
 * Globals:         None.
 *
 *****************************************************************************/
int logf_remove_threshold( logf_instance* instance,
                           int evt_error_code )
{
	/* create new threshold for comparison and other purposes */
	logf_threshold *temp, *retvalue;
	temp = ( logf_threshold* ) malloc( sizeof( logf_threshold ) );

	/* For comparison, as the hashing function needs the error code */
	temp->evt_error_code = evt_error_code;

	/* Sanity checks */

	if ( instance == NULL ) {
		free( temp );
		return LOGF_API_INST_ERR;
	}

	/* check to see if threshold exists */
	retvalue = ( logf_threshold* )hashtable_search( instance->thresholds, temp );

	if ( retvalue == NULL ) {
		free( temp );
		return 0;
	}

	/* remove from hash table.. memory is free'd */
	hashtable_remove( instance->thresholds, retvalue );

	free( temp );

	return 1;

}

/******************************************************************************
 *
 * Title:              int logf_set_threshold( logf_instance* instance,
 *                                                                 int evt_error_code
 *                                                                )
 *
 * Description:  This function is used to reconfigure a threshold.
 *
 * Input:            instance: the logging instance
 *                       evt_error_code: error code associated with this threshold
 *
 * Output:         TRUE or FALSE on whether the operation was successful.
 *
 * Globals:         None.
 *
 *****************************************************************************/
int logf_set_threshold( logf_instance* instance, int evt_error_code,
                        int threshold, user_call udef )
{

	/* create new threshold for comparison and other purposes */
	logf_threshold *temp, *retvalue;
	temp = ( logf_threshold* ) malloc( sizeof( logf_threshold ) );

	/* For comparison, as the hashing function needs the error code */
	temp->evt_error_code = evt_error_code;

	/* Sanity checks */

	if ( instance == NULL ) {
		free( temp );
		return LOGF_API_INST_ERR;
	}

	/* check to see if threshold exists */
	retvalue = ( logf_threshold* )hashtable_search( instance->thresholds, temp );

	if ( retvalue == NULL ) {
		free( temp );
		return 0;
	}

	/* reassing the settings */
	retvalue->threshold_hits = 0;

	if ( retvalue->threshold_func != NULL )
		retvalue->threshold_func = udef;

	retvalue->evt_error_code = evt_error_code;

	retvalue->threshold_limit = threshold;

	return 1;
}

/******************************************************************************
 *
 * Title:              int logf_set_error_calls (
 *                           logf_instance* instance,
 *                           user_call call_fatal_error,
 *                           user_call call_error,
 *                           user_call call_warn,
 *                           user_call call_info,
 *                           user_call call_debug )
 *
 * Description:  This function is used to set the user-defined functions for the various types
 *                         of errors. These function are basically callbacks that are invoked when the
 *                         type of error is encountered.
 *
 * Input:            instance: the logging instance
 *                       call_fatal_error: user callback when "fatal error" is encountered.
 *                       call_error: user callback when "error" is encountered.
 *                       call_warn: user callback when "warning" is encountered.
 *                       call_info: user callback when "info" is encountered.
 *                       call_debug: user callback when "debug" is encountered.
 *
 * Output:         TRUE or FALSE on whether the operation was successful.
 *
 * Globals:         None.
 *
 *****************************************************************************/
int logf_set_error_calls( logf_instance* instance,
                          user_call call_fatal_error,
                          user_call call_error,
                          user_call call_warn,
                          user_call call_info,
                          user_call call_debug )
{

	struct hashtable_itr* itr = NULL;
	logf_threshold *k, *v;

	if ( instance == NULL )
		return LOGF_API_INST_ERR;

	/* woah, not so fast. Check to see if the function already exists.
	        as a threshold, there may be a case of inifinite loop if a user-definedfunction  triggered by a threshold
	        prints an error message on the same error level */

	if ( hashtable_count( instance->thresholds ) > 0 ) {
		itr = ( struct hashtable_itr* )hashtable_iterator( instance->thresholds );

		do {
			k = ( logf_threshold* )hashtable_iterator_key( itr );
			v = ( logf_threshold* )hashtable_iterator_value( itr );
			/* all must pass or function fails */

			if ( ( k->threshold_func == call_fatal_error ) ||
			        ( k->threshold_func == call_error )       ||
			        ( k->threshold_func == call_warn )        ||
			        ( k->threshold_func == call_info )        ||
			        ( k->threshold_func == call_info ) ) {
				free( itr );
				return 0;
			}

		}
		while ( hashtable_iterator_advance( itr ) );
	}

	free( itr );

	/* now, assign the stuff */
	instance->call_fatal_error = call_fatal_error;
	instance->call_error = call_error;
	instance->call_warn = call_warn;
	instance->call_info = call_info;
	instance->call_debug = call_debug;

	return 1;

}

/******************************************************************************
 *
 * Title:              int logf_reset_threshold (
 *                           logf_instance* instance,
 *                           int evt_error_code )
 *
 * Description:  This function is used to reset the threshold of the given error code.
 *
 * Input:            instance: the logging instance
 *                        evt_error_code: the user-defined error code.
 *
 * Output:         TRUE or FALSE on whether the operation was successful.
 *
 * Globals:         None.
 *
 *****************************************************************************/
int logf_reset_threshold( logf_instance* instance,
                          int evt_error_code )
{

	/* create new threshold for comparison and other purposes */
	logf_threshold *temp, *retvalue;
	temp = ( logf_threshold* ) malloc( sizeof( logf_threshold ) );

	/* For comparison, as the hashing function needs the error code */
	temp->evt_error_code = evt_error_code;

	/* Sanity checks */

	if ( instance == NULL ) {
		free( temp );
		return LOGF_API_INST_ERR;
	}

	/* check to see if threshold exists */
	retvalue = ( logf_threshold* )hashtable_search( instance->thresholds, temp );

	if ( retvalue == NULL ) {
		free( temp );
		return 0;
	}

	/* reset number of hits for this threshold */
	retvalue->threshold_hits = 0;

	return 1;

}

/******************************************************************************
 *
 * Title:              int logf_cleanup(  logf_instance* instance )
 *
 * Description: This function is used to clean up the logging instance.
 *
 * Input:            instance: the logging instance
 *
 * Output:          Nothing.
 *
 * Globals:         None.
 *
 *****************************************************************************/
void logf_cleanup( logf_instance* instance )
{

	struct hashtable_itr* itr = NULL;
	logf_buffer *v = NULL;


	if ( instance == NULL ) {
		return;
	}

	/* delete all buffers and thresholds */
	/*if (hashtable_count(instance->thresholds) > 0)
	  {
	      itr = (struct hashtable_itr*)hashtable_iterator(instance->thresholds);
	      do {

	          v2 = (logf_threshold*)hashtable_iterator_value(itr);

	          free(v2);


	      } while (hashtable_iterator_advance(itr));
	  }

	  */

	hashtable_destroy( instance->thresholds, 0 );

	if ( hashtable_count( instance->buffers ) > 0 ) {
		itr = ( struct hashtable_itr* )hashtable_iterator( instance->buffers );

		do {

			v = ( logf_buffer* )hashtable_iterator_value( itr );
			/* flush on cleanup */
			logf_flush( v, 1 );

			/* close file stream */
			fflush( ( FILE* )v->stream );
			/* error checks here in case user manually closed it? */
			fclose( ( FILE* )v->stream );





		}
		while ( hashtable_iterator_advance( itr ) );
	}

	free( itr );

	hashtable_destroy( instance->buffers, 0 );


	free( instance );

	//instance = 0;

}

/******************************************************************************
 *
 * Title:              void logf_flush(  logf_buffer* buffer, int remove )
 *
 * Description: This function is used to  flush all events in the heap to the log file.
 *
 * Input:            instance: the logging instance
 *
 * Output:          Nothing.
 *
 * Globals:         None.
 *
 *****************************************************************************/
void logf_flush( logf_buffer* buffer, int remove )
{

	int i = 0;

	if ( buffer == NULL )
		return ;


	/* write everything in the hash table to file */
	for ( i = 0; i < buffer->event_counter; i++ ) {

		fprintf( ( FILE* )buffer->stream, "%s", buffer->events[i]->evt_msg );

		if ( remove ) {

			free( buffer->events[i] );

		}

	}

	/* reset counters */
	buffer->count = 0;

	buffer->event_counter = 0;

	buffer->last_pos = 0;
}

/*
 * Copyright (c) 2008, Robert chen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*!
 * \file logframe.h
 *
 * \author Robert Chen
 * \date Dec-15-08
 *
 * Header file for logging library.
 */

#ifndef __LOGFRAME__
#define __LOGFRAME__

#include <stdio.h>
#include <stdarg.h>

#include "hashtable.h"

#if __WORDSIZE == 64
#define __64BIT__ 1
#endif /* __WORDSIZE */

#define LOGF_MAX_BUFFERS 255
#define LOGF_MAX_THRESHOLDS 255
#define LOGF_MAX_EVENTS 255
#define LOGF_MAX_MSG_LEN 255



/**
 *  This enum defines lists the type of buffers
 *   possible.
 */
enum logf_buf_types {
	/*!
	         * Write logging information to file
	         */
	LOGF_FILE,
	/*!
	         * Write logging information to stderr
	         */
	LOGF_STDERR = 32,
	/*!
	         * Write logging information to socket
	         *  (To be implemented.)
	         */
	LOGF_SOCKET

};

/**
 *  This enum defines the type of errors
 *   possible.
 */
enum logf_errors {
	/*!
	         * \b Top priority error calling for attention:
	         * Usually occurs due to a \b catastrophic \b failure of a module.
	         */
	LOGF_FATAL = 16,
	/*!
	         * \b High priority error:
	         * Usually occurs due to a \b module \b encountering \b problems.
	         */
	LOGF_ERROR = 8,
	/*!
	         *  \b Medium priority anomaly:
	         *  Usually occurs due to a module detecting an \b anomaly not
	         * encountered during normal operation.
	         */
	LOGF_WARN = 4,
	/*!
	        *  \b Low priority anomaly:
	        *  Provides interesting \b information.
	        */
	LOGF_INFO = 2,
	/*!
	         *  \b Informational purposes:
	         *  Provides \b raw \b data and other debug messages.
	         */
	LOGF_DEBUG = 1

};

/**
 *  This enum defines all the errors the logf_* functions return
 *   where applicable.
 */
enum logf_api_errors {
	LOGF_API_INST_ERR = -1, /**< logf instance error, i.e null pointer,*/
	LOGF_API_BUFF_ERR = -2 /**< logf api buffer write error, i.e. null pointer*/
};



/*!
 * \struct logf_event
 *
 * \brief Event object.
 *
 * This struct contains information regarding an event. \n
 * This struct is provided to a user via the user defined functions
 *  as an argument. \n
 * This struct is also eturned to the user via the following functions.
 *  @see logf_get_event_code, logf_get_event_msg
 *
 */

typedef struct logf_event {
	unsigned int evt_timestamp; /**< Timestamp of when the event occured*/

	/** User defined error code for this particular event*/
	int evt_error_code;
	/** Message for this particular event */
	char evt_msg[LOGF_MAX_MSG_LEN];

} logf_event;



/** This is the format of a user defined error handler call.
 */
typedef void ( *user_call )( void );



/*!
 * \struct logf_instance
 *
 * \brief  Main logging instance struct.
 *
 * This struct holds information regarding the current logging setup.\n
 * This also contains function pointers for user-defined trigger functions.
 * @see logf_init, logf_set_error_calls
 */

typedef struct logf_instance {
	/** Stores all the buffer structures  associated with this logging instance */

	struct hashtable * buffers;
	/** Stores all the thresholds structures  associated with this logging instance */

	struct hashtable * thresholds;

	/** Counter for number of fatal errors encountered */
	int fatal_error_hits,
	/** Counter for number of errors encountered */
	error_hits,
	/** Counter for number of warnings encountered */
	warn_hits,
	/** Counter for number of information messages encountered */
	info_hits,
	/** Counter for number of debug messages encountered */
	debug_hits;

	// Watchdog functions: user defined, called by library
	/** User defined trigger function called when an error with that level is encountered */
	user_call call_fatal_error;
	/** User defined trigger function called when an error with that level is encountered */
	user_call call_error;
	/** User defined trigger function called when an error with that level is encountered */
	user_call call_warn;
	/** User defined trigger function called when an error with that level is encountered */
	user_call call_info;
	/** User defined trigger function called when an error with that level is encountered */
	user_call call_debug;


} logf_instance;


/*!
 * \struct logf_buffer
 *
 * \brief Buffer struct.
 *
 * This struct holds information as to where the debugging messages \n
 *  are written to, be it to a socket or a file descriptor.\n
 *  Buffer is created by the following functions:\n
 *  @see logf_open_file, logf_open_stderr
 *
 */

typedef struct logf_buffer {
	void* stream;
	int buf_size;
	int logging_level;
	int count;
	int last_pos;

	int event_counter;
	/** Store the events written to this buffer on the heap */
	//struct hashtable * events;
	logf_event* events[LOGF_MAX_EVENTS];

	// User defined functions
	int ( *init )( int type, void*, char* );
	int ( *add )();
	int ( *write )( char* );
	int ( *close )(); // defined too

} logf_buffer;

/*!
 * \struct logf_threshold
 *
 * \brief  Threshold information struct.
 *
 * This struct holds the threshold limits for certain errors \n
 *  as defined by the user.
 * @see logf_add_threshold, logf_remove_threshold, logf_set_threshold
 */

typedef struct logf_threshold {

	/** User defined error code */
	int evt_error_code;
	/** Threshold limit */
	int threshold_limit;
	/** Current number of hits for this threshold */
	int threshold_hits;

	/** User-defined function to call when we've hit the limit.
	        * We will also provide the \b event \b struct that \b caused the
	        * threshold to \b exceed its limit as an argument to the user-
	        * defined function. \n
	        * If this is \b NULL, nothing will happen when the threshold is
	        * exceeded.
	        */
	user_call threshold_func;

} logf_threshold;


/*!
 * \brief This function initializes the logging library and creates a logging instance.
 *
 * A new instance struct is allocated. Values inside the struct are intialized.
 * Two hash tables are created for thresholds and buffers
 *
 * \return A new logging instance.
 * \sa logf_instance
 */
// ----- external  use
logf_instance* logf_init( void );

/*!
 * \brief Adds a new threshold.
 *
 * This function allocates memory for a new threshold object.
 * This object is inserted into the thresholds hash table, and is
 * first checked ot see if it already exists as a callback function.
 *
 * \param instance the logging instance
 * \param evt_error_code error code associated with this
 * \param udef user defined function
 * \return TRUE or FALSE on whether the operation was successful.
 * \sa logf_threshold, logf_instance, logf_errors, user_call
 * \note All  user defined error codes must be unique, and cannot equal -1.
 * \warning None
 */
int logf_add_threshold( logf_instance* instance,
                        int evt_error_code,
                        int threshold,
                        user_call udef );
/*!
 * \brief This function is used to set the user-defined callback functions .
 *
 *
 * The given function are basically callbacks which are invoked when the
 *  type of error is encountered.
 *
 * \param instance the logging instance.
 * \param  call_fatal_error user callback for when "fatal error" is encountered.
 * \param  call_error user callback for when "error" is encountered.
 * \param call_warn user callback for when "warning" is encountered.
 * \param call_info user callback for when "info" is encountered.
 * \param call_debug user callback for when "debug" is encountered.
 *
 * \return TRUE or FALSE on whether the operation was successful.
 * \sa logf_instance, user_call
 * \note It's ok to pass NULL as arguments for the callback functions.
 * \warning None.
 */
// optional, function only if user wants to be alerted after every error
int logf_set_error_calls( logf_instance* instance,
                          user_call call_fatal_error,
                          user_call call_error,
                          user_call call_warn,
                          user_call call_info,
                          user_call call_debug );

/*!
 * \brief Opens a log file as a buffer.
 *
 * Initializes and returns a new buffer object. The provided
 * properties for the buffer are also assigned.
 *
 * \param instance the logging instance.
 * \param filename the filename of the log file
 * \param logging_level what to log?
 *
 * \return  A new logf_buffer object, NULL if it fails
 * \sa logf_buffer, logf_instance
 * \note You should check to see if what this function returns is NULL.
 * \warning None.
 */
// open buffer,
logf_buffer* logf_open_file( logf_instance *instance, char* filename,
                             int logging_level, int size );


// error function calls
/*!
 * \brief This function is called for fatal errors.
 *
 * Chainloads off to logf_print_error.
 *
 * \param instance the logging instance
 * \param buffer target buffer
 * \param evt_error_code error code associated with this event
 * \param format printf-style format of the error message
 * \param ... variable arguments
 *
 * \return TRUE or FALSE depending on success.
 * \sa logf_fatal_error, logf_instance, logf_buffer, logf_error, logf_warn
 * \sa logf_info, logf_debug
 * \note None.
 * \warning None.
 */
int logf_fatal_error( logf_instance* instance, logf_buffer* buffer,
                      int evt_error_code, char* format, ... );
/*!
 * \brief This function is called for errors.
 *
 * Chainloads off to logf_print_error.
 *
 * \param instance the logging instance
 * \param buffer target buffer
 * \param evt_error_code error code associated with this event
 * \param format printf-style format of the error message
 * \param ... variable arguments
 *
 * \return TRUE or FALSE depending on success.
 * \sa logf_error, logf_instance, logf_buffer, logf_fatal_error, logf_warn
 * \sa logf_info, logf_debug
 * \note None.
 * \warning None.
 */
int logf_error( logf_instance* instance, logf_buffer* buffer,
                int evt_error_code, char* format, ... );
/*!
 * \brief This function is called for warnings.
 *
 * Chainloads off to logf_print_error.
 *
 * \param instance the logging instance
 * \param buffer target buffer
 * \param evt_error_code error code associated with this event
 * \param format printf-style format of the error message
 * \param ... variable arguments
 *
 * \return TRUE or FALSE depending on success.
 * \sa logf_warn, logf_instance, logf_buffer, logf_fatal_error, logf_error
 * \sa logf_info, logf_debug
 * \note None.
 * \warning None.
 */
int logf_warn( logf_instance* instance, logf_buffer* buffer,
               char* format, ... );
/*!
 * \brief This function is called to log informational messages.
 *
 * Chainloads off to logf_info.
 *
 * \param instance the logging instance
 * \param buffer target buffer
 * \param evt_error_code error code associated with this event
 * \param format printf-style format of the error message
 * \param ... variable arguments
 *
 * \return TRUE or FALSE depending on success.
 * \sa logf_info, logf_instance, logf_buffer, logf_fatal_error,
 * \sa logf_error, logf_warn, logf_debug
 * \note None.
 * \warning None.
 */
int logf_info( logf_instance* instance, logf_buffer* buffer,
               char* format, ... );

/*!
 * \brief This function is called to print debug messages
 *
 * Chainloads off to logf_print_error.
 *
 * \param instance the logging instance
 * \param buffer target buffer
 * \param evt_error_code error code associated with this event
 * \param format printf-style format of the error message
 * \param ... variable arguments
 *
 * \return TRUE or FALSE depending on success.
 * \sa logf_debug, logf_instance, logf_buffer, logf_fatal_error
 * \sa logf_error, logf_warn, logf_info
 * \note None.
 * \warning None.
 */
int logf_debug( logf_instance* instance, logf_buffer* buffer,
                char* format, ... );

/*!
 * \brief [brief description]
 *
 * [detailed description]
 *
 * \param [in] [name of input parameter] [its description]
 * \param[ out] [name of output parameter] [its description]
 * \return [information about return value]
 * \sa [see also section]
 * \note [any note about the function you might have]
 * \warning [any warning if necessary]
 */
// searches for an event, returns NULL if it cannot find it.
// Maybe use hash table here for codes
// either or argument
logf_event* logf_get_event_code( logf_instance* instance, logf_buffer* buffer,
                                 int evt_error_code );
/*!
 * \brief [brief description]
 *
 * [detailed description]
 *
 * \param [in] [name of input parameter] [its description]
 * \param[ out] [name of output parameter] [its description]
 * \return [information about return value]
 * \sa [see also section]
 * \note [any note about the function you might have]
 * \warning [any warning if necessary]
 */
logf_event* logf_get_event_msg( logf_instance* instance, logf_buffer* buffer,
                                char* message );

/*!
 * \brief Set threshold  value for a particular threshold with user_code
 *
 * Changes the threshold value for threshold with user_code.
 * Pass NULL to udef if you don't want to change the function.
 *
 * \param instance the logging instance
 * \param evt_error_code
 * \param threshold
 * \param udef user defined callback function.
 *
 * \return TRUE or FALSE on whether action was successful
 * \sa logf_threshold, logf_instance, user_call
 * \note None
 * \warning None
 */
int logf_set_threshold( logf_instance* instance, int evt_error_code,
                        int threshold, user_call udef );
/*!
 * \brief Remove the specified threshold.
 *
 * Searches hash table for logf_threshold object with specified error code and "removes" it.
 *
 * \param instance the logging instance.
 * \param evt_error_code error code associated with this threshold
 * \return TRUE or FALSE on whether operation was successful.
 * \sa logf_threshold, logf_instance
 * \note May be the cause of problems in threading applications.
 * \warning None.
 */
int logf_remove_threshold( logf_instance* instance, int evt_error_code );

/*!
 * \brief Reset threshold hit values manually.
 *
 * This function should not be used by mortals.
 *
 * \param instance the logging instance.
 * \param evt_error_code error code associated with this threshold
 * \return TRUE or FALSE depending on success of operation
 * \sa logf_threshold, logf_instance
 * \note None
 * \warning None
 */
int logf_reset_threshold( logf_instance* instance, int evt_error_code );

/*!
 * \brief This function is used to clean up the logging instance.
 *
 * [detailed description]
 *
 * \param instance the logging instance
 * \return Nothing.
 * \sa logf_instance
 * \note None.
 * \warning This function has been a headache. It frees memory properly,
 * \warning but Rational Purify says that it's freeing too much. I don't think it
 * \warning should be a problem when we use this function at the end. So don't
 * \warning call this in the middle of the program, especially with threads.
 * \warning \b j/k, \b I \b fixed \b it.
 */
void logf_cleanup( logf_instance* instance );

/*!
 * \brief This function is used to flush the buffer to file.
 *
 *  This function goes through the array of events, writing them all
 *  to file.
 *
 * \param buffer target buffer
 * \param remove whether to free the events removed or not
 * \return Nothing.
 * \sa logf_buffer
 * \note None.
 * \warning AT THIS POINT, ALL FLUSHES CLEAN OUT THE
 * \warning ARRAY. IMPLEMENTATION FOR PARTIAL FLUSH WHILE RETAINING DATA
 * \warning IS NOT COMPLETED. ALWAYS CALL IT WITH SECOND ARGUMENT AS 1.
 */
void logf_flush( logf_buffer* buffer, int remove );
#endif

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

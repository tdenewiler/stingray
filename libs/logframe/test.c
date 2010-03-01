/* Test example */

/*
LOGF_FATAL 	Top priority error calling for attention: Usually occurs due to a catastrophic failure of a module.
LOGF_ERROR 	High priority error: Usually occurs due to a module encountering problems.
LOGF_WARN 	Medium priority anomaly: Usually occurs due to a module detecting an anomaly not encountered during normal operation.
LOGF_INFO 	Low priority anomaly: Provides interesting information.
LOGF_DEBUG 	Informational purposes: Provides raw data and other debug messages.

Output and explanation:
Format: Time, Priority, Code, Message
12:53:05  9ms, INFO, None, LOGGING BUFFER INITIALIZED: "labjack.txt" // our buffer is intialized
12:53:05  15ms, FATAL, 100, Last data collected: 1: 0 // first call to fatal...
woah, threshold tripped // triggers the threshold function, which prints this message.. and ....
12:53:05  19ms, INFO, 0, THRESHOLD TRIPPED! // calls logf_info to log this threshold tripping.
12:53:05  19ms, ERROR, 100, Last data collected: 2: 0 // we return to main, and call logf_error this time
meh // logf_error responds first to the calll defined by set_error_calls function.
woah, threshold tripped // before calling the threshold function. Think of it this way. set_error_calls respond to priority of errors, whereas threshold responds to # of calls for a particular error with a user-defined code.
12:53:05  21ms, INFO, 0, THRESHOLD TRIPPED! // our custom function logs this.
12:53:05  23ms, FATAL, 100, Last data collected: 3: 0 // we call again, and  the same thing happens
woah, threshold tripped
12:53:05  24ms, INFO, 0, THRESHOLD TRIPPED! // after this, we return to main and disable thresholds for that error code (100).
12:53:05  25ms, FATAL, 100, Last data collected: 4: 0 // now we're able to call it without triggering the threshold.
12:53:05  25ms, FATAL, 100, Last data collected: 5: 0


*/


/* Step #1: Include logframe.h */
#include "include/logframe.h"

/* Step #2. Define your own custom error codes. */
#define BAD_DATA_ERROR 100


void call_me_if_threshold_reached( void );
void call_me_if_error( void );

/* Step #3. Define your buffer(s), buffers are where log messages will be written to. */
logf_buffer* labjack_module_log_file;

/* Step #4:  Define your main logging instance. */
logf_instance* default_log;

int main()
{
	int data_number = 0;


	/* Step #5:  Initialize your main logging instance. */
	default_log = logf_init();

	/* Step #6: Initialize buffer(s) */
	labjack_module_log_file = logf_open_file(
	                              default_log, /* our log instance */
	                              "labjack.txt", /* filename */
	                              /* we'll only log the fatal and error messages, drop everything else */
	                              /* print to stderr too !*/
	                              LOGF_FATAL | LOGF_ERROR | LOGF_INFO | LOGF_STDERR,
	                              0 /* write to disk after 10 messages , set to 0 if you want to manually control flushes.
                                                      You might want to do this in a time-critical loop. Delay flushes until later. */
	                          );
	/* Step #7: Set Thresholds  and error callbacks */
	/* Note that these calls will still trigger the thresholds and error callbacks regardless of whether they are filtered out
	         by the buffer or not! */
	logf_add_threshold( default_log, /* our log instance */
	                    BAD_DATA_ERROR, /* this threshold is for our bad data (user-defined) */
	                    1, /* If this error occurs 3 times, call our function */
	                    call_me_if_threshold_reached /* call this one */
	                  );
	/* this will call the function when an error is encountered. Note that the function cannot be that of a threshold. */
	logf_set_error_calls( default_log, NULL, call_me_if_error, NULL, NULL, NULL );


	/* Step #8: Use it! */


	logf_fatal_error(
	    default_log, /* our log instance */
	    labjack_module_log_file, /* our buffer */
	    BAD_DATA_ERROR, /* specific fail code (user-defined) */
	    /* print stuff out in our friendly printf format */
	    "Last data collected: %d: %d", 1, data_number
	);

	logf_error(
	    default_log, /* our log instance */
	    labjack_module_log_file, /* our buffer */
	    BAD_DATA_ERROR, /* specific fail code (user-defined) */
	    /* print stuff out in our friendly printf format */
	    "Last data collected: %d: %d", 2, data_number
	);


	/* this message is never logged, it is filtered out */
	logf_warn(
	    default_log, /* our log instance */
	    labjack_module_log_file, /* our buffer */
	    /* print stuff out in our friendly printf format */
	    "Yo, wassup?"
	);

	logf_fatal_error(
	    default_log, /* our log instance */
	    labjack_module_log_file, /* our buffer */
	    BAD_DATA_ERROR, /* specific fail code (user-defined) */
	    /* print stuff out in our friendly printf format */
	    "Last data collected: %d: %d", 3, data_number
	);

	/* now, flush at a convenient time */
	logf_flush( labjack_module_log_file, 1 );

	/* we can get rid of our threshold */
	logf_remove_threshold( default_log, BAD_DATA_ERROR );

	/* these two subsequent calls do not trigger the threshold anymore */
	logf_fatal_error(
	    default_log, /* our log instance */
	    labjack_module_log_file, /* our buffer */
	    BAD_DATA_ERROR, /* specific fail code (user-defined) */
	    /* print stuff out in our friendly printf format */
	    "Last data collected: %d: %d", 4, data_number
	);

	logf_fatal_error(
	    default_log, /* our log instance */
	    labjack_module_log_file, /* our buffer */
	    BAD_DATA_ERROR, /* specific fail code (user-defined) */
	    /* print stuff out in our friendly printf format */
	    "Last data collected: %d: %d", 5, data_number
	);

	logf_flush( labjack_module_log_file, 1 );


	/* Step #9: Cleanup */
	logf_cleanup( default_log );

	return 0;

}

/* Step #10: Define our custom error functions */
void call_me_if_error( void )
{
	printf( "meh\n" );
}

void call_me_if_threshold_reached( void )
{

	printf( "woah, threshold tripped\n" );

	// you must manually log the errors here!
	logf_info(
	    default_log,
	    labjack_module_log_file,
	    "THRESHOLD TRIPPED!"
	);
	// threshold is automatically reset

}


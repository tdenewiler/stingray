/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif


	enum LOGLEVEL {
		LOGLEVEL_NONE = 0,
		LOGLEVEL_CRITICAL = 1,
		LOGLEVEL_IMPORTANT = 2,
		LOGLEVEL_INFO = 3,
		LOGLEVEL_DEBUG = 4,
		LOGLEVEL_VERBOSE = 6
	};

#ifdef DEBUG

#define LOGSTRLEN	256

	void logfile_LogPrint( char *str );
	void logfile_OpenFile( char *filename );
	void logfile_CloseFile();

	void logfile_LogLevelSet( int level );
	void logfile_LogLevelPrint( int level, char *str );
	void logfile_OpenSyslog( int facility );

#endif

#ifdef __cplusplus
}

#endif

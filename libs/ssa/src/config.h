/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

#define MAXFILENAMELEN 64
#define MAXSERVERNAMESTRLEN 64
#define MAXUSERNAMESTRLEN 64
#define SMALLCOMMANDLEN 256
#define MAXPATHLEN 512
#define MAXIPADDRLEN 16
#define MAXROBOTNAMELEN	32

	typedef struct {

		char localdatapath[MAXPATHLEN];
		char localcopydir[MAXPATHLEN];

		char cmdfilebase[MAXFILENAMELEN];
		char cmdfileext[MAXFILENAMELEN];

		char telemfilebase[MAXFILENAMELEN];
		char telemfileext[MAXFILENAMELEN];

		char logfilename[MAXFILENAMELEN];
		int  loglevel;

		char telemserver_ipaddr[MAXIPADDRLEN];
		int telemserver_baseportfetch;
		int telemserver_baseportput;

		char cmdserver_ipaddr[MAXIPADDRLEN];
		int cmdserver_baseportfetch;
		int cmdserver_baseportput;

		int numrobots;			// number of robots to process
		int docommand;			// see INSTALL file

		int reportperiod;		//telemetry reporting period in seconds

		int robotnumber;
		char robotname[MAXROBOTNAMELEN];

//	char rsynchostname[MAXSERVERNAMESTRLENGTH];
//	char rsyncsharename[MAXUSERNAMESTRLENGTH];

//	char wgethostname[MAXSERVERNAMESTRLENGTH];
//	char wgethostpath[MAXPATHLENGTH];


// for irc_client
		char javauserdir[MAXPATHLEN];
		char javaclasspath[MAXPATHLEN];

		char ircresource[MAXPATHLEN];

// for mocu_client
		char mocudir[MAXPATHLEN];
		char archivedir[MAXPATHLEN];

// for mysql_client
		char dbservername[MAXSERVERNAMESTRLEN];
		char dbdatabasename[MAXSERVERNAMESTRLEN];
		char dbusername[MAXUSERNAMESTRLEN];
		char dbuserpasswd[MAXUSERNAMESTRLEN];

	}CONFIGDATA;

//CONFIGDATA readconfig( int argc, char *argv[]);
	CONFIGDATA config_readconfig( int argc, char *argv[], char *defcfgfilename );
	void config_PrintConfig( CONFIGDATA cf );

#undef MAXFILENAMELEN
#undef MAXSERVERNAMESTRLEN
#undef MAXUSERNAMESTRLEN
#undef SMALLCOMMANDLEN
#undef MAXPATHLEN
#undef MAXPATHLEN
#undef MAXIPADDRLEN
#undef MAXROBOTNAMELEN


#ifdef __cplusplus
}

#endif

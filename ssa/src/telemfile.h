/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	STATUS telemfile_ParseTelemFile( char *filespec );

	STATUS telemfile_ReadFile( char *filespec );

	void telemfile_WriteFile( char *filespec, STATUS s );

	void telemfile_WriteEngStatus( char *filespec, STATUS s );

	void telemfile_WriteSciData( char *name, int robotID
	                             , char *filespec, SCI_DBL s, int places );


	void telemfile_WriteTelemServer( char *ipaddress, char *portnum, STATUS st );
	void telemfile_FetchTelemServer( char *sIPaddr, char *sPortNum, char *sOutfile );
	void telemfile_WriteFileDirect( char *filespec, STATUS st );


#ifdef __cplusplus
}

#endif

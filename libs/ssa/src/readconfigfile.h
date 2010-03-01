/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	int readconfig_int( char *filespec, char *section, char *varname, int defval );
	float readconfig_float( char *filespec, char *section, char *varname, float defval );
	char * readconfig_str( char *filespec, char *section, char *varname, char *defval );
	void readconfig_str2( char *outstr, int outstrlen, char *filespec, char *section, char *varname, char *defval );


#ifdef __cplusplus
}

#endif

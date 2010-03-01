/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	void socket_FetchFile( char *sIPaddr, char *sPortNum, char *sOutfile );

	void socket_FetchToString( char *sIPaddr, char *sPortNum, char *sOutString, int nSize );

	int socket_OpenSocket( char *sIPaddr, char *sPortNum );

#ifdef __cplusplus
}

#endif

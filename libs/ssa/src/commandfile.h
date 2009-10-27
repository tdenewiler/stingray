/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

//void commandfile_HandleCmdFile(CONFIGDATA cf);
//COMMAND commandfile_ParseCommandFile(CONFIGDATA cf, int robnum);
	COMMAND commandfile_ParseCommandFile( char *filespec );
//void commandfile_FetchCommandFile(CONFIGDATA cf, int robnum);

///void commandfile_FetchCommandServer2(char *sIPaddr,char *sPortNum,char *sOutfile);

	void commandfile_WriteCommandServer( char *sIPaddr, char *sPortNum, COMMAND cmd );

	void commandfile_WriteFileDirect( FILE *fp, COMMAND cmd );

	COMMAND commandfile_GetStatus( void );
	int commandfile_StartThread( CONFIGDATA cf );



#ifdef __cplusplus
}

#endif

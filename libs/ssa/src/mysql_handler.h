/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

	typedef MYSQL *pMYSQL;

	int init_MySQL( pMYSQL db, char *dbhost, char *dbname, char *dbuser, char *dbpswd );

	void writeMySQL_status( pMYSQL db, STATUS st, STATUS old );
	void writeMySQL_status2( pMYSQL db, STATUS st );


#ifdef __cplusplus
}

#endif

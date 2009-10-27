/**
*  @file
*  @ingroup common
**/

#ifdef __cplusplus
extern "C"
{
#endif

#define NUMPLATTYPES	10

#define PLATTYPESTRLEN	32

#ifdef NEED_TYPENAMES
	static char platformtypenames[NUMPLATTYPES][PLATTYPESTRLEN] = {
		"OASIS",
		"SimBoat",
		"Buoy",
		"SimBuoy",
		"Aerostat",
		"MSB",
		"LSB",
		"UUV",
		"NULL",
		"NULL"
	};
#endif

	typedef enum {
		PLATFORMTYPE_OASIS,
		PLATFORMTYPE_SIMBOAT,
		PLATFORMTYPE_BUOY,
		PLATFORMTYPE_SIMBUOY,
		PLATFORMTYPE_AEROSTAT,
		PLATFORMTYPE_MSB,
		PLATFORMTYPE_LSB,
		PLATFORMTYPE_UUV,
		PLATFORMTYPE_TYPE8,
		PLATFORMTYPE_TYPE9
	}PLATFORMTYPE;


#ifdef __cplusplus
}

#endif

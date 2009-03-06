/**
 *  \file parser.h
 *  \brief Parses configuration files for initialization values.
 */

#ifndef _PARSER_H_
#define _PARSER_H_

/******************************
**
** #defines
**
******************************/

#ifndef TRUE
#define TRUE  1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef STRING_SIZE
#define STRING_SIZE 64
#endif /* STRING_SIZE */

#ifndef MAX_TOKENS
#define MAX_TOKENS  10
#endif /* MAX_TOKENS */

#ifndef APPS
#define APPS
#define STINGRAY    1
#define CLIENT_GUI  2
#endif /* APPS */


/******************************
**
** Data types
**
******************************/

#ifndef _CONF_VARS_
#define _CONF_VARS_

//! Variables to initialize. Preferably using a configuration file that is
//! parsed at the start of the program. Otherwise default values are used.

typedef struct _CONF_VARS {
	int         enable_labjack;
	int         enable_imu;
	int         imu_baud;
	int         imu_stab;
	char        imu_port[STRING_SIZE];
	int         enable_net;
	int         net_mode;
	short int   api_port;
	char        server_IP[STRING_SIZE];
	char        vision_IP[STRING_SIZE];
	int         enable_vision;
	short int   vision_port;
	int         vision_window;
	char        planner_IP[STRING_SIZE];
	int         enable_planner;
	short int   planner_port;
	short int   labjackd_port;
	int         max_api_clients;
	double      kp_yaw;
	double      ki_yaw;
	double      kd_yaw;
	double      kp_roll;
	double      ki_roll;
	double      kd_roll;
	double      kp_pitch;
	double      ki_pitch;
	double      kd_pitch;
	double      kp_depth;
	double      ki_depth;
	double      kd_depth;
	double      kp_ax;
	double      ki_ax;
	double      kd_ax;
	double      kp_ay;
	double      ki_ay;
	double      kd_ay;
	double      kp_az;
	double      ki_az;
	double      kd_az;
	float       target_pitch;
	float       target_roll;
	float       target_yaw;
	float       target_depth;
	float       target_ax;
	float       target_ay;
	float       target_az;
	int         period_pitch;
	int         period_roll;
	int         period_yaw;
	int         period_depth;
	int         period_ax;
	int         period_ay;
	int         period_az;
	int         period_vision;
	int         period_planner;
	int         enable_pololu;
	int         pololu_baud;
	char        pololu_port[STRING_SIZE];
	int         lthruster_center;
	int         lthruster_min;
	int         lthruster_max;
	int         lservo_rad_center;
	int         lservo_rad_min;
	int         lservo_rad_max;
	int         lservo_ang_center;
	int         lservo_ang_min;
	int         lservo_ang_max;
	int         lwing_center;
	int         lwing_min;
	int         lwing_max;
	int         ltail_center;
	int         ltail_min;
	int         ltail_max;
	int         rthruster_center;
	int         rthruster_min;
	int         rthruster_max;
	int         rservo_rad_center;
	int         rservo_rad_min;
	int         rservo_rad_max;
	int         rservo_ang_center;
	int         rservo_ang_min;
	int         rservo_ang_max;
	int         rwing_center;
	int         rwing_min;
	int         rwing_max;
	int         rtail_center;
	int         rtail_min;
	int         rtail_max;
	int         enable_gui;
	int         window_height;
	int         window_width;
	int         enable_gps;
	char        gps_port[STRING_SIZE];
	int         gps_baud;
	int         enable_relay;
	char        relay_port[STRING_SIZE];
	int         relay_baud;
	int         debug_level;
	int         op_mode;
	int         enable_log;
} CONF_VARS;

#endif /* _CONF_VARS_ */


/******************************
**
** Function declarations
**
******************************/

//! Parses a line for configuration values.
//! \param config An array of initialization variables.
void parse_line( CONF_VARS *config );

//! Parses a configuration file for variable data.
//! \param filename File to open for configuration values.
//! \param config An array of initialization variables.
void parse_config( const char *filename,
                   CONF_VARS *config
                 );

//! Parses the command line arguments looking for valid options.
//! Returns -1 if help is invoked or 0 on success.
//! \param argc Number of command lin arguments.
//! \param argv Array of command line arguments.
//! \param app Indicates which application is calling this function.
//! \param filename Configuration file to use.
//! \return 0 on success, else -1 when help is invoked.
int parse_cla( int argc,
               char *argv[],
               CONF_VARS *config,
               int app,
               const char *filename
             );

//! Sets default values for the initialization variables. This function should
//! be followed by a call to parse_cla() in order to fill in the variables with
//! values set in a configuration file.
//! \param config An array of initialization variables.
void parse_default_config( CONF_VARS *config );


#endif /* _PARSER_H_ */


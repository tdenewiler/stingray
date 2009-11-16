/**
 *  \file parser.h
 *  \brief Parses configuration files for initialization values.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

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
    int         enable_server;
	int			enable_nav;
    int         net_mode;
    short int   nav_port;
	short int	server_port;
    char        nav_IP[STRING_SIZE];
    char        vision_IP[STRING_SIZE];
    int         enable_vision;
    short int   vision_port;
    int         vision_window;
    int			vision_angle;
    char		vision_task[STRING_SIZE];
    char        planner_IP[STRING_SIZE];
    int         enable_planner;
    short int   planner_port;
    char        labjackd_IP[STRING_SIZE];
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
    double      kp_fx;
    double      ki_fx;
    double      kd_fx;
    double      kp_fy;
    double      ki_fy;
    double      kd_fy;
    double      kp_buoy;
    double      ki_buoy;
    double      kd_buoy;
    double      kp_buoy_depth;
    double      ki_buoy_depth;
    double      kd_buoy_depth;
    double 		kp_roll_lateral;
    double 		kp_depth_forward;
    double 		kp_place_holder;
    float       target_pitch;
    float       target_roll;
    float       target_yaw;
    float       target_depth;
    float       target_ax;
    float       target_ay;
    float       target_az;
    float       target_fx;
    float       target_fy;
    float       target_speed;
    float       period_pitch;
    float       period_roll;
    float       period_yaw;
    float       period_depth;
    float       period_ax;
    float       period_ay;
    float       period_az;
    float       period_vision;
    float       period_planner;
	float		period_input;
    int         enable_pololu;
    int         pololu_baud;
    char        pololu_port[STRING_SIZE];
    float       pipe_hL;
    float       pipe_hH;
    float       pipe_sL;
    float       pipe_sH;
    float       pipe_vL;
    float       pipe_vH;
    float       buoy_hL;
    float       buoy_hH;
    float       buoy_sL;
    float       buoy_sH;
    float       buoy_vL;
    float       buoy_vH;
    float       fence_hL;
    float       fence_hH;
    float       fence_sL;
    float       fence_sH;
    float       fence_vL;
    float       fence_vH;
    float       gate_hL;
    float       gate_hH;
    float       gate_sL;
    float       gate_sH;
    float       gate_vL;
    float       gate_vH;
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
    float       enable_log;
	float		depth_gate;
	float		depth_pipe;
	float		depth_buoy;
	float		depth_fence;
	float		depth_boxes;
	float		depth_suitcase_search;
	float		depth_suitcase_pickup;
	float		depth_octagon;
	float		depth_surface;
	float		fence_min;
	float		heading_buoy;
	float		heading_gate;
	int			task_start;
	int			subtask_start;
	int			course_start;
	int			dock_time;
	int			buoy_blind_time;
	int			save_image_front;
	int			save_image_bottom;
	int			save_image_post;
	float		save_image_rate;
	char		save_image_dir[STRING_SIZE];
	int			save_log_post;
	float		open_image_rate;
	char		open_image_dir[STRING_SIZE];
	float		task_init_yaw;
	int			input_size;
	int			input_type;
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

//! Prints out the values of the configuration file variables.
//! \param config An array of initialization variables.
void parse_print_config( CONF_VARS *config );


#endif /* PARSER_H */

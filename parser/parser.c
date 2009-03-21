/******************************************************************************
 *
 *  Title:        parser.c
 *
 *  Description:  Parses configuration files to initialize configuration values.
 *
 *****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "parser.h"


static char tokens[MAX_TOKENS][STRING_SIZE];


/******************************************************************************
 *
 * Title:       void parse_line( CONF_VARS *config )
 *
 * Description: Parses a line for configuration values.
 *
 * Input:       config: An array of configuration variables.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void parse_line( CONF_VARS *config )
{
	int ii;
	int ip_octal[4];

	for ( ii = 0; ii < 4; ii++ ) {
		memset( &( ip_octal[ii] ), -1, sizeof( ip_octal[ii] ) );
	}

	/* Check here to find out which variables to fill in. */
	/* Server parameters */
	if ( strncmp( tokens[0], "server", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d.%d.%d.%d"
			        , &( ip_octal[0] )
			        , &( ip_octal[1] )
			        , &( ip_octal[2] )
			        , &( ip_octal[3] )
			      );
			strncpy( config->server_IP, tokens[2], STRING_SIZE );
		}
	} /* end server parameters */

	/* Vision parameters */
	if ( strncmp( tokens[0], "vision", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d.%d.%d.%d"
			        , &( ip_octal[0] )
			        , &( ip_octal[1] )
			        , &( ip_octal[2] )
			        , &( ip_octal[3] )
			      );
			strncpy( config->vision_IP, tokens[2], STRING_SIZE );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%hd", &config->vision_port );
		}
	} /* end vision parameters */

	/* Planner parameters */
	if ( strncmp( tokens[0], "planner", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d.%d.%d.%d"
			        , &( ip_octal[0] )
			        , &( ip_octal[1] )
			        , &( ip_octal[2] )
			        , &( ip_octal[3] )
			      );
			strncpy( config->planner_IP, tokens[2], STRING_SIZE );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%hd", &config->planner_port );
		}
	} /* end planner parameters */

	/* GUI parameters */
	else if ( strncmp( tokens[0], "window", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "height", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->window_height );
		}
		else if ( strncmp( tokens[1], "width", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->window_width );
		}
	}

	/* end GUI parameters */

	/* net parameters */
	else if ( strncmp( tokens[0], "api", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "clients", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->max_api_clients );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%hd", &config->api_port );
		}
	}
	else if ( strncmp( tokens[0], "net", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "mode", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->net_mode );
		}
	}
	else if ( strncmp( tokens[0], "vision", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%hd", &config->vision_port );
		}
		else if ( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d.%d.%d.%d"
			        , &( ip_octal[0] )
			        , &( ip_octal[1] )
			        , &( ip_octal[2] )
			        , &( ip_octal[3] )
			      );
			strncpy( config->vision_IP, tokens[2], STRING_SIZE );
		}
		else if ( strncmp( tokens[1], "window", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->vision_window );
		}
	}

	/* end net parameters */

	/* enable parameters */
	else if ( strncmp( tokens[0], "enable", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "imu", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_imu );
		}
		else if ( strncmp( tokens[1], "logs", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_log );
		}
		else if ( strncmp( tokens[1], "gui", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_gui );
		}
		else if ( strncmp( tokens[1], "gps", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_gps );
		}
		else if ( strncmp( tokens[1], "relay", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_relay );
		}
		else if ( strncmp( tokens[1], "pololu", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_pololu );
		}
		else if ( strncmp( tokens[1], "net", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_net );
		}
		else if ( strncmp( tokens[1], "labjack", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_labjack );
		}
		else if ( strncmp( tokens[1], "vision", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_vision );
		}
		else if ( strncmp( tokens[1], "planner", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->enable_planner );
		}
	}

	/* end enable parameters */

	/* imu parameters */
	else if ( strncmp( tokens[0], "imu", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->imu_baud );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			strncpy( config->imu_port, tokens[2], STRING_SIZE );
		}
		else if ( strncmp( tokens[1], "stab", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->imu_stab );
		}
	}

	/* end imu parameters */

	/* pololu parameters */
	else if ( strncmp( tokens[0], "pololu", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->pololu_baud );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			strncpy( config->pololu_port, tokens[2], STRING_SIZE );
		}
	}

	/* end pololu parameters */

	/* operating mode parameters */
	else if ( strncmp( tokens[0], "op", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "mode", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->op_mode );
		}
	}

	/* end operating mode parameters */

	/* labjackd parameters */
	else if ( strncmp( tokens[0], "labjackd", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d.%d.%d.%d"
			        , &( ip_octal[0] )
			        , &( ip_octal[1] )
			        , &( ip_octal[2] )
			        , &( ip_octal[3] )
			      );
			strncpy( config->labjackd_IP, tokens[2], STRING_SIZE );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%hd", &config->labjackd_port );
		}
	}

	/* end labjackd parameters */

	/* GPS parameters */
	else if ( strncmp( tokens[0], "gps", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->gps_baud );
		}
		else if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			strncpy( config->gps_port, tokens[2], STRING_SIZE );
		}
	}

	/* end GPS parameters */

	/* USB relay board */
	else if ( strncmp( tokens[0], "relay", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
			strncpy( config->relay_port, tokens[2], STRING_SIZE );
		}
		else if ( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->relay_baud );
		}
	}

	/* end USB relay board */

	/* Set the debug level. */
	else if ( strncmp( tokens[0], "debug", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "level", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->debug_level );
		}
	}

	/* end debug level */

	/* PID parameters. */
	else if ( strncmp( tokens[0], "kp", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_yaw );
		}
		else if ( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_roll );
		}
		else if ( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_pitch );
		}
		else if ( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_ax );
		}
		else if ( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_ay );
		}
		else if ( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_az );
		}
		else if ( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kp_depth );
		}
	}

	else if ( strncmp( tokens[0], "ki", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_yaw );
		}
		else if ( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_roll );
		}
		else if ( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_pitch );
		}
		else if ( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_ax );
		}
		else if ( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_ay );
		}
		else if ( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_az );
		}
		else if ( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->ki_depth );
		}
	}

	else if ( strncmp( tokens[0], "kd", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_yaw );
		}
		else if ( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_roll );
		}
		else if ( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_pitch );
		}
		else if ( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_ax );
		}
		else if ( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_ay );
		}
		else if ( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_az );
		}
		else if ( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%lf", &config->kd_depth );
		}
	}
	else if ( strncmp( tokens[0], "period", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_pitch );
		}
		else if ( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_roll );
		}
		else if ( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_yaw );
		}
		else if ( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_depth );
		}
		else if ( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_ax );
		}
		else if ( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_ay );
		}
		else if ( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_az );
		}
		else if ( strncmp( tokens[1], "vision", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_vision );
		}
		else if ( strncmp( tokens[1], "planner", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->period_planner );
		}
	}
	/* end PID parameters */

	/* target values */
	else if ( strncmp( tokens[0], "target", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_pitch );
		}
		else if ( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_roll );
		}
		else if ( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_yaw );
		}
		else if ( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_depth );
		}
		else if ( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_ax );
		}
		else if ( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_ay );
		}
		else if ( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->target_az );
		}
	}

	/* end target values */

	/* Actuator parameters */
	else if ( strncmp( tokens[0], "left", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "thruster", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "center", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->lthruster_center );
			}
			else if ( strncmp( tokens[2], "low", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->lthruster_min );
			}
			else if ( strncmp( tokens[2], "high", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->lthruster_max );
			}
		}
		else if ( strncmp( tokens[1], "servo", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "ang", STRING_SIZE ) == 0 ) {
				if ( strncmp( tokens[3], "center", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->lservo_ang_center );
				}
				else if ( strncmp( tokens[3], "low", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->lservo_ang_min );
				}
				else if ( strncmp( tokens[3], "high", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->lservo_ang_max );
				}
			}
			else if ( strncmp( tokens[2], "rad", STRING_SIZE ) == 0 ) {
				if ( strncmp( tokens[3], "center", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->lservo_rad_center );
				}
				else if ( strncmp( tokens[3], "low", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->lservo_rad_min );
				}
				else if ( strncmp( tokens[3], "high", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->lservo_rad_max );
				}
			}
		}
		else if ( strncmp( tokens[1], "wing", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "center", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->lwing_center );
			}
			else if ( strncmp( tokens[2], "low", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->lwing_min );
			}
			else if ( strncmp( tokens[2], "high", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->lwing_max );
			}
		}
		else if ( strncmp( tokens[1], "tail", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "center", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->ltail_center );
			}
			else if ( strncmp( tokens[2], "low", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->ltail_min );
			}
			else if ( strncmp( tokens[2], "high", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->ltail_max );
			}
		}
	} /* end left actuators */

	/* right actuators */
	else if ( strncmp( tokens[0], "right", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "thruster", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "center", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rthruster_center );
			}
			else if ( strncmp( tokens[2], "low", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rthruster_min );
			}
			else if ( strncmp( tokens[2], "high", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rthruster_max );
			}
		}
		else if ( strncmp( tokens[1], "servo", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "ang", STRING_SIZE ) == 0 ) {
				if ( strncmp( tokens[3], "center", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->rservo_ang_center );
				}
				else if ( strncmp( tokens[3], "low", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->rservo_ang_min );
				}
				else if ( strncmp( tokens[3], "high", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->rservo_ang_max );
				}
			}
			else if ( strncmp( tokens[2], "rad", STRING_SIZE ) == 0 ) {
				if ( strncmp( tokens[3], "center", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->rservo_rad_center );
				}
				else if ( strncmp( tokens[3], "low", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->rservo_rad_min );
				}
				else if ( strncmp( tokens[3], "high", STRING_SIZE ) == 0 ) {
					sscanf( tokens[4], "%d", &config->rservo_rad_max );
				}
			}
		}
		else if ( strncmp( tokens[1], "wing", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "center", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rwing_center );
			}
			else if ( strncmp( tokens[2], "low", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rwing_min );
			}
			else if ( strncmp( tokens[2], "high", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rwing_max );
			}
		}
		else if ( strncmp( tokens[1], "tail", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "center", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rtail_center );
			}
			else if ( strncmp( tokens[2], "low", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rtail_min );
			}
			else if ( strncmp( tokens[2], "high", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->rtail_max );
			}
		}
	} /* end right actuators */
	/* end actuator parameters */

} /* end parse_line() */


/******************************************************************************
 *
 * Title:       void parse_config( const char *filename,
 *                                 CONF_VARS *config
 *                                )
 *
 * Description: Parses a configuration file for variable data such as IP
 *              addresses.
 *
 * Input:       filename: File to open for configuration values.
 *              config: An array of initialization variables.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void parse_config( const char *filename,
                   CONF_VARS *config
                 )
{
	char token_buf[STRING_SIZE] = "\0";
	int  filling_in_token       = FALSE;
	int  line_number            = 0;
	int  comment                = FALSE;
	unsigned int ii             = 0;
	int jj                      = 0;
	int last_value_space        = FALSE;
	int token_number            = 0;
	FILE *cfg_fd;

	cfg_fd = fopen( filename, "r" );
	if ( cfg_fd == NULL ) {
		//printf( "PARSE_CONFIG: Error opening config file %s\n", filename );
		return;
	}

	for ( ii = 0; ii < MAX_TOKENS; ii++ ) {
		memset( &( tokens[ii] ), 0, sizeof( tokens[ii] ) );
	}
	memset( &( token_buf ), 0, sizeof( token_buf ) );

	if ( fgets( token_buf, STRING_SIZE, cfg_fd ) == NULL ) {
		/* Do nothing. */
	}

	while ( feof( cfg_fd ) == 0 ) {
		line_number++;
		for ( ii = 0; ii < sizeof( token_buf ); ii++ ) {
			if ( filling_in_token == FALSE ) {
				if ( token_buf[ii] == '\n' ) {
					break;
				}
				else if ( token_buf[ii] == '#' ) {
					comment = TRUE;
					/* Prevent whitespace betweeen last token and comment from
					 * increasing token count. */
					if ( last_value_space == TRUE ) {
						token_number--;
						last_value_space = FALSE;
					}
					parse_line( config );
					jj = 0;
					break;
				}
				else if ( isspace( token_buf[ii] ) ) {
					/* Do nothing. Still looking on current line. */
				}
				else {
					filling_in_token = TRUE;
					tokens[token_number][jj++] = token_buf[ii];
				}
			}
			else {
				if ( token_buf[ii] == '\n' ) {
					parse_line( config );
					token_number++;
					jj = 0;
					filling_in_token = FALSE;
					break;
				}
				else if ( token_buf[ii] == '#' ) {
					parse_line( config );
					token_number++;
					jj = 0;
					filling_in_token = FALSE;
					break;
				}
				else if ( isspace( token_buf[ii] ) ) {
					last_value_space = TRUE;
					token_number++;
					jj = 0;
					filling_in_token = FALSE;
				}
				else {
					tokens[token_number][jj++] = token_buf[ii];
				}
			}
		}

		jj = 0;
		token_number = 0;

		for ( ii = 0; ii < MAX_TOKENS; ii++ ) {
			memset( &( tokens[ii] ), 0, sizeof( tokens[ii] ) );
		}
		memset( &( token_buf ), 0, sizeof( token_buf ) );
		if ( fgets( token_buf, STRING_SIZE, cfg_fd ) == NULL ) {
			/* Do nothing. */
		}
	}

	fclose( cfg_fd );
} /* end parse_config() */


/******************************************************************************
 *
 * Title:       int parse_cla( int argc,
 *                             char *argv[],
 *                             CONF_VARS *config,
 *                             int app
 *                             const char *filename
 *                            )
 *
 * Description: Parses command line arguments and sets appropriate variables.
 *
 * Input:       argc: Number of command lin arguments.
 *              argv: Array of command line arguments.
 *              config: An array of initialization variables.
 *              app: Indicates which application is calling this function.
 *              filename: Configuration file to use.
 *
 * Output:      0 on success.
 *              -1 when help is invoked.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int parse_cla( int argc,
               char *argv[],
               CONF_VARS *config,
               int app,
               const char *filename
             )
{
	/* Get command line arguments and parse configuration file. */
	char *cvalue = NULL;
	int index;
	int cmd;

	/* Parse the configuration file passed in on command line. */
	opterr = 0;

	while ( ( cmd = getopt( argc, argv, "c:h" ) ) != -1 ) {
		switch ( cmd ) {

			case 'c':
				cvalue = optarg;
				break;

			case 'h':
				///print_help( );
				///print_config_help( );
				return -1;

			default:
				///print_help( );
				///print_config_help( );
				return -1;
		}
	}

	//printf( "\nPARSE_CLA: Configuration file specified = %s\n", cvalue );

	for ( index = optind; index < argc; index++ ) {
		//printf( "PARSE_CLA: Non-option argument %s\n", argv[index] );
	}

	if ( cvalue != NULL ) {
		parse_config( cvalue, config );
		//printf( "\n" );
	}
	else {
		//printf( "PARSE_CLA: Using default configuration file %s.\n\n",
		//        filename);
		//print_help( );
		parse_config( filename, config );
		//printf( "\n" );
	}
	/* End of get command line arguments and parse configuration file. */

	return 0;
} /* end parse_cla() */


/******************************************************************************
 *
 * Title:       void parse_default_config( CONF_VARS *config )
 *
 * Description: Sets default values for the initialization variables. This
 *              function should be followed by a call to parse_cla() in order to
 *              fill in the variables with values set in a configuration file.
 *
 * Input:       config: An array of initialization variables.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void parse_default_config( CONF_VARS *config )
{
	/* labjack */
	config->enable_labjack = TRUE;

	/* imu */
	config->enable_imu = TRUE;
	config->imu_baud = 38400;
	config->imu_stab = TRUE;
	strncpy( config->imu_port, "/dev/ttyUSB5", STRING_SIZE );

	/* net */
	config->enable_net = TRUE;
	config->net_mode = 1;
	config->api_port = 2000;
	strncpy( config->server_IP, "127.0.0.1", STRING_SIZE );
	config->max_api_clients = 5;

	/* pid */
	config->kp_yaw = 1;
	config->ki_yaw = 0;
	config->kd_yaw = 0;
	config->kp_roll = 1;
	config->ki_roll = 0;
	config->kd_roll = 0;
	config->kp_pitch = 1;
	config->ki_pitch = 0;
	config->kd_pitch = 0;
	config->kp_depth = 0;
	config->ki_depth = 0;
	config->kd_depth = 0;
	config->kp_ax = 1;
	config->ki_ax = 0;
	config->kd_ax = 0;
	config->kp_ay = 1;
	config->ki_ay = 0;
	config->kd_ay = 0;
	config->kp_az = 1;
	config->ki_az = 0;
	config->kd_az = 0;

	/* target values */
	config->target_pitch = 0.0;
	config->target_roll = 0.0;
	config->target_yaw = 0.0;
	config->target_depth = 0.0;
	config->target_ax = 0.0;
	config->target_ay = 0.0;
	config->target_az = 0.0;

	/* servos */
	config->enable_pololu = TRUE;
	config->pololu_baud = 9600;
	strncpy( config->pololu_port, "/dev/ttyUSB4", STRING_SIZE );
	config->lthruster_center = 0;
	config->lthruster_min = 0;
	config->lthruster_max = 100;
	config->lservo_rad_center = 0;
	config->lservo_rad_min = 0;
	config->lservo_rad_max = 100;
	config->lservo_ang_center = 0;
	config->lservo_ang_min = 0;
	config->lservo_ang_max = 100;
	config->lwing_center = 0;
	config->lwing_min = 0;
	config->lwing_max = 100;
	config->ltail_center = 0;
	config->ltail_min = 0;
	config->ltail_max = 100;
	config->rthruster_center = 0;
	config->rthruster_min = 0;
	config->rthruster_max = 100;
	config->rservo_rad_center = 0;
	config->rservo_rad_min = 0;
	config->rservo_rad_max = 100;
	config->rservo_ang_center = 0;
	config->rservo_ang_min = 0;
	config->rservo_ang_max = 100;
	config->rwing_center = 0;
	config->rwing_min = 0;
	config->rwing_max = 100;
	config->rtail_center = 0;
	config->rtail_min = 0;
	config->rtail_max = 100;

	/* gui */
	config->enable_gui = FALSE;
	config->window_height = 600;
	config->window_width = 800;

	/* gps */
	config->enable_gps = FALSE;
	strncpy( config->gps_port, "/dev/ttyUSB6", STRING_SIZE );
	config->gps_baud = 9600;

	/* relay */
	config->enable_relay = FALSE;
	strncpy( config->relay_port, "/dev/ttyUSB7", STRING_SIZE );
	config->relay_baud = 9600;

	/* other */
	config->debug_level = 5;
	config->op_mode = 1;
	config->enable_log = FALSE;
} /* end parse_default_config() */

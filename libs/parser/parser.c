/*------------------------------------------------------------------------------
 *
 *  Title:        parser.c
 *
 *  Description:  Parses configuration files to initialize configuration values.
 *
 *----------------------------------------------------------------------------*/

#include "parser.h"

static char tokens[MAX_TOKENS][STRING_SIZE];

/*------------------------------------------------------------------------------
 * void parse_line()
 * Parses a line for configuration values.
 *----------------------------------------------------------------------------*/

void parse_line(CONF_VARS *config)
{
    int ii;
    int ip_octal[4];

    for( ii = 0; ii < 4; ii++ ) {
        memset( &( ip_octal[ii] ), -1, sizeof( ip_octal[ii] ) );
    }

    /// Check here to find out which variables to fill in.
    /// Nav parameters
    if( strncmp( tokens[0], "nav", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d.%d.%d.%d"
                    , &( ip_octal[0] )
                    , &( ip_octal[1] )
                    , &( ip_octal[2] )
                    , &( ip_octal[3] )
                  );
            strncpy( config->nav_IP, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%hd", &config->nav_port );
        }
    } /// end nav parameters

	/// Server parameters
    if( strncmp( tokens[0], "server", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%hd", &config->server_port );
        }
    } /// end server parameters

    /// Vision parameters
    if( strncmp( tokens[0], "vision", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d.%d.%d.%d"
                    , &( ip_octal[0] )
                    , &( ip_octal[1] )
                    , &( ip_octal[2] )
                    , &( ip_octal[3] )
                  );
            strncpy( config->vision_IP, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%hd", &config->vision_port );
        }
        else if ( strncmp( tokens[1], "task", STRING_SIZE ) == 0 ) {
        	strncpy( config->vision_task, tokens[2], STRING_SIZE );
		}
    }
	if( strncmp( tokens[0], "save", STRING_SIZE ) == 0 ) {
		if( strncmp( tokens[1], "image", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "front", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->save_image_front );
			}
			else if ( strncmp( tokens[2], "bottom", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->save_image_bottom );
			}
			else if ( strncmp( tokens[2], "color", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->save_image_color );
			}
			else if ( strncmp( tokens[2], "binary", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->save_image_binary );
			}
			else if ( strncmp( tokens[2], "rate", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%f", &config->save_image_rate );
			}
			else if ( strncmp( tokens[2], "dir", STRING_SIZE ) == 0 ) {
				strncpy( config->save_image_dir, tokens[3], STRING_SIZE );
			}
		}
	}
	if( strncmp( tokens[0], "open", STRING_SIZE ) == 0 ) {
		if ( strncmp( tokens[1], "image", STRING_SIZE ) == 0 ) {
			if ( strncmp( tokens[2], "rate", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%f", &config->open_image_rate );
			}
			else if ( strncmp( tokens[2], "dir", STRING_SIZE ) == 0 ) {
				strcpy( config->open_image_dir, tokens[3] );
			}
		}
	}
	/// end vision parameters

    /// Planner parameters
    if( strncmp( tokens[0], "planner", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d.%d.%d.%d"
                    , &( ip_octal[0] )
                    , &( ip_octal[1] )
                    , &( ip_octal[2] )
                    , &( ip_octal[3] )
                  );
            strncpy( config->planner_IP, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%hd", &config->planner_port );
        }
    } /// end planner parameters

    /// GUI parameters
    if( strncmp( tokens[0], "window", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "height", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->window_height );
        }
        else if( strncmp( tokens[1], "width", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->window_width );
        }
    }
    /// end GUI parameters

    /// net parameters
    if( strncmp( tokens[0], "api", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "clients", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->max_api_clients );
        }
    }
    else if( strncmp( tokens[0], "net", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "mode", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->net_mode );
        }
    }
    else if( strncmp( tokens[0], "vision", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%hd", &config->vision_port );
        }
        else if( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d.%d.%d.%d"
                    , &( ip_octal[0] )
                    , &( ip_octal[1] )
                    , &( ip_octal[2] )
                    , &( ip_octal[3] )
                  );
            strncpy( config->vision_IP, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "window", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->vision_window );
        }
        else if( strncmp( tokens[1], "angle", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->vision_angle );
        }
    }
    /// end net parameters

    /// enable parameters
    else if( strncmp( tokens[0], "enable", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "imu", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_imu );
        }
        else if( strncmp( tokens[1], "logs", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->enable_log );
        }
        else if( strncmp( tokens[1], "gui", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_gui );
        }
        else if( strncmp( tokens[1], "gps", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_gps );
        }
        else if( strncmp( tokens[1], "relay", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_relay );
        }
        else if( strncmp( tokens[1], "pololu", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_pololu );
        }
        else if( strncmp( tokens[1], "server", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_server );
        }
        else if( strncmp( tokens[1], "labjack", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_labjack );
        }
        else if( strncmp( tokens[1], "vision", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_vision );
        }
        else if( strncmp( tokens[1], "planner", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_planner );
        }
        else if( strncmp( tokens[1], "nav", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->enable_nav );
        }
    }
    /// end enable parameters

    /// imu parameters
    else if( strncmp( tokens[0], "imu", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->imu_baud );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            strncpy( config->imu_port, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "stab", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->imu_stab );
        }
    }
    /// end imu parameters

    /// pololu parameters
    else if( strncmp( tokens[0], "pololu", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->pololu_baud );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            strncpy( config->pololu_port, tokens[2], STRING_SIZE );
        }
    }
    /// end pololu parameters

    /// operating mode parameters
    else if( strncmp( tokens[0], "op", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "mode", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->op_mode );
        }
    }
    /// end operating mode parameters

    /// labjackd parameters
    else if( strncmp( tokens[0], "labjackd", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "ip", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d.%d.%d.%d"
                    , &( ip_octal[0] )
                    , &( ip_octal[1] )
                    , &( ip_octal[2] )
                    , &( ip_octal[3] )
                  );
            strncpy( config->labjackd_IP, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%hd", &config->labjackd_port );
        }
    }
    /// end labjackd parameters

    /// GPS parameters
    else if( strncmp( tokens[0], "gps", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->gps_baud );
        }
        else if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            strncpy( config->gps_port, tokens[2], STRING_SIZE );
        }
    }
    /// end GPS parameters

    /// USB relay board
    else if( strncmp( tokens[0], "relay", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "port", STRING_SIZE ) == 0 ) {
            strncpy( config->relay_port, tokens[2], STRING_SIZE );
        }
        else if( strncmp( tokens[1], "baud", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->relay_baud );
        }
    }
    /// end USB relay board

    /// Set the debug level
    else if( strncmp( tokens[0], "debug", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "level", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%d", &config->debug_level );
        }
    }
    /// end debug level

    /// PID parameters
    else if( strncmp( tokens[0], "kp", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kp_yaw );
        }
        else if( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
        	if( strncmp( tokens[2], "lateral", STRING_SIZE ) == 0 ) {
        		sscanf( tokens[3], "%lf", &config->kp_roll_lateral );
			}
			else {
	            sscanf( tokens[2], "%lf", &config->kp_roll );
			}
        }
        else if( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kp_pitch );
        }
        else if( strncmp( tokens[1], "fx", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kp_fx );
        }
        else if( strncmp( tokens[1], "fy", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kp_fy );
        }
        else if( strncmp( tokens[1], "buoy", STRING_SIZE ) == 0 ) {
        	if( strncmp( tokens[2], "depth", STRING_SIZE ) == 0 ) {
        		sscanf( tokens[3], "%lf", &config->kp_buoy_depth );
			}
			else {
	            sscanf( tokens[2], "%lf", &config->kp_buoy );
			}
        }
        else if( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
        	if( strncmp( tokens[2], "lateral", STRING_SIZE ) == 0 ) {
        		sscanf( tokens[3], "%lf", &config->kp_depth_forward );
			}
			else {
            	sscanf( tokens[2], "%lf", &config->kp_depth );
			}
        }
    }

    else if( strncmp( tokens[0], "ki", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->ki_yaw );
        }
        else if( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->ki_roll );
        }
        else if( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->ki_pitch );
        }
        else if( strncmp( tokens[1], "fx", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->ki_fx );
        }
        else if( strncmp( tokens[1], "fy", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->ki_fy );
        }
        else if( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->ki_depth );
        }
        else if( strncmp( tokens[1], "buoy", STRING_SIZE ) == 0 ) {
        	if( strncmp( tokens[2], "depth", STRING_SIZE ) == 0 ) {
        		sscanf( tokens[3], "%lf", &config->ki_buoy_depth );
			}
			else {
	            sscanf( tokens[2], "%lf", &config->ki_buoy );
			}
        }
    }

    else if( strncmp( tokens[0], "kd", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kd_yaw );
        }
        else if( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kd_roll );
        }
        else if( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kd_pitch );
        }
        else if( strncmp( tokens[1], "fx", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kd_fx );
        }
        else if( strncmp( tokens[1], "fy",STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kd_fy );
        }
        else if( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%lf", &config->kd_depth );
        }
        else if( strncmp( tokens[1], "buoy", STRING_SIZE ) == 0 ) {
        	if( strncmp( tokens[2], "depth", STRING_SIZE ) == 0 ) {
        		sscanf( tokens[3], "%lf", &config->kd_buoy_depth );
			}
			else {
	            sscanf( tokens[2], "%lf", &config->kd_buoy );
			}
        }
    }

    else if( strncmp( tokens[0], "period", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_pitch );
        }
        else if( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_roll );
        }
        else if( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_yaw );
        }
        else if( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_depth );
        }
        else if( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_ax );
        }
        else if( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_ay );
        }
        else if( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_az );
        }
        else if( strncmp( tokens[1], "vision", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_vision );
        }
        else if( strncmp( tokens[1], "planner", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->period_planner );
        }
    }
    /// end PID parameters

    /// target values
    else if( strncmp( tokens[0], "target", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "pitch", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_pitch );
        }
        else if( strncmp( tokens[1], "roll", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_roll );
        }
        else if( strncmp( tokens[1], "yaw", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_yaw );
        }
        else if( strncmp( tokens[1], "depth", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_depth );
        }
        else if( strncmp( tokens[1], "ax", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_ax );
        }
        else if( strncmp( tokens[1], "ay", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_ay );
        }
        else if( strncmp( tokens[1], "az", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_az );
        }
        else if( strncmp( tokens[1], "fx", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_fx );
        }
        else if( strncmp( tokens[1], "fy", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_fy );
        }
        else if( strncmp( tokens[1], "speed", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->target_speed );
        }
    }
    /// end target values

    /// Vision HSV values
    else if( strncmp( tokens[0], "pipe", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "hL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->pipe_hL );
        }
        else if( strncmp( tokens[1], "hH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->pipe_hH );
        }
        else if( strncmp( tokens[1], "sL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->pipe_sL );
        }
        else if( strncmp( tokens[1], "sH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->pipe_sH );
        }
        else if( strncmp( tokens[1], "vL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->pipe_vL );
        }
        else if( strncmp( tokens[1], "vH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->pipe_vH );
        }
    }
    else if( strncmp( tokens[0], "buoy", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "hL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->buoy_hL );
        }
        else if( strncmp( tokens[1], "hH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->buoy_hH );
        }
        else if( strncmp( tokens[1], "sL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->buoy_sL );
        }
        else if( strncmp( tokens[1], "sH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->buoy_sH );
        }
        else if( strncmp( tokens[1], "vL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->buoy_vL );
        }
        else if( strncmp( tokens[1], "vH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->buoy_vH );
        }
		else if( strncmp( tokens[1], "blind", STRING_SIZE ) == 0 ) {
			if( strncmp( tokens[2], "time", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%d", &config->buoy_blind_time );
			}
		}
    }
    else if( strncmp( tokens[0], "fence", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "hL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_hL );
        }
        else if( strncmp( tokens[1], "hH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_hH );
        }
        else if( strncmp( tokens[1], "sL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_sL );
        }
        else if( strncmp( tokens[1], "sH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_sH );
        }
        else if( strncmp( tokens[1], "vL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_vL );
        }
        else if( strncmp( tokens[1], "vH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_vH );
        }
        else if( strncmp( tokens[1], "min", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->fence_min );
        }
    }
    else if( strncmp( tokens[0], "gate", STRING_SIZE ) == 0 ) {
        if( strncmp( tokens[1], "hL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->gate_hL );
        }
        else if( strncmp( tokens[1], "hH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->gate_hH );
        }
        else if( strncmp( tokens[1], "sL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->gate_sL );
        }
        else if( strncmp( tokens[1], "sH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->gate_sH );
        }
        else if( strncmp( tokens[1], "vL", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->gate_vL );
        }
        else if( strncmp( tokens[1], "vH", STRING_SIZE ) == 0 ) {
            sscanf( tokens[2], "%f", &config->gate_vH );
        }
    }
    /// end vision HSV values

	/// depth values
	else if( strncmp( tokens[0], "depth", STRING_SIZE ) == 0 ) {
		if( strncmp( tokens[1], "gate", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_gate );
		}
		else if( strncmp( tokens[1], "pipe", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_pipe );
		}
		else if( strncmp( tokens[1], "buoy", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_buoy );
		}
		else if( strncmp( tokens[1], "fence", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_fence );
		}
		else if( strncmp( tokens[1], "boxes", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_boxes );
		}
		else if( strncmp( tokens[1], "suitcase", STRING_SIZE ) == 0 ) {
			if( strncmp( tokens[2], "search", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%f", &config->depth_suitcase_search );
			}
			else if( strncmp( tokens[2], "pickup", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%f", &config->depth_suitcase_pickup );
			}
		}
		else if( strncmp( tokens[1], "octagon", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_octagon );
		}
		else if( strncmp( tokens[1], "surface", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->depth_surface );
		}
	}
	/// end depth values

	/// heading values
	else if( strncmp( tokens[0], "heading", STRING_SIZE ) == 0 ) {
		if( strncmp( tokens[1], "buoy", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->heading_buoy );
		}
		else if( strncmp( tokens[1], "gate", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%f", &config->heading_gate );
		}
	}
	/// end heading values

	/// task start values
	else if( strncmp( tokens[0], "start", STRING_SIZE ) == 0 ) {
		if( strncmp( tokens[1], "task", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->task_start );
		}
		else if( strncmp( tokens[1], "subtask", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->subtask_start );
		}
		else if( strncmp( tokens[1], "course", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->course_start );
		}
	}
	else if( strncmp( tokens[0], "dock", STRING_SIZE ) == 0 ) {
		if( strncmp( tokens[1], "time", STRING_SIZE ) == 0 ) {
			sscanf( tokens[2], "%d", &config->dock_time );
		}
	}
	else if( strncmp( tokens[0], "task", STRING_SIZE ) == 0 ) {
		if( strncmp( tokens[1], "init", STRING_SIZE ) == 0 ) {
			if( strncmp( tokens[2], "yaw", STRING_SIZE ) == 0 ) {
				sscanf( tokens[3], "%f", &config->task_init_yaw );
			}
		}
	}
	/// end task start values
} /* end parse_line() */


/*------------------------------------------------------------------------------
 * void parse_config()
 * Parses a configuration file for variable data such as IP addresses.
 *----------------------------------------------------------------------------*/

void parse_config(const char *filename, CONF_VARS *config)
{
	/// Declare variables.
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
    if( cfg_fd == NULL ) {
        printf("PARSE_CONFIG: WARNING!!! Error opening config file %s\n", filename);
        return;
    }

    for( ii = 0; ii < MAX_TOKENS; ii++ ) {
        memset( &( tokens[ii] ), 0, sizeof( tokens[ii] ) );
    }
    memset( &( token_buf ), 0, sizeof( token_buf ) );

    if( fgets( token_buf, STRING_SIZE, cfg_fd ) == NULL ) {
        /// Do nothing.
    }

    while( feof( cfg_fd ) == 0 ) {
        line_number++;
        for( ii = 0; ii < sizeof( token_buf ); ii++ ) {
            if( filling_in_token == FALSE ) {
                if( token_buf[ii] == '\n' ) {
                    break;
                }
                else if( token_buf[ii] == '#' ) {
                    comment = TRUE;
                    /// Prevent whitespace betweeen last token and comment from
                    /// increasing token count. */
                    if( last_value_space == TRUE ) {
                        token_number--;
                        last_value_space = FALSE;
                    }
                    parse_line( config );
                    jj = 0;
                    break;
                }
                else if( isspace( token_buf[ii] ) ) {
                    /// Do nothing. Still looking on current line.
                }
                else {
                    filling_in_token = TRUE;
                    tokens[token_number][jj++] = token_buf[ii];
                }
            }
            else {
                if( token_buf[ii] == '\n' ) {
                    parse_line( config );
                    token_number++;
                    jj = 0;
                    filling_in_token = FALSE;
                    break;
                }
                else if( token_buf[ii] == '#' ) {
                    parse_line( config );
                    token_number++;
                    jj = 0;
                    filling_in_token = FALSE;
                    break;
                }
                else if( isspace( token_buf[ii] ) ) {
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

        for( ii = 0; ii < MAX_TOKENS; ii++ ) {
            memset( &( tokens[ii] ), 0, sizeof( tokens[ii] ) );
        }
        memset( &( token_buf ), 0, sizeof( token_buf ) );
        if( fgets( token_buf, STRING_SIZE, cfg_fd ) == NULL ) {
            /// Do nothing.
        }
    }

    fclose( cfg_fd );
} /* end parse_config() */


/*------------------------------------------------------------------------------
 * int parse_cla()
 * Parses command line arguments and sets appropriate variables.
 *----------------------------------------------------------------------------*/

int parse_cla(int argc, char *argv[], CONF_VARS *config, int app, const char *filename)
{
    /// Get command line arguments and parse configuration file.
    char *cvalue = NULL;
    int cmd;

    /// Parse the configuration file passed in on command line.
    opterr = 0;

    while( ( cmd = getopt( argc, argv, "c:h" ) ) != -1 ) {
        switch ( cmd ) {

            case 'c':
                cvalue = optarg;
                break;

            case 'h':
                //print_help( );
                //print_config_help( );
                return -1;

            default:
                //print_help( );
                //print_config_help( );
                return -1;
        }
    }

    if( cvalue != NULL ) {
        parse_config( cvalue, config );
    }
    else {
        parse_config( filename, config );
    }

    return 0;
} /* end parse_cla() */


/*------------------------------------------------------------------------------
 * void parse_default_config()
 * Sets default values for the initialization variables. This function should be
 * followed by a call to parse_cla() in order to fill in the variables with
 * values set in a configuration file.
 *----------------------------------------------------------------------------*/

void parse_default_config( CONF_VARS *config )
{
    /// labjack
    config->enable_labjack = TRUE;

    /// imu
    config->enable_imu = TRUE;
    config->imu_baud = 38400;
    config->imu_stab = TRUE;
    strncpy( config->imu_port, "/dev/ttyUSB5", STRING_SIZE );

    /// net
    config->enable_server = TRUE;
	config->server_port = 0;
    config->net_mode = 1;

	/// nav
	config->enable_nav = FALSE;
    config->nav_port = 2000;
    strncpy( config->nav_IP, "127.0.0.1", STRING_SIZE );
    config->max_api_clients = 5;

	/// planner
	config->enable_planner = FALSE;
	config->planner_port = 2002;
	strncpy( config->planner_IP, "127.0.0.1", STRING_SIZE );

    /// pid
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
    config->kp_fx = 1;
    config->ki_fx = 0;
    config->kd_fx = 0;
    config->kp_fy = 1;
    config->ki_fy = 0;
    config->kd_fy = 0;
    config->kp_buoy = 0;
    config->ki_buoy = 0;
    config->kd_buoy = 0;
    config->kp_buoy_depth = 0;
    config->ki_buoy_depth = 0;
    config->kd_buoy_depth = 0;
    config->kp_roll_lateral = 0;
    config->kp_depth_forward = 0;
    config->kp_place_holder = 0;

    /// target values
    config->target_pitch = 0.0;
    config->target_roll = 0.0;
    config->target_yaw = 0.0;
    config->target_depth = 0.0;
    config->target_ax = 0.0;
    config->target_ay = 0.0;
    config->target_az = 0.0;
    config->target_fx = 0.0;
    config->target_fy = 0.0;
    config->target_speed = 0.0;
    config->task_init_yaw = 0.0;

	/// hsv values
    config->pipe_hL = 0.0;
    config->pipe_hH = 0.0;
    config->pipe_sL = 0.0;
    config->pipe_sH = 0.0;
    config->pipe_vL = 0.0;
    config->pipe_vH = 0.0;
    config->buoy_hL = 0.0;
    config->buoy_hH = 0.0;
    config->buoy_sL = 0.0;
    config->buoy_sH = 0.0;
    config->buoy_vL = 0.0;
    config->buoy_vH = 0.0;
    config->fence_hL = 0.0;
    config->fence_hH = 0.0;
    config->fence_sL = 0.0;
    config->fence_sH = 0.0;
    config->fence_vL = 0.0;
    config->fence_vH = 0.0;
    config->gate_hL = 0.0;
    config->gate_hH = 0.0;
    config->gate_sL = 0.0;
    config->gate_sH = 0.0;
    config->gate_vL = 0.0;
    config->gate_vH = 0.0;

	/// depths
	config->depth_boxes = 0.600;
	config->depth_buoy = 0.600;
	config->depth_fence = 0.600;
	config->depth_gate = 0.600;
	config->depth_octagon = 0.600;
	config->depth_suitcase_pickup = 0.600;
	config->depth_suitcase_search = 0.600;
	config->depth_surface = 0.600;
	config->fence_min = 0.600;

	/// headings
	config->heading_buoy = 0.0;
	config->heading_gate = 0.0;

	/// task starts
	config->task_start = 1;
	config->subtask_start = 2;
	config->course_start = 0;

    /// pololu
    config->enable_pololu = TRUE;
    config->pololu_baud = 9600;
    strncpy( config->pololu_port, "/dev/ttyUSB4", STRING_SIZE );

    /// gui
    config->enable_gui = FALSE;
    config->window_height = 768;
    config->window_width = 1024;

    /// gps
    config->enable_gps = FALSE;
    strncpy( config->gps_port, "/dev/ttyUSB6", STRING_SIZE );
    config->gps_baud = 9600;

    /// relay
    config->enable_relay = FALSE;
    strncpy( config->relay_port, "/dev/ttyUSB7", STRING_SIZE );
    config->relay_baud = 9600;

	/// vision
	config->vision_window = FALSE;
	config->vision_angle = 0;
	config->save_image_front = 0;
	config->save_image_bottom = 0;
	config->save_image_color = 0;
	config->save_image_binary = 0;
	config->save_image_rate = 0;
	strncpy( config->save_image_dir, "", STRING_SIZE );
	config->open_image_rate = 0;
	strncpy( config->open_image_dir, "", STRING_SIZE );

    /// other
    config->debug_level = 5;
    config->op_mode = 1;
    config->enable_log = 0;
	config->dock_time = 0;
	config->buoy_blind_time = 0;
} /* end parse_default_config() */


/*------------------------------------------------------------------------------
 * void parse_print_config()
 * Prints out the values of all the configuration file variables.
 *****************************************************************************/

void parse_print_config(CONF_VARS *config)
{
    printf("PARSE_PRINT_CONFIG: enable_labjack = %d\n", config->enable_labjack);
    printf("PARSE_PRINT_CONFIG: enable_imu = %d\n", config->enable_imu);
    printf("PARSE_PRINT_CONFIG: imu_baud = %d\n", config->imu_baud);
    printf("PARSE_PRINT_CONFIG: imu_stab = %d\n", config->imu_stab);
    printf("PARSE_PRINT_CONFIG: imu_port[STRING_SIZE] = %s\n", config->imu_port);
    printf("PARSE_PRINT_CONFIG: enable_server = %d\n", config->enable_server);
	printf("PARSE_PRINT_CONFIG: enable_nav = %d\n", config->enable_nav);
    printf("PARSE_PRINT_CONFIG: net_mode = %d\n", config->net_mode);
    printf("PARSE_PRINT_CONFIG: nav_port = %hd\n", config->nav_port);
	printf("PARSE_PRINT_CONFIG: server_port = %hd\n", config->server_port);
    printf("PARSE_PRINT_CONFIG: nav_IP[STRING_SIZE] = %s\n", config->nav_IP);
    printf("PARSE_PRINT_CONFIG: vision_IP[STRING_SIZE] = %s\n", config->vision_IP);
    printf("PARSE_PRINT_CONFIG: enable_vision = %d\n", config->enable_vision);
    printf("PARSE_PRINT_CONFIG: vision_port = %hd\n", config->vision_port);
    printf("PARSE_PRINT_CONFIG: vision_window = %d\n", config->vision_window);
    printf("PARSE_PRINT_CONFIG: vision_angle = %d\n", config->vision_angle);
    printf("PARSE_PRINT_CONFIG: vision_task[STRING_SIZE] = %s\n", config->vision_task);
    printf("PARSE_PRINT_CONFIG: planner_IP[STRING_SIZE] = %s\n", config->planner_IP);
    printf("PARSE_PRINT_CONFIG: enable_planner = %d\n", config->enable_planner);
    printf("PARSE_PRINT_CONFIG: planner_port = %hd\n", config->planner_port);
    printf("PARSE_PRINT_CONFIG: labjackd_IP[STRING_SIZE] = %s\n", config->labjackd_IP);
    printf("PARSE_PRINT_CONFIG: labjackd_port = %hd\n", config->labjackd_port);
    printf("PARSE_PRINT_CONFIG: max_api_clients = %d\n", config->max_api_clients);
    printf("PARSE_PRINT_CONFIG: kp_yaw = %lf\n", config->kp_yaw);
    printf("PARSE_PRINT_CONFIG: ki_yaw = %lf\n", config->ki_yaw);
    printf("PARSE_PRINT_CONFIG: kd_yaw = %lf\n", config->kd_yaw);
    printf("PARSE_PRINT_CONFIG: kp_roll = %lf\n", config->kp_roll);
    printf("PARSE_PRINT_CONFIG: ki_roll = %lf\n", config->ki_roll);
    printf("PARSE_PRINT_CONFIG: kd_roll = %lf\n", config->kd_roll);
    printf("PARSE_PRINT_CONFIG: kp_pitch = %lf\n", config->kp_pitch);
    printf("PARSE_PRINT_CONFIG: ki_pitch = %lf\n", config->ki_pitch);
    printf("PARSE_PRINT_CONFIG: kd_pitch = %lf\n", config->kd_pitch);
    printf("PARSE_PRINT_CONFIG: kp_depth = %lf\n", config->kp_depth);
    printf("PARSE_PRINT_CONFIG: ki_depth = %lf\n", config->ki_depth);
    printf("PARSE_PRINT_CONFIG: kd_depth = %lf\n", config->kd_depth);
    printf("PARSE_PRINT_CONFIG: kp_fx = %lf\n", config->kp_fx);
    printf("PARSE_PRINT_CONFIG: ki_fx = %lf\n", config->ki_fx);
    printf("PARSE_PRINT_CONFIG: kd_fx = %lf\n", config->kd_fx);
    printf("PARSE_PRINT_CONFIG: kp_fy = %lf\n", config->kp_fy);
    printf("PARSE_PRINT_CONFIG: ki_fy = %lf\n", config->ki_fy);
    printf("PARSE_PRINT_CONFIG: kd_fy = %lf\n", config->kd_fy);
    printf("PARSE_PRINT_CONFIG: target_pitch = %f\n", config->target_pitch);
    printf("PARSE_PRINT_CONFIG: target_roll = %f\n", config->target_roll);
    printf("PARSE_PRINT_CONFIG: target_yaw = %f\n", config->target_yaw);
    printf("PARSE_PRINT_CONFIG: target_depth = %f\n", config->target_depth);
    printf("PARSE_PRINT_CONFIG: target_ax = %f\n", config->target_ax);
    printf("PARSE_PRINT_CONFIG: target_ay = %f\n", config->target_ay);
    printf("PARSE_PRINT_CONFIG: target_az = %f\n", config->target_az);
    printf("PARSE_PRINT_CONFIG: target_fx = %f\n", config->target_fx);
    printf("PARSE_PRINT_CONFIG: target_fy = %f\n", config->target_fy);
    printf("PARSE_PRINT_CONFIG: target_speed = %f\n", config->target_speed);
    printf("PARSE_PRINT_CONFIG: period_pitch = %f\n", config->period_pitch);
    printf("PARSE_PRINT_CONFIG: period_roll = %f\n", config->period_roll);
    printf("PARSE_PRINT_CONFIG: period_yaw = %f\n", config->period_yaw);
    printf("PARSE_PRINT_CONFIG: period_depth = %f\n", config->period_depth);
    printf("PARSE_PRINT_CONFIG: period_ax = %f\n", config->period_ax);
    printf("PARSE_PRINT_CONFIG: period_ay = %f\n", config->period_ay);
    printf("PARSE_PRINT_CONFIG: period_az = %f\n", config->period_az);
    printf("PARSE_PRINT_CONFIG: period_vision = %f\n", config->period_vision);
    printf("PARSE_PRINT_CONFIG: period_planner = %f\n", config->period_planner);
    printf("PARSE_PRINT_CONFIG: enable_pololu = %d\n", config->enable_pololu);
    printf("PARSE_PRINT_CONFIG: pololu_baud = %d\n", config->pololu_baud);
    printf("PARSE_PRINT_CONFIG: pololu_port[STRING_SIZE] = %s\n", config->pololu_port);
    printf("PARSE_PRINT_CONFIG: pipe_hL = %f\n", config->pipe_hL);
    printf("PARSE_PRINT_CONFIG: pipe_hH = %f\n", config->pipe_hH);
    printf("PARSE_PRINT_CONFIG: pipe_sL = %f\n", config->pipe_sL);
    printf("PARSE_PRINT_CONFIG: pipe_sH = %f\n", config->pipe_sH);
    printf("PARSE_PRINT_CONFIG: pipe_vL = %f\n", config->pipe_vL);
    printf("PARSE_PRINT_CONFIG: pipe_vH = %f\n", config->pipe_vH);
    printf("PARSE_PRINT_CONFIG: buoy_hL = %f\n", config->buoy_hL);
    printf("PARSE_PRINT_CONFIG: buoy_hH = %f\n", config->buoy_hH);
    printf("PARSE_PRINT_CONFIG: buoy_sL = %f\n", config->buoy_sL);
    printf("PARSE_PRINT_CONFIG: buoy_sH = %f\n", config->buoy_sH);
    printf("PARSE_PRINT_CONFIG: buoy_vL = %f\n", config->buoy_vL);
    printf("PARSE_PRINT_CONFIG: buoy_vH = %f\n", config->buoy_vH);
    printf("PARSE_PRINT_CONFIG: fence_hL = %f\n", config->fence_hL);
    printf("PARSE_PRINT_CONFIG: fence_hH = %f\n", config->fence_hH);
    printf("PARSE_PRINT_CONFIG: fence_sL = %f\n", config->fence_sL);
    printf("PARSE_PRINT_CONFIG: fence_sH = %f\n", config->fence_sH);
    printf("PARSE_PRINT_CONFIG: fence_vL = %f\n", config->fence_vL);
    printf("PARSE_PRINT_CONFIG: fence_vH = %f\n", config->fence_vH);
    printf("PARSE_PRINT_CONFIG: buoy_hL = %f\n", config->gate_hL);
    printf("PARSE_PRINT_CONFIG: buoy_hH = %f\n", config->gate_hH);
    printf("PARSE_PRINT_CONFIG: buoy_sL = %f\n", config->gate_sL);
    printf("PARSE_PRINT_CONFIG: buoy_sH = %f\n", config->gate_sH);
    printf("PARSE_PRINT_CONFIG: buoy_vL = %f\n", config->gate_vL);
    printf("PARSE_PRINT_CONFIG: buoy_vH = %f\n", config->gate_vH);
    printf("PARSE_PRINT_CONFIG: enable_gui = %d\n", config->enable_gui);
    printf("PARSE_PRINT_CONFIG: window_height = %d\n", config->window_height);
    printf("PARSE_PRINT_CONFIG: window_width = %d\n", config->window_width);
    printf("PARSE_PRINT_CONFIG: enable_gps = %d\n", config->enable_gps);
    printf("PARSE_PRINT_CONFIG: gps_port[STRING_SIZE] = %s\n", config->gps_port);
    printf("PARSE_PRINT_CONFIG: gps_baud = %d\n", config->gps_baud);
    printf("PARSE_PRINT_CONFIG: enable_relay = %d\n", config->enable_relay);
    printf("PARSE_PRINT_CONFIG: relay_port[STRING_SIZE] = %s\n", config->relay_port);
    printf("PARSE_PRINT_CONFIG: relay_baud = %d\n", config->relay_baud);
    printf("PARSE_PRINT_CONFIG: debug_level = %d\n", config->debug_level);
    printf("PARSE_PRINT_CONFIG: op_mode = %d\n", config->op_mode);
    printf("PARSE_PRINT_CONFIG: enable_log = %f\n", config->enable_log);
	printf("PARSE_PRINT_CONFIG: depth_gate = %f\n", config->depth_gate);
	printf("PARSE_PRINT_CONFIG: depth_pipe = %f\n", config->depth_pipe);
	printf("PARSE_PRINT_CONFIG: depth_buoy = %f\n", config->depth_buoy);
	printf("PARSE_PRINT_CONFIG: depth_fence = %f\n", config->depth_fence);
	printf("PARSE_PRINT_CONFIG: depth_boxes = %f\n", config->depth_boxes);
	printf("PARSE_PRINT_CONFIG: depth_suitcase_search = %f\n", config->depth_suitcase_search);
	printf("PARSE_PRINT_CONFIG: depth_suitcase_pickup = %f\n", config->depth_suitcase_pickup);
	printf("PARSE_PRINT_CONFIG: depth_octagon = %f\n", config->depth_octagon);
	printf("PARSE_PRINT_CONFIG: depth_surface = %f\n", config->depth_surface);
	printf("PARSE_PRINT_CONFIG: fence_min = %f\n", config->fence_min);
	printf("PARSE_PRINT_CONFIG: heading_buoy = %f\n", config->heading_buoy);
	printf("PARSE_PRINT_CONFIG: heading_gate = %f\n", config->heading_gate);
	printf("PARSE_PRINT_CONFIG: task_start = %d\n", config->task_start);
	printf("PARSE_PRINT_CONFIG: subtask_start = %d\n", config->subtask_start);
	printf("PARSE_PRINT_CONFIG: course_start = %d\n", config->course_start);
	printf("PARSE_PRINT_CONFIG: dock_time = %d\n", config->dock_time);
	printf("PARSE_PRINT_CONFIG: buoy_blind_time = %d\n", config->buoy_blind_time);
	printf("PARSE_PRINT_CONFIG: save_image_front = %d\n", config->save_image_front );
	printf("PARSE_PRINT_CONFIG: save_image_bottom = %d\n", config->save_image_bottom );
	printf("PARSE_PRINT_CONFIG: save_image_color = %d\n", config->save_image_color );
	printf("PARSE_PRINT_CONFIG: save_image_binary = %d\n", config->save_image_binary );
	printf("PARSE_PRINT_CONFIG: save_image_rate = %f\n", config->save_image_rate);
	printf("PARSE_PRINT_CONFIG: save_image_dir = %s\n", config->save_image_dir);
	printf("PARSE_PRINT_CONFIG: open_image_rate = %f\n", config->open_image_rate);
	printf("PARSE_PRINT_CONFIG: open_image_dir = %s\n", config->open_image_dir);
	printf("PARSE_PRINT_CONFIG: kp_buoy = %lf\n", config->kp_buoy);
	printf("PARSE_PRINT_CONFIG: ki_buoy = %lf\n", config->ki_buoy);
	printf("PARSE_PRINT_CONFIG: kd_buoy = %lf\n", config->kd_buoy);
	printf("PARSE_PRINT_CONFIG: kp_buoy = %lf\n", config->kp_buoy_depth);
	printf("PARSE_PRINT_CONFIG: ki_buoy = %lf\n", config->ki_buoy_depth);
	printf("PARSE_PRINT_CONFIG: kd_buoy = %lf\n", config->kd_buoy_depth);
	printf("PARSE_PRINT_CONFIG: task_init_yaw = %f\n", config->task_init_yaw);
} /* end parse_default_config() */

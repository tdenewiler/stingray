/**
 *  \file msgtypes.h
 *  \brief Shared head file for types that we want shared.
 */

#ifndef _MSGTYPES_H_
#define _MSGTYPES_H_


/******************************
**
** Data types
**
******************************/

#ifndef _HEADER_
#define _HEADER_
/*! Header for API messages. */
typedef struct _HEADER {
	unsigned char msgstart;	//!< Beginning of message character
    unsigned char msgid;	//!< Message ID for decoding on other end of connection
} HEADER;
#endif /* _HEADER_ */

#ifndef _FOOTER_
#define _FOOTER_
/*! Footer for API messages. */
typedef struct _FOOTER {
	unsigned char msgend; //!< End of message character
} FOOTER;
#endif /* _FOOTER_ */

#ifndef _OPEN_MSG_
#define _OPEN_MSG_
/*! Default message to send to API server. */
typedef struct _OPEN_MSG {
    HEADER hdr; //!< Header struct
	FOOTER ftr; //!< Footer struct
} OPEN_MSG;
#endif /* _OPEN_MSG_ */

#ifndef _MSTRAIN_DATA_
#define _MSTRAIN_DATA_
/*! Struct to store data from the IMU. */
typedef struct _MSTRAIN_DATA {
	int   serial_number;      //!< Serial number
	float temp;               //!< Temperature inside the IMU housing
	float ticks;              //!< Timer tick interval
	float mag[3];             //!< Magnetometer vector
	float accel[3];           //!< Acceleration vector
	float ang_rate[3];        //!< Angular rate vector
	float quat[4];            //!< Quaternion vector
	float transform[3][3];    //!< Transform matrix
	float orient[3][3];       //!< Orientation matrix
	float pitch;              //!< Pitch angle, from Euler angles
	float roll;               //!< Roll angle, from Euler angles
	float yaw;                //!< Yaw angle, from Euler angles
	short eeprom_address;     //!< EEPROM address
	short eeprom_value;       //!< EEPROM value
	short int accel_gain;
	short int mag_gain;
	short int bias_gain;
} MSTRAIN_DATA;
#endif /* _MSTRAIN_DATA_ */

#ifndef _MSTRAIN_MSG_
#define _MSTRAIN_MSG_
/*! API message with IMU data. */
typedef struct _MSTRAIN_MSG {
    HEADER hdr;         //!< Header struct
    MSTRAIN_DATA data;  //!< IMU Data struct
	FOOTER ftr;			//!< Footer struct
} MSTRAIN_MSG;
#endif /* _MSTRAIN_MSG_ */

#ifndef _SERVO_MSG_
#define _SERVO_MSG_
/*! Manual servo and thruster values. */
typedef struct _SERVO {
    unsigned char sync;     //!< Sync byte
    unsigned char servo;    //!< Port that servo is plugged into on SSC
    unsigned char position; //!< Position to move servo to
} SERVO;

/*! API message to send servo and thruster values. */
typedef struct _SERVO_MSG {
    HEADER hdr; //!< Header struct
    SERVO data; //!< Servo struct
	FOOTER ftr;	//!< Footer struct
} SERVO_MSG;
#endif /* _SERVO_MSG_ */

#ifndef _STOP_MSG_
#define _STOP_MSG_
/*! Emergency stop message. */
typedef struct _STOP {
    unsigned char state;    //!< State
} STOP;

/*! API message to send emergency stop command. */
typedef struct _STOP_MSG {
    HEADER hdr; //!< Header struct
    STOP data;  //!< Stop struct
	FOOTER ftr;	//!< Footer struct
} STOP_MSG;
#endif /* _STOP_MSG_ */

#ifndef _CLIENT_MSG_
#define _CLIENT_MSG_
/*! uuv functionality that can be modified remotely. Mostly used for
    debugging */
typedef struct _CLIENT {
    int enable_servos;      //!< Enable use of the servos
    int enable_log;         //!< Enable saving data to a log file
    int enable_imu;         //!< Enable use of the IMU
    int imu_stab;           //!< Ask for gyro-stabilized values from the IMU
    int debug_level;        //!< Changes the vervosity of the UUV debug printing
    int dropper;            //!< Position to set the dropper servo to
} CLIENT;

/*! API message to change specific uuv functionality. */
typedef struct _CLIENT_MSG {
    HEADER hdr;  //!< Header struct
    CLIENT data; //!< Client struct
	FOOTER ftr;	 //!< Footer struct
} CLIENT_MSG;
#endif /* _CLIENT_MSG_ */

#ifndef _TELEOP_MSG_
#define _TELEOP_MSG_
typedef struct _TELEOP {
	float pitch;
	float roll;
	float yaw;
	float depth;
	float fx;
	float fy;
	float speed;
} TELEOP;

typedef struct _TELEOP_MSG {
	HEADER hdr;  //!< Header struct
	TELEOP data; //!< Teleop struct
	FOOTER ftr;	 //!< Footer struct
} TELEOP_MSG;
#endif /* _TELEOP_MSG_ */

#ifndef _TARGET_MSG_
#define _TARGET_MSG_
/*! Operational mode and target pose and motion values. */
typedef struct _TARGET {
    int mode;       	//!< Operational mode
    float pitch;    	//!< Desired pitch angle
    float roll;     	//!< Desired roll angle
    float yaw;      	//!< Desired yaw angle
    float yaw_previous; //!< Desired yaw angle from previous pipe event
    float yaw_detected; //!< Yaw angle from previous event detection
    float depth;    	//!< Desired depth
    float fx;       	//!< Desired force in x direction
    float fy;       	//!< Desired force in y direction
    float speed;    	//!< Desired speed.
    int task;			//!< Current task.
    int vision_status;	//!< Current vision status.
} TARGET;

/*! API message to change the uuv operational mode. */
typedef struct _TARGET_MSG {
    HEADER hdr;     //!< Header struct
    TARGET data;    //!< Target struct
	FOOTER ftr;		//!< Footer struct
} TARGET_MSG;
#endif /* _TARGET_MSG_ */

#ifndef _GAIN_MSG_
#define _GAIN_MSG_
/*! Gains used by the uuv. Typically used with a PID controller */
typedef struct _GAIN {
    char mode;         		 	//!< Gains mode. Either GET_GAINS or SET_GAINS
    double kp_yaw;      		//!< Proportional gain for the yaw angle
    double ki_yaw;      		//!< Integrator gain for the yaw angle
    double kd_yaw;      		//!< Derivative gain for the yaw angle
    double kp_pitch;    		//!< Proportional gain for the pitch angle
    double ki_pitch;    		//!< Integrator gain for the pitch angle
    double kd_pitch;   		 	//!< Derivative gain for the pitch angle
    double kp_roll;     		//!< Proportional gain for the roll angle
    double ki_roll;     		//!< Integrator gain for the roll angle
    double kd_roll;     		//!< Derivative gain for the roll angle
    double kp_depth;    		//!< Proportional gain for depth
    double ki_depth;    		//!< Integrator gain for depth
    double kd_depth;    		//!< Derivative gain for depth
    double kp_fx; 				//!< Proportional gain for fx
    double ki_fx;				//!< Integrator gain for fx
    double kd_fx;				//!< Derivative gain for fx
    double kp_fy;				//!< Proportional gain for fy
    double ki_fy;				//!< Integrator gain for fy
    double kd_fy;				//!< Derivative gain for fy
    double kp_ax;       		//!< Proportional gain for acceleration in X
    double ki_ax;       		//!< Integrator gain for acceleration in X
    double kd_ax;       		//!< Derivative gain for acceleration in X
    double kp_ay;       		//!< Proportional gain for acceleration in Y
    double ki_ay;       		//!< Integrator gain for acceleration in Y
    double kd_ay;       		//!< Derivative gain for acceleration in Y
    double kp_az;       		//!< Proportional gain for acceleration in Z
    double ki_az;       		//!< Integrator gain for acceleration in Z
    double kd_az;       		//!< Derivative gain for acceleration in Z
    double kp_roll_lateral;		//!< Proportional gain for coupling between roll and lateral thrust
    double kp_depth_forward;	//!< Proportional gain for coupling between depth and forward thrust
    double kp_place_holder;		//!< Proportional gain place holder
} GAIN;

/*! API message to change the gains used by the UUV in PID loops. */
typedef struct _GAIN_MSG {
    HEADER hdr;  //!< Header struct
    GAIN data;   //!< Gains struct
	FOOTER ftr;	 //!< Footer struct
} GAIN_MSG;
#endif /* _GAIN_MSG_ */

#ifndef _STATUS_MSG_
#define _STATUS_MSG_
typedef struct _STATUS {
    float mag[3];       //!< Magnetometer vector
    float accel[3];     //!< Acceleration vector
    float ang_rate[3];  //!< Angular rate vector
    float quat[4];      //!< Quaternion vector
    float pitch;        //!< Pitch angle, from Euler angles
    float roll;         //!< Roll angle, from Euler angles
    float yaw;          //!< Yaw angle, from Euler angles
    float depth;        //!< Depth from pressure sensor
    float fx;			//!< fx from pid.c
    float fy;			//!< fx from pid.c
    float pitch_perr;   //!< Pitch P error
    float pitch_ierr;   //!< Pitch I error
    float pitch_derr;   //!< Pitch D error
    int pitch_period;   //!< Pitch dt
    float roll_perr;    //!< Roll P error
    float roll_ierr;    //!< Roll I error
    float roll_derr;    //!< Roll D error
    int roll_period;    //!< Roll dt
    float yaw_perr;     //!< Yaw P error
    float yaw_ierr;     //!< Yaw I error
    float yaw_derr;     //!< Yaw D error
    int yaw_period;     //!< Yaw dt
    float depth_perr;   //!< Depth P error
    float depth_ierr;   //!< Depth I error
    float depth_derr;   //!< Depth D error
    int depth_period;   //!< Depth dt
    float fx_perr;	    //!< fx P error
    float fx_ierr;	    //!< fx I error
    float fx_derr;	    //!< fx D error
    int fx_period;	    //!< fx dt
    float fy_perr;	    //!< fx P error
    float fy_ierr;	    //!< fx I error
    float fy_derr;	    //!< fx D error
    int fy_period;	    //!< fx dt
	double fps;			//!< Frames per second of currently used camera.
} STAT;

typedef struct _STATUS_MSG {
    HEADER  hdr;    //!< Header struct
    STAT    data;   //!< Status struct
	FOOTER  ftr;	//!< Footer struct
} STATUS_MSG;
#endif /* _STATUS_MSG_ */

#ifndef _VISION_MSG_
#define _VISION_MSG_
typedef struct _VISION {
    int front_x;    //!< The x component of the detected object in front camera.
    int front_y;    //!< The y component of the detected object in front camera.
    int bottom_x;   //!< The x component of the detected object in bottom camera.
    int bottom_y;   //!< The y component of the detected object in bottom camera.
    float bearing;  //!< The bearing of the pipe
	int box1_x;		//!< The x component of the first box found.
	int box1_y;		//!< The y component of the first box found.
	int box2_x;		//!< The x component of the second box found.
	int box2_y;		//!< The y component of the second box found.
	int suitcase_x; //!< The x component of the suitcase.
	int suitcase_y; //!< The y component of the suitcase.
	int status;    	//!< The status of the vision detection.
	int confidence;	//!< The status of the vision detection.
	int mode;		//!< Which window to display video in and the video type.
	double fps;		//!< Frames per second of currently used camera.
} VISION;

typedef struct _VISION_MSG {
    HEADER hdr;     //!< Header struct
    VISION data;    //!< Vision struct
	FOOTER ftr;		//!< Footer struct
} VISION_MSG;
#endif /* _VISION_MSG_ */

#ifndef _TASK_MSG_
#define _TASK_MSG_
typedef struct _TASK {
    /*int num;	        //!< The task to attempt.*/
    int task;			//!< The task to attempt.
	int subtask;		//!< The subtask to attempt. Used when the main task is
						//!< to complete the entire course.
    int course;			//!< Flag to indicate that th stingray is in course mode
    float time_forward; //!< The time to go forward in the square task.
    float time_left;    //!< The time to go left in the square task.
    float time_reverse; //!< The time to go reverse in the square task.
    float time_right;   //!< The time to go right in the square task.
} TASK;

typedef struct _TASK_MSG {
    HEADER hdr;     //!< Header struct
    TASK data;      //!< Tasks struct
	FOOTER ftr;		//!< Footer struct
} TASK_MSG;
#endif /* _TASK_MSG_ */

#ifndef _LJ_MSG_
#define _LJ_MSG_
typedef struct _LJ_DATA {
    float battery1;	//!< Motor battery.
    float battery2;	//!< Computer battery.
    float pressure;	//!< Depth sensor.
    float water;	//!< Water leak detection sensor.
} LJ_DATA;

typedef struct _LJ_MSG {
    HEADER hdr;   //!< Header struct
    LJ_DATA data; //!< Labjack struct
	FOOTER ftr;	  //!< Footer struct
} LJ_MSG;
#endif /* _LJ_MSG_ */

#ifndef _VSETTING_MSG_
#define _VSETTING_MSG_
typedef struct _HSV_HL {
    float hL;	//!< Hue low threshold.
    float hH;	//!< Hue high threshold.
    float sL;	//!< Saturation low threshold.
    float sH;	//!< Saturation high threshold.
    float vL;	//!< Value low threshold.
    float vH;	//!< Value high threshold.
} HSV_HL;

typedef struct _VSETTING {
    HSV_HL pipe_hsv;	//!< HSV limits for pipe.
    HSV_HL buoy_hsv;	//!< HSV limits for pipe.
    HSV_HL fence_hsv;	//!< HSV limits for pipe.
    int save_bframe;	//!< Save bottom frame.
    int save_fframe;	//!< Save front frame.
    int save_bvideo;	//!< Save bottom video.
    int save_fvideo;	//!< Save front video.
    int vision_angle;	//!< The vision angle.
} VSETTING;

typedef struct _VSETTING_MSG {
    HEADER hdr;     //!< Header struct
    VSETTING data;  //!< Vision settings struct
	FOOTER ftr;		//!< Footer struct
} VSETTING_MSG;
#endif /* _VSETTING_MSG_ */

#ifndef _MSG_DATA_
#define _MSG_DATA_
typedef struct _MSG_DATA {
    OPEN_MSG open;
    MSTRAIN_MSG mstrain;
    SERVO_MSG servo;
    CLIENT_MSG client;
    TARGET_MSG target;
    GAIN_MSG gain;
    STATUS_MSG status;
    VISION_MSG vision;
    TASK_MSG task;
    STOP_MSG stop;
    LJ_MSG lj;
    VSETTING_MSG vsetting;
    TELEOP_MSG teleop;
} MSG_DATA;
#endif /* _MSG_DATA_ */


#endif /* _MSGTYPES_H_ */

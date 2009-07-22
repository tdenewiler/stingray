/**
 *  \file messages.h
 *  \brief Messages that can be sent to/from the API server and client.
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "microstrain.h"
#include "labjack.h"


/******************************
**
** #defines
**
******************************/

#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef STRING_SIZE
#define STRING_SIZE 64
#endif /* STRING_SIZE */

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 65536
#endif /* MAX_MSG_SIZE */

#ifndef SSC_SYNC
#define SSC_SYNC 255
#endif /* SSC_SYNC */

#ifndef GETSETGAIN
#define GAIN_SET 1
#define GAIN_GET 2
#endif /* GETSETGAIN */

#ifndef API_MSGID
/** @name Message IDs for API commands. */
//@{
#define API_MSGID
#define OPEN_MSGID          1
#define MSTRAIN_MSGID       2
#define SERVO_MSGID         3
#define CLIENT_MSGID        4
#define TARGET_MSGID        5
#define GAIN_MSGID          6
#define STATUS_MSGID        7
#define VISION_MSGID        8
#define STOP_MSGID          9
#define TASK_MSGID          10
#define VSETTING_MSGID      11
#define LJ_MSGID            12
#define TELEOP_MSGID		13
//@}
#endif /* API_MSGID */

/* Operating modes. */
#ifndef OPERATING_MODES
/** @name Valid operational modes for the uuv. */
//@{
#define OPERATING_MODES
#define HOLD_YAW        1
#define HOLD_ROLL       2
#define HOLD_PITCH      3
#define HOLD_ACCEL      4
#define HOLD_ANG_RATE   5
#define MANUAL          6
#define AUTONOMOUS      7
#define ZERO_PID_ERRORS 8
//@}
#endif /* OPERATING_MODES */

/* Header and footer. */
#ifndef MSG_START
#define MSG_START 'A'
#endif /* MSG_START */

#ifndef MSG_END
#define MSG_END 'Z'
#endif /* MSG_END */


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
    char mode;          //!< Gains mode. Either GET_GAINS or SET_GAINS
    double kp_yaw;      //!< Proportional gain for the yaw angle
    double ki_yaw;      //!< Integrator gain for the yaw angle
    double kd_yaw;      //!< Derivative gain for the yaw angle
    double kp_pitch;    //!< Proportional gain for the pitch angle
    double ki_pitch;    //!< Integrator gain for the pitch angle
    double kd_pitch;    //!< Derivative gain for the pitch angle
    double kp_roll;     //!< Proportional gain for the roll angle
    double ki_roll;     //!< Integrator gain for the roll angle
    double kd_roll;     //!< Derivative gain for the roll angle
    double kp_depth;    //!< Proportional gain for depth
    double ki_depth;    //!< Integrator gain for depth
    double kd_depth;    //!< Derivative gain for depth
    double kp_fx; 		//!< Proportional gain for fx
    double ki_fx;		//!< Integrator gain for fx
    double kd_fx;		//!< Derivative gain for fx
    double kp_fy;		//!< Proportional gain for fy
    double ki_fy;		//!< Integrator gain for fy
    double kd_fy;		//!< Derivative gain for fy
    double kp_ax;       //!< Proportional gain for acceleration in X
    double ki_ax;       //!< Integrator gain for acceleration in X
    double kd_ax;       //!< Derivative gain for acceleration in X
    double kp_ay;       //!< Proportional gain for acceleration in Y
    double ki_ay;       //!< Integrator gain for acceleration in Y
    double kd_ay;       //!< Derivative gain for acceleration in Y
    double kp_az;       //!< Proportional gain for acceleration in Z
    double ki_az;       //!< Integrator gain for acceleration in Z
    double kd_az;       //!< Derivative gain for acceleration in Z
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


/******************************
**
** Function prototypes
**
******************************/

//! Send the API message to the server.
//! \param fd Socket to send message to.
//! \param msg_id Message ID.
//! \param msg Pointer to message data.
void messages_send( int fd, int msg_id, MSG_DATA *msg );

//! Decode received API messages.
//! \param fd Network file descriptor.
//! \param buf A buffer to store network data.
//! \param msg Pointer to message data.
//! \param bytes Number of bytes in buffer.
//! \return Number of bytes remaining in buffer.
int messages_decode( int fd, char *buf, MSG_DATA *msg, int bytes );

//! Updates status data with current data.
//! \param msg Pointer to message data.
void messages_update( MSG_DATA *msg );

//! Initialize the header and footer variables.
//! \param msg Pointer to message data.
void messages_init( MSG_DATA *msg );


#endif /* _MESSAGES_H_ */

/**
 *  \file microstrain.h
 *  \brief Sending and receiving data with the MicroStrain IMU. We are using
 *         model 3DM-GX1. The API is described in the document
 *         3DM_GX1_Data_Communication_Protocol_203101.pdf. See
 *         <a href="http://www.microstrain.com/3dm-gx1.aspx">Microstrain</a>
 *         for more details.
 */

#ifndef _MICROSTRAIN_H_
#define _MICROSTRAIN_H_

/******************************
 *
 * #defines
 *
 *****************************/

/* Command Set Summary for the MicroStrain 3DM-GX1 IMU. */
/** @name Command values for the IMU. */
//@{
#define IMU_RAW_SENSOR                  0x01
#define IMU_GYRO_STAB_VECTORS           0x02
#define IMU_INST_VECTORS                0x03
#define IMU_INST_QUAT                   0x04
#define IMU_GYRO_STAB_QUAT              0x05
#define IMU_GYRO_BIAS                   0x06
#define IMU_TEMPERATURE                 0x07
#define IMU_READ_EEPROM                 0x08
#define IMU_WRITE_EEPROM                0x09
#define IMU_INST_ORIENT_MATRIX          0x0A
#define IMU_GYRO_STAB_ORIENT_MATRIX     0x0B
#define IMU_GYRO_STAB_QUAT_VECTORS      0x0C
#define IMU_INST_EULER_ANGLES           0x0D
#define IMU_GYRO_STAB_EULER_ANGLES      0x0E
#define IMU_TARE_COORDINATE_SYSTEM      0x0F
#define IMU_CONTINUOUS_MODE             0x10
#define IMU_REMOVE_TARE                 0x11
#define IMU_GYRO_STAB_QUAT_INST_VECTORS 0x12
#define IMU_WRITE_SYSTEM_GAINS          0x24
#define IMU_READ_SYSTEM_GAINS           0x25
#define IMU_SELF_TEST                   0x27
#define IMU_READ_EEPROM_CHECKSUM        0x28
#define IMU_WRITE_EEPROM_CHECKSUM       0x29
#define IMU_GYRO_STAB_EULER_VECTORS     0x31
#define IMU_INIT_HARD_IRON_CALIB        0x40
#define IMU_HARD_IRON_CALIB_DATA        0x41
#define IMU_HARD_IRON_CALIB             0x42
#define IMU_FIRMWARE_VERSION            0xF0
#define IMU_SERIAL_NUMBER               0xF1
//@}

/** @name Response lengths for the IMU messages. */
//@{
#define IMU_LENGTH_01 23
#define IMU_LENGTH_02 23
#define IMU_LENGTH_03 23
#define IMU_LENGTH_04 13
#define IMU_LENGTH_05 13
#define IMU_LENGTH_06  5
#define IMU_LENGTH_07  7
#define IMU_LENGTH_08  0
#define IMU_LENGTH_09  2
#define IMU_LENGTH_0A 23
#define IMU_LENGTH_0B 23
#define IMU_LENGTH_0C 31
#define IMU_LENGTH_0D 11
#define IMU_LENGTH_0E 11
#define IMU_LENGTH_0F  5
#define IMU_LENGTH_10  7
#define IMU_LENGTH_11  5
#define IMU_LENGTH_12 31
#define IMU_LENGTH_24  5
#define IMU_LENGTH_25 11
#define IMU_LENGTH_27  5
#define IMU_LENGTH_28  7
#define IMU_LENGTH_29  7
#define IMU_LENGTH_31 23
#define IMU_LENGTH_40  5
#define IMU_LENGTH_41 23
#define IMU_LENGTH_42 11
#define IMU_LENGTH_F0  5
#define IMU_LENGTH_F1  5
//@}

/** @name Masks for moving data around. */
//@{
#ifndef LSB_MASK
#define LSB_MASK 0xFF
#endif /* LSB_MASK */

#ifndef MSB_MASK
#define MSB_MASK 0xFF00
#endif /* MSB_MASK */

#ifndef CHECKSUM_MASK
#define CHECKSUM_MASK 0xFFFF
#endif /* CHECKSUM_MASK */
//@}

/** @name Bytes for setting and removing tare for coordinate system. */
//@{
#define IMU_TARE_BYTE1	0xC1
#define IMU_TARE_BYTE2	0xC3
#define IMU_TARE_BYTE3	0xC5
//@}

#ifndef MSTRAIN_SERIAL_DELAY
#define MSTRAIN_SERIAL_DELAY 40000
#endif /* MSTRAIN_SERIAL_DELAY */

#ifndef IMU_ERROR_HEADER
#define IMU_SUCCESS			1
#define IMU_ERROR_HEADER	-1
#define IMU_ERROR_CHECKSUM	-2
#define IMU_ERROR_LENGTH	-3
#endif /* IMU_ERROR_HEADER */


/******************************
 *
 * Data types
 *
 *****************************/

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
} MSTRAIN_DATA;

#endif /* _MSTRAIN_DATA_ */


/******************************
 *
 * Function prototypes
 *
 *****************************/

//! Get the Euler angles from the IMU.
//! \param fd A file descriptor for the IMU port.
//! \param roll A pointer to store the roll value.
//! \param pitch A pointer to store the pitch value.
//! \param yaw A pointer to store the yaw value.
//! \return 1 on success, 0 on failure.
int mstrain_euler_angles( int fd,
                          float *roll,
                          float *pitch,
                          float *yaw
                        );

//! Get the serial number from the IMU.
//! \param fd A file descriptor for the IMU port.
//! \param serial_number A pointer to store the serial number value.
//! \return 1 on success, 0 on failure.
int mstrain_serial_number( int fd,
                           int *serial_number
                         );

//! Establish communications with the IMU.
//! \param portname The name of the port that the IMU is plugged into.
//! \param baud The baud rate to use for the IMU serial port.
//! \return A file descriptor for the Microstrain.
int mstrain_setup( char *portname,
                   int baud
                 );

//! Get quaternions from the IMU.
//! \param fd A file descriptor for the IMU port.
//! \param gyro_stab Whether to use gyro-stabilized values or not.
//! \param quat A pointer to store the quaternion values.
//! \return 1 on success, 0 on failure.
int mstrain_quaternions( int fd,
                         int gyro_stab,
                         float *quat[4]
                       );

//! Get the temperature inside the IMU housing.
//! \param fd A file descriptor for the IMU port.
//! \param temp A pointer to store the temperature value.
//! \return 1 on success, 0 on failure.
int mstrain_temperature( int fd,
                         float *temp
                       );

//! Get quaternions and accelerations from the IMU.
//! \param fd A file descriptor for the IMU port.
//! \param quat A pointer to store the quaternion values.
//! \param mag A pointer to store the magnetic values.
//! \param accel A pointer to store the acceleration values.
//! \param ang_rate A pointer to store the angular rate values.
//! \return 1 on success, 0 on failure.
int mstrain_quaternions_vectors( int fd,
                                 float *quat[4],
                                 float *mag[3],
                                 float *accel[3],
                                 float *ang_rate[3]
                               );

//! Get Euler angles and accelerations from the IMU.
//! \param fd A file descriptor for the IMU port.
//! \param roll A pointer to store the roll value.
//! \param pitch A pointer to store the pitch value.
//! \param yaw A pointer to store the yaw value.
//! \param accel A pointer to store the acceleration values.
//! \param ang_rate A pointer to store the angular rate values.
//! \return 1 on success, 0 on failure.
int mstrain_euler_vectors( int fd,
                           float *roll,
                           float *pitch,
                           float *yaw,
                           float *accel,
                           float *ang_rate
                         );

//! Get gyro-stabilized orientation matrix from the IMU.
//! \param fd A file descriptor for the IMU port.
//! \param gyro_stab Whether to use gyro-stabilized values or not.
//! \param orient A pointer to store the orientation values.
//! \return 1 on success, 0 on failure.
int mstrain_orientation( int fd,
                         int gyro_stab,
                         float *orient[3][3]
                       );

//! Sets tare for the Microstrain.
int mstrain_set_tare( int fd );

//! Removes tare for the Microstrain.
int mstrain_remove_tare( int fd );

//! Gets vectors from IMU.
//! \param fd A file descriptor for the IMU port.
//! \param gyro_stab Whether to use gyro-stabilized values or not.
//! \param mag A pointer to store the magnetic values.
//! \param accel A pointer to store the acceleration values.
//! \param ang_rate A pointer to store the angular rate values.
//! \return 1 on success, 0 on failure.
int mstrain_vectors( int fd,
                     int gyro_stab,
                     float *mag,
                     float *accel,
                     float *ang_rate
                   );

//! Calculate the checksum for a message.
//! \param buffer The response buffer with the IMU message data.
//! \param length The size of the message.
//! \return 1 on success, error code on failure.
int mstrain_calc_checksum( char *buffer, int length );

//! Convert two adjacent bytes to an integer value.
//! Returns an integer value.
//! \param buffer Pointer to first byte.
//! \return Resulting integer.
int convert2int( char *buffer );

//! Convert two adjacent bytes to a short value.
//! Returns a short integer value.
//! \param buffer Pointer to first byte.
//! \return Resulting short integer.
short convert2short( char *buffer );


#endif /* _MICROSTRAIN_H_ */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "serial.h"
#include "microstrain.h"


int main()
{
	int imu_fd = -1;
	int status = -1;
	int enable_log = 1;
	char port_name[15];
	char *imu_port;

	//strncpy(port_name, "/dev/ttyS0", 15);
	strncpy(port_name, "/dev/ttyUSB0", 15);
	imu_port = port_name;
	
	/// Declare timestamp variables.
	struct timeval ctime;
    struct tm ct;
	char write_time[80] = {0};
	FILE *f_log;
	FILE *f_errors;

    /// Open data log file if flag set.
	if (enable_log) {
		f_log = fopen("imu_data.csv", "w");
	}
	
	/// Open error log file if flag is set.
	if (enable_log) {
		f_errors = fopen("imu_errors.txt", "w");
	}

	/// Declare variables.
	float pitch = 0;
	float roll = 0;
	float yaw = 0;
	float accel[3] = {0};
	float ang_rate[3] = {0};
	float mag[3] = {0};
	float temp = 0;
	int serial_num = 0;
	int gyro_stab = 0;
	
	/// Initialize variables.
	memset(&accel, '\0', sizeof(accel));
	memset(&ang_rate, '\0', sizeof(ang_rate));
	memset(&mag, '\0', sizeof(mag));

	/// Baud rate must be 38400 unless IMU is reconfigured. To reconfigure, use the code that comes with the Microstrain software package.
	printf("MAIN: Trying IMU at %s with 38400.\n", imu_port);
	imu_fd = mstrain_setup(imu_port, 38400);
	printf("imu_fd = %d\n", imu_fd);

	if (imu_fd < 0) {
		printf("MAIN: No IMU, quitting...\n");
		return 0;
	}

	status = mstrain_temperature(imu_fd, &temp);
	status = mstrain_serial_number(imu_fd, &serial_num);
	printf("MAIN: Temp: %f\t\tSN: %d\n", temp, serial_num);

	while (1) {
		/// Get vectors.
		printf("MAIN: Trying mstrain_vectors()...\n");
		status = mstrain_vectors(imu_fd, gyro_stab, mag, accel, ang_rate);
		printf("Mag Vector: %f %f %f\n", mag[0], mag[1], mag[2]);
		printf("Accel Vector: %f %f %f\n", accel[0], accel[1], accel[2]);
		printf("Ang Rate Vector: %f %f %f\n\n", ang_rate[0], ang_rate[1], ang_rate[2]);

		/// Get Euler angles.
		printf("MAIN: Trying mstrain_euler_angles()...\n");
		status = mstrain_euler_angles(imu_fd, &roll, &pitch, &yaw);
		printf("Roll: %f\nPitch: %f\nYaw: %f\n\n", roll, pitch, yaw);
		
		/// Get Euler angles and vectors.
		printf("MAIN: Trying mstrain_euler_vectors()...\n");
		status = mstrain_euler_vectors(imu_fd, &roll, &pitch, &yaw, accel, ang_rate);
		if (status < 0) {
			close(imu_fd);
			usleep(MSTRAIN_SERIAL_DELAY * 10);
			imu_fd = mstrain_setup(imu_port, 38400);
			if (enable_log) {
				/// Get a timestamp and use for log.
				gettimeofday(&ctime, NULL);
				ct = *(localtime ((const time_t*) &ctime.tv_sec));
				strftime(write_time, sizeof(write_time), "20%y%m%d_%H%M%S", &ct);
				snprintf(write_time + strlen(write_time),
						strlen(write_time), ".%03ld", ctime.tv_usec);

				/// Log errors.
				fprintf(f_errors, "%s %d\n", write_time, status);
			}
		}
		printf("Pitch: %f\nRoll: %f\nYaw: %f\n\n", pitch, roll, yaw);

		/// Log data.
		if (enable_log) {
			/// Get a timestamp and use for log.
			gettimeofday(&ctime, NULL);
			ct = *(localtime ((const time_t*) &ctime.tv_sec));
			strftime(write_time, sizeof(write_time), "20%y%m%d_%H%M%S", &ct);
            snprintf(write_time + strlen(write_time),
            		strlen(write_time), ".%03ld", ctime.tv_usec);

			/// Log data.
			fprintf(f_log, "%s, %.04f, %.04f, %.04f, %.04f, %.04f, %.04f "
				"%.04f, %.04f, %.04f, %.04f, %.04f, %.04f\n",
				write_time, pitch, roll, yaw, mag[0], mag[1], mag[2],
				accel[0], accel[1], accel[2],
				ang_rate[0], ang_rate[1], ang_rate[2]);
		}
	}

	/* Close the serial port. */
	close(imu_fd);

	return 0;
} /* end main() */

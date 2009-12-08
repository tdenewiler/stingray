/**
 *  \file vision.h
 *  \brief General vision processing functions.
 */

#ifndef VISION_H
#define VISION_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <cvcompat.h>

#include "msgtypes.h"

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

#ifndef VISION_BORDER
#define VISION_BORDER 0.95
#endif /* VISION_BORDER */

#ifndef VISION_RECT_THRESH
#define VISION_RECT_THRESH 5
#endif /* VISION_RECT_THRESH */

#ifndef VISION_MIN_ANGLE
#define VISION_MIN_ANGLE 0.16
#endif /* VISION_MIN_ANGLE */

#ifndef VISION_THRESH
#define VISION_BINARY   1
#define VISION_ADAPTIVE 2
#endif /* VISION_THRESH */

#ifndef VISION_ASPECT_RATIOS
#define VISION_ASPECT_RATIOS
#define VISION_AR_PIPE		8
#define VISION_AR_BOX		2
#define VISION_AR_SUITCASE	1
#define VISION_AR_THRESH	0.4
#endif /* VISION_ASPECT_RATIOS */

#ifndef VISION_TASK
#define VISION_TASK
#define VISION_PIPE		1
#define VISION_BUOY		2
#define VISION_FENCE	3
#define VISION_BOX		4
#define VISION_SUITCASE	5
#endif /* VISION_TASK */

#ifndef VISION_PIPE_TYPE
#define VISION_PIPE_TYPE
#define VISION_PIPE_HSV		1
#define VISION_PIPE_BOX		2
#endif /* VISION_PIPE_TYPE */

#ifndef VISION_CHANNELS
#define VISION_CHANNELS
#define VISION_CHANNEL1	1
#define VISION_CHANNEL2	2
#define VISION_CHANNEL3	4
#endif /* VISION_CHANNELS */


/******************************
**
** Function prototypes
**
******************************/

//! Finds the centroid of the pixels in an image.
//! \param binImage The binary image to find the centroid of.
//! \param thresh The amount of pixels for centroid to be valid.
//! \return The x and y coordinates of the centroid.
CvPoint vision_find_centroid( IplImage *binImage, int thresh );

//! Finds a circular object from a camera.
//! \param dotx Pointer to variable for x position of dot.
//! \param doty Pointer to variable for y position of dot.
//! \param angle The angle to rotate the image by.
//! \param srcImg The unprocessed image.
int vision_find_dot( int *dotx,
                     int *doty,
                     int angle,
                     IplImage *srcImg,
                     IplImage *binImg,
                     HSV_HL *hsv
                   );

//! Creates a binary image based on boost buoy predicter.
//! \param srcImg The image to convert to a binary image.
//! \param binImg The image to hold the binary result image.
int vision_boost_buoy( IplImage *srcImg, IplImage *binImg );

//! Finds a pipe object from a camera.
//! \param pipex Pointer to variable for x position of pipe.
//! \param pipey Pointer to variable for y position of pipe.
//! \param bearing Pointer to variable for bearing of pipe.
//! \param srcImg The unprocessed image.
int vision_find_pipe( int *pipex,
					  int *pipey,
                      double *bearing,
                      IplImage *srcImg,
                      IplImage *binImg,
                      HSV_HL *hsv
                    );

//! Finds the centroid of the pixels in an image.
//! \param img The binary image to find the bearing of.
//! \return The angle of the pipe in degrees.
double vision_get_bearing( IplImage *img );

int vision_get_fence_bottom( IplImage *inputBinImg);

//! Finds a fence object from a camera.
//! \param pipex Pointer to variable for x position of pipe.
//! \param bearing Pointer to variable for bearing of pipe.
//! \param srcImg The unprocessed image.
int vision_find_fence( int *pipex,
                      int *y_max,
                      IplImage *srcImg,
                      IplImage *binImg,
					  HSV_HL *hsv
                    );
					
//! Finds the centroid of the gate using the smae logic of find dot
//! \param dotx Pointer to variable for x position of gate centroid.
//! \param doty Pointer to variable for y position of gate centroid.
//! \param angle The angle to rotate the image by.
//! \param srcImg The unprocessed image.

int vision_find_gate( int *dotx,
                      int *doty,
                      int angle,
                      IplImage *srcImg,
                      IplImage *binImg,
					  HSV_HL *hsv
                   );

//! Finds the centroid of the suitcase structure in an image.
//! \param srcImg The binary image to find the suitcase in.
//! \param result A sequence containing the centroids of the rectangles found.
//! \param squares A sequence containing the vertices of the rectangles found.
//! \return 1 if suitcase found, 0 otherwise.
int vision_suitcase( IplImage *srcImg, CvSeq *result, CvSeq *squares );

//! Captures an image, calls findSquares.
int vision_find_boxes( IplImage *srcImg, CvSeq *result,
					   CvSeq *squares, int task, float *angle );

//! Helper function for finding boxes (angle calculations)
double vision_angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0, IplImage *img,
					 int task, float *angle );

//! Finds rectangle centers from a camera
int vision_find_squares( IplImage *img, CvMemStorage *storage, CvSeq *box_centers,
						 CvSeq *squares, int task, float *angle );

//! Finds a circle in an image.
//! \param cap The camera to capture images from.
//! \param srcImg The binary image to find the suitcase in.
//! \param circles A sequence to store detected circle data in.
//! \return 1 if circle found, 0 otherwise.
int vision_find_circle( IplImage *srcImg, CvSeq *circles );

//! Finds the centroid in a binary image using the sum of a window.
//! \return 1 if circle found, 0 otherwise.
int vision_window_filter( IplImage *img,
						IplImage *bin_img,
					    CvPoint *center,
						int sizex,
						int sizey );

//! Performs Gaussian smoothing on an image.
//! \param img The image to smooth.
//! \param mode Sort of a mask to figure out which channels to operate on. Set
//! channel 1 = 1, channel 2 = 2, channel 3 = 4. Then add the channels that are
//! to be operated on.
void vision_smooth( IplImage *img, int mode );

//! Performs histogram equalization on an image.
//! \param img The image to equalize histogram of.
//! \param mode Sort of a mask to figure out which channels to operate on. Set
//! channel 1 = 1, channel 2 = 2, channel 3 = 4. Then add the channels that are
//! to be operated on.
void vision_hist_eq( IplImage *img, int mode );

//! Balances the colors in an image.
//! \param img The image to modify.
void vision_white_balance( IplImage *img );

//! Saturates the colors of each channel in an image.
//! \param img The image to saturate.
void vision_saturate( IplImage *img );

//! Thresholds an image.
void vision_threshold( IplImage *img,
						IplImage *bin_img,
						int type,
						int size,
						double thresh);
						
//! Thresholds an image with rgb ratios.
void vision_rgb_ratio_filter( IplImage *img , double * rgb_thresh );

//! Thresholds an image with rgb sums.
void vision_rgb_sum_filter( IplImage *img , short * rgb_sum );

//! Concatenates two image images
//! \param img1 The first image to concatenate
//! \param img2 The second image to concatenate
void vision_concat_images(IplImage *img1, IplImage *img2, IplImage *new_img);

//! Saves an image to disk.
//! \param img The image to save to disk.
//! \param dir The directory in which to save the image.
//! \param name The name of the file to sive (optional).
void vision_save_frame( IplImage *img, char *dir, char *name = NULL );


#endif /* VISION_H */

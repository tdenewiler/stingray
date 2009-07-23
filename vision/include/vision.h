/**
 *  \file vision.h
 *  \brief General vision processing functions.
 */

#ifndef _VISION_H_
#define _VISION_H_

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

#ifndef VISION_ASPECT_RATIOS
#define VISION_ASPECT_RATIOS
#define VISION_AR_PIPE		8
#define VISION_AR_BOX		2
#define VISION_AR_SUITCASE	1
#define VISION_AR_THRESH	0.5
#endif /* VISION_ASPECT_RATIOS */

#ifndef VISION_TASK
#define VISION_TASK
#define VISION_PIPE		1
#define VISION_BUOY		2
#define VISION_FENCE	3
#define VISION_BOX		4
#define VISION_SUITCASE	5
#endif /* VISION_TASK */


/******************************
**
** Data types
**
******************************/



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
//! \param cam A pointer to an open camera.
//! \param srcImg The unprocessed image.
int vision_find_dot( int *dotx,
                     int *doty,
                     int angle,
                     CvCapture *cam,
                     IplImage *srcImg,
                     IplImage *binImg,
                     float hL,
                     float hH,
                     float sL,
                     float sH,
                     float vL,
                     float vH
                   );

//! Finds a pipe object from a camera.
//! \param pipex Pointer to variable for x position of pipe.
//! \param pipey Pointer to variable for y position of pipe.
//! \param bearing Pointer to variable for bearing of pipe.
//! \param cam A pointer to an open camera.
//! \param srcImg The unprocessed image.
int vision_find_pipe( int *pipex,
					  int *pipey,
                      double *bearing,
                      CvCapture *cam,
                      IplImage *srcImg,
                      IplImage *binImg,
                      float hL,
                      float hH,
                      float sL,
                      float sH,
                      float vL,
                      float vH
                    );

//! Finds the centroid of the pixels in an image.
//! \param img The binary image to find the bearing of.
//! \return The angle of the pipe in degrees.
double vision_get_bearing( IplImage *img );

int vision_get_fence_bottom( IplImage *inputBinImg, int *center );

//! Finds a fence object from a camera.
//! \param pipex Pointer to variable for x position of pipe.
//! \param bearing Pointer to variable for bearing of pipe.
//! \param cam A pointer to an open camera.
//! \param srcImg The unprocessed image.
int vision_find_fence( int *pipex,
                      int *y_max,
                      CvCapture *cam,
                      IplImage *srcImg,
                      IplImage *binImg,
                      float hL,
                      float hH,
                      float sL,
                      float sH,
                      float vL,
                      float vH
                    );

//! Finds the centroid of the suitcase structure in an image.
//! \param cap The camera to capture images from.
//! \param srcImg The binary image to find the suitcase in.
//! \param result A sequence containing the centroids of the rectangles found.
//! \param squares A sequence containing the vertices of the rectangles found.
//! \return 1 if suitcase found, 0 otherwise.
int vision_suitcase( CvCapture *cap, IplImage *srcImg, CvSeq *result, CvSeq *squares );

//! Captures an image, calls findSquares.
int vision_find_boxes( CvCapture *cap, IplImage *srcImg, CvSeq *result,
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
int vision_find_circle( CvCapture *cap, IplImage *srcImg, CvSeq *circles );

//! Finds the centroid in a binary image using the sum of a window.
//! \return 1 if circle found, 0 otherwise.
int vision_window_filter( IplImage *img,
					    CvPoint *center,
						int sizex,
						int sizey );

//! Performs Gaussian smoothing on an image.
//! \param img The image to smooth.
void vision_smooth( IplImage *img );

//! Performs histogram equalization on an image.
//! \param img The image to equalize histogram of.
void vision_hist_eq( IplImage *img );


#endif /* _VISION_H_ */

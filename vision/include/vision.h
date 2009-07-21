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

//! Captures an image,calls findSquares4
int vision_find_boxes( CvCapture *cap, IplImage *srcImg, CvSeq *result, CvSeq *squares );
						 
//! Helper function for finding boxes (angle calculations)
double vision_angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 );

//! Finds rectangle centers from a camera
int vision_find_squares4( IplImage *img, CvMemStorage *storage, CvSeq *box_centers, CvSeq *squares );

//! Finds a circle in an image.
//! \param cap The camera to capture images from.
//! \param srcImg The binary image to find the suitcase in.
//! \param circles A sequence to store detected circle data in.
//! \return 1 if circle found, 0 otherwise.
int vision_find_circle( CvCapture *cap, IplImage *srcImg, CvSeq *circles );


#endif /* _VISION_H_ */

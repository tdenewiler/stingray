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
                     int *width,
                     int *height,
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
//! \param bearing Pointer to variable for bearing of pipe.
//! \param cam A pointer to an open camera.
//! \param srcImg The unprocessed image.
int vision_find_pipe( int *pipex,
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

#endif /* _VISION_H_ */

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
//! \param amt The amount of erosion/dilation to perform.
//! \param cam A pointer to an open camera.
//! \param srcImg The unprocessed image.
//! \param outImg The image to be processed.
int vision_find_dot( int *dotx,
                     int *doty,
                     int *width,
                     int *height,
                     int amt,
                     CvCapture *cam,
                     IplImage *srcImg,
                     IplImage *outImg,
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
//! \param outImg The image to be processed.
int vision_find_pipe( int *pipex,
                      double *bearing,
                      CvCapture *cam,
                      IplImage *srcImg,
                      IplImage *outImg,
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

//! Returns a black and white thresholded image from a source image and the
//! hue, saturation, and value thresholds.
IplImage *vision_segment_image( float hL,
                                float hH,
                                float sL,
                                float sH,
                                float vL,
                                float vH,
                                int closingAmount,
                                IplImage* img,
                                int boxWidth,
                                int boxHeight
                              );

//! Returns a black and white thresholded image from a source image and the
//! hue, saturation, and value thresholds.
IplImage* segmentImage(float hL, float hH, float sL, float sH, float vL, float vH, int closingAmount, IplImage* img, int boxWidth, int boxHeight);

#endif /* _VISION_H_ */

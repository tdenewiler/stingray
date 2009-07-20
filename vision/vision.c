/******************************************************************************
 *
 *  Title:        vision.c
 *
 *  Description:  General vision processing functions.
 *
 *****************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <cvcompat.h>

#include "vision.h"


/******************************************************************************
 *
 * Title:       int vision_find_dot(    int *dotx,
 *                                      int *doty,
 *                                      int amt,
 *                                      CvCapture *cap,
 *                                      IplImage *srcImg,
 *                                      IplImage *outImg
 *                                  )
 *
 * Description: Finds a circular object from a camera.
 *
 * Input:       dotx: Pointer to variable for x position of dot.
 *              doty: Pointer to variable for y position of dot.
 *              amt: The amount of erosion/dilation to perform.
 *              cap: A pointer to an open camera.
 *
 * Output:      status: 1 on success, 0 on failure.
 *
 *****************************************************************************/

int vision_find_dot( int *dotx,
                     int *doty,
                     int *width,
                     int *height,
                     int angle,
                     CvCapture *cap,
                     IplImage *srcImg,
                     IplImage *binImg,
                     float hL,
                     float hH,
                     float sL,
                     float sH,
                     float vL,
                     float vH
                   )
{
    CvPoint center;
    IplImage *hsvImg = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *w = cvCreateStructuringElementEx( 2, 2,
            (int)floor( ( 3.0 ) / 2 ), (int)floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
    CvSize sz = cvSize( srcImg->width & -2, srcImg->height & -2 );
    IplImage *hsv_clone = NULL;
    IplImage *tgrayH = NULL;
    IplImage *tgrayS = NULL;
    IplImage *tgrayV = NULL;

    /* Initialize to impossible values. */
    center.x = -1;
    center.y = -1;

	/* Capture a new source image. */
    srcImg = cvQueryFrame( cap );
    if( !srcImg ) {
        return 0;
    }

	/* Create images to work on. */
    hsvImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 1 );

    /* Flip the source image. */
    cvFlip( srcImg, srcImg );

    /* Segment the flipped image into a binary image. */
    cvCvtColor( srcImg, hsvImg, CV_RGB2HSV );
	hsv_clone = cvCloneImage( hsvImg );

	/* Create three separate grayscale images, one for each channel. */
    tgrayH = cvCreateImage( sz, 8, 1 );
	tgrayS = cvCreateImage( sz, 8, 1 );
	tgrayV = cvCreateImage( sz, 8, 1 );

    /* Find squares in every color plane of the image. Filter each plane with a
	 * Gaussian and then merge back to HSV image.  */
    cvSetImageCOI( hsv_clone, 1 );
    cvCopy( hsv_clone, tgrayH, 0 );
	cvSmooth( tgrayH, tgrayH, CV_GAUSSIAN, 5, 5 );

	cvSetImageCOI( hsv_clone, 2 );
    cvCopy( hsv_clone, tgrayS, 0 );
	cvSmooth( tgrayS, tgrayS, CV_GAUSSIAN, 5, 5 );

	cvSetImageCOI( hsv_clone, 3 );
    cvCopy( hsv_clone, tgrayV, 0 );
	cvSmooth( tgrayV, tgrayV, CV_GAUSSIAN, 5, 5 );

	cvMerge( tgrayH, tgrayS, tgrayV, NULL, hsvImg );

	/* Threshold all three channels using our own values. */
    cvInRangeS( hsvImg, cvScalar(hL, sL, vL), cvScalar(hH, sH, vH), binImg );

    /* Perform erosion, dilation, and conversion. */
    cvErode( binImg, binImg, w );
    cvDilate( binImg, binImg, w );
    cvConvertScale( binImg, outImg, 255.0 );

    /* Find the centroid. */
    center = vision_find_centroid( outImg, 5 );
    *dotx = center.x;
    *doty = center.y;

    /* Clear variables to free memory. */
    cvReleaseImage( &hsvImg );
    cvReleaseImage( &outImg );

	/* Check that the values of dotx & doty are not negative */
	if( dotx < 0 || doty < 0 ) {
		/* Clear variables to free memory. */
		cvReleaseImage( &hsvImg );
		cvReleaseImage( &outImg );
		cvReleaseImage( &hsv_clone );
		cvReleaseImage( &tgrayH );
		cvReleaseImage( &tgrayS );
		cvReleaseImage( &tgrayV );

		return 0;
	}

    /* Clear variables to free memory. */
    cvReleaseImage( &hsvImg );
    cvReleaseImage( &outImg );
    cvReleaseImage( &hsv_clone );
    cvReleaseImage( &tgrayH );
    cvReleaseImage( &tgrayS );
    cvReleaseImage( &tgrayV );

    return 1;
} /* end vision_find_dot() */


/******************************************************************************
 *
 * Title:       int vision_find_pipe(   int *pipex,
 *                                      float *bearing,
 *                                      CvCapture *cap,
 *                                      IplImage *srcImg,
 *                                      IplImage *outImg
 *                                   )
 *
 * Description: Finds a pipe object from a camera.
 *
 * Input:       pipex: Pointer to variable for x position of pipe.
 *              bearing: Pointer to variable for bearing of pipe.
 *              cap: A pointer to an open camera.
 *
 * Output:      status: 1 on success, 0 on failure.
 *
 *****************************************************************************/

int vision_find_pipe( int *pipex,
                      double *bearing,
                      CvCapture *cap,
                      IplImage *srcImg,
                      IplImage *binImg,
                      float hL,
                      float hH,
                      float sL,
                      float sH,
                      float vL,
                      float vH
                    )
{
    CvSize sz = cvSize( srcImg->width & -2, srcImg->height & -2 );
    CvPoint center;
    IplImage *hsv_image = NULL;
    IplImage *hsv_clone = NULL;
    IplImage *outImg = NULL;
    IplImage *tgrayH = NULL;
    IplImage *tgrayS = NULL;
    IplImage *tgrayV = NULL;
	IplConvKernel *wE = cvCreateStructuringElementEx( 2, 2,
            (int)floor( ( 3.0 ) / 2 ), (int)floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
    IplConvKernel *wD = cvCreateStructuringElementEx( 3, 3,
            (int)floor( ( 3.0 ) / 2 ), (int)floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );

    /* Initialize to impossible values. */
    center.x = -1;
    center.y = -1;

    srcImg = cvQueryFrame( cap );
    if( !srcImg ) {
        return 0;
    }

    hsv_image = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 1 );

    /* Convert to HSV and clone image for smoothing */
    cvCvtColor( srcImg, hsv_image, CV_RGB2HSV );
	hsv_clone = cvCloneImage( hsv_image );

	/* Create three separate grayscale images, one for each channel. */
    tgrayH = cvCreateImage( sz, 8, 1 );
	tgrayS = cvCreateImage( sz, 8, 1 );
	tgrayV = cvCreateImage( sz, 8, 1 );

    /* Find squares in every color plane of the image. Filter each plane with a
	 * Gaussian and then merge back to HSV image.  */
    cvSetImageCOI( hsv_clone, 1 );
    cvCopy( hsv_clone, tgrayH, 0 );
	cvSmooth( tgrayH, tgrayH, CV_GAUSSIAN, 5, 5 );

	cvSetImageCOI( hsv_clone, 2 );
    cvCopy( hsv_clone, tgrayS, 0 );
	cvSmooth( tgrayS, tgrayS, CV_GAUSSIAN, 5, 5 );

	cvSetImageCOI( hsv_clone, 3 );
    cvCopy( hsv_clone, tgrayV, 0 );
	cvSmooth( tgrayV, tgrayV, CV_GAUSSIAN, 5, 5 );

	cvMerge( tgrayH, tgrayS, tgrayV, NULL, hsv_image );

	/* Threshold all three channels using our own values. */
	cvInRangeS( hsv_image, cvScalar(hL,sL,vL), cvScalar(hH,sH,vH), binImg );

    /* Perform erosion, dilation, and conversion. */
    cvErode( binImg, binImg, wE );
    cvDilate( binImg, binImg, wD );
    cvConvertScale( binImg, outImg, 255.0 );

    /* Process the image. */
    *bearing = vision_get_bearing( outImg );
    center = vision_find_centroid( outImg, 0 );
    *pipex = center.x;

    /* Clear variables to free memory. */
    cvReleaseImage( &hsv_image );
    cvReleaseImage( &hsv_clone );
    cvReleaseImage( &outImg );
    cvReleaseImage( &tgrayH );
    cvReleaseImage( &tgrayS );
    cvReleaseImage( &tgrayV );

    return 1;
} /* end vision_find_pipe() */


/******************************************************************************
 *
 * Title:       float vision_get_bearing( IplImage *inputBinImg )
 *
 * Description: Fits edges of pipe to a line and calculates its angle.
 *
 * Input:       inputBinImage: The binary image to find the bearing of.
 *
 * Output:      bearing: The angle of the pipe in radians.
 *
 *****************************************************************************/

double vision_get_bearing( IplImage *inputBinImg )
{
    CvScalar Right_STD;
    CvScalar Left_STD;
    Right_STD.val[0] = HUGE_VAL;
    Left_STD.val[0] = HUGE_VAL;
    int leftEdgeCount = 0;
    int rightEdgeCount = 0;
    int imHeight = inputBinImg->height;
    int imWidth = inputBinImg->width;
    int edgeThreshold = 3;
    double maxSTD = 5000;
    int k;
	CvSeq *point_seq;
    CvMemStorage *storage;
    CvMat *LError;
    CvMat *RError;
    CvPoint2D32f point;

    /* Slope (m) and b for left, right and combined estimate. */
    double mL = 0.0;
    double mR = 0.0;
    double m = 0.0;

    /* Edge vectors, first element = x, second = y. */
    int leftEdge[imHeight][2];
    int rightEdge[imHeight][2];

    int i = 0;
    int j = 0;

    /* Initialize edge arrays, mset may be better. */
    for( i = 0; i < imHeight; i++ ) {
        leftEdge[i][0] = 0;
        leftEdge[i][1] = 0;
        rightEdge[i][0] = 0;
        rightEdge[i][1] = 0;
    }
    for( i = 0; i < imHeight - 10; i++ ) {
        /* Scan through each line of image and look for first non zero pixel
         * then get the (i,j) pixel value. */
        while( (cvGet2D(inputBinImg, i, j).val[0] < 1) && (j<imWidth - 1) ) {
            j++;
        }
        /* If we exit before getting to end of row, edge exists. */
        if( (j < imWidth - 1) && (j > 0) ) {
            leftEdge[leftEdgeCount][0] = i; //FLIP i, j, here worksish
            leftEdge[leftEdgeCount][1] = j;
            leftEdgeCount++;
            /* Continue scanning to find right edge. */
            while( (cvGet2D(inputBinImg, i, j).val[0] > 0) && (j < imWidth - 1) ) {
                j++;
            }
            if( j < imWidth - 2 ) { /* Scan didn't get to end of image so
            						  * right edge exists. */
                rightEdge[rightEdgeCount][0] = i;
                rightEdge[rightEdgeCount][1] = j;
                rightEdgeCount++;
            }
        }
        j = 0;
    }

    if( (leftEdgeCount < edgeThreshold) && (rightEdgeCount < edgeThreshold) ) {
        return 0.0;
    }

    /* Begin fitline. */
    float *left_line = new float[4];
    float *right_line = new float[4];

    storage = cvCreateMemStorage( 0 );
    point_seq = cvCreateSeq( CV_32FC2, sizeof(CvSeq), sizeof(CvPoint2D32f), storage );

    if( leftEdgeCount > edgeThreshold ) {
        for( i = 0; i < leftEdgeCount; i++ ) {
        	point = cvPoint2D32f(leftEdge[i][0],leftEdge[i][1]);
            cvSeqPush( point_seq, &point );
        }
        cvFitLine( point_seq, CV_DIST_L12, 0, 0.001, 0.001, left_line );
        mL = atan2( left_line[1], left_line[0] );
        LError = cvCreateMat( 1, leftEdgeCount - 1, CV_32SC1 );
        for( k = 0; k < leftEdgeCount - 1; k++ ) {
            /* Save errors in vector LError. */
            cvSetReal2D( LError, 0, k, (double)(leftEdge[k][2])
                - (double)(( left_line[1] / left_line[0] ) * (leftEdge[k][0] - left_line[1]) + left_line[3]) );
        }
        /* Calculate standard deviation of error. */
        cvAvgSdv( LError, NULL, &Left_STD, NULL );
        cvClearSeq( point_seq );
    }

    if( rightEdgeCount > edgeThreshold ) {
        for( i = 0; i < rightEdgeCount; i++ ) {
        	point = cvPoint2D32f(leftEdge[i][0],leftEdge[i][1]);
            cvSeqPush( point_seq, &point );
        }
        cvFitLine( point_seq, CV_DIST_L12, 0, 0.001, 0.001, right_line );
        mR = atan2( right_line[1], right_line[0] );
        RError = cvCreateMat( 1, rightEdgeCount - 1, CV_32SC1 );
        for( k = 0; k < rightEdgeCount - 1; k++ ) {
            cvSetReal2D( RError, 0, k, (double)(rightEdge[k][2])
                - (double)(( left_line[1] / left_line[0] ) * (rightEdge[k][0]-right_line[1]) + right_line[3]) );
        }
        cvAvgSdv( RError, NULL, &Right_STD, NULL);
        cvClearSeq( point_seq );
    }

    /* If estimate is really poor, do not update bearing. */
    if( (Right_STD.val[0] > maxSTD) && (Left_STD.val[0] > maxSTD) ) {
        return m;
    }

    /* Only a left edge, ignore right. */
    if( rightEdgeCount <= edgeThreshold ) {
        m = mL;
    }
    /* Only a right edge, ignore left. */
    else if( leftEdgeCount <= edgeThreshold ) {
        m = mR;
    }
    /* Both edges exist, scale each estimate by variances. */
    else {
        m = (Right_STD.val[0] * mL + Left_STD.val[0] * mR) /
        	(Right_STD.val[0] + Left_STD.val[0]);
    }

    delete left_line;
    delete right_line;
    cvReleaseMemStorage( &storage );

    return m;
} /* end vision_get_bearing() */


/******************************************************************************
 *
 * Title:       CvPoint vision_find_centroid( IplImage *binImage, int thresh )
 *
 * Description: Finds the centroid of the pixels in the image.
 *
 * Input:       binImage: The binary image to find the centroid of.
 *              thresh: The amount of pixels required for centroid to be valid.
 *
 * Output:      centroid: The x and y coordinates of the centroid.
 *
 *****************************************************************************/

CvPoint vision_find_centroid( IplImage *binImage, int thresh )
{
    /* Return value. */
    CvPoint centroid;

    /* Get image width and height. */
    int width = binImage->width;
    int height = binImage->height;

    /* Totals. */
    unsigned int rowTotal = 0;
    unsigned int colTotal = 0;
    int count = 0;

    /* Counters. */
    unsigned int ii = 0;
    unsigned int jj = 0;
    bool detected = FALSE;

    /* Find centroid. */
    for( ii = ( binImage->roi == NULL ? 0 : ( unsigned int )binImage->roi->xOffset );
            ii < ( binImage->roi == NULL ? ( unsigned int )width : ( unsigned int )( binImage->roi->xOffset + binImage->roi->width ) );
            ii++ ) {
        for( jj = ( binImage->roi == NULL ? 0 : ( unsigned int )binImage->roi->yOffset );
                jj < ( binImage->roi == NULL ? ( unsigned int )height : ( unsigned int )( binImage->roi->yOffset + binImage->roi->height ) );
                jj++ ) {
            if( binImage->imageData[ii + jj * width] != 0 ) {
                if( binImage->roi == NULL ) {
                    colTotal += ii;
                    rowTotal += jj;
                }
                else {
                    colTotal += ii - ( unsigned int )binImage->roi->xOffset;
                    rowTotal += jj - ( unsigned int )binImage->roi->yOffset;
                }
                count++;
            }
        }
    }

    /* Check if an object is detected. */
    if( count > thresh )
        detected = true;

    /* If the centroid was detected convert it */
    if( detected ) {
        centroid.x = (int)colTotal / count;
        centroid.y = (int)rowTotal / count;
    }
    else { /* Send back negatives if we do not have a positive detection */
        centroid.x = -1;
        centroid.y = -1;
    }

    return centroid;
} /* end vision_find_centroid() */


/******************************************************************************
 *
 * Title:       int vision_find_fence(   int *pipex,
 *                                      float *bearing,
 *                                      CvCapture *cap,
 *                                      IplImage *srcImg,
 *                                      IplImage *outImg
 *                                   )
 *
 * Description: Finds a pipe object from a camera.
 *
 * Input:       pipex: Pointer to variable for x position of pipe.
 *              bearing: Pointer to variable for bearing of pipe.
 *              cap: A pointer to an open camera.
 *
 * Output:      status: 1 on success, 0 on failure.
 *
 *****************************************************************************/

int vision_find_fence( int *fence_center,
                      int *y_max,
                      CvCapture *cap,
                      IplImage *srcImg,
                      IplImage *binImg,
                      float hL,
                      float hH,
                      float sL,
                      float sH,
                      float vL,
                      float vH
                    )
{
	int center = 0;
	int sum_x = 0;
    int ii = 0;
    int jj = 0;
    int kk = 0;

    IplImage *hsv_image = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *wE = cvCreateStructuringElementEx( 2, 2,
            1, 1, CV_SHAPE_RECT );
    IplConvKernel *wD = cvCreateStructuringElementEx( 9, 9,
            (int)floor( ( 9.0 ) / 2 ), (int)floor( ( 9.0 ) / 2 ), CV_SHAPE_RECT );
    CvSize sz = cvSize( srcImg->width & -2, srcImg->height & -2 );
    IplImage *hsv_clone = NULL;
    IplImage *tgrayH = NULL;
    IplImage *tgrayS = NULL;
    IplImage *tgrayV = NULL;
    IplImage *filterImg = NULL;
    CvMat *filter;
    int kernelCols = 7;
    int kernelRows = 3;
    double kernel[] = {1, 0, 0, 0, 0, 0, 0,
    					0, 1, 0, 0, 0, 0, 0,
    					0, 0, 1, 0, 0, 0, 0};

	/* Capture a new source image. */
    srcImg = cvQueryFrame( cap );
    if( !srcImg ) {
        return 0;
    }

    /* Flip the source image. */
  	cvFlip( srcImg, srcImg );
    center = srcImg->width / 2;

	/* Create intermediate images. */
    hsv_image = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 1 );
    filterImg = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 1 );

    /* Segment the image into a binary image. */
    cvCvtColor( srcImg, hsv_image, CV_RGB2HSV );
	hsv_clone = cvCloneImage( hsv_image );

	/* Create three separate grayscale images, one for each channel. */
    tgrayH = cvCreateImage( sz, 8, 1 );
	tgrayS = cvCreateImage( sz, 8, 1 );
	tgrayV = cvCreateImage( sz, 8, 1 );

    /* Find squares in every color plane of the image. Filter each plane with a
	 * Gaussian and then merge back to HSV image.  */
    cvSetImageCOI( hsv_clone, 1 );
    cvCopy( hsv_clone, tgrayH, 0 );
	cvSmooth( tgrayH, tgrayH, CV_GAUSSIAN, 5, 5 );

	cvSetImageCOI( hsv_clone, 2 );
    cvCopy( hsv_clone, tgrayS, 0 );
	cvSmooth( tgrayS, tgrayS, CV_GAUSSIAN, 5, 5 );

	cvSetImageCOI( hsv_clone, 3 );
    cvCopy( hsv_clone, tgrayV, 0 );
	cvSmooth( tgrayV, tgrayV, CV_GAUSSIAN, 5, 5 );

	cvMerge( tgrayH, tgrayS, tgrayV, NULL, hsv_image );

	/* Threshold all three channels using our own values. */
    cvInRangeS( hsv_image, cvScalar(hL,sL,vL), cvScalar(hH,sH,vH), binImg );

    /* Perform erosion, dilation, and conversion. */
    cvErode( binImg, binImg, wE );
    cvDilate( binImg, binImg, wD );
    cvConvertScale( binImg, outImg, 255.0 );

    /* Filter the image using convolution. */
    filter = cvCreateMatHeader( kernelRows, kernelCols, CV_64FC1 );
    cvSetData( filter, kernel, kernelCols * 8 );
    cvFilter2D( outImg, filterImg, filter, cvPoint(-1,-1) );

    /* Process the image. */
    *y_max = vision_get_fence_bottom( outImg, &center );

    /* Compute Centroid. */
    for( ii = 0; ii < binImg->height; ii++ ) {
        for( jj = 0; jj < binImg->width; jj++ ) {
        	if( cvGet2D(binImg, ii, jj).val[0] != 0 ) {
        		sum_x += jj;
        		kk++;
			}
		}
	}
    *fence_center = floor(sum_x / kk);

    /* Clear variables to free memory. */
    cvReleaseImage( &hsv_image );
    cvReleaseImage( &outImg );
    cvReleaseImage( &hsv_clone );
    cvReleaseImage( &tgrayH );
    cvReleaseImage( &tgrayS );
    cvReleaseImage( &tgrayV );
    cvReleaseImage( &filterImg );

    return 1;
} /* end vision_find_pipe() */


/******************************************************************************
 *
 * Title:       float vision_get_fence_bottom( IplImage *inputBinImg )
 *
 * Description: Fits edges of pipe to a line and calculates its angle.
 *
 * Input:       inputBinImage: The binary image to find the bearing of.
 *
 * Output:      bearing: The angle of the pipe in radians.
 *
 *****************************************************************************/

int vision_get_fence_bottom( IplImage *inputBinImg, int *center )
{
    int y_max = 0;
    int imHeight = inputBinImg->height;
    int imWidth = inputBinImg->width;
    //int minPipeWidth = 20;
    int minPipeWidth = 10;
    int edgeThreshold = 2;
    int c= 0;
    int k=0;

    /* Edge vectors, first element = x, second = y. */
    int leftEdge[imHeight];
    int rightEdge[imHeight];
    int i = 0; /* rows */
    int j = 0; /* columns */

    /* Initialize edge arrays, memset may be better. */
    memset( &leftEdge, 0, sizeof(leftEdge) );
    memset( &rightEdge, 0, sizeof(rightEdge) );
    //for( i = 0; i < imHeight; i++ ) {
        //leftEdge[i] = 0;
        //rightEdge[i] = imWidth;
    //}
    for( i = 0; i < imHeight - 1; i++ ) {
        /* Scan through each line of image and look for first non zero pixel
         * then get the (i,j) pixel value. */
        while( (cvGet2D(inputBinImg, i, j).val[0] < 1) && (j < imWidth - 2) ) {
            j++;
        }
        /* If we exit before getting to end of row, edge exists. */
        if( (j < imWidth) && (j > minPipeWidth) ) {
            leftEdge[k] = j;
            /* Countinue scanning to find bottom edge. */
            while( (cvGet2D(inputBinImg, i, j).val[0] > 0) && (j < imWidth - 2) ) {
                j++;
            }
            if( j < imWidth - 2) {
            	/* Scan didn't get to end of image, bottom edge exists. */
                rightEdge[k] = j;
            }
            if( rightEdge[k] - leftEdge[k] > minPipeWidth ) {
            	y_max = i;
			}
        }
        k++;
        j = 0;
    }

    /* We found a fence. */
    if( y_max > edgeThreshold ) {
    	for( i = 0; i < k; i++ ) {
        		c += rightEdge[i] - leftEdge[i];
		}
	}
	*center = c / k;

    return y_max;
} /* end vision_get_fence_bottom() */


/******************************************************************************
 *
 * Title:       int vision_find_boxes(  CvCapture *cap,
 * 					   			        IplImage *srcImg,
 * 										CvSeq *boxes
 *                                    )
 *
 * Description: Finds rectangles in an image from a camera.
 *
 * Input:       cap: A pointer to an open camera.
 *				srcImg: Memory location to store a captured image.
 *
 * Output:      status: 1 on success, 0 on failure.
 *
 *****************************************************************************/

int vision_find_boxes( CvCapture *cap,
                       IplImage *srcImg,
					   CvSeq *result,
					   CvSeq *squares )
{
	/* Declare variables. */
	IplImage *img = NULL;
	CvMemStorage *storage = 0;
	int status = -1;

	/* Initialize variables. */
	storage = cvCreateMemStorage(0);

    /* Capture a new source image. */
    srcImg = cvQueryFrame( cap );
    if ( !srcImg ) {
        return 0;
    }

	/* Clone the source image so that we have an image we can write over. The
	 * source image needs to be kept clean so that we can display it later. */
	img = cvCloneImage( srcImg );
    status = vision_find_squares4( img, storage, result, squares );

    /* Clear memory storage and reset free space position. */
    cvReleaseImage( &img );
    cvClearMemStorage( storage );

    return status;
} /* end vision_find_boxes() */


/******************************************************************************
 *
 * Title:       double vision_angle(  CvPoint *pt1,
 * 									  CvPoint *pt2,
 * 									  CvPoint *pt0
 *                                  )
 *
 * Description: Finds the cosine of the angle between two vectors.
 *
 * Input:       pt1: First point.
 *				pt2: Second point.
 * 				pt0: Point used to find angle from.
 *
 * Output:      The angle between the points.
 *
 *****************************************************************************/

double vision_angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 )
{
	/* Calculate length between points. */
    double dx1 = pt1->x - pt0->x;
    double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;

	/* The find box code detects a box at the image boundary. To remove this we
	 * are going to exclude boxes that are within a small band around the edge
	 * of the image. There are magic numbers here for a 640x480 image that
	 * should be changed to be a percentage of the actual image being
	 * processed. That would involve passing the image size in as an extra
	 * argument. */
	if( (fabsf(dy1 - dy2) > 450.) || (fabsf(dx1 - dx2) > 610.) ) {
		return 100000.;
	}

	/* Return the cosine between the two points. */
    return( dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10 );
} /* end vision_angle() */


/******************************************************************************
 *
 * Title:       int vision_find_squares4(  IplImage *img,
 * 										   CvMemStorage *storage,
 * 										   CvSeq *box_centers,
 * 										   CvSeq *squares
 *                                       )
 *
 * Description: Finds all the rectangles in an image.
 *
 * Input:       img: The image to search for rectangle shapes.
 *				storage: Memory location to store square locations.
 * 				box_centers: A sequence to store the box center locations in.
 *
 * Output:      The number of rectangles found in the image.
 *
 *****************************************************************************/

int vision_find_squares4( IplImage *img, CvMemStorage *storage, CvSeq *box_centers, CvSeq *squares )
{
	/* Declare variables. */
    CvSeq* contours;
    CvMoments moments;
	CvPoint box_centroid;
    int i, c, l, N = 11;
    CvSize sz = cvSize( img->width & -2, img->height & -2 );
    IplImage* timg = cvCloneImage( img );
    IplImage* gray = cvCreateImage( sz, 8, 1 );
    IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 3 );
    IplImage* tgray = NULL;
    CvSeq* result;
    double s, t;
	int thresh = 50;
	int status = 0;

    /* Select the maximum ROI in the image with the width and height divisible by 2. */
    cvSetImageROI( timg, cvRect( 0, 0, sz.width, sz.height ));

    /* Down-scale and upscale the image to filter out the noise. */
    cvPyrDown( timg, pyr, 7 );
    cvPyrUp( pyr, timg, 7 );

	/* Create a grayscale image. */
    tgray = cvCreateImage( sz, 8, 1 );

    /* Find squares in every color plane of the image. */
    for( c = 0; c < 3; c++ ) {
        /* Extract the c-th color plane. */
        cvSetImageCOI( timg, c + 1 );
        cvCopy( timg, tgray, 0 );

        /* Try several threshold levels. */
        for( l = 0; l < N; l++ ) {
            /* !!! HACK !!!: Use Canny instead of zero threshold level. Canny
			 * helps to catch squares with gradient shading. */
            if( l == 0 ) {
                /* Apply Canny and use the upper threshold and set the lower to
				 * 0 (which forces edges merging). */
                cvCanny( tgray, gray, 0, thresh, 5 );
                /* Dilate Canny output to remove potential holes between edge
				 * segments. */
                cvDilate( gray, gray, 0, 1 );
            }
            else {
                /* Apply threshold if l != 0. */
                cvThreshold( tgray, gray, (l+1)*255/N, 255, CV_THRESH_BINARY );
            }

            /* Find contours and store them all as a list. */
            cvFindContours( gray, storage, &contours, sizeof(CvContour),
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

            /* Test each contour to see if it is a rectangle. */
            while( contours ) {
                /* Approximate contour with accuracy proportional to the contour perimeter. */
                result = cvApproxPoly( contours, sizeof(CvContour), storage,
                    CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
                /* Square contours should have 4 vertices after approximation
                 * relatively large area (to filter out noisy contours) and be convex.
                 * Note: absolute value of an area is used because area may be
				 * positive or negative in accordance with the contour orientation. */
                if( result->total == 4 && fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 1000 &&
                    cvCheckContourConvexity(result) ) {
                    s = 0;
                    for( i = 0; i < 5; i++ ) {
                        /* Find the minimum angle between joint edges (max of cos). */
                        if( i >= 2 ) {
                            t = fabs(vision_angle(
								(CvPoint*)cvGetSeqElem( result, i ),
								(CvPoint*)cvGetSeqElem( result, i-2 ),
								(CvPoint*)cvGetSeqElem( result, i-1 )));
                            s = s > t ? s : t;
                        }
                    }

                    /* If cosines of all angles are small (all angles are ~90 degree)
					 * then write rectangle vertices to resultant sequence. */
                    if( s < 0.16 ) {
                        for( i = 0; i < 4; i++ ) {
                         	cvContourMoments( result, &moments );
        				 	box_centroid.x = (int)(moments.m10 / moments.m00);
						 	box_centroid.y = (int)(moments.m01 / moments.m00);
                         	cvSeqPush( box_centers, &box_centroid );
                         	cvSeqPush( squares, (CvPoint *)cvGetSeqElem(result, i) );
							status++;
                        }
					}
				}
				/* Look at the next contour to see if it is a rectangle. */
				contours = contours->h_next;
            }
        }
    }

    /* Release all temporary images to free up memory. */
    cvReleaseImage( &gray );
    cvReleaseImage( &pyr );
    cvReleaseImage( &tgray );
    cvReleaseImage( &timg );

    return status;
} /* end vision_find_squares4() */


/*****************************************************************
 the function draws all the squares in the image
 * ***************************************************************/
/*void drawSquares( IplImage* img, CvSeq* squares )
{
    CvSeqReader reader;
    IplImage* cpy = cvCloneImage( img );
    int i;

    // initialize reader of the sequence
    cvStartReadSeq( squares, &reader, 0 );

    // read 4 sequence elements at a time (all vertices of a square)
    for( i = 0; i < squares->total; i += 4 )
    {
        CvPoint pt[4], *rect = pt;
        int count = 4;

        // read 4 vertices
        CV_READ_SEQ_ELEM( pt[0], reader );
        CV_READ_SEQ_ELEM( pt[1], reader );
        CV_READ_SEQ_ELEM( pt[2], reader );
        CV_READ_SEQ_ELEM( pt[3], reader );

        // draw the square as a closed polyline
        cvPolyLine( cpy, &rect, &count, 1, 1, CV_RGB(0,255,0), 3, CV_AA, 0 );
    }

    // show the resultant image
    cvShowImage( wndname, cpy );
    cvReleaseImage( &cpy );
}
*/

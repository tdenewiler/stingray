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

#include "vision.h"


/******************************************************************************
 *
 * Title:       int vision_find_dot(    int *dotx,
 *                                      int *doty,
 *                                      int amt,
 *                                      CvCapture *cap,
 *										IplImage *srcImg,
 * 										IplImage *outImg
 * 									)
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
 * Globals:     None.
 *
 *****************************************************************************/

int vision_find_dot(	int *dotx,
                     int *doty,
                     int amt,
                     CvCapture *cap,
                     IplImage *srcImg,
                     IplImage *outImg
                   )
{
	CvPoint center;
	IplImage *binImg = NULL;

	/* Initialize to impossible values. */
	center.x = -1;
	center.y = -1;

	/* Get a frame. Return if no video. */
	srcImg = cvQueryFrame( cap );
	if ( !srcImg ) {
		return -2;
	}

	/* Create output image. */
	outImg = cvCreateImage( cvSize( srcImg->width, srcImg->height ), IPL_DEPTH_8U, 1 );

	/* Flip the source image. */
	cvFlip( srcImg, srcImg );

	/* Segment the flipped image into a binary image. */
	binImg = vision_segment_image( 2.0f, 12.0f, 10.0f, 250.00f, 10.0f, 250.0f, amt + 5, srcImg, 0, 0 );
	cvConvertScale( binImg, outImg, 255.0 );

	/* Process the image. */
	center = vision_find_centroid( outImg, 0 );
	*dotx = center.x;
	*doty = center.y;

	/* Release the binary image. */
	cvReleaseImage( &binImg );

	return 1;
} /* end vision_find_dot() */


/******************************************************************************
 *
 * Title:       int vision_find_pipe(	int *pipex,
 *                                      float *bearing,
 *                                      CvCapture *cap,
 * 										IplImage *srcImg,
 * 										IplImage *outImg
 * 									 )
 *
 * Description: Finds a pipe object from a camera.
 *
 * Input:       pipex: Pointer to variable for x position of pipe.
 *              bearing: Pointer to variable for bearing of pipe.
 *              cap: A pointer to an open camera.
 *
 * Output:      status: 1 on success, 0 on failure.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int vision_find_pipe(	int *pipex,
                      float *bearing,
                      CvCapture *cap,
                      IplImage *srcImg,
                      IplImage *outImg
                    )
{
	CvPoint center;
	IplImage *binImg;

	/* Initialize to impossible values. */
	center.x = -1;
	center.y = -1;

	/* Get a frame. */
	srcImg = cvQueryFrame( cap );
	if ( !srcImg ) {
		return -2;
	}

	/* Create output images. */
	binImg = cvCreateImage( cvSize( srcImg->width, srcImg->height ), IPL_DEPTH_8U, 1 );
	outImg = cvCreateImage( cvSize( srcImg->width, srcImg->height ), IPL_DEPTH_8U, 1 );

	/* Segment the image into a binary image. */
	binImg = vision_segment_image( 10.0f, 33.0f, 1.0f, 250.00f, 2.0f, 250.0f, 3, srcImg, 0 , 0 );
	cvConvertScale( binImg, outImg, 255.0 );

	/* Process the image. */
	*bearing = vision_get_bearing( outImg );
	center = vision_find_centroid( outImg, 0 );
	*pipex = center.y;

	cvReleaseImage( &binImg );

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
 * Output:      bearing: The angle of the pipe in degrees.
 *
 * Globals:     None.
 *
 *****************************************************************************/

float vision_get_bearing( IplImage *img )
{
	int leftEdgeCount = 0;
	int rightEdgeCount = 0;
	int imHeight = img->height;
	int imWidth = img->width;
	float LError = 0;
	float RError = 0;
	int k = 0;
	int sumXL = 0;
	int sumXR = 0;
	int sumYL = 0;
	int sumYR = 0;
	int xlDOTxl = 0;
	int xrDOTxr = 0;
	int xlDOTyl = 0;
	int xrDOTyr = 0;
	float mL;
	float mR;
	float bL;
	float bR;
	float m;
	float b;

	/* First element = x, second = y. */
	int leftEdge[imHeight][2];
	int rightEdge[imHeight][2];

	/* Image row and column indices. */
	int i = 0;
	int j = 0;

	/* Initialize edge arrays. */
	for ( i = 0; i < imHeight; i++ ) {
		leftEdge[i][1]  = 0;
		leftEdge[i][2]  = 0;
		rightEdge[i][1] = 0;
		rightEdge[i][2] = 0;
	}

	for ( i = 0; i < imHeight - 1; i++ ) {
		/* Scan through each line of img and look for first non zero pixel. */
		/* Get the (i,j) pixel value. */
		while ( ( cvGet2D( img, i, j ).val[0] < 1 ) && ( j < imWidth - 1 ) ) {
			j++;
		}
		/* If we exit before getting to end of row, edge exists. */
		if ( ( j < imWidth - 1 ) && ( j > 0 ) ) {
			leftEdge[leftEdgeCount][1] = i;
			leftEdge[leftEdgeCount][2] = j;
			leftEdgeCount++;
			/* Continue scanning to find right edge. */
			while ( ( cvGet2D( img, i, j ).val[0] > 0 ) && ( j < imWidth - 1 ) ) {
				j++;
			}
			/* Scan didn't get to end of img. */
			if ( j < imWidth - 2 ) {
				rightEdge[rightEdgeCount][1] = i;
				rightEdge[rightEdgeCount][2] = j;
				rightEdgeCount++;
			}
		}
		j = 0;
	}

	/* Do least squares filtering. */
	for ( k = 0; k < leftEdgeCount - 1; k++ ) {
		sumXL   = sumXL   + leftEdge[k][1];
		sumYL   = sumYL   + leftEdge[k][2];
		xlDOTxl = xlDOTxl + leftEdge[k][1] * leftEdge[k][1];
		xlDOTyl = xlDOTyl + leftEdge[k][1] * leftEdge[k][2];
	}

	mL = ( float )( leftEdgeCount * xlDOTyl - sumXL * sumYL / leftEdgeCount * xlDOTxl - sumXL * sumXL );
	bL = ( float )sumYL - mL * ( float )sumXL / ( float )leftEdgeCount;

	for ( k = 0; k < rightEdgeCount - 1; k++ ) {
		sumXR   = sumXR   + rightEdge[k][1];
		sumYR   = sumYR   + rightEdge[k][2];
		xrDOTxr = xrDOTxr + rightEdge[k][1] * rightEdge[k][1];
		xrDOTyr = xrDOTyr + rightEdge[k][1] * rightEdge[k][2];
	}

	mR = ( float )( rightEdgeCount * xrDOTyr - sumXR * sumYR / rightEdgeCount * xrDOTxr - sumXR * sumXR );
	bR = ( float )sumYR - mR * ( float )sumXR / ( float )rightEdgeCount;

	/* Choose minimum variance estimator. */
	for ( k = 0; k < leftEdgeCount - 1; k++ ) {
		LError = pow( leftEdge[k][2] - mL * leftEdge[k][1] + bL, 2 );
	}
	for ( k = 0; k < rightEdgeCount - 1; k++ ) {
		RError = pow( rightEdge[k][2] - mR * rightEdge[k][1] + bR, 2 );
	}
	if ( LError < RError ) {
		m = atan( mL ) * 180.0 / M_PI;
		b = bL;
	}
	else {
		m = atan( mR ) * 180.0 / M_PI;
		b = bR;
	}

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
 * Globals:     None.
 *
 *****************************************************************************/

CvPoint vision_find_centroid( IplImage *binImage, int thresh )
{
	/* Get image width and height. */
	int width = binImage->width;
	int height = binImage->height;

	bool detected = FALSE;
	char *data = binImage->imageData;
	CvPoint centroid;

	/* Totals. */
	unsigned int rowTotal = 0;
	unsigned int colTotal = 0;
	int count = 0;

	/* Final centroid position coordinates. */
	int x, y;
	unsigned int ii = 0;
	unsigned int jj = 0;

	/* Find centroid. */
	for ( ii = ( binImage->roi == NULL ? 0 : ( unsigned int )binImage->roi->xOffset );
	        ii < ( binImage->roi == NULL ? ( unsigned int )width : ( unsigned int )( binImage->roi->xOffset + binImage->roi->width ) );
	        ii++ ) {
		for ( jj = ( binImage->roi == NULL ? 0 : ( unsigned int )binImage->roi->yOffset );
		        jj < ( binImage->roi == NULL ? ( unsigned int )height : ( unsigned int )( binImage->roi->yOffset + binImage->roi->height ) );
		        jj++ ) {
			if ( data[ii + jj * width] != 0 ) {
				if ( binImage->roi == NULL ) {
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
	if ( count > thresh ) {
		detected = true;
	}

	if ( detected ) {
		x = ( int )colTotal / count;
		y = ( int )rowTotal / count;

		/* Create centroid point. */
		centroid = cvPoint( x, y );
	}
	else {
		centroid = cvPoint( -1, -1 );
	}

	return centroid;
} /* end vision_find_centroid() */


/******************************************************************************
 *
 * Title:       IplImage *vision_segment_image( float hL,
 *                              float hH,
 *                              float sL,
 *                              float sH,
 *                              float vL,
 *                              float vH,
 *                              int closingAmount,
 *                              IplImage* img,
 *                              int boxWidth,
 *                              int boxHeight
 *                              )
 *
 * Description: Returns a black and white thresholded image from a source image
 *              and the hue, saturation, and value thresholds.
 *
 * Input:       binImage: The binary image to find the centroid of.
 *              thresh: The amount of pixels required for centroid to be valid.
 *
 * Output:      binImage: Returns a black and white thresholded image from a
 *              source image and the hue, saturation, and value thresholds.
 *
 * Globals:     None.
 *
 *****************************************************************************/

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
                              )
{
	/* Get original image's width and height. */
	int width = 0;
	int height = 0;
	width = img->width;
	height = img->height;

	/* Create structuring element. */
	IplConvKernel *w = cvCreateStructuringElementEx( 10, 10, ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
	IplConvKernel *wBig = cvCreateStructuringElementEx( 20, 20, ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
	//IplConvKernel *wSmall = cvCreateStructuringElementEx( 3, 3, (int)floor((3.0)/2), (int)floor((3.0)/2), CV_SHAPE_RECT );
	IplConvKernel *wCircle = cvCreateStructuringElementEx( 7, 7, ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_ELLIPSE );

	/* Adjusted thresholding values. */
	CvScalar THL = cvScalar( hL );
	CvScalar THH = cvScalar( hH );
	CvScalar TSL = cvScalar( sL );
	CvScalar TSH = cvScalar( sH );
	CvScalar TVL = cvScalar( vL );
	CvScalar TVH = cvScalar( vH );

	/* Convert original image to HSV from BGR. */
	IplImage* hsv_image = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 3 );
	cvCvtColor( img, hsv_image, CV_BGR2HSV );

	/* Seperate image into hue, saturation, and value components. */
	IplImage* hue_com = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
	IplImage* sat_com = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
	IplImage* val_com = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
	cvSplit( hsv_image, hue_com, sat_com, val_com, 0 );

	/* Mask for hue. */
	CvMat* hue_mask = cvCreateMat( height, width, CV_8UC1 );
	cvInRangeS( hue_com, THL, THH, hue_mask );

	/* Mask for saturation. */
	CvMat* sat_mask = cvCreateMat( height, width, CV_8UC1 );
	cvInRangeS( sat_com, TSL, TSH, sat_mask );

	/* Mask for value. */
	CvMat* val_mask = cvCreateMat( height, width, CV_8UC1 );
	cvInRangeS( val_com, TVL, TVH, val_mask );

	/* Combine all the masks into one matrix via element multiplication. */
	CvMat* hsv_mask = cvCreateMat( height, width, CV_8UC1 );
	cvCopy( hue_mask, hsv_mask );
	cvMul( sat_mask, hsv_mask, hsv_mask );
	cvMul( val_mask, hsv_mask, hsv_mask );

	/* Disabled scaling. */
	//cvScale( hsv_mask, hsv_mask, 1/255.0 );

	IplImage* temp_image = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
	/* Apply the hsv mask to hue component of image and store in hsv_image channel 1. */
	cvSetImageCOI( hsv_image, 1 );
	cvMul( hue_com, hsv_mask, temp_image );
	cvCopy( temp_image, hsv_image );

	/* Apply the hsv mask to saturation component of image and store in hsv_image channel 2. */
	cvSetImageCOI( hsv_image, 2 );
	cvMul( sat_com, hsv_mask, temp_image );
	cvCopy( temp_image, hsv_image );

	/* Apply the hsv mask to value component of image and store in hsv_image channel 3. */
	cvSetImageCOI( hsv_image, 3 );
	cvMul( val_com, hsv_mask, temp_image );
	cvCopy( temp_image, hsv_image );

	/* Reset the hsv_seg's channel of interest to all. */
	cvSetImageCOI( hsv_image, 0 );
	IplImage* masked_image = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 3 );

	/* Convert HSV image to a BGR image stored in masked_image. */
	cvCvtColor( hsv_image, masked_image, CV_HSV2BGR );

	/* Convert the masked image to grayscale. */
	IplImage* bin_image = cvCreateImage( cvSize( width, height ), IPL_DEPTH_8U, 1 );
	cvCvtColor( masked_image, bin_image, CV_BGR2GRAY );

	/* Do thresholding. */
	char *data = bin_image->imageData;

	for ( int i = 0; i < bin_image->width; i++ ) {
		for ( int j = 0; j < bin_image->height; j++ ) {
			/* Set outside of desired box to zero. */
			if ( ( boxHeight > 0 ) &&
			        ( ( i < ( bin_image->width - boxWidth ) / 2 ) ||
			          ( j < ( bin_image->height - boxHeight ) / 2 ) ||
			          ( i > bin_image->width - ( ( bin_image->width - boxWidth ) / 2 ) ) ||
			          ( j > bin_image->height - ( ( bin_image->height - boxHeight ) / 2 ) ) ) ) {
				data[i + j * bin_image->widthStep] = 0;
			}
			else if ( data[i + j * bin_image->widthStep] != 0 ) {
				data[i + j * bin_image->widthStep] = 1;
			}
		}
	}

	/* Perform dilation. */
	cvErode( bin_image, bin_image, w );
	cvDilate( bin_image, bin_image, wCircle );

	for ( int i = 0; i < closingAmount; i++ ) {
		cvDilate( bin_image, bin_image, w );
	}

	/* Perform erosion. */
	for ( int i = 0; i < closingAmount; i++ ) {
		cvErode( bin_image, bin_image, w );
	}

	//cvErode( bin_image, bin_image, wBig );
	//cvDilate( bin_image, bin_image, wBig );
	cvDilate( bin_image, bin_image, wBig );
	cvDilate( bin_image, bin_image, wCircle );

	//for( int i = 0; i < closingAmount; i++ ) {
	//  cvErode( bin_image, bin_image, wBig );
	//}

	/* Release data structures. */
	cvReleaseMat( &hue_mask );
	cvReleaseMat( &sat_mask );
	cvReleaseMat( &val_mask );
	cvReleaseMat( &hsv_mask );
	cvReleaseImage( &hue_com );
	cvReleaseImage( &sat_com );
	cvReleaseImage( &val_com );
	cvReleaseImage( &hsv_image );
	cvReleaseImage( &temp_image );
	cvReleaseImage( &masked_image );
	cvReleaseStructuringElement( &w );
	cvReleaseStructuringElement( &wBig );
	cvReleaseStructuringElement( &wCircle );

	/* Return binary image. */
	return bin_image;
} /* end vision_segment_image() */

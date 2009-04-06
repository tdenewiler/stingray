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
 * Globals:     None.
 *
 *****************************************************************************/

int vision_find_dot( int *dotx,
                     int *doty,
                     int *width,
                     int *height,
                     int amt,
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
    IplImage *hsv_image = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *w = cvCreateStructuringElementEx( 2, 2,
            ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );

    /* Initialize to impossible values. */
    center.x = -1;
    center.y = -1;

    srcImg = cvQueryFrame( cap );
    if ( !srcImg ) {
        return 0;
    }

    hsv_image = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 1 );

    /* Flip the source image. */
    cvFlip( srcImg, srcImg );

    /* Segment the flipped image into a binary image. */
    cvCvtColor( srcImg, hsv_image, CV_RGB2HSV );
    cvInRangeS( hsv_image, cvScalar(hL, sL, vL), cvScalar(hH, sH, vH), binImg );

    /* Perform erosion and dilation. */
    cvErode( binImg, binImg, w );
    cvDilate( binImg, binImg, w );

    cvConvertScale( binImg, outImg, 255.0 );

    /* Find the centroid. */
    center = vision_find_centroid( outImg, 5 );
    *dotx = center.x;
    *doty = center.y;

    /* Clear variables to free memory. */
    cvReleaseImage( &hsv_image );
    cvReleaseImage( &outImg );

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
 * Globals:     None.
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
    CvPoint center;
    IplImage *hsv_image = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *wE = cvCreateStructuringElementEx( 2, 2,
            ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
    IplConvKernel *wD = cvCreateStructuringElementEx( 3, 3,
            ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );

    /* Initialize to impossible values. */
    center.x = -1;
    center.y = -1;

    srcImg = cvQueryFrame( cap );
    if ( !srcImg ) {
        return 0;
    }

    hsv_image = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 1 );

    /* Segment the image into a binary image. */
    cvCvtColor( srcImg, hsv_image, CV_RGB2HSV );
    cvInRangeS( hsv_image, cvScalar(hL,sL,vL), cvScalar(hH,sH,vH), binImg );

    /* Perform erosion and dilation. */
    cvErode( binImg, binImg, wE );
    cvDilate( binImg, binImg, wD );

    cvConvertScale( binImg, outImg, 255.0 );

    /* Process the image. */
    *bearing = vision_get_bearing( outImg );
    center = vision_find_centroid( outImg, 0 );
    *pipex = center.y;

    /* Clear variables to free memory. */
    cvReleaseImage( &hsv_image );
    cvReleaseImage( &outImg );

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
 * Globals:     None.
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
    //minimum num of edge points before we declare a line
    int edgeThreshold = 5;

    //define max standard deviation of the estimate
    double maxSTD = 75; // need to determine this val experimentally
    int k; // an index

    //slope (m) and b for left, right and combined estimate
    double mL = 0.0;
    double mR = 0.0;
    double m = 0.0;

    //edge vectors, first element = x, second = y
    int leftEdge[imHeight][2];
    int rightEdge[imHeight][2];
    int i = 0; //image row index
    int j = 0; //image column index

    //initialize edge arrays, mset may be better
    for(i = 0;i<imHeight;i++){
        leftEdge[i][1]=0;
        leftEdge[i][2]=0;
        rightEdge[i][1]=0;
        rightEdge[i][2]=0;
    }
    for(i = 0;i<imHeight-240;i++){
        //scan through each line of image and look for first non zero pixel
        // get the (i,j) pixel value
        while(cvGet2D(inputBinImg,i,j).val[0]<1 && j<imWidth-1){
            j++;
        }
        //if we exit before getting to end of row, edge exists
        if(j<imWidth-1 && j>0){
            leftEdge[leftEdgeCount][1]=i;
            leftEdge[leftEdgeCount][2]=j;
            leftEdgeCount++;
            //countinue scanning to find right edge
            while(cvGet2D(inputBinImg,i,j).val[0]>0 && j<imWidth-1){
                j++;
            }
            if(j<imWidth-2){ //scan didnt get to end of image, right edge exists
                rightEdge[rightEdgeCount][1]=i;
                rightEdge[rightEdgeCount][2]=j;
                rightEdgeCount++;
            }
        }
        j = 0;
    }

    if(leftEdgeCount<edgeThreshold && rightEdgeCount<edgeThreshold){
        return 0.0;
    }

    //begin fitline
    float *left_line = new float[4];
    float *right_line = new float[4];

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* point_seq = cvCreateSeq( CV_32FC2, sizeof(CvSeq), sizeof(CvPoint2D32f), storage );

    if(leftEdgeCount>edgeThreshold){
        for (i=0; i<leftEdgeCount; i++){
            cvSeqPush(point_seq, &cvPoint2D32f(leftEdge[i][1],leftEdge[i][2]));
        }
        cvFitLine(point_seq,CV_DIST_L2,0,0.01,0.01,left_line);
        mL = left_line[1]/left_line[0];

        CvMat* LError = cvCreateMat(1,leftEdgeCount-1,CV_32SC1);
        for ( k = 0; k < leftEdgeCount - 1; k++ ) {
            //save errors in vector LError
            cvSetReal2D(LError, 0, k, (double)(leftEdge[k][2])
                -(double)(mL* (leftEdge[k][1]-left_line[2]) + left_line[3]));
        }
        //calculate standard deviation of error
        cvAvgSdv( LError, NULL, &Left_STD, NULL );
        cvClearSeq( point_seq );
    }

    if(rightEdgeCount>edgeThreshold){
        for (i=0; i<rightEdgeCount; i++){
            cvSeqPush(point_seq, &cvPoint2D32f(rightEdge[i][1],rightEdge[i][2]));
        }
        cvFitLine(point_seq,CV_DIST_L2,0,0.01,0.01,right_line);
        mR = right_line[1]/right_line[0];
        CvMat* RError = cvCreateMat(1,rightEdgeCount-1,CV_32SC1);
        for(k = 0;k<rightEdgeCount-1;k++){
            cvSetReal2D(RError, 0, k, (double)(rightEdge[k][2])
                -(double)(mR* (rightEdge[k][1]-right_line[2]) + right_line[3]));
        }
        cvAvgSdv( RError, NULL, &Right_STD,NULL);
        cvClearSeq(point_seq);
    }

    //if estimate is really poor, do not update bearing
    if(Right_STD.val[0] >maxSTD && Left_STD.val[0]>maxSTD){
        return m;
    }

    //only a left edge, ignore right
    if(rightEdgeCount<=edgeThreshold){
        m = mL;
    }
    //only a right edge, ignore left
    else if(leftEdgeCount<=edgeThreshold){
        m = mR;
    }
    //both edges exist, scale each estimate by variances
    else{
        m = (Right_STD.val[0]*mL+ Left_STD.val[0]*mR)/(Right_STD.val[0]+Left_STD.val[0]);
    }

    delete left_line;
    delete right_line;
    cvReleaseMemStorage(&storage);

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
    for ( ii = ( binImage->roi == NULL ? 0 : ( unsigned int )binImage->roi->xOffset );
            ii < ( binImage->roi == NULL ? ( unsigned int )width : ( unsigned int )( binImage->roi->xOffset + binImage->roi->width ) );
            ii++ ) {
        for ( jj = ( binImage->roi == NULL ? 0 : ( unsigned int )binImage->roi->yOffset );
                jj < ( binImage->roi == NULL ? ( unsigned int )height : ( unsigned int )( binImage->roi->yOffset + binImage->roi->height ) );
                jj++ ) {
            if ( binImage->imageData[ii + jj * width] != 0 ) {
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
    if ( count > thresh )
        detected = true;

    if ( detected ) {
        centroid.x = ( int )colTotal / count;
        centroid.y = ( int )rowTotal / count;
    }
    else {
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
 * Globals:     None.
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
    IplImage *hsv_image = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *wE = cvCreateStructuringElementEx( 2, 2,
            ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
    IplConvKernel *wD = cvCreateStructuringElementEx( 3, 3,
            ( int )floor( ( 3.0 ) / 2 ), ( int )floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );

    srcImg = cvQueryFrame( cap );
    srcImg = cvQueryFrame( cap );

    if ( !srcImg ) {
        return 0;
    }

    /* Flip the source image if find dot didnt */
  	cvFlip( srcImg, srcImg );
    int center = srcImg->width/2;

    hsv_image = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize( srcImg ), IPL_DEPTH_8U, 1 );

    /* Segment the image into a binary image. */
    cvCvtColor( srcImg, hsv_image, CV_RGB2HSV );
    cvInRangeS( hsv_image, cvScalar(hL,sL,vL), cvScalar(hH,sH,vH), binImg );

    /* Perform erosion and dilation. */
    cvErode( binImg, binImg, wE );
    cvDilate( binImg, binImg, wD );

    cvConvertScale( binImg, outImg, 255.0 );

    /* Process the image. */
    *y_max = vision_get_fence_bottom( outImg, &center );
    //printf("ymax = %d\n", *y_max);
    *fence_center = center;

    /* Clear variables to free memory. */
    cvReleaseImage( &hsv_image );
    cvReleaseImage( &outImg );

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
 * Globals:     None.
 *
 *****************************************************************************/

int vision_get_fence_bottom( IplImage *inputBinImg, int *center )
{
    int y_max = 0;
    int imHeight = inputBinImg->height;
    int imWidth = inputBinImg->width;
    int minPipeWidth = 20;
    //minimum num of edge points before we declare a line
    int edgeThreshold = 2;
    int c= 0;

    int k=0; // an index

    //edge vectors, first element = x, second = y
    int leftEdge[imHeight];
    int rightEdge[imHeight];
    int i = 0; //image row index
    int j = 0; //image column index

    //initialize edge arrays, mset may be better
    for ( i = 0; i < imHeight; i++ ) {
        leftEdge[i] = 0;
        rightEdge[i] = imWidth;
    }
    for ( i = 0; i < imHeight - 1; i++ ) {
        //scan through each line of image and look for first non zero pixel
        // get the (i,j) pixel value
        while ( (cvGet2D(inputBinImg,i,j).val[0] < 1) && (j < imWidth - 2) ) {
            j++;
        }
        //if we exit before getting to end of row, edge exists
        if ( (j < imWidth) && (j > minPipeWidth) ) {
            leftEdge[k] = j;
            //countinue scanning to find bottom edge
            while ( (cvGet2D(inputBinImg,i,j).val[0] > 0) && (j < imWidth - 2) ) {
                j++;
            }
            if ( j < imWidth - 2) { //scan didnt get to end of image, bottom edge exists
                rightEdge[k] = j;
            }
            if ( rightEdge[k] - leftEdge[k] > minPipeWidth ) {
            	y_max = i;
            	printf( "FNC_BTM: ymax = %d\n", y_max );
			}
        }
        k++;
        j = 0;
    }
    //we found a fence
    if(y_max>edgeThreshold){
    	for(i = 0;i<k;i++){
        		c = c + rightEdge[i]-leftEdge[i];
		}
	}
	*center = c/k;
    return y_max;
} /* end vision_get_fence_bottom() */

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

int vision_find_dot( int *dotx,
                     int *doty,
                     int *width,
                     int *height,
                     int amt,
                     CvCapture *cap,
                     IplImage *srcImg,
                     IplImage *outImg,
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
	//int ii = 0;
	//int jj = 0;
	//int numPoints = 0;
	//CvBox2D buoy;
	//CvMemStorage *storage = NULL;
	//CvSeq *contours = 0;

	/* Initialize to impossible values. */
	*dotx = -1;
	*doty = -1;
	*width = -1;
	*height = -1;
	center.x = -1;
	center.y = -1;

	srcImg = cvQueryFrame( cap );
	if ( !srcImg ) {
		return 0;
	}

	/* Flip the source image. */
	cvFlip( srcImg, srcImg );

	/* Segment the flipped image into a binary image. */
	vision_segment_image( hL, hH, sL, sH, vL, vH, amt, srcImg, binImg, 0, 0 );
	cvConvertScale( binImg, outImg, 255.0 );

	/* Find buoy with contours and fit ellipse. */
    //contours = cvCreateSeq( CV_32FC2, sizeof(CvSeq), sizeof(CvPoint2D32f), storage );
    //for ( jj = 0; jj < outImg->width - 1; jj++ ) {
    	//for ( ii = 0; ii < outImg->height - 1; ii++ ) {
    		//if ( cvGet2D(outImg,ii,jj).val[0] == 1 ) {
    			//numPoints++;
    			//printf( "contours point ii, jj = %d, %d \n", ii, jj );
      			//cvSeqPush( contours, &cvPoint2D32f(ii,jj) );
    		//}
		//}
	//}
	//if ( numPoints > 6 ) {
    	//buoy = cvFitEllipse2( contours );
    	//*dotx = (int)buoy.center.x;
		//*doty = (int)buoy.center.y;
		//*width = (int)buoy.size.width;
		//*height = (int)buoy.size.height;
	//}
	//cvClearSeq( contours );

	/* Process the image. */
	center = vision_find_centroid( outImg, 0 );
    *dotx = center.x;
	*doty = center.y;

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

int vision_find_pipe( int *pipex,
                      double *bearing,
                      CvCapture *cap,
                      IplImage *srcImg,
                      IplImage *outImg,
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

	/* Initialize to impossible values. */
	center.x = -1;
	center.y = -1;

	srcImg = cvQueryFrame( cap );
	if ( !srcImg ) {
		return 0;
	}
	outImg = cvCreateImage( cvSize( srcImg->width, srcImg->height ), IPL_DEPTH_8U, 1 );

	/* Segment the image into a binary image. */
	vision_segment_image( hL, hH, sL, sH, vL, vH, 2, srcImg, binImg, 0 , 0 );
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
    double mL = 0;
    double mR = 0;
    double m = 0;

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
    	//fprintf(stdout,"Left: v=(%f,%f),vy/vx=%f,(x,y)=(%f,%f),",left_line[0],left_line[1],left_line[1]/left_line[0],left_line[2],left_line[3]);
    	mL = left_line[1]/left_line[0];

    	CvMat* LError = cvCreateMat(1,leftEdgeCount-1,CV_32SC1);
    	for ( k = 0; k < leftEdgeCount - 1; k++ ) {
    		//save errors in vector LError
    		cvSetReal2D(LError, 0, k, (double)(leftEdge[k][2])
    			-(double)(mL* (leftEdge[k][1]-left_line[2]) + left_line[3]));
		}

   		//calculate standard deviation of error
		cvAvgSdv( LError, NULL, &Left_STD,NULL);
    	cvClearSeq(point_seq);
	}
	if(rightEdgeCount>edgeThreshold){
    	for (i=0; i<rightEdgeCount; i++){
      		cvSeqPush(point_seq, &cvPoint2D32f(rightEdge[i][1],rightEdge[i][2]));
    	}
    	cvFitLine(point_seq,CV_DIST_L2,0,0.01,0.01,right_line);
    	//fprintf(stdout,"v=(%f,%f),vy/vx=%f,(x,y)=(%f,%f),",right_line[0],right_line[1],right_line[1]/right_line[0],right_line[2],right_line[3]);
		mR = right_line[1]/right_line[0];

		CvMat* RError = cvCreateMat(1,rightEdgeCount-1,CV_32SC1);
		for(k = 0;k<rightEdgeCount-1;k++){
			cvSetReal2D(RError, 0, k, (double)(rightEdge[k][2])
				-(double)(mR* (rightEdge[k][1]-right_line[2]) + right_line[3]));
		}
		cvAvgSdv( RError, NULL, &Right_STD,NULL);
		cvClearSeq(point_seq);
	}

/* Choose minimum variance estimator. */


	//if no edged, set bearing update to zero
	m = 0.0;

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

    //if(debug)
  //  cout << "Right std = "<< Right_STD.val[0] << endl;
  //  cout << "Left std = "<< Left_STD.val[0] << endl;

  //  cout << "m in deg = " << atan(m)*180/ M_PI << "\n";
    // cout << "b = " << b << "\n";
   // cout << "mr in deg = " << atan(mR)*180.0 /M_PI << "\n";
    //cout << "br = " << bR << "\n";
   // cout << "ml in deg = " << atan(mL)*180.0 / M_PI << "\n";
    //cout << "bl = " << bL << "\n";
    // cout << "Left error  = " << LError << "\n";
    // cout << "Right error  = " << RError << "\n";


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
	/* Get image width and height. */
	int width = binImage->width;
	int height = binImage->height;

	bool detected = FALSE;

	char *data = binImage->imageData;

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
	if ( count > thresh )
		detected = true;

	CvPoint centroid;

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

int vision_segment_image( float hL,
                          float hH,
                          float sL,
                          float sH,
                          float vL,
                          float vH,
                          int closingAmount,
                          IplImage* img,
                          IplImage* bin_img,
                          int boxWidth,
                          int boxHeight
                         )
{
	/* Get original image's width and height. */
	int width = img->width;
	int height = img->height;

	/* Initialize Filter Kernels. */
	double hue_kernel_size = 3.0;
    double hue_filter_val = 1.0/(hue_kernel_size*hue_kernel_size);
	double sat_kernel_size = 3.0;
    double sat_filter_val = 1.0/(sat_kernel_size*sat_kernel_size);

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

	/* Filter hue and sat components. */

	CvPoint anchor=cvPoint(-1,-1);

	CvMat* hue_kernel = cvCreateMat(hue_kernel_size, hue_kernel_size, CV_32FC1);
	cvSet( hue_kernel, cvScalar(hue_filter_val), NULL );

	CvMat* sat_kernel = cvCreateMat(sat_kernel_size, sat_kernel_size, CV_32FC1);
	cvSet( sat_kernel, cvScalar(sat_filter_val), NULL );

	//cvFilter2D(hue_com, hue_com,hue_kernel, anchor);
	//cvFilter2D(sat_com, sat_com,sat_kernel, anchor);

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
	cvCvtColor( masked_image, bin_img, CV_BGR2GRAY );

	 /*Do thresholding. */
	//char *data = bin_img->imageData;

	//for ( int i = 0; i < bin_img->width; i++ ) {
		//for ( int j = 0; j < bin_img->height; j++ ) {
			/* Set outside of desired box to zero. */
			//if ( ( boxHeight > 0 ) &&
			        //( ( i < ( bin_img->width - boxWidth ) / 2 ) ||
			          //( j < ( bin_img->height - boxHeight ) / 2 ) ||
			          //( i > bin_img->width - ( ( bin_img->width - boxWidth ) / 2 ) ) ||
			          //( j > bin_img->height - ( ( bin_img->height - boxHeight ) / 2 ) ) ) ) {
				//data[i + j * bin_img->widthStep] = 0;
			//}
			//else if ( data[i + j * bin_img->widthStep] != 0 ) {
				//data[i + j * bin_img->widthStep] = 1;
			//}
		//}
	//}

	/* Perform dilation. */
	cvErode( bin_img, bin_img, w );
	cvDilate( bin_img, bin_img, wCircle );

	for ( int i = 0; i < closingAmount; i++ ) {
		cvDilate( bin_img, bin_img, w );
	}

	/* Perform erosion. */
	for ( int i = 0; i < closingAmount; i++ ) {
		cvErode( bin_img, bin_img, w );
	}

	//cvErode( bin_img, bin_img, wBig );
	//cvDilate( bin_img, bin_img, wBig );
	cvDilate( bin_img, bin_img, wBig );

	cvDilate( bin_img, bin_img, wCircle );
    //cvErode( bin_img, bin_img, wSmall );
	//for( int i = 0; i < closingAmount; i++ ) {
	//  cvErode( bin_img, bin_img, w );
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

	return 1;
} /* end vision_segment_image() */

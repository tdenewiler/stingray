/*------------------------------------------------------------------------------
 *
 *  Title:        vision.c
 *
 *  Description:  General vision processing functions.
 *
 *----------------------------------------------------------------------------*/

#include "vision.h"
#include "buoy.c"

// 0 = DEFAULT (From AUVSI 2009)
// 1 = Simple Boost off HSV
#define BUOY_TECHNIQUE 1

/*------------------------------------------------------------------------------
 * int vision_find_dot()
 * Finds a circular object from a camera.
 *----------------------------------------------------------------------------*/

int vision_find_dot(int *dotx, int *doty, int angle, IplImage *srcImg, IplImage *binImg, HSV_HL *hsv )
{
    CvPoint center;
    IplImage *hsvImg = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *wS = cvCreateStructuringElementEx( 3, 3,
            (int)floor( ( 3.0 ) / 2 ), (int)floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
	int num_pix = 0;
	int touch_thresh = 150000;
    int detect_thresh = 40;

    /// Initialize to impossible values.
    center.x = -10000;
    center.y = -10000;

	/// Create intermediate images for scratch space.
    hsvImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 1 );

	/// Enhance the red channel of the source image.
	//vision_white_balance( srcImg );

    /// Convert the image from RGB to HSV color space.
    cvCvtColor( srcImg, hsvImg, CV_RGB2HSV );

	/// Equalize the histograms of each channel.
	//vision_hist_eq( hsvImg, VISION_CHANNEL3 );
		//VISION_CHANNEL1 + VISION_CHANNEL2 + VISION_CHANNEL3 );

	if ( BUOY_TECHNIQUE == 1 )
	{
		/// Buoy Boost Technique
		vision_boost_buoy( hsvImg, binImg );
	}
	else
	{
		/// Threshold all three channels using our own values.
		cvInRangeS( hsvImg, cvScalar(hsv->hL, hsv->sL, hsv->vL),
			cvScalar(hsv->hH, hsv->sH, hsv->vH), binImg );

		/// Use a median filter image to remove outliers.
		cvSmooth( binImg, outImg, CV_MEDIAN, 7, 7, 0. ,0. );
		cvMorphologyEx( binImg, binImg, wS, NULL, CV_MOP_CLOSE, 1);
	}
	
    /// Find the centroid.
    center = vision_find_centroid( binImg, 5 );
    *dotx = center.x;
    *doty = center.y;

    /// Clear variables to free memory.
    cvReleaseImage( &hsvImg );
    cvReleaseImage( &outImg );

	/// Check to see how many pixels of are detected in the image.
	num_pix = cvCountNonZero( binImg );
	//printf("VISION_FIND_DOT: num_pix = %d\n" , num_pix);
	if( num_pix > touch_thresh ) {
		return 2;
	}
    if( num_pix < detect_thresh) {
		return 0;
	}
	/// Check that the values of dotx & doty are not negative.
	if( dotx < 0 || doty < 0 ) {
		return 0;
	}

    return 1;
} /* end vision_find_dot() */

/*------------------------------------------------------------------------------
 * int vision_boost_buoy()
 * Creates a binary image using the boosting predictor.
 *----------------------------------------------------------------------------*/
int vision_boost_buoy( IplImage *srcImg, IplImage *binImg )
{
	/// Declare variables.
	uchar *srcData = ( uchar* )srcImg->imageData;
	uchar *binData = ( uchar* )binImg->imageData;
	double dst[2] = {0,0};
	double thresh = 1.0;
	int i,j,res = 0;
	double h, s, v = 0.0;
	double *hsv[3] = {&h, &s, &v};
	IplConvKernel* B;
	
	/// Smooth the image first
	//cvSmooth( srcImg, srcImg, CV_GAUSSIAN, 5, 5 );
	
	/// Loop through the first image to fill the left part of the new image.
	for ( i = 0; i < srcImg->height; i++ ) {
		for ( j = 0; j < srcImg->width; j++ ) {
			*hsv[0] = srcData[i * srcImg->widthStep + j * srcImg->nChannels + 0];
			*hsv[1] = srcData[i * srcImg->widthStep + j * srcImg->nChannels + 1];
        	*hsv[2] = srcData[i * srcImg->widthStep + j * srcImg->nChannels + 2];
        	
        	/// Predict on this point
        	if ( predict( (void**)hsv, dst ) )
        	{
        		if ( dst[1] > thresh )
        		{
        			res = 0xff;
				}
				else
				{
					res = 0;
				}
			}
        	
        	binData[i * binImg->widthStep + j * binImg->nChannels + 0] = res;
        	binData[i * binImg->widthStep + j * binImg->nChannels + 1] = res;
        	binData[i * binImg->widthStep + j * binImg->nChannels + 2] = res;
		}
	}
	
	/// Create Erosion/Dilation Kernel
	B = cvCreateStructuringElementEx( 3, 3, 1, 1, CV_SHAPE_RECT );
	
	/// First use closing to eliminate noise-driven segments
	//cvMorphologyEx( binImg, binImg, NULL, B, CV_MOP_CLOSE, 2 );
	
	/// Second use opening to clean up remaining regions
	cvMorphologyEx( binImg, binImg, NULL, B, CV_MOP_OPEN, 1 );
	
	cvReleaseStructuringElement( &B );
	
    return 1;
} /* end vision_boost_buoy() */
	

/*------------------------------------------------------------------------------
 * int vision_find_pipe()
 * Finds a pipe object from a camera.
 *----------------------------------------------------------------------------*/

int vision_find_pipe(int *pipex, int *pipey, double *bearing, IplImage *srcImg, IplImage *binImg, HSV_HL *hsv)
{
    CvPoint center;
    IplImage *hsvImg = NULL;
    IplImage *outImg = NULL;
    //IplConvKernel *wS = cvCreateStructuringElementEx( 2, 2,
    //        (int)floor( ( 2.0 ) / 2 ), (int)floor( ( 2.0 ) / 2 ), CV_SHAPE_RECT );
	int detect_thresh = 15000;
	int num_pix = 0;

    /// Initialize to impossible values.
    center.x = -10000;
    center.y = -10000;

	/// Create intermediate images for scratch space.
    hsvImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 1 );

	/// Enhance the red channel of the source image.
	//vision_white_balance( srcImg );

    /// Flip the source image.
    cvFlip( srcImg, srcImg );

    /// Segment the flipped image into a binary image.
    cvCvtColor( srcImg, hsvImg, CV_RGB2HSV );

	/// Equalize the histograms of each channel.
	//vision_hist_eq( hsvImg,
	//	VISION_CHANNEL1 + VISION_CHANNEL2 + VISION_CHANNEL3 );

	/// Threshold all three channels using our own values.
    cvInRangeS( hsvImg, cvScalar(hsv->hL, hsv->sL, hsv->vL),
		cvScalar(hsv->hH, hsv->sH, hsv->vH), binImg );

	/// Use a median filter to remove outliers.
	cvSmooth( binImg, outImg, CV_MEDIAN, 5, 5, 0. ,0. );
	//cvDilate( outImg, binImg, 0, 2 );
	//cvErode( outImg, binImg, 0, 4 );
	//cvDilate( outImg, binImg, 0, 2 );

    /// Process the image to get the bearing and centroid.
    *bearing = vision_get_bearing( outImg );
    center = vision_find_centroid( outImg, 0 );
    *pipex = center.x;
    *pipey = center.y;

    /// Clear variables to free memory.
    cvReleaseImage( &hsvImg );
    cvReleaseImage( &outImg );

	/// Check to see how many pixels are detected in the image.
	num_pix = cvCountNonZero( binImg );
	//printf("VISION_FIND_PIPE: num_pix = %d\n" , num_pix);
	if( num_pix < detect_thresh ) {
		return 0;
	}

    return 1;
} /* end vision_find_pipe() */


/*------------------------------------------------------------------------------
 * float vision_get_bearing()
 * Fits edges of pipe to a line and calculates its angle.
 *----------------------------------------------------------------------------*/

double vision_get_bearing(IplImage *inputBinImg)
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

    /// Slope (m) and b for left, right and combined estimate.
    double mL = 0.0;
    double mR = 0.0;
    double m = 0.0;

    /// Edge vectors, first element = x, second = y.
    int leftEdge[imHeight][2];
    int rightEdge[imHeight][2];
    int i = 0;
    int j = 0;

    /// Initialize edge arrays, mset may be better.
    for( i = 0; i < imHeight; i++ ) {
        leftEdge[i][0] = 0;
        leftEdge[i][1] = 0;
        rightEdge[i][0] = 0;
        rightEdge[i][1] = 0;
    }
    for( i = 0; i < imHeight - 10; i++ ) {
        /// Scan through each line of image and look for first non zero pixel
        /// then get the (i,j) pixel value.
        while( (cvGet2D(inputBinImg, i, j).val[0] < 1) && (j<imWidth - 1) ) {
            j++;
        }
        /// If we exit before getting to end of row, edge exists.
        if( (j < imWidth - 1) && (j > 0) ) {
            leftEdge[leftEdgeCount][0] = i; //FLIP i, j, here worksish
            leftEdge[leftEdgeCount][1] = j;
            leftEdgeCount++;
            /// Continue scanning to find right edge.
            while( (cvGet2D(inputBinImg, i, j).val[0] > 0) && (j < imWidth - 1) ) {
                j++;
            }
            if( j < imWidth - 2 ) { /// Scan didn't get to end of image so right edge exists.
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

    /// Begin fitline.
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
            /// Save errors in vector LError.
            cvSetReal2D( LError, 0, k, (double)(leftEdge[k][2])
                - (double)(( left_line[1] / left_line[0] ) * (leftEdge[k][0] - left_line[1]) + left_line[3]) );
        }
        /// Calculate standard deviation of error.
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

    /// If estimate is really poor, do not update bearing.
    if( (Right_STD.val[0] > maxSTD) && (Left_STD.val[0] > maxSTD) ) {
		delete left_line;
		delete right_line;
		cvReleaseMemStorage( &storage );
        return 0.0;
    }

    /// Only a left edge, ignore right.
    if( rightEdgeCount <= edgeThreshold ) {
        m = mL;
    }
    /// Only a right edge, ignore left.
    else if( leftEdgeCount <= edgeThreshold ) {
        m = mR;
    }
    /// Both edges exist, scale each estimate by variances.
    else {
        m = (Right_STD.val[0] * mL + Left_STD.val[0] * mR) /
        	(Right_STD.val[0] + Left_STD.val[0]);
    }

    delete left_line;
    delete right_line;
    cvReleaseMemStorage( &storage );

    return m;
} /* end vision_get_bearing() */


/*------------------------------------------------------------------------------
 * CvPoint vision_find_centroid()
 * Finds the centroid of the pixels in the image.
 *----------------------------------------------------------------------------*/

CvPoint vision_find_centroid(IplImage *binImage, int thresh)
{
    /// Return value.
    CvPoint centroid;

    /// Get image width and height.
    int width = binImage->width;
    int height = binImage->height;

    /// Totals.
    unsigned int rowTotal = 0;
    unsigned int colTotal = 0;
    int count = 0;

    /// Counters.
    unsigned int ii = 0;
    unsigned int jj = 0;
    bool detected = FALSE;

    /// Find centroid.
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

    /// Check if an object is detected.
    if( count > thresh )
        detected = true;

    /// If the centroid was detected convert it.
    if( detected ) {
        centroid.x = (int)colTotal / count;
        centroid.y = (int)rowTotal / count;
    }
    else { ///* Send back negatives if we do not have a positive detection.
        centroid.x = -1;
        centroid.y = -1;
    }

    return centroid;
} /* end vision_find_centroid() */


/*------------------------------------------------------------------------------
 * int vision_find_fence()
 * Finds a pipe object from a camera.
 *----------------------------------------------------------------------------*/

int vision_find_fence(int *fence_center, int *y_max, IplImage *srcImg, IplImage *binImg, HSV_HL *hsv)
{
	/// Declare variables.
	//int sum_x = 0;
    //int ii = 0;
    //int jj = 0;
    //int kk = 0;
    CvPoint center;
	IplImage *hsvImg = NULL;
    IplImage *outImg = NULL;
    //IplConvKernel *wS = cvCreateStructuringElementEx( 2, 2,
    //        (int)floor( ( 2.0 ) / 2 ), (int)floor( ( 2.0 ) / 2 ), CV_SHAPE_RECT );
    CvSize sz = cvSize( srcImg->width & -2, srcImg->height & -2 );
 	int num_pix = 0;
	int detect_thresh = 1;

	/// Initialize center to impossible value.
	center.x = -1000;
	center.y = -1000;

	/// Create intermediate images for scratch space.
    hsvImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 1 );

	/// Enhance the red channel of the source image.
	//vision_white_balance( srcImg );

    /// Segment the flipped image into a binary image.
    cvCvtColor( srcImg, hsvImg, CV_RGB2HSV );

	/// Equalize the histograms of each channel.
	//vision_hist_eq( hsvImg,
	//	VISION_CHANNEL1 + VISION_CHANNEL2 + VISION_CHANNEL3 );

	/// Threshold all three channels using our own values.
    cvInRangeS( hsvImg, cvScalar(hsv->hL, hsv->sL, hsv->vL),
		cvScalar(hsv->hH, hsv->sH, hsv->vH), binImg );

	/// Median filter image to remove outliers.
	cvSmooth( binImg, outImg, CV_MEDIAN, 5, 11, 0. ,0. );
	//cvErode( outImg, binImg, wS );

	/// Check to see how many pixels are detected in the image.
	num_pix = cvCountNonZero( binImg );
	//printf("VISION_FIND_FENCE: num_pix = %d\n" , num_pix);

	/// Find the centroid.
    center = vision_find_centroid( binImg, 5 );
    *fence_center = floor(center.x);
    *y_max = floor(center.y);

    /// Compute centroid.
    /*for( ii = 0; ii < binImg->height; ii++ ) {
        for( jj = 0; jj < binImg->width; jj++ ) {
        	if( cvGet2D(binImg, ii, jj).val[0] != 0 ) {
        		sum_x += jj;
        		kk++;
			}
		}
	}
	/// Make sure we don't divide by 0 here.
	if( kk == 0 ) {
		kk = 1;
	}
    *fence_center = floor(sum_x / kk);*/

    /// Clear variables to free memory.
    cvReleaseImage( &hsvImg );
    cvReleaseImage( &outImg );

	if( num_pix > detect_thresh ) {
		/// We have found enough pixels to qualify as detecting the fence.
		return 1;
	}

    return 0;
} /* end vision_find_fence() */


/*------------------------------------------------------------------------------
 * float vision_get_fence_bottom()
 * Fits edges of pipe to a line and calculates its angle.
 *----------------------------------------------------------------------------*/

int vision_get_fence_bottom(IplImage *inputBinImg)
{
    int y_max = 0;
    int imHeight = inputBinImg->height;
    int imWidth = inputBinImg->width;
    int minPipeWidth = 2;
    int edgeThreshold = 2;

    /// Edge vectors, first element = x, second = y.
    int leftEdge[imHeight];
    int rightEdge[imHeight];
    int i = 0; /// rows
    int j = 0; /// columns
    int k = 0;
    /// Initialize edge arrays, memset may be better.
    memset( &leftEdge, 0, sizeof(leftEdge) );
    memset( &rightEdge, 0, sizeof(rightEdge) );
    for( i = 0; i < imHeight - 1; i++ ) {
        /// Scan through each line of image and look for first non zero pixel
        /// then get the (i,j) pixel value. */
        while( (cvGet2D(inputBinImg, i, j).val[0] < 1) && (j < imWidth - 2) ) {
            j++;
        }
        /// If we exit before getting to end of row, edge exists.
        if( (j < imWidth) && (j > minPipeWidth) ) {
            leftEdge[k] = j;
            /// Countinue scanning to find bottom edge.
            while( (cvGet2D(inputBinImg, i, j).val[0] > 0) &&
				(j < imWidth - 2) ) {
                j++;
            }
            if( j < imWidth - 2) {
            	/// Scan didn't get to end of image, bottom edge exists.
                rightEdge[k] = j;
            }
            if( rightEdge[k] - leftEdge[k] > minPipeWidth ) {
            	y_max = i;
			}
        }
        k++;
        j = 0;
    }

    /// We found a fence.
    if( y_max > edgeThreshold ) {
		return y_max;
    }
	return -1;
} /* end vision_get_fence_bottom() */


/*------------------------------------------------------------------------------
 * int vision_find_boxes()
 * Finds rectangles in an image from a camera.
 *----------------------------------------------------------------------------*/

int vision_find_boxes(IplImage *srcImg, CvSeq *result, CvSeq *squares, int task, float *angle)
{
	/// Declare variables.
	IplImage *img = NULL;
	CvMemStorage *storage = 0;
	int status = -1;

	/// Initialize variables.
	storage = cvCreateMemStorage( 0 );

	/// Clone the source image so that we have an image we can write over. The
	/// source image needs to be kept clean so that we can display it later.
	img = cvCloneImage( srcImg );

	/// Smooth the image with a Gaussian filter. Add the channels to smooth to the second argument.
	vision_smooth( img, VISION_CHANNEL3 );

	/// Equalize the histograms of each channel.
	vision_hist_eq( img,
		VISION_CHANNEL1 + VISION_CHANNEL2 + VISION_CHANNEL3 );

	/// Look for boxes in the image.
	status = vision_find_squares( img, storage, result, squares, task, angle );

    /// Clear memory storage and reset free space position.
    cvReleaseImage( &img );
    cvClearMemStorage( storage );
    cvReleaseMemStorage( &storage );

    return status;
} /* end vision_find_boxes() */


/*------------------------------------------------------------------------------
 * double vision_angle()
 * Finds the cosine of the angle between two vectors.
 *----------------------------------------------------------------------------*/

double vision_angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0, IplImage *img, int task, float *angle)
{
	/// Calculate length between points.
    double dx1 = pt1->x - pt0->x;
    double dy1 = pt1->y - pt0->y;
    double dx2 = pt2->x - pt0->x;
    double dy2 = pt2->y - pt0->y;
	double dx = sqrt(dx1 * dx1 + dy1 * dy1);
	double dy = sqrt(dx2 * dx2 + dy2 * dy2);
	double borderx = img->width * VISION_BORDER;
	double bordery = img->height * VISION_BORDER;
	double ar = 0;
	double arinv = 0;
	int ar_passed = FALSE;

	/// The find box code detects a box at the image boundary. To remove this we
	/// are going to exclude boxes that are within a small band around the edge of the image.
	if( (dx > borderx) || (dy > bordery) ) {
		return 100000.;
	}

	/// Check that the lengths of the found box are within the bounds of the
	/// aspect ratios for the individual targets. They can be off by a certain
	/// percentage set by VISION_AR_THRESH to accomodate looking at the objects
	/// from an angle. */
	if( dy == 0 ) {
		ar = 0.;
	}
	else {
		ar = dx / dy;
	}
	if( dx == 0 ) {
		arinv = 0.;
	}
	else {
		arinv = dy / dx;
	}
	switch( task ) {
	case VISION_SUITCASE:
		/// Check the aspect ratio.
		if( ar > VISION_AR_SUITCASE * (1 - VISION_AR_THRESH) &&
			ar < VISION_AR_SUITCASE * (1 + VISION_AR_THRESH) ) {
			/// Found a valid box.
			ar_passed = TRUE;
		}
		/// Check to see if we are looking at the box rotated by 90 degrees.
		if( !ar_passed ) {
			if( arinv < VISION_AR_SUITCASE * (1 - VISION_AR_THRESH) ||
				arinv > VISION_AR_SUITCASE * (1 + VISION_AR_THRESH) ) {
				return 100000.;
			}
		}
		/// Else we have found a box with a valid aspect ratio.
		else {
			break;
		}
	case VISION_PIPE:
		/// Check the aspect ratio.
		if( ar > VISION_AR_PIPE * (1 - VISION_AR_THRESH) &&
			ar < VISION_AR_PIPE * (1 + VISION_AR_THRESH) ) {
			/// Found a valid box.
			ar_passed = TRUE;

			/// Find the angle of the pipe so that we follow the longer edge.
			if( dy1 > dy2 ) {
				*angle = atan2( dy1, dx2 ) * 180 / M_PI;
			}
			else {
				*angle = atan2( dy2, dx2 ) * 180 / M_PI;
			}
			//printf("VISION_ANGLE: %f\n", *angle);
		}
		/// Check to see if we are looking at the box rotated by 90 degrees.
		if( !ar_passed ) {
			if( arinv < VISION_AR_PIPE * (1 - VISION_AR_THRESH) ||
				arinv > VISION_AR_PIPE * (1 + VISION_AR_THRESH) ) {
				return 100000.;
			}
			/// Find the angle of the pipe so that we follow the longer edge.
			if( dy1 > dy2 ) {
				*angle = atan2( dy1, dx2 ) * 180 / M_PI;
			}
			else {
				*angle = atan2( dy2, dx2 ) * 180 / M_PI;
			}
			//printf("VISION_ANGLE: %f\n", *angle);
		}
		/// Else we have found a box with a valid aspect ratio.
		else {
			break;
		}
	case VISION_BOX:
		/// Check the aspect ratio.
		if( ar > VISION_AR_BOX * (1 - VISION_AR_THRESH) &&
			ar < VISION_AR_BOX * (1 + VISION_AR_THRESH) ) {
			/// Found a valid box.
			ar_passed = TRUE;

			/// Find the angle of the pipe so that we follow the longer edge.
			if( dy1 > dy2 ) {
				*angle = atan2( dy1, dx2 );
			}
			else {
				*angle = atan2( dy2, dx2 );
			}
			//printf("VISION_ANGLE: %f\n", *angle);
		}
		/// Check to see if we are looking at the box rotated by 90 degrees.
		if( !ar_passed ) {
			if( arinv < VISION_AR_BOX * (1 - VISION_AR_THRESH) ||
				arinv > VISION_AR_BOX * (1 + VISION_AR_THRESH) ) {
				return 100000.;
			}
			/// Find the angle of the pipe so that we follow the longer edge.
			if( dy1 > dy2 ) {
				*angle = atan2( dy1, dx2 );
			}
			else {
				*angle = atan2( dy2, dx2 );
			}
			//printf("VISION_ANGLE: %f\n", *angle);
		}
		/// Else we have found a box with a valid aspect ratio.
		else {
			break;
		}
	}

	/// Return the cosine between the two points.
    return( dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10 );
} /* end vision_angle() */


/*------------------------------------------------------------------------------
 * int vision_find_squares()
 * Finds all the rectangles in an image.
 *----------------------------------------------------------------------------*/

int vision_find_squares(IplImage *img, CvMemStorage *storage, CvSeq *box_centers,
	CvSeq *squares, int task, float *angle)
{
	/// Declare variables.
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

    /// Select the maximum ROI in the image with the width and height divisible by 2.
    cvSetImageROI( timg, cvRect( 0, 0, sz.width, sz.height ));

    /// Down-scale and upscale the image to filter out the noise.
    cvPyrDown( timg, pyr, 7 );
    cvPyrUp( pyr, timg, 7 );

	/// Create a grayscale image.
    tgray = cvCreateImage( sz, 8, 1 );

    /// Find squares in every color plane of the image.
    for( c = 0; c < 3; c++ ) {
        /// Extract the c-th color plane.
        cvSetImageCOI( timg, c + 1 );
        cvCopy( timg, tgray, 0 );

        /// Try several threshold levels.
        for( l = 0; l < N; l++ ) {
            /* !!! HACK !!!: Use Canny instead of zero threshold level. Canny
			 * helps to catch squares with gradient shading. */
            if( l == 0 ) {
                /// Apply Canny and use the upper threshold and set the lower to
				/// 0 (which forces edges merging). */
                cvCanny( tgray, gray, 0, thresh, 5 );
                /// Dilate Canny output to remove potential holes between edge segments.
                cvDilate( gray, gray, 0, 1 );
            }
            else {
                /// Apply threshold if l != 0.
                cvThreshold( tgray, gray, (l + 1) * 255 / N, 255, CV_THRESH_BINARY );
            }

            /// Find contours and store them all as a list.
            cvFindContours( gray, storage, &contours, sizeof(CvContour),
                CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );

            /// Test each contour to see if it is a rectangle.
            while( contours ) {
                /// Approximate contour with accuracy proportional to the contour perimeter.
                result = cvApproxPoly( contours, sizeof(CvContour), storage,
                    CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
                /// Square contours should have 4 vertices after approximation
                /// relatively large area (to filter out noisy contours) and be convex.
                /// Note: absolute value of an area is used because area may be
				/// positive or negative in accordance with the contour orientation. */
                if( result->total == 4 && fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 1000 &&
                    cvCheckContourConvexity(result) ) {
                    s = 0;
                    for( i = 0; i < 5; i++ ) {
                        /// Find the minimum angle between joint edges (max of cos).
                        if( i >= 2 ) {
                            t = fabs(vision_angle(
								(CvPoint*)cvGetSeqElem( result, i ),
								(CvPoint*)cvGetSeqElem( result, i-2 ),
								(CvPoint*)cvGetSeqElem( result, i-1 ),
								gray, task, angle ));
                            s = s > t ? s : t;
                        }
                    }

                    /// If cosines of all angles are small (all angles are ~90 degree)
					/// then write rectangle vertices to resultant sequence. */
                    if( s < VISION_MIN_ANGLE ) {
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
				/// Look at the next contour to see if it is a rectangle.
				contours = contours->h_next;
            }
        }
    }

    /// Release all temporary images to free up memory.
    cvReleaseImage( &gray );
    cvReleaseImage( &pyr );
    cvReleaseImage( &tgray );
    cvReleaseImage( &timg );

    return status;
} /* end vision_find_squares() */


/*------------------------------------------------------------------------------
 * int vision_suitcase()
 * Finds the centroid of the suitcase structure in an image.
 *----------------------------------------------------------------------------*/

int vision_suitcase(IplImage *srcImg, CvSeq *result, CvSeq *squares )
{
	/// Declare variables.
	IplImage *img = NULL;
	CvMemStorage *storage = 0;
	int status = -1;
	float angle = 0;

	/// Initialize variables.
	storage = cvCreateMemStorage( 0 );

	/// Clone the source image so that we have an image we can write over. The
	/// source image needs to be kept clean so that we can display it later.
	img = cvCloneImage( srcImg );
    status = vision_find_squares( img, storage, result, squares, VISION_SUITCASE, &angle );

    /// Clear memory storage and reset free space position.
    cvReleaseImage( &img );
    cvReleaseMemStorage( &storage );

    return status;
} /* end vision_suitcase() */


/*------------------------------------------------------------------------------
 * int vision_find_circle()
 * Finds a circle in an image.
 *----------------------------------------------------------------------------*/

int vision_find_circle(IplImage *srcImg, CvSeq *circles )
{
	/// Declare variables.
	IplImage *gray = NULL;
	CvMemStorage *storage = 0;

	/// Initialize variables.
	storage = cvCreateMemStorage( 0 );

	/// Convert captured image to grayscale.
    gray = cvCreateImage( cvGetSize(srcImg), 8, 1 );
	cvCvtColor( srcImg, gray, CV_BGR2GRAY );
	cvSmooth( gray, gray, CV_GAUSSIAN, 9, 9 );

	/// Look for circles.
	circles = cvHoughCircles( gray, storage, CV_HOUGH_GRADIENT, 2,
		gray->height / 4, 200, 100 );

    /// Clear memory storage and reset free space position.
    cvReleaseImage( &gray );
    cvReleaseMemStorage( &storage );

    return circles->total;
} /* end vision_find_circle() */


/*------------------------------------------------------------------------------
 * int vision_window_filter()
 * Finds the centroid in a binary image using the sum of a window.
 *----------------------------------------------------------------------------*/

int vision_window_filter(IplImage *img, IplImage *bin_img, CvPoint *center, int sizex, int sizey )
{
	/// Declare variables.
	int rows = 0;
	int cols = 0;
	double val = 0.;
	IplImage *timg = NULL;
	double retval = 0.;

	/// Create temporary image.
    timg = cvCreateImage( cvGetSize(img), IPL_DEPTH_8U, 1 );

	/// Look for the maximum value in the filtered image.
	for( cols = 0; cols < img->height - (sizey + 1); cols++ ) {
		for( rows = 0; rows < img->width - (sizex + 1); rows++ ) {
			/// Search through the window to find the sum of the pixels.
			val = cvGet2D( timg, cols, rows ).val[0];
			if( val > retval ) {
				/// We have found a new max correlation.
				retval = val;
				center->x = rows;
				center->y = cols;
			}
		}
	}

	/// Release elements to free memory.
	cvReleaseImage( &timg );

	if( retval > 0. ) {
		return 1;
	}
	else {
		return 0;
	}
} /* end vision_window_filter() */


/*------------------------------------------------------------------------------
 * void vision_smooth()
 * Performs Gaussian smoothing on an image.
 *----------------------------------------------------------------------------*/

void vision_smooth(IplImage *img, int mode)
{
	/// Declare variables.
    IplImage *clone = NULL;
    IplImage *tgray1 = NULL;
    IplImage *tgray2 = NULL;
    IplImage *tgray3 = NULL;
    CvSize sz = cvSize( img->width & -2, img->height & -2 );
    int smooth_size = 9;

	/// Clone the original image.
	clone = cvCloneImage( img );

	/// Create three separate grayscale images, one for each channel.
    tgray1 = cvCreateImage( sz, 8, 1 );
	tgray2 = cvCreateImage( sz, 8, 1 );
	tgray3 = cvCreateImage( sz, 8, 1 );

    /// Filter each plane with a Gaussian and merge back to original image.
    cvSetImageCOI( clone, 1 );
    cvCopy( clone, tgray1, 0 );
	if( mode == 1 || mode == 3 || mode == 5 || mode == 7 ) {
		cvSmooth( tgray1, tgray1, CV_GAUSSIAN, smooth_size, smooth_size );
	}

	cvSetImageCOI( clone, 2 );
    cvCopy( clone, tgray2, 0 );
	if( mode == 2 || mode == 3 || mode == 6 || mode == 7 ) {
		//cvSmooth( tgray2, tgray2, CV_GAUSSIAN, smooth_size, smooth_size );
	}

	cvSetImageCOI( clone, 3 );
    cvCopy( clone, tgray3, 0 );
	if( mode == 4 || mode == 5 || mode == 6 || mode == 7 ) {
		cvSmooth( tgray3, tgray3, CV_GAUSSIAN, smooth_size, smooth_size );
	}

	cvMerge( tgray1, tgray2, tgray3, NULL, img );

    /// Clear variables to free memory.
    cvReleaseImage( &clone );
    cvReleaseImage( &tgray1 );
    cvReleaseImage( &tgray2 );
    cvReleaseImage( &tgray3 );
} /* end vision_smooth() */


/*------------------------------------------------------------------------------
 * void vision_hist_eq()
 * Performs histogram equalization on an image.
 *----------------------------------------------------------------------------*/

void vision_hist_eq(IplImage *img, int mode)
{
	/// Declare variables.
    IplImage *clone = NULL;
    IplImage *tgray1 = NULL;
    IplImage *tgray2 = NULL;
    IplImage *tgray3 = NULL;
    IplImage *tgray1eq = NULL;
    IplImage *tgray2eq = NULL;
    IplImage *tgray3eq = NULL;
    CvSize sz = cvSize( img->width & -2, img->height & -2 );

	/// Clone the original image.
	clone = cvCloneImage( img );

	/// Create three separate grayscale images, one for each channel.
    tgray1 = cvCreateImage( sz, 8, 1 );
	tgray2 = cvCreateImage( sz, 8, 1 );
	tgray3 = cvCreateImage( sz, 8, 1 );
    tgray1eq = cvCreateImage( sz, 8, 1 );
	tgray2eq = cvCreateImage( sz, 8, 1 );
	tgray3eq = cvCreateImage( sz, 8, 1 );

    /// Find squares in every color plane of the image. Filter each plane with a
	/// Gaussian and then merge back to original image.
    cvSetImageCOI( clone, 1 );
    cvCopy( clone, tgray1, 0 );

	cvSetImageCOI( clone, 2 );
    cvCopy( clone, tgray2, 0 );

	cvSetImageCOI( clone, 3 );
    cvCopy( clone, tgray3, 0 );

	/// Equalize the histograms of each channel.
	if( mode == 1 || mode == 3 || mode == 5 || mode == 7 ) {
		cvEqualizeHist( tgray1, tgray1eq );
	}
	if( mode == 2 || mode == 3 || mode == 6 || mode == 7 ) {
		cvEqualizeHist( tgray2, tgray2eq );
	}
	if( mode == 4 || mode == 5 || mode == 6 || mode == 7 ) {
		cvEqualizeHist( tgray3, tgray3eq );
	}

	cvMerge( tgray1eq, tgray2eq, tgray3eq, NULL, img );

    /// Clear variables to free memory.
    cvReleaseImage( &clone );
    cvReleaseImage( &tgray1 );
    cvReleaseImage( &tgray2 );
    cvReleaseImage( &tgray3 );
	cvReleaseImage( &tgray1eq );
	cvReleaseImage( &tgray2eq );
	cvReleaseImage( &tgray3eq );
} /* end vision_hist_eq() */


/*------------------------------------------------------------------------------
 * void vision_saturate()
 * Saturates channels of an image.
 *----------------------------------------------------------------------------*/

void vision_saturate(IplImage *img)
{
	/// Declare variables.
    IplImage *clone = NULL;
    IplImage *tgray1 = NULL;
    IplImage *tgray2 = NULL;
    IplImage *tgray3 = NULL;
    CvSize sz = cvSize( img->width & -2, img->height & -2 );
	int ii = 0;
	int jj = 0;
	double power = 0.99;
	CvScalar val;

	/// Clone the original image.
	clone = cvCloneImage( img );

	/// Create three separate grayscale images, one for each channel.
    tgray1 = cvCreateImage( sz, 8, 1 );
	tgray2 = cvCreateImage( sz, 8, 1 );
	tgray3 = cvCreateImage( sz, 8, 1 );

	/// Split the three channel image into three grayscale images using set channel of interest.
    cvSetImageCOI( clone, 1 );
	cvCopy( clone, tgray1, 0 );
	/// Go through the image and set the value of each pixel to zero.
	for( ii = 0; ii < img->height; ii++ ) {
		for( jj = 0; jj < img->width; jj++ ) {
			val = cvGet2D( img, ii, jj );
			val.val[0] = 0;
			val.val[1] = 0;
			val.val[2] = 0;
			val.val[3] = 0;
			cvSet2D( img, ii, jj, val );
		}
	}

	cvSetImageCOI( clone, 2 );
	cvCopy( clone, tgray2, 0 );

	cvSetImageCOI( clone, 3 );
	cvCopy( clone, tgray3, 0 );
	/// Go through the image and raise the value of the pixel to a power.
	for( ii = 0; ii < img->height; ii++ ) {
		for( jj = 0; jj < img->width; jj++ ) {
			val = cvGet2D( img, ii, jj );
			val.val[0] = pow( val.val[0], power );
			cvSet2D( img, ii, jj, val );
		}
    }

	/// Merge the grayscale images back to a three channel image.
	cvMerge( tgray1, tgray2, tgray3, NULL, img );

    /// Clear variables to free memory.
    cvReleaseImage( &clone );
    cvReleaseImage( &tgray1 );
    cvReleaseImage( &tgray2 );
    cvReleaseImage( &tgray3 );
} /* end vision_saturate() */


/*------------------------------------------------------------------------------
 * void vision_white_balance()
 * Balance the color of an image.
 *----------------------------------------------------------------------------*/

void vision_white_balance(IplImage *img)
{
	/// Declare variables.
	int ii = 0;
	int jj = 0;
	uchar *temp_ptr;
	//double scale = 0.65;
	double rscale = 255. / 255.;
	double bscale = 255. / 245.;
	double gscale = 255. / 255.;

	/// For each channel in the original image modify the RGB values.
	for( ii = 0; ii < img->height; ii++ ) {
		for( jj = 0; jj < img->width; jj++ ) {
			temp_ptr = &((uchar *)(img->imageData + img->widthStep * ii))[jj * 3];
			temp_ptr[0] *= bscale;
			temp_ptr[1] *= gscale;
			temp_ptr[2] *= rscale;
		}
	}
} /* end vision_white_balance() */


/*------------------------------------------------------------------------------
 * void vision_threshold()
 * Thresholds an image.
 *----------------------------------------------------------------------------*/

void vision_threshold(IplImage *img, IplImage *bin_img, int type, int size, double thresh)
{
	/// Declare variables.
	int ii = 0;
	int jj = 0;
	double maxval = 255.;
	CvMat *kernel = NULL;

	/// Create a matrix to replace my ASCII art of 1's and 0's. Might be able to find in old revision.
	kernel = cvCreateMat( size, size, CV_32FC1 );
	for( ii = 0; ii < size; ii++ ) {
		for( jj = 0; jj < size; jj++ ) {
			if( pow(ii - ceil(size/2), 2) + pow(jj - ceil(size/2), 2) < floor(size/2) ) {
				cvSet2D( kernel, ii, jj, cvScalar(1./pow((double)size, 2) ) );
			}
			else {
				cvSet2D( kernel, ii, jj, cvScalar(0.) );
							}
		}
	}
    /// Filter the image using convolution.
    cvFilter2D( img, bin_img, kernel, cvPoint(floor(size/2),floor(size/2)) );
	img = cvCloneImage(bin_img);

	/// Threshold the image based on type of threshold -- normal else adaptive.
	if( type == VISION_BINARY ) {
		cvThreshold( img, bin_img, thresh, maxval, CV_THRESH_BINARY );
	}
	else {
		cvAdaptiveThreshold( img, bin_img, maxval, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 3, 5 );
	}

} /* end vision_threshold() */


/*------------------------------------------------------------------------------
 * void vision_rgb_ratio_filter()
 * Turns any pixel black that doesn't match the input RGB thresholds.
 *----------------------------------------------------------------------------*/

void vision_rgb_ratio_filter(IplImage *img , double * rgb_thresh)
{
	/// Declare variables.
	int ii = 0;
	int jj = 0;
	enum rgb_index { b , g , r };
	unsigned char *pixel;
	double rg = 0.0;
	double rb = 0.0;
	double gb = 0.0;

	/// For each channel in the original image modify the RGB values.
	for( ii = 0; ii < img->height; ii++ ) {
		for( jj = 0; jj < img->width; jj++ ) {
			pixel = &((uchar *)(img->imageData + img->widthStep * ii))[jj * 3];

			/// Compute ratios. Check for div/0.
			if( pixel[g] != 0 ) {
				rg = (double)pixel[r] / pixel[g];
			}
			else {
				rg = 0;
			}
			if( pixel[b] != 0 ) {
				rb = (double)pixel[r] / pixel[b];
				gb = (double)pixel[g] / pixel[b];
			}
			else {
				rb = 0;
				gb = 0;
			}

			/// If the pixel is outside the threshold, turn it to black.
			if( !( rg >= rgb_thresh[0] && rg <= rgb_thresh[1] &&
				   rb >= rgb_thresh[2] && rb <= rgb_thresh[3] &&
				   gb >= rgb_thresh[4] && gb <= rgb_thresh[5] ) ) {

				pixel[b] =
				pixel[g] =
				pixel[r] = 0;
			}
			else {
				/// If the product of the ratios is too small when the sum
				/// is below a certain threshold, throw away pixel. */
				if( (rg*rb*gb) < rgb_thresh[6] &&
					 (pixel[r]+pixel[g]+pixel[b]) <= rgb_thresh[7] ) {
				  	pixel[b] =
					pixel[g] =
					pixel[r] = 0;
				}
			}
		}
	}
} /* end vision_rgb_ratio_filter() */


/*------------------------------------------------------------------------------
 * void vision_rgb_sum_filter()
 * Turns any pixel black that doesn't match the input RGB thresholds.
 *----------------------------------------------------------------------------*/

void vision_rgb_sum_filter(IplImage *img , short * rgb_sum)
{
	/// Declare variables.
	int ii = 0;
	int jj = 0;
	enum rgb_index { b , g , r };
	unsigned char *pixel;
	short sum;

	/// For each channel in the original image modify the RGB values.
	for( ii = 0; ii < img->height; ii++ ) {
		for( jj = 0; jj < img->width; jj++ ) {
			pixel = &((uchar *)(img->imageData + img->widthStep * ii))[jj * 3];
			sum = pixel[r] + pixel[g] + pixel[b];

			/// If the pixel is outside the threshold, turn it to black.
			if( sum < rgb_sum[0] || sum > rgb_sum[1] ) {
				pixel[b] = 0;
				pixel[g] = 0;
				pixel[r] = 0;
			}
		}
	}
} /* end vision_rgb_ratio_filter() */


/*------------------------------------------------------------------------------
 * int vision_find_gate()
 * Finds a circular object from a camera.
 *----------------------------------------------------------------------------*/

int vision_find_gate(int *dotx, int *doty, int angle, IplImage *srcImg, IplImage *binImg, HSV_HL *hsv)
{
    CvPoint center;
    IplImage *hsvImg = NULL;
    IplImage *outImg = NULL;
    IplConvKernel *wS = cvCreateStructuringElementEx( 3, 3,
            (int)floor( ( 3.0 ) / 2 ), (int)floor( ( 3.0 ) / 2 ), CV_SHAPE_RECT );
	int num_pix = 0;
	int touch_thresh = 50000;
    int detect_thresh = 1500;
    /// Initialize to impossible values.
    center.x = -10000;
    center.y = -10000;

	/// Create intermediate images for scratch space.
    hsvImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 3 );
    outImg = cvCreateImage( cvGetSize(srcImg), IPL_DEPTH_8U, 1 );

	/// Enhance the red channel of the source image.
	//vision_white_balance( srcImg );

    /// Segment the flipped image into a binary image.
    cvCvtColor( srcImg, hsvImg, CV_RGB2HSV );

	/// Equalize the histograms of each channel.
	//vision_hist_eq( hsvImg, VISION_CHANNEL3 );
		//VISION_CHANNEL1 + VISION_CHANNEL2 + VISION_CHANNEL3 );

	/// Threshold all three channels using our own values.
    cvInRangeS( hsvImg, cvScalar(hsv->hL, hsv->sL, hsv->vL),
		cvScalar(hsv->hH, hsv->sH, hsv->vH), binImg );

	/// Use a median filter image to remove outliers.
	cvSmooth( binImg, outImg, CV_MEDIAN, 11, 11, 0. ,0. );
	cvMorphologyEx( binImg, binImg, wS, NULL, CV_MOP_CLOSE, 1);

    /// Find the centroid.
    center = vision_find_centroid( binImg, 5 );
    *dotx = center.x;
    *doty = center.y;

    /// Clear variables to free memory.
    cvReleaseImage( &hsvImg );
    cvReleaseImage( &outImg );

	/// Check to see how many pixels of are detected in the image.
	num_pix = cvCountNonZero( binImg );

	if( num_pix > touch_thresh ) {
		return 2;
	}
    if( num_pix < detect_thresh) {
		return 0;
	}
	/// Check that the values of dotx & doty are not negative.
	if( dotx < 0 || doty < 0 ) {
		return 0;
	}

    return 1;
} /* end vision_find_gate() */


/*------------------------------------------------------------------------------
 * void vision_concat_frames()
 * Makes two images into one side by side.
 *----------------------------------------------------------------------------*/
void vision_concat_images(IplImage *img1, IplImage *img2, IplImage *new_img)
{
	/// Declare variables.
	uchar *data1 = ( uchar* )img1->imageData;
	uchar *data2 = ( uchar* )img2->imageData;
	uchar *new_data = ( uchar* )new_img->imageData;
	int i,j,r,g,b;
	
	/// Loop through the first image to fill the left part of the new image.
	for ( i = 0; i < img1->height; i++ ) {
		for ( j = 0; j < img1->width; j++ ) {
			r = data1[i * img1->widthStep + j * img1->nChannels + 0];
			g = data1[i * img1->widthStep + j * img1->nChannels + 1];
        	b = data1[i * img1->widthStep + j * img1->nChannels + 2];
        	
        	new_data[i * new_img->widthStep + j * new_img->nChannels + 0] = r;
        	new_data[i * new_img->widthStep + j * new_img->nChannels + 1] = g;
        	new_data[i * new_img->widthStep + j * new_img->nChannels + 2] = b;
		}
	}
	
	/// Loop through the second image to fill the left part of the new image.
	for ( i = 0; i < img2->height; i++ ) {
		for ( j = 0; j < img2->width; j++ ) {
			r = data2[i * img2->widthStep + j * img2->nChannels + 0];
			g = data2[i * img2->widthStep + j * img2->nChannels + 1];
        	b = data2[i * img2->widthStep + j * img2->nChannels + 2];
        	
        	new_data[i * new_img->widthStep + (j+img1->width) * new_img->nChannels + 0] = r;
        	new_data[i * new_img->widthStep + (j+img1->width) * new_img->nChannels + 1] = g;
        	new_data[i * new_img->widthStep + (j+img1->width) * new_img->nChannels + 2] = b;
		}
	}
	
} /* end vision_concat_images() */

/*------------------------------------------------------------------------------
 * void vision_save_frame()
 * Saves an image to disk.
 *----------------------------------------------------------------------------*/

void vision_save_frame(IplImage *img, char *dir, char *name)
{
	/// Declare variables.
    struct timeval ctime;
    struct tm ct;
    char write_time[128] = {0};

	if ( name == NULL ) {
		/// Get a timestamp and use for filename.
		gettimeofday( &ctime, NULL );
		ct = *( localtime ((const time_t*) &ctime.tv_sec) );
		strcpy( write_time, dir );
		strftime( write_time + strlen(write_time), sizeof(write_time), "20%y%m%d_%H%M%S", &ct);
		snprintf( write_time + strlen(write_time),
				strlen(write_time), ".%.03ld.jpg", ctime.tv_usec );
	}
	else {
		/// Use the given name - good for comparisons sake
		sprintf( write_time, "%s%s", dir, name );
	}
			
			
	cvSaveImage( write_time, img );
} /* end vision_save_frame() */

#include "ds.h"

/* Non-named namespace, global constants */
namespace{

	/* Background Mode */
#if defined (BGS_MODE) && (BGS_MODE == ON)
	const int BGM_FRAME_COUNT = 20;
#else
	const int BGM_FRAME_COUNT = 0;
#endif 

	/* Optical Flow Parameters */
	const int MAX_CORNERS = 10000;
	const int WIN_SIZE = 5; //default: 5

	/* Processing Window Size (Frame) */
	const unsigned int PROCESSING_WINDOWS = 15; //default: 15

	/* Fire-like Region Threshold */
	const int RECT_WIDTH_THRESHOLD = 5; //default: 5
	const int RECT_HEIGHT_THRESHOLD = 5; //default: 5
	const int CONTOUR_AREA_THRESHOLD = 12; //default: 12
	const int CONTOUR_POINTS_THRESHOLD = 12; //default: 12
}

string model_ = "D:\\work\\GIT\\Fire-Detection\\Motion2Cpp\\Motion2Cpp\\LBP-SVM-model.xml";

	//const string& model = model_;
	Ptr<SVM> svm = cv::ml::SVM::load<cv::ml::SVM>(model_);
	Mat feature_;

void ComputeLBPImage_Uniform(const Mat &srcImage, Mat &LBPImage)
{
    CV_Assert(srcImage.depth() == CV_8U&&srcImage.channels() == 1);
    LBPImage.create(srcImage.size(), srcImage.type());

    Mat extendedImage;
    copyMakeBorder(srcImage, extendedImage, 1, 1, 1, 1, BORDER_DEFAULT);

    static const int table[256] = { 1, 2, 3, 4, 5, 0, 6, 7, 8, 0, 0, 0, 9, 0, 10, 11, 12, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 14, 0, 15, 16, 17, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 20, 0, 21, 22, 23, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25,
        0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 27, 0, 28, 29, 30, 31, 0, 32, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 34, 0, 0, 0, 0
        , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 36, 37, 38, 0, 39, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 41, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 42
        , 43, 44, 0, 45, 0, 0, 0, 46, 0, 0, 0, 0, 0, 0, 0, 47, 48, 49, 0, 50, 0, 0, 0, 51, 52, 53, 0, 54, 55, 56, 57, 58 };

    int heightOfExtendedImage = extendedImage.rows;
    int widthOfExtendedImage = extendedImage.cols;
    int widthOfLBP=LBPImage.cols;
    uchar *rowOfExtendedImage = extendedImage.data+widthOfExtendedImage+1;
    uchar *rowOfLBPImage = LBPImage.data;
    for (int y = 1; y <= heightOfExtendedImage - 2; ++y,rowOfExtendedImage += widthOfExtendedImage, rowOfLBPImage += widthOfLBP)
    {
        uchar *colOfExtendedImage = rowOfExtendedImage;
        uchar *colOfLBPImage = rowOfLBPImage;
        for (int x = 1; x <= widthOfExtendedImage - 2; ++x, ++colOfExtendedImage, ++colOfLBPImage)
        {
            int LBPValue = 0;
            if (colOfExtendedImage[0 - widthOfExtendedImage - 1] >= colOfExtendedImage[0])
                LBPValue += 128;
            if (colOfExtendedImage[0 - widthOfExtendedImage] >= colOfExtendedImage[0])
                LBPValue += 64;
            if (colOfExtendedImage[0 - widthOfExtendedImage + 1] >= colOfExtendedImage[0])
                LBPValue += 32;
            if (colOfExtendedImage[0 + 1] >= colOfExtendedImage[0])
                LBPValue += 16;
            if (colOfExtendedImage[0 + widthOfExtendedImage + 1] >= colOfExtendedImage[0])
                LBPValue += 8;
            if (colOfExtendedImage[0 + widthOfExtendedImage] >= colOfExtendedImage[0])
                LBPValue += 4;
            if (colOfExtendedImage[0 + widthOfExtendedImage - 1] >= colOfExtendedImage[0])
                LBPValue += 2;
            if (colOfExtendedImage[0 - 1] >= colOfExtendedImage[0])
                LBPValue += 1;

            colOfLBPImage[0] = table[LBPValue];

        } // x

    }// y
}


void ComputeLBPFeatureVector_Uniform(const Mat &srcImage, Size cellSize, Mat &featureVector)
{
    
    CV_Assert(srcImage.depth() == CV_8U&&srcImage.channels() == 1);

    Mat LBPImage;
    ComputeLBPImage_Uniform(srcImage,LBPImage);

   
    int widthOfCell = cellSize.width;
    int heightOfCell = cellSize.height;
    int numberOfCell_X = srcImage.cols / widthOfCell;
	int numberOfCell_Y = srcImage.rows / heightOfCell;

    int numberOfDimension = 58 * numberOfCell_X*numberOfCell_Y;
    featureVector.create(1, numberOfDimension, CV_32FC1);
    featureVector.setTo(Scalar(0));

    int stepOfCell=srcImage.cols;
    int index = -58;
    float *dataOfFeatureVector=(float *)featureVector.data;
    for (int y = 0; y <= numberOfCell_Y - 1; ++y)
    {
        for (int x = 0; x <= numberOfCell_X - 1; ++x)
        {
            index+=58;

            Mat cell = LBPImage(Rect(x * widthOfCell, y * heightOfCell, widthOfCell, heightOfCell));
            uchar *rowOfCell=cell.data;
            int sum = 0; 
            for(int y_Cell=0;y_Cell<=cell.rows-1;++y_Cell,rowOfCell+=stepOfCell)
            {
                uchar *colOfCell=rowOfCell;
                for(int x_Cell=0;x_Cell<=cell.cols-1;++x_Cell,++colOfCell)
                {
                    if(colOfCell[0]!=0)
                    {
                        ++dataOfFeatureVector[index + colOfCell[0]-1];
                        ++sum;
                    }
                }
            }

            for (int i = 0; i <= 57; ++i)
                dataOfFeatureVector[index + i] /= sum;
        }
    }
}

bool checkContourPoints(Centroid & ctrd, const unsigned int thrdcp, const unsigned int pwindows) {

	std::deque< std::vector< Feature > >::iterator itrDeq = ctrd.dOFRect.begin();

	unsigned int countFrame = 0;

	// contour points of each frame
	for (; itrDeq != ctrd.dOFRect.end(); ++itrDeq){

		if ((*itrDeq).size() < thrdcp) {
			++countFrame;
		}
	}

	return (countFrame < pwindows / 3) ? true : false;
}


void motionOrientationHist(std::vector< Feature > & vecFeature, unsigned int orien[4]){

	std::vector< Feature >::iterator itrVecFeature;

	/* each point of contour  */
	for (itrVecFeature = vecFeature.begin(); itrVecFeature != vecFeature.end(); ++itrVecFeature) {

		/* orientation */
		if ((*itrVecFeature).perv.x >= (*itrVecFeature).curr.x){
			if ((*itrVecFeature).perv.y >= (*itrVecFeature).curr.y){
				++orien[0];   // up-left 
			}
			else{
				++orien[2];   // down-left
			}
		}
		else{
			if ((*itrVecFeature).perv.y >= (*itrVecFeature).curr.y){
				++orien[1];   // up-right
			}
			else{
				++orien[3];   // down-right
			}
		}
	}
}

double getEnergy(std::vector< Feature > & vecFeature, unsigned int & staticCount, unsigned int & totalPoints){

	std::vector< Feature >::iterator itrVecFeature;

	/* initialization */
	double tmp, energy = 0.0;

	/* each contour point */
	for (itrVecFeature = vecFeature.begin(); itrVecFeature != vecFeature.end(); ++itrVecFeature) {

		/* energy */
		tmp = pow(abs((*itrVecFeature).curr.x - (*itrVecFeature).perv.x), 2) + pow(abs((*itrVecFeature).curr.y - (*itrVecFeature).perv.y), 2);
		if (tmp < 1.0){
			++staticCount;
		}
		else if (tmp < 100.0){
			energy += tmp;
			++totalPoints;
		}
	}
	return energy;
}

bool checkContourEnergy(Centroid & ctrd, const unsigned int pwindows) {

	std::deque< std::vector< Feature > >::iterator itrDeq = ctrd.dOFRect.begin();

	std::vector< Feature >::iterator itrVecFeature;

	unsigned int staticCount = 0, orienCount = 0, orienFrame = 0, totalPoints = 0, passFrame = 0, staticFrame = 0;
	unsigned int orien[4] = { 0 };

	/* contour motion vector of each frame */
	for (; itrDeq != ctrd.dOFRect.end(); ++itrDeq){

		/* flash */
		staticCount = staticFrame = orienCount = staticFrame = staticCount = totalPoints = 0;
		/* energy analysis */
		if (getEnergy((*itrDeq), staticCount, totalPoints) > totalPoints >> 1){
			++passFrame;
		}
		if (staticCount > (*itrDeq).size() >> 1){
			++staticFrame;
		}

		/* flash */
		memset(orien, 0, sizeof(unsigned int) << 2);

		/* orientation analysis */
		motionOrientationHist((*itrDeq), orien);

		for (int i = 0; i < 4; ++i){
			if (orien[i] == 0){
				++orienCount;
			}
		}
		if (orienCount >= 1){
			++orienFrame;
		}
	}

	/* by experience */
	static const unsigned int thrdPassFrame = pwindows >> 1, thrdStaticFrame = pwindows >> 2, thrdOrienFrame = (pwindows >> 3) + 1;

	return (passFrame > thrdPassFrame && staticFrame < thrdStaticFrame && orienFrame < thrdOrienFrame) ? true : false;
}

void matchCentroid(
	cv::Mat& imgFireAlarm,
	std::list< Centroid > & listCentroid,
	std::multimap< int, OFRect > & mulMapOFRect,
	int currentFrame, const int thrdcp,
	const unsigned int pwindows
){

	static std::list< Centroid >::iterator itCentroid;		             // iterator of listCentroid
	static std::multimap< int, OFRect >::iterator itOFRect, itOFRectUp;  // iterator of multimapBRect

	static bool flagMatch = false;                                       // checking the list(centroid) and map(rect) match or not
	static cv::Rect* pRect = NULL;
	static cv::Rect rectFire(0, 0, 0, 0);


	/* Check listCentroid node by node */
	itCentroid = listCentroid.begin();
	static unsigned int rectCount = 0;

	while (itCentroid != listCentroid.end()) {

		/* setup mulMapBRect upper bound */
		itOFRectUp = mulMapOFRect.upper_bound((*itCentroid).centroid.x);

		flagMatch = false;

		/* visit mulMapOFRect between range [itlow,itup) */
		for (itOFRect = mulMapOFRect.begin(); itOFRect != itOFRectUp; ++itOFRect) {

			/* for easy access info */
#ifndef CENTROID
#define CENTROID (*itCentroid).centroid 				
#endif			
			/* for quick access info */
			pRect = &(*itOFRect).second.rect;

			/* matched */
			if (CENTROID.y >= (*pRect).y && ((*pRect).x + (*pRect).width) >= CENTROID.x && ((*pRect).y + (*pRect).height) >= CENTROID.y) {

				/* push rect to the matched listCentorid node */
				(*itCentroid).vecRect.push_back(*pRect);

				/* push vecFeature to matched listCentorid node */
				(*itCentroid).dOFRect.push_back((*itOFRect).second.vecFeature);

				/* Update countFrame and judge the threshold of it */
				if (++((*itCentroid).countFrame) == pwindows) {
					/* GO TO PROCEESING DIRECTION MOTION */
					if (judgeDirectionsMotion((*itCentroid).vecRect, rectFire) && checkContourPoints(*itCentroid, thrdcp, pwindows) && checkContourEnergy(*itCentroid, pwindows)) {

						/* recting the fire region */
						cv::rectangle(imgFireAlarm, rectFire, CV_RGB(0, 100, 255), 3);

						cout << "Alarm: " << currentFrame << endl;
						cv::imshow("Video", imgFireAlarm);

						/* halt */

						/* HALT_MODE */
#if defined (HALT_MODE) && (HALT_MODE == ON)
						cvWaitKey(0);
#endif 
						/* checking the optical flow information */
						//writeContourPointsInfo( ofInfoFileFolder, *itCentroid, currentFrame, rectCount );

						/* save as image */
						//sprintf( outfile, RectImgFilePath, currentFrame-PROCESSING_WINDOWS+1, currentFrame );
						//cvSaveImage( outfile, imgDisplay );

						/* save rect information */
						//fileStream< CvRect > fsOut;
						//sprintf( outfile, RectInfoFilePath, currentFrame-PROCESSING_WINDOWS+1, ++rectCount );
						//fsOut.WriteXls( (*itCentroid).vecRect, outfile, currentFrame-PROCESSING_WINDOWS+1 );

						// then go to erase it
					}
					else{
						break;   // if not on fire go to erase it
					}
				}

				/* mark this rect as matched */
				(*itOFRect).second.match = true;
				flagMatch = true;
				++itCentroid;
				break;    // if mateched break the inner loop
			}
			// if ended the maprect and not mateched anyone go to erase it

		} // for (multimapBRect)

		/* if not found erase node */
		if (!flagMatch) {
			itCentroid = listCentroid.erase(itCentroid); 
		}
	}

	//cout << "======================================================================================" << endl;
	/* push new rect to listCentroid */
	for (itOFRect = mulMapOFRect.begin(); itOFRect != mulMapOFRect.end(); ++itOFRect) {

		if (!(*itOFRect).second.match) {
			/* push new node to listCentroid */
			listCentroid.push_back(centroid((*itOFRect).second));
			//cout << "after rect: " << endl;
			//cout << (*itBRect).second << endl;	x
		}
	}

	//cout <<"after list count: "<< listCentroid.size() << endl;

	/* clear up container */
	mulMapOFRect.clear();
}


int main()
{
    VideoCapture cap("D:\\work\\EarlyFireDetection-master\\EarlyFireDetection\\EarlyFireDetection\\test_6.mp4"); // open the default camera
    if(!cap.isOpened()){
		cout << "Error opening video stream or file" << endl;
    return -1;
  }
	cv::Mat imgSrc;
	cap.read(imgSrc);
	if (imgSrc.empty()) {
		fprintf(stderr, "Cannot open video!\n");
		return 1;
	}
	int w = 500;
	int h = int(imgSrc.rows*500/imgSrc.cols);
	cv::resize(imgSrc, imgSrc, cv::Size(w, h));
	cv::Size sizeImg = imgSrc.size();
	Mat frameDelta, firstFrame;

	Mat imgCurr(sizeImg, CV_8UC1);
	Mat imgGray(sizeImg, CV_8UC1);
	int bgThresh = 25, minContourArea = 80;

	Rect rectContour;
	time_t start,end;
	
	// notify the current frame 
	unsigned long currentFrame = 0;

	/* Rect Motion */
	std::list< Centroid > listCentroid;							  // Centroid container
	std::vector< OFRect > vecOFRect;							  // tmp container for ofrect
	std::multimap< int, OFRect > mulMapOFRect;					  // BRect container
	
	// Buffer for Pyramid image  
	std::vector<cv::Point2f> featuresPrev;
	std::vector<cv::Point2f> featuresCurr;
	
	// Pyramid Lucas-Kanade 
	std::vector<uchar> featureFound;
	std::vector<float> featureErrors;
	cv::Size sizeWin(WIN_SIZE, WIN_SIZE);
	RectThrd rThrd = rectThrd(RECT_WIDTH_THRESHOLD, RECT_HEIGHT_THRESHOLD, CONTOUR_AREA_THRESHOLD);
	while(1)
    {
		featuresPrev.clear();
		featuresPrev.push_back(cv::Point2f(0, 0));
		featuresCurr.clear();
		featuresCurr.push_back(cv::Point2f(0, 0));
		
		time(&start);

        cap.read(imgSrc);  // get the first frame 
		if (imgSrc.empty()) {
			break;   // exit if unsuccessful or Reach the end of the video
		}
		cv::resize(imgSrc, imgSrc, cv::Size(640, 360));
		// convert rgb to gray 
		cv::cvtColor(imgSrc, imgGray, CV_BGR2GRAY);
		
		cap.read(imgSrc); // get the second frame

		if (imgSrc.empty()) {
			break;
		}
		cv::resize(imgSrc, imgSrc, cv::Size(640, 360));
		// the second frame ( gray level )
		cv::cvtColor(imgSrc, imgCurr, CV_BGR2GRAY);
		
		resize(imgSrc,imgSrc,sizeImg);
        cvtColor(imgSrc, imgGray, COLOR_BGR2GRAY);
		cv::resize(imgSrc, imgSrc, sizeImg);
		// the second frame ( gray level )
		cv::cvtColor(imgSrc, imgCurr, CV_BGR2GRAY);
		Mat threshImg = Mat::zeros(sizeImg, CV_8UC1);
        GaussianBlur(imgGray, imgGray, Size(21,21), 1.5, 1.5);
       if(firstFrame.empty())
		{
			firstFrame =  imgGray.clone();
		}
		//<________________________________________MOTION_________________________________________>
		//compute the absolute difference between the current frame and
		absdiff(firstFrame, imgGray, frameDelta);
		threshold(frameDelta, threshImg, bgThresh,255, THRESH_BINARY);
		//dilate(threshImg, threshImg, iterations = 2); 
		vector<Vec4i> hierarchy;
		vector<vector<Point>> contour;
		//cout<<contours.size();
		findContours(threshImg.clone(), contour, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); 
		//time(&end);
		//float seconds = difftime(end,start);
		//cout<< "Fps: " <<(float) 1/seconds << endl;
		imshow("thresh",threshImg);
		imshow("imgSrc", imgSrc);

		//________________________________EARLY FIRE DETECTION___________________________________
		/* assign feature points and get the number of feature */
		getContourFeatures(contour, hierarchy, vecOFRect, rThrd, featuresPrev, featuresCurr);
//		
//		// Pyramid L-K Optical Flow
		cv::calcOpticalFlowPyrLK(
			imgGray,
			imgCurr,
			featuresPrev,
			featuresCurr,
			featureFound,
			featureErrors,
			sizeWin,
			2,
			cv::TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.3),
			0);
//		
////		/* assign feature points to fire-like obj and then push to multimap */
		assignFeaturePoints(mulMapOFRect, vecOFRect, featureFound, featuresPrev, featuresCurr);
////
////		/* compare the mulMapOFRect space with listCentroid space, if matching insert to listCentroid space as candidate fire-like obj */
		matchCentroid(imgSrc, listCentroid, mulMapOFRect, currentFrame, CONTOUR_POINTS_THRESHOLD, PROCESSING_WINDOWS);
////
//		//writer.write(imgFireAlarm);
//		
//		std::cout << "< Frame >: " << currentFrame++ << endl;
//
		char c=(char)waitKey(25);
		if(c==27)
			break;
        
   }

	cap.release();
	imgCurr.release();
	imgGray.release();
	frameDelta.release();
	cv::destroyAllWindows();
	destroyAllWindows();
	return 0;
}
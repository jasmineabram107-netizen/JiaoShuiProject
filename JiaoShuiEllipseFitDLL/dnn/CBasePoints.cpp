#include "CBasePoints.h"
#include <opencv2/opencv.hpp>
#include "opencv2\features2d.hpp"
#include "../cvLib/cvcommon.h"
#include "detect_features.h"
#include "..\fitting\fit_util.h"
#include "..\imgproc\HarrisCorner.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

using namespace cv;
using namespace cv::dnn;

cv::Mat getMask(const cv::Mat& roi) 
{
	cv::Mat gray, edges;
	std::vector<std::vector<cv::Point>> contours;
	cv::GaussianBlur(roi, gray, cv::Size(5, 5), 0);
	cv::Canny(gray, edges, 100, 200);
	cv::Mat dilated, eroded;
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 1));
	cv::dilate(edges, dilated, kernel2);
	cv::erode(dilated, edges, kernel2);
	cv::findContours(edges, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

	// Create a mask with the same size as the input image, initialized to zeros (black)
	Mat mask = Mat::ones(roi.size(), CV_8UC1);

	for (int i = 0; i < contours.size(); i++) {
		double area = contourArea(contours[i]);
		if (contours[i].size() < 10 || (contours[i].size() < 300 && area > 0)) {
			drawContours(mask, contours, (int)i, Scalar(0), FILLED);
		}
	}

	cv::Mat kernel1 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
	cv::erode(mask, mask, kernel1);

	return mask;
}

int refineCornerPair_v2(
	const cv::Mat& img,
	const cv::Point2f& leftPt,
	const cv::Point2f& rightPt,
	MainShapeType mainShape, // not use still, but I think it may be useful in the future
	RectangleF* _outRect,
	bool bHasLimit
) {
	if (img.empty() || _outRect == nullptr)
		return 0;

	cv::Mat grey;
	if (img.channels() == 3)
		cv::cvtColor(img, grey, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 1)
		grey = img.clone();  // Already grayscale
	else
		throw std::runtime_error("Unexpected channel count");

	auto left = leftPt, right = rightPt;
	if (left.x > right.x) {
		std::swap(left, right);
	}
	_outRect->left = left.x;
	_outRect->top = left.y;
	_outRect->right = right.x;
	_outRect->bottom = right.y;

	int w = std::min((int)(right.x - left.x), 130) / 2 + 65;
	int x0 = std::max(0, (int)left.x - w);
	int x1 = std::min(img.cols - 1, (int)right.x + w);
	int ww = x1 - x0;
	if (ww < 20)
		return 0;

	int centerY = (int)(left.y + right.y) / 2;
	int y0 = std::max(0, centerY - ww / 2);
	int y1 = std::min(img.rows - 1, centerY + ww / 2);
	int hh = y1 - y0;
	cv::Rect rcRoi(x0, y0, ww, hh);

	HarrisCornerDetector cornerDet;
	if (rcRoi.empty())
		return 0;
	auto corners = cornerDet.DetectShiTomasi(grey(rcRoi));
	
	if (corners.empty())
		return 0;
	std::vector<float> cornerStrengths(corners.size(), 0.0f);
	float maxScore = 0.0f;
	float minScore = 1e6;
	for(int i = 0; i < (int)corners.size(); i++) {
		cornerStrengths[i] = cornerDet.GetCornerStrength(i);
		maxScore = std::max(maxScore, cornerStrengths[i]);
		minScore = std::min(minScore, cornerStrengths[i]);
	}
	float scoreThreshold = 0.2f * (maxScore - minScore) + minScore; // 20% of the range
#if DEBUG_IMG
	cv::Mat debugImg = img.clone();
	// Convert to absolute coordinates
	for (auto pt : corners) {
		pt.x += x0;
		pt.y += y0;
		cv::drawMarker(debugImg, pt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 4, 1);
		float score = cornerDet.GetCornerStrength(&pt - &corners[0]);
	}
	SaveDebugImg("harris_corners", debugImg);
#endif
#if DEBUG_IMG
	cv::Mat tmpImg = img.clone();
#endif
	std::vector<ScoredPoint> leftCandidates, rightCandidates;
	for (int i = 0; i < corners.size(); i++) {
		auto pt = corners[i];
		cv::Point2f absPt = pt + cv::Point2f((float)x0, (float)y0);
		double scoreLeft = -1.0, scoreRight = -1.0;
		if (bHasLimit) {
			float left_dis = distance(absPt, left);
			float right_dis = distance(absPt, right);
			if (left_dis < UPDATE_DIS) {
				scoreLeft = 1.0f - left_dis / UPDATE_DIS;
			}
			if (right_dis < UPDATE_DIS) {
				scoreRight = 1.0f - right_dis / UPDATE_DIS;
			}
		}
		else {
			float cornerLevel = std::min(1.f, cornerStrengths[i] / scoreThreshold);
			score_droplet_corner(grey, absPt, left, right, mainShape, scoreLeft, scoreRight, cornerLevel);
		}		
				
		if (scoreLeft > 0) {
			leftCandidates.push_back({ absPt, scoreLeft, scoreRight });
		}
		if (scoreRight > 0) {
			rightCandidates.push_back({ absPt, scoreLeft, scoreRight });
		}
	}
	// Sort by left score
	auto sortByScoreLeft = [](const ScoredPoint& a, const ScoredPoint& b) { return a.scoreLeft > b.scoreLeft; };
	auto sortByScoreRight = [](const ScoredPoint& a, const ScoredPoint& b) { return a.scoreRight > b.scoreRight; };
	std::sort(leftCandidates.begin(), leftCandidates.end(), sortByScoreLeft);
	std::sort(rightCandidates.begin(), rightCandidates.end(), sortByScoreRight);

	// Default to top-1 from each
	cv::Point2f bestLeft = left;
	cv::Point2f bestRight = right;

	if (!leftCandidates.empty() && !rightCandidates.empty()) {
		if (leftCandidates[0].pt != rightCandidates[0].pt) {
			bestLeft = leftCandidates[0].pt;
			bestRight = rightCandidates[0].pt;
		}
		else {
			// Resolve conflict
			if (leftCandidates[0].scoreLeft >= rightCandidates[0].scoreRight) {
				bestLeft = leftCandidates[0].pt;
				if (rightCandidates.size() > 1)
					bestRight = rightCandidates[1].pt;
			}
			else {
				bestRight = rightCandidates[0].pt;
				if (leftCandidates.size() > 1)
					bestLeft = leftCandidates[1].pt;
			}
		}
	}
	else {
		if (!leftCandidates.empty())
			bestLeft = leftCandidates[0].pt;
		if (!rightCandidates.empty())
			bestRight = rightCandidates[0].pt;
	}

	// Save result
	_outRect->left = bestLeft.x;
	_outRect->top = bestLeft.y;
	_outRect->right = bestRight.x;
	_outRect->bottom = bestRight.y;

#if DEBUG_IMG	
	cv::drawMarker(tmpImg, bestLeft, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 4, 1);
	cv::drawMarker(tmpImg, bestRight, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 4, 1);
	SaveDebugImg("basepoints", tmpImg);
#endif
	return 1;
}

bool isForkCandidate(
	const cv::Mat& edge,         // Canny edge image (CV_8UC1)
	const cv::Mat& angle,        // gradient angle image (CV_32FC1, from cartToPolar)
	const cv::Point& pt,         // point to evaluate
	float& out_angle_diff,       // angle difference between two dominant directions
	int radius,              // neighborhood radius
	int binCount,           // angle bins (10 degrees)
	int minVotes             // threshold per dominant bin
) {
	if (pt.x < radius || pt.y < radius || pt.x >= edge.cols - radius || pt.y >= edge.rows - radius)
		return false;

	std::vector<int> histogram(binCount, 0);
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			int x = pt.x + dx, y = pt.y + dy;
			if (edge.at<uchar>(y, x) > 0) {
				float ang = angle.at<float>(y, x);  // radians [0, 2pi]
				int bin = static_cast<int>(ang * 180.0f / CV_PI) / (360 / binCount);
				histogram[bin % binCount]++;
			}
		}
	}

	// Find top 2 dominant bins
	int firstIdx = -1, secondIdx = -1;
	int firstVotes = 0, secondVotes = 0;
	for (int i = 0; i < binCount; ++i) {
		int count = histogram[i];
		if (count > firstVotes) {
			secondVotes = firstVotes;
			secondIdx = firstIdx;
			firstVotes = count;
			firstIdx = i;
		}
		else if (count > secondVotes) {
			secondVotes = count;
			secondIdx = i;
		}
	}

	if (firstVotes < minVotes || secondVotes < minVotes)
		return false;

	float angle1 = firstIdx * (360.0f / binCount);
	float angle2 = secondIdx * (360.0f / binCount);
	float diff = std::abs(angle1 - angle2);
	if (diff > 180) diff = 360 - diff;

	out_angle_diff = diff;

	return (diff >= 10.0f && diff <= 170.0f);  // typical fork range
}

int refineCornerPair(
	const cv::Mat& img, 
	const cv::Point2f& leftPt, 
	const cv::Point2f& rightPt, 
	MainShapeType mainShape, // not use still, but I think it may be useful in the future
	RectangleF* rect, 
	bool bHasLimit
) {
	if (img.empty() || rect == nullptr)
		return 0;	

	cv::Mat grey;
	if (img.channels() == 3)
		cv::cvtColor(img, grey, cv::COLOR_BGR2GRAY);
	else if (img.channels() == 1)
		grey = img.clone();  // Already grayscale
	else
		throw std::runtime_error("Unexpected channel count");

	auto left = leftPt, right = rightPt;	
	if (left.x > right.x) {
		std::swap(left, right);
	}
	rect->left = left.x;
	rect->top = left.y;
	rect->right = right.x;
	rect->bottom = right.y;

	int w = std::min((int)(right.x - left.x), 130) / 2 + 65;
	int x0 = std::max(0, (int)left.x - w);
	int x1 = std::min(img.cols - 1, (int)right.x + w);
	int ww = x1 - x0;
	if (ww < 20)
		return 0;

	int centerY = (int)(left.y + right.y) / 2;
	int y0 = std::max(0, centerY - ww / 2);
	int y1 = std::min(img.rows - 1, centerY + ww / 2);
	int hh = y1 - y0;
	cv::Rect rcRoi(x0, y0, ww, hh);

	cv::Mat roi = grey(rcRoi);
	cv::Mat mask = getMask(roi);
	std::vector<cv::Point2f> corners;
	cv::goodFeaturesToTrack(roi, corners, 40, 0.03, 10, mask, 9, false, 0.04);

	if (corners.empty())
		return 0;

	cv::cornerSubPix(
		roi, // Input image
		corners, // Vector of corners (input and output)
		cv::Size(5, 5), // Half side length of search window
		cv::Size(-1, -1), // Half side length of dead zone (-1=none)
		cv::TermCriteria(
			cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
			40, // Maximum number of iterations
			0.001 // Minimum change per iteration
		)
	);

	cv::Ptr<cv::GFTTDetector> detector = cv::GFTTDetector::create(
		20, 0.03, 10, 9, true, 0.04);
	std::vector<cv::KeyPoint> keypoints;
	detector->detect(roi, keypoints, mask);
	for (const auto& kp : keypoints) {
		cv::Scalar color(0, 0, 255); // High = green, Low = red
		auto pt = kp.pt + cv::Point2f((float)x0, (float)y0);
		bool exist = false;
		for (const auto& alredy : corners) {
			if (alredy == kp.pt) {
				exist = true;
				break;
			}
			else if(distance(alredy, kp.pt) < 256.0f) { // Allow small distance
				exist = true;
				break;
			}
		}
		if(!exist)
			corners.push_back(kp.pt);
	}
	int left_min_dis = INT_MAX, right_min_dis = INT_MAX;
	cv::Point2f left_best, right_best;
	double bestLeftScore = -1.0, bestRightScore = -1.0;
	
#if DEBUG_IMG
	cv::Mat debugImg = img.clone();
	for (const auto& pt : corners) {
		cv::Point2f absPt = pt + cv::Point2f((float)x0, (float)y0);
		cv::drawMarker(debugImg, absPt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 10, 1);
	}
	SaveDebugImg("old_harris__corners", debugImg);
#endif
	std::vector<ScoredPoint> leftCandidates, rightCandidates;
	for (const auto& pt : corners) {
		cv::Point2f absPt = pt + cv::Point2f((float)x0, (float)y0);
		double scoreLeft = -1.0, scoreRight = -1.0;
		if (bHasLimit) {
			float left_dis = distance(absPt, left);
			float right_dis = distance(absPt, right);
			if (left_dis < UPDATE_DIS) {
				scoreLeft = 1.0f - left_dis / UPDATE_DIS;
			}
			if(right_dis < UPDATE_DIS) {
				scoreRight = 1.0f - right_dis / UPDATE_DIS;
			}
		}
		else {
			score_droplet_corner(grey, absPt, left, right, mainShape, scoreLeft, scoreRight);			
		}		
		if(scoreLeft > 0)
			leftCandidates.push_back({ absPt, scoreLeft, scoreRight });
		if(scoreRight > 0)
			rightCandidates.push_back({ absPt, scoreLeft, scoreRight });
	}
	// Sort by left score
	auto sortByScoreLeft = [](const ScoredPoint& a, const ScoredPoint& b) { return a.scoreLeft > b.scoreLeft; };
	auto sortByScoreRight = [](const ScoredPoint& a, const ScoredPoint& b) { return a.scoreRight > b.scoreRight; };
	std::sort(leftCandidates.begin(), leftCandidates.end(), sortByScoreLeft);
	std::sort(rightCandidates.begin(), rightCandidates.end(), sortByScoreRight);

	// Default to top-1 from each
	cv::Point2f bestLeft = left;
	cv::Point2f bestRight = right;

	if (!leftCandidates.empty() && !rightCandidates.empty()) {
		if (leftCandidates[0].pt != rightCandidates[0].pt) {
			bestLeft = leftCandidates[0].pt;
			bestRight = rightCandidates[0].pt;
		}
		else {
			// Resolve conflict
			if (leftCandidates[0].scoreLeft >= rightCandidates[0].scoreRight) {
				bestLeft = leftCandidates[0].pt;
				if (rightCandidates.size() > 1)
					bestRight = rightCandidates[1].pt;
			}
			else {
				bestRight = rightCandidates[0].pt;
				if (leftCandidates.size() > 1)
					bestLeft = leftCandidates[1].pt;
			}
		}
	}
	else {
		if (!leftCandidates.empty())
			bestLeft = leftCandidates[0].pt;
		if (!rightCandidates.empty())
			bestRight = rightCandidates[0].pt;
	}

	// Save result
	rect->left = bestLeft.x;
	rect->top = bestLeft.y;
	rect->right = bestRight.x;
	rect->bottom = bestRight.y;

	return 1;
}

std::vector<cv::Point2f> filterCorners(const cv::Mat& roi, const std::vector<cv::Point2f>& corners)
{
	cv::Mat gray, edges;
	std::vector<std::vector<cv::Point>> contours;
	cv::GaussianBlur(roi, gray, cv::Size(5, 5), 0);
	cv::Canny(gray, edges, 100, 200);
	cv::Mat dilated, eroded;
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 1));
	cv::dilate(edges, dilated, kernel2);
	cv::erode(dilated, edges, kernel2);
	cv::findContours(edges, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	std::vector<cv::Point2f> new_corners;
	for (int i = 0; i < corners.size(); i++) {
		cv::Point2f pt = corners[i];
		for (int j = 0; j < contours.size(); j++) {

		}
	}
	return new_corners;
}


CBasePoints::CBasePoints() 
	:m_ONet(NULL)
{	
	isInitd = false;
}

CBasePoints::~CBasePoints()
{
	if (m_ONet) {
		delete m_ONet;
		m_ONet = nullptr;
		isInitd = false;
	}
}

bool CBasePoints::InitModel(const TCHAR* appPath)
{
	isInitd = false;
	SetCurrentDirToExecutablePath(appPath);
	std::string ini = ELLIPSEBASEPOINT_CFG;
	std::string dic = ELLIPSEBASEPOINT_WEIGHTS;
	/*auto basepath = TCHARToCvString(appPath);
	std::string path = "..\\Bin64\\";
	std::string ini = path + ELLIPSEBASEPOINT_CFG;
	std::string dic = path + ELLIPSEBASEPOINT_WEIGHTS;*/

	try
	{
		m_ONet = new cv::dnn::Net(cv::dnn::readNetFromCaffe(ini, dic));
		m_ONet->setPreferableBackend(DNN_BACKEND_DEFAULT);
		m_ONet->setPreferableTarget(DNN_TARGET_CPU);
		isInitd = true;
	}
	catch(const cv::Exception& e)
	{
		std::cerr << "Error loading model: " << e.what() << std::endl;
		isInitd = false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error loading model: " << e.what() << std::endl;
		isInitd = false;
	}
	catch (...)
	{
		std::cerr << "Unknown error loading model" << std::endl;
		isInitd = false;
	}	
	if (!isInitd) {
		if (m_ONet) {
			delete m_ONet;
			m_ONet = nullptr;
		}
	}
	return isInitd;
}


/*
* 识别基线点
* img：输入图像
*
* 返回 - 两的点坐标
*/
bool 
CBasePoints::getBaselinePoints(
	const cv::Mat& img,
	MainShapeType mainShape, // not use still, but I think it may be useful in the future
	RectangleF* _outRect
)
{
	bool res = false;
	const int INPUT_DATA_WIDTH = 144;
	const int INPUT_DATA_HEIGHT = 144;

	const float IMG_MEAN = 127.5f;
	const float IMG_INV_STDDEV = 1.f / 128.f;

	cv::Size windowSize = cv::Size(INPUT_DATA_WIDTH, INPUT_DATA_HEIGHT);

	cv::Mat roi ;
	
	cv::resize(img, roi, windowSize, 0, 0, cv::INTER_AREA);

	// BGR problem 

	// build blob images from the inputs
	auto blobInput =
		cv::dnn::blobFromImage(roi, IMG_INV_STDDEV, cv::Size(),
			cv::Scalar(IMG_MEAN, IMG_MEAN, IMG_MEAN), false);

	m_ONet->setInput(blobInput, "data");

	const std::vector<cv::String> outBlobNames{ "conv6i-2", "conv6-3", "prob1" };
	std::vector<cv::Mat> outputBlobs;

	m_ONet->forward(outputBlobs, outBlobNames);

	cv::Mat regressionsBlob = outputBlobs[0];
	cv::Mat landMarkBlob = outputBlobs[1];
	cv::Mat scoresBlob = outputBlobs[2];

	const float* scores_data = (float*)scoresBlob.data;
	const float* landmark_data = (float*)landMarkBlob.data;
	const float* reg_data = (float*)regressionsBlob.data;

	if (scores_data[1] >= 0.3) {
		res = true;		
		float w = (float)img.cols;
		float h = (float)img.rows;
		cv::Point2f left(landmark_data[0] * w, landmark_data[1] * h);
		cv::Point2f right(landmark_data[2] * w, landmark_data[3] * h);
		refineCornerPair_v2(img, left, right, mainShape, _outRect, false);
#if DEBUG_IMG
		cv::Mat landMat = img.clone();
		for (int p = 0; p < 4; p++) {
			cv::drawMarker(
				landMat,
				cv::Point2f(landmark_data[2*p] * w, landmark_data[2*p + 1] * h), 
				cv::Scalar(0, 0, 255), 
				cv::MARKER_CROSS, 10, 
				1
			);
		}		
		SaveDebugImg("landmarks", landMat);
#endif
	}
	return res;
}

int
CBasePoints::getBaselinePointsByPath(
	const TCHAR* imagePath,
	MainShapeType mainShape, // not use still, but I think it may be useful in the future
	RectangleF* rect
) {
	int res = 0;
	Mat img = loadImageFromUnicodePath(imagePath);
	if(img.data == NULL)
		res = 0;
	else  {
		bool v = getBaselinePoints(img, mainShape, rect);
		if (v)
			res = 1;
	}
	return res;
}

bool 
CBasePoints::getBaselinePointsByPath(
	const cv::Mat& frame, 
	MainShapeType mainShape,  // not use still, but I think it may be useful in the future
	RectangleF* rect
) {
	bool res = false;
	if (frame.data == NULL)
		return res;
	else {
		res = getBaselinePoints(frame, mainShape, rect);
	}
	return res;
}


int
CBasePoints::updateBasePoints(
	const TCHAR* imagePath, 
	MainShapeType mainShape, // not use still, but I think it may be useful in the future
	RectangleF* _outRect, 
	bool bHasLimit
) {	
	Mat img = loadImageFromUnicodePath(imagePath);
	if (img.data == NULL) {
		return 0;
	}
	cv::Point2f left((float)_outRect->left, (float)_outRect->top);
	cv::Point2f right((float)_outRect->right, (float)_outRect->bottom);
	return refineCornerPair_v2(img, left, right, mainShape, _outRect, bHasLimit);
}

int 
CBasePoints::updateBasePoints(
	const cv::Mat& frame, 
	MainShapeType mainShape, // not use still, but I think it may be useful in the future
	RectangleF* _outRect,
	bool bHasLimit
) {
	if (frame.data == NULL) {
		return 0;
	}
	cv::Point2f left((float)_outRect->left, (float)_outRect->top);
	cv::Point2f right((float)_outRect->right, (float)_outRect->bottom);
	return refineCornerPair_v2(frame, left, right, mainShape, _outRect, bHasLimit);
}
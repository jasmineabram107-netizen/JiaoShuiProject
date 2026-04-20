#pragma once
#include <opencv2/imgproc.hpp>
#include "opencv2/dnn.hpp"
#include <tchar.h>
#include "common.h"

#define WHITE_DIS 3
#define UPDATE_DIS 81

struct ScoredPoint {
	cv::Point2f pt;
	double scoreLeft;
	double scoreRight;
};

int refineCornerPair(const cv::Mat& img, const cv::Point2f& left, const cv::Point2f& right, MainShapeType mainShape, RectangleF* rect, bool bHasLimit);
int refineCornerPair_v2(const cv::Mat& img, const cv::Point2f& left, const cv::Point2f& right, MainShapeType mainShape, RectangleF* _outRect, bool bHasLimit);
bool isForkCandidate(
	const cv::Mat& edge,         // Canny edge image (CV_8UC1)
	const cv::Mat& angle,        // gradient angle image (CV_32FC1, from cartToPolar)
	const cv::Point& pt,         // point to evaluate
	float& out_angle_diff,       // angle difference between two dominant directions
	int radius = 5,              // neighborhood radius
	int binCount = 36,           // angle bins (10 degrees)
	int minVotes = 3             // threshold per dominant bin
);

cv::Mat getMask(const cv::Mat& roi);
std::vector<cv::Point2f> filterCorners(const cv::Mat& roi, const std::vector<cv::Point2f>& corners);

class CBasePoints
{

public:
	CBasePoints();
	virtual ~CBasePoints();

	cv::dnn::Net* m_ONet;

	/*
	* 识别基线点
	* img：输入图像
	*
	* 返回 - 两的点坐标
	*/
	bool getBaselinePoints(const cv::Mat& img, MainShapeType mainShape, RectangleF* points);

	int getBaselinePointsByPath(const TCHAR* imagePath, MainShapeType mainShape, RectangleF* points);
	bool getBaselinePointsByPath(const cv::Mat& image, MainShapeType mainShape, RectangleF* points);

	int updateBasePoints(const TCHAR* imagePath, MainShapeType mainShape, RectangleF* points, bool bHasLimit);
	int updateBasePoints(const cv::Mat& image, MainShapeType mainShape, RectangleF* points, bool bHasLimit);
	bool InitModel(const TCHAR* appPath);
private:
	bool isInitd = false;
};
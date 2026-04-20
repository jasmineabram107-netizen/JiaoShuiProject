#ifndef __JIAOSHUI_DETECT_FEATURES_H__
#define __JIAOSHUI_DETECT_FEATURES_H__
#include "common.h"
#include "opencv2\opencv.hpp"

void detect_corner_points(const cv::Mat& src, std::vector<cv::Point2f>& corners, int maxCorners = 20, double qualityLevel = 0.1, double minDistance = 20, int blockSize = 9, bool useHarrisDetector = true, double k = 0.04);
bool is_valid_droplet_corner(const cv::Mat& gray, const cv::Point2f& pt, int blockSize = 21);
void score_droplet_corner(
	const cv::Mat& gray,
	const cv::Point2f& pt,
	const cv::Point2f& leftRef,
	const cv::Point2f& rightRef,
	MainShapeType mainShape,
	double& _outLeftScore,
	double& _outRightScor,
	float cornerScore = 1.f,
	bool bHasLimit = false, 
	int blockSize = 31
);
#endif
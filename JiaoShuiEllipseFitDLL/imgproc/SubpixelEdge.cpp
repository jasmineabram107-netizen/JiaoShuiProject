#include "SubpixelEdge.h"
#include "subpixel_corner.h"

CSubpixelEdgeDetector::CSubpixelEdgeDetector()
{
}

CSubpixelEdgeDetector::~CSubpixelEdgeDetector()
{
	m_resEdges.clear();
}

bool
CSubpixelEdgeDetector::Detect(
	const cv::Mat& image,
	double stddev, 
	double thr0, 
	double thr1
) {
	bool res = false;
	m_resEdges.clear();
	if (image.empty())
		return res;
	int w = image.cols;
	int h = image.rows;

	// Convert to grayscale if necessary
	cv::Mat gray;
	if (image.channels() == 3) {
		cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
	}
	else {
		gray = image.clone();
	}

	double* grayData = new double[w*h];

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			grayData[i * gray.cols + j] = static_cast<float>(gray.at<uchar>(i, j));
		}
	}
	double* x = NULL, * y = NULL;
	int* curve_limits = NULL;
	int N = 0, cntOfCurves = 0;
	devernay(&x, &y, &N, &curve_limits, &cntOfCurves, grayData, w, h, stddev, thr0, thr1);
	delete[] grayData;

	if (x != NULL && y != NULL && curve_limits != NULL) {
		for (int k = 0; k < cntOfCurves; k++) {
			std::vector<cv::Point2f> curve;
			for (int i = curve_limits[k]; i < curve_limits[k + 1]; i++) {
				curve.emplace_back(static_cast<float>(x[i]), static_cast<float>(y[i]));
			}
			if (!curve.empty()) {
				m_resEdges.push_back(curve);
			}
		}
	}
	if (x != NULL) {
		free(x); 
		x = NULL;
	}
	if (y != NULL) {
		free(y);
		x = NULL;
	}
	if (curve_limits != NULL) {
		free(curve_limits);
		curve_limits = NULL;
	}
	res = true;
	return res;
}

std::vector<std::vector<cv::Point>> 
CSubpixelEdgeDetector::getIntEdge() const 
{
	std::vector<std::vector<cv::Point>> res;	

	for (const auto& contour : m_resEdges) {
		std::vector<cv::Point> intContour;
		intContour.reserve(contour.size());
		for (const auto& pt : contour) {
			intContour.emplace_back(cv::Point(cvRound(pt.x), cvRound(pt.y)));
		}
		res.push_back(intContour);
	}

	return res;
}

std::vector<std::vector<cv::Point2f>>
CSubpixelEdgeDetector::getFloatEdge() const
{	
	return m_resEdges;
}
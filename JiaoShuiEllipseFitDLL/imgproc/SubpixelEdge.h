#ifndef __JIAOSHUI_SUBPIXEL_EDGE_H__
#define __JIAOSHUI_SUBPIXEL_EDGE_H__

#include "opencv2\opencv.hpp"

/*
* @brief Subpixel Edge Detector using Devernay's method.(wrapped with openCV)
*/
class CSubpixelEdgeDetector
{
public:
	CSubpixelEdgeDetector();
	virtual ~CSubpixelEdgeDetector();

	/*
	* @brief Detects subpixel edges in an image using the Devernay method.
	*
	* @param image Input image (grayscale or color).
	* @param stddev Standard deviation for Gaussian smoothing.(default: 1.0, min: 0.0, max = 3.0)
	* @param thr0 First threshold for edge detection.(default: 5.0, min: 0, max: 50)
	* @param thr1 Second threshold for edge detection.(default: 15.0, min: 0, max: 50)
	* @return result 
	*/
	bool Detect(
		const cv::Mat& image, 
		double stddev = 1.6, 
		double thr0 = 5.0, 
		double thr1 = 15.0
	);

	std::vector<std::vector<cv::Point>> getIntEdge() const;
	std::vector<std::vector<cv::Point2f>> getFloatEdge() const;
private:
	std::vector<std::vector<cv::Point2f>> m_resEdges;
};

#endif//__JIAOSHUI_SUBPIXEL_EDGE_H__
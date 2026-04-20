#ifndef __JIAOSHUI_HARRIS_CORNER_H__
#define __JIAOSHUI_HARRIS_CORNER_H__

#include "harris_common.h"
#include "opencv2\opencv.hpp"

#include <vector>

/*
* Harris Corner Detection Parameters
 * This structure defines the parameters for the Harris corner detection algorithm.
 * It includes settings for Gaussian convolution, gradient computation, corner strength measures,
 * and corner selection strategies. 
*/
typedef struct HarrisCornerParam{
	int Nscales;					// Apply zoom out on the image(1, 2, 4, 8, 16)
	GaussianType gaussian;			// Gaussian convolution method
	GradientType gradient;			// Strategy for computing the gradient
	CornerStyle measure;			// Types of corner strength functions
	float k;						// Harris constant (k: 0 ~ 0.2).
	float sigma_d;					// Gaussian standard deviation for smoothing (image denoising): 0 ~ 10.0
	float sigma_i;					// Gaussian standard deviation for smoothing (pixel neighborhood) : 0 ~ 20.0
	float threshold;				// Threshold for eliminating low values: 0 ~ 200.0
	CornerSelectStrategies strategy;// Strategy for selecting the output corners
	int cells;						// in case of distributed corners, this is the number of cells in each dimension: 1 ~ 50
	int Nselect;					// in case of N corners, this is the number of corners to select: 0 ~ 10000
	InterpolationType precision;	// Strategy for subpixel accuracy
	HarrisCornerParam()
		: Nscales(1)
		, gaussian(FAST_GAUSSIAN)
		, gradient(CENTRAL_DIFFERENCES)
		, measure(SHI_TOMASI_MEASURE)
		, k(0.06f)
		, sigma_d(1.0f)
		, sigma_i(2.5f)
		, threshold(10.0f)
		, strategy(ALL_CORNERS)
		, cells(3)
		, Nselect(1000)
		, precision(QUADRATIC_APPROXIMATION)
	{
	}
} harrisCornerParam;

class HarrisCornerDetector
{
public:
	HarrisCornerDetector();
	virtual ~HarrisCornerDetector();
	std::vector<cv::Point2f> DetectShiTomasi(const cv::Mat& src, float threshold = 10.0f);
	std::vector<cv::Point2f> DetectHarris(const cv::Mat& src, float k = 0.06f, float threshold = 100.0f);
	std::vector<cv::Point2f> DetectHarmonicMean(const cv::Mat& src, float threshold = 15.0f);
	float GetCornerStrength(int index) const 
	{
		if (index < 0 || index >= static_cast<int>(m_cornerStrengths.size())) {
			return 0.0f; // Invalid index
		}
		return m_cornerStrengths[index];
	}
private:
	std::vector<cv::Point2f> Detect(const cv::Mat& src);
private:
	HarrisCornerParam m_param;
	std::vector<cv::Point2f> m_corners;
	std::vector<float> m_cornerStrengths;
};

#endif//__JIAOSHUI_HARRIS_CORNER_H__
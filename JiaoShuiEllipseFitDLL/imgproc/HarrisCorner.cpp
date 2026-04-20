#include "HarrisCorner.h"
#include "harris_corner.h"

HarrisCornerDetector::HarrisCornerDetector() 
	: m_param()
{
}	

HarrisCornerDetector::~HarrisCornerDetector()
{
}

std::vector<cv::Point2f> 
HarrisCornerDetector::DetectHarris(
	const cv::Mat& src, 
	float k,
	float threshold
) {
	m_param.Nscales = 1;
	m_param.gaussian = FAST_GAUSSIAN;
	m_param.gradient = CENTRAL_DIFFERENCES;		
	m_param.sigma_d = 1.0f;
	m_param.sigma_i = 2.5f;	
	m_param.strategy = ALL_CORNERS;
	m_param.cells = 3;
	m_param.Nselect = 1000;
	m_param.precision = QUADRATIC_APPROXIMATION;

	m_param.measure = HARRIS_MEASURE;
	m_param.k = k; // Common value for Harris corner detection
	m_param.threshold = threshold;

	return Detect(src);
}

std::vector<cv::Point2f> 
HarrisCornerDetector::DetectShiTomasi(
	const cv::Mat& src,
	float threshold
) {
	m_param.Nscales = 1;
	m_param.gaussian = FAST_GAUSSIAN;
	m_param.gradient = CENTRAL_DIFFERENCES;	
	m_param.k = 0.06f;
	m_param.sigma_d = 1.0f;
	m_param.sigma_i = 2.5f;	
	m_param.strategy = ALL_CORNERS;
	m_param.cells = 3;
	m_param.Nselect = 1000;
	m_param.precision = QUADRATIC_APPROXIMATION;

	m_param.measure = SHI_TOMASI_MEASURE;
	m_param.threshold = threshold;

	return Detect(src);
}

std::vector<cv::Point2f> 
HarrisCornerDetector::DetectHarmonicMean(
	const cv::Mat& src, 
	float threshold
) {
	m_param.Nscales = 1;
	m_param.gaussian = FAST_GAUSSIAN;
	m_param.gradient = CENTRAL_DIFFERENCES;
	m_param.k = 0.06f;
	m_param.sigma_d = 1.0f;
	m_param.sigma_i = 2.5f;
	m_param.strategy = ALL_CORNERS;
	m_param.cells = 3;
	m_param.Nselect = 1000;
	m_param.precision = QUADRATIC_APPROXIMATION;

	m_param.measure = HARMONIC_MEAN_MEASURE;
	m_param.threshold = threshold;

	return Detect(src);
}

std::vector<cv::Point2f> 
HarrisCornerDetector::Detect(
	const cv::Mat& src
) {
	m_corners.clear();
	m_cornerStrengths.clear();

	std::vector<cv::Point2f> corners;
	const int w = src.cols;
	const int h = src.rows;

	if (src.empty()) {
		return corners;
	}

	// Convert to grayscale if necessary
	cv::Mat gray;
	if (src.channels() == 3) {
		cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	} else {
		gray = src.clone();
	}

	float* grayData = new float[w*h];

	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			grayData[i * w + j] = static_cast<float>(gray.at<uchar>(i, j));
		}
	}
	std::vector<harris_corner> harrisCorners;
	// Call the Harris corner detection function
	harris_scale(
		grayData, harrisCorners, 
		m_param.Nscales, m_param.gaussian, m_param.gradient,
		m_param.measure, m_param.k, m_param.sigma_d, m_param.sigma_i,
		m_param.threshold, m_param.strategy, m_param.cells, m_param.Nselect,
		m_param.precision, w, h
	);

	delete[] grayData;
	
	std::sort(
		harrisCorners.begin(),
		harrisCorners.end(), 
		[](const harris_corner& a, const harris_corner& b) {
			return a.R > b.R; 
		}
	);

	for (const auto& corner : harrisCorners) {
		m_corners.emplace_back(corner.x, corner.y);
		m_cornerStrengths.push_back(corner.R);
		corners.push_back(cv::Point2f(corner.x, corner.y));
	}	
	return corners;
}
//.EOF
#include "detect_features.h"
#include "../cvLib/cvcommon.h"

template<typename T>
T clamp(const T& v, const T& lo, const T& hi) {
	return (v < lo) ? lo : (v > hi) ? hi : v;
}

void get_harris_corners(
	const cv::Mat& gray,
	std::vector<cv::Point>& points,
	std::vector<float>& scores,
	int maxCorners,
	double qualityLevel,
	double minDistance, 
	int blockSize, 
	bool useHarrisDetector,
	double k
) {
	// cv::cornerHarris(gray, dst, 2, 3, 0.04);  // blockSize=2, aperture=3, k=0.04
	//cv::normalize(gray, gray, 0, 255, cv::NORM_MINMAX);
	// cv::threshold(dst_norm, dst_norm_scaled, 0.01 * 255, 255, cv::THRESH_BINARY);
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.5);
	// Detect corners using Shi-Tomasi method	
	std::vector<cv::Point2f> corners;
	cv::Mat harrisResponse;
	int apertureSize = 3;
	cv::cornerHarris(gray, harrisResponse, blockSize, apertureSize, k);
	cv::Mat harrisNormalized;
	cv::normalize(harrisResponse, harrisNormalized, 0, 255, cv::NORM_MINMAX, CV_32FC1);
	cv::goodFeaturesToTrack(gray, corners, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize, useHarrisDetector, k);
	cv::cornerSubPix(
		gray, // Input image
		corners, // Vector of corners (input and output)
		cv::Size(5, 5), // Half side length of search window
		cv::Size(-1, -1), // Half side length of dead zone (-1=none)
		cv::TermCriteria(
			cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
			40, // Maximum number of iterations
			0.001 // Minimum change per iteration
		)
	);
	// Filter corners based on their score
	for (const auto& corner : corners) {
		float score = harrisNormalized.at<float>(cvRound(corner.y), cvRound(corner.x));		
		points.push_back(corner);
		scores.push_back(score);
	}
}

void detect_corner_points(
	const cv::Mat& src, 
	std::vector<cv::Point2f>& _outCorners, 
	int maxCorners, 
	double qualityLevel, 
	double minDistance, 
	int blockSize, 
	bool useHarrisDetector, 
	double k
) {
	// Convert to grayscale if the image is not already in grayscale
	cv::Mat gray;
	if (src.channels() == 3)
	{
		cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
	}
	else
	{
		gray = src.clone();
	}
	std::vector<cv::Point> corners;
	std::vector<float> scores;
	get_harris_corners(gray, corners, scores, maxCorners, qualityLevel, minDistance, blockSize, useHarrisDetector, k);
	cv::Mat norm;
	cv::equalizeHist(gray, norm);
	get_harris_corners(norm, corners, scores, maxCorners, qualityLevel, minDistance, blockSize, useHarrisDetector, k);

	int n = (int)corners.size();
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			if (scores[i] < scores[j]) {
				std::swap(corners[i], corners[j]);
				std::swap(scores[i], scores[j]);
			}
		}
	}
#if DEBUG_IMG
	cv::Mat image1 = src.clone();
	n = (int)corners.size();
	for (int i = 0; i < n; i++) {
		auto pt = corners[i];
		cv::drawMarker(image1, pt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 10, 1);
		cv::putText(image1, std::to_string(scores[i]), pt, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
	}

	SaveDebugImg("corners0", image1);
#endif
	// Filter corners based on their score
	for (const auto& corner : corners) {
		bool res = is_valid_droplet_corner(gray, corner);
		if (!res)
			continue;
		_outCorners.push_back(corner);
	}
#if DEBUG_IMG
	cv::Mat image = src.clone();
	for (const auto& pt : _outCorners) {
		cv::drawMarker(image, pt, cv::Scalar(0, 255, 0), cv::MARKER_TILTED_CROSS, 10, 1);
	}
	SaveDebugImg("corners1", image);
#endif
}

void score_droplet_corner(
	const cv::Mat& gray, 
	const cv::Point2f& pt,
	const cv::Point2f& leftRef, 
	const cv::Point2f& rightRef,
	MainShapeType mainShape,  // not use still, but I think it may be useful in the future
	double& _outLeftScore,
	double& _outRightScore,
	float cornerScore,
	bool bHasLimit, 
	int blockSize
) {
	_outLeftScore = _outRightScore = -1.0;
	if (gray.empty())
		return;

	int half = blockSize / 2;
	int x1 = std::max(0, static_cast<int>(pt.x) - half);
	int y1 = std::max(0, static_cast<int>(pt.y) - half);
	int x2 = std::min(gray.cols - 1, static_cast<int>(pt.x) + half);
	int y2 = std::min(gray.rows - 1, static_cast<int>(pt.y) + half);

	if (x2 <= x1 || y2 <= y1) 
		return;

	cv::Mat roi = gray(cv::Rect(x1, y1, x2 - x1 + 1, y2 - y1 + 1));
	cv::Point ptInRoi((int)pt.x - x1, (int)pt.y - y1);
	auto maxDis = std::abs(rightRef.x - leftRef.x);
	// Reject if too bright (e.g. due to reflections)
	double meanIntensity = cv::mean(roi)[0];
	if (meanIntensity > 180) 
		return;

	int splitY = static_cast<int>(pt.y) - y1;
	if (splitY <= 1 || splitY >= roi.rows - 2)
		return;

	cv::Mat upper = roi(cv::Range(0, splitY), cv::Range::all());
	cv::Mat lower = roi(cv::Range(splitY, roi.rows), cv::Range::all());

	float darkUpperRatio = 1.0f - (cv::countNonZero(upper > 100) / (float)upper.total());
	float darkLowerRatio = 1.0f - (cv::countNonZero(lower > 100) / (float)lower.total());
	float darkSum = darkUpperRatio + darkLowerRatio;

	// Enforce darkness constraints
	if (darkUpperRatio <= 0.1f || darkUpperRatio >= 0.8f) 
		return;
	if (darkLowerRatio > 1.0f)
		return;
	if (darkLowerRatio <= 0.5f)
		return;
	/*if (darkLowerRatio <= darkUpperRatio) {
		return;
	}*/	
	
	if (darkSum >= 1.8f) 
		return;

	// Gradient magnitude
	cv::Mat gradX, gradY, mag, angle;
	cv::Sobel(roi, gradX, CV_32F, 1, 0);
	cv::Sobel(roi, gradY, CV_32F, 0, 1);
	//cv::magnitude(gradX, gradY, mag);
	cv::cartToPolar(gradX, gradY, mag, angle, true); // Convert to polar coordinates

	auto magVal = mag.at<float>(ptInRoi); // Ignore the corner point itself
	auto angleVal = angle.at<float>(ptInRoi); // Ignore the corner point itself
	auto edge_val = (float)cv::mean(mag)[0];
	const float edgeStrength = std::min((float)edge_val / 100.f, 1.f); // normalized

	// Distance-based closeness boost
	const float distLeft = (float)cv::norm(pt - leftRef);
	const float distRight = (float)cv::norm(pt - rightRef);
	const float proximityLeft = 1.0f - std::min(distLeft / maxDis, 1.0f);
	const float proximityRight = 1.0f - std::min(distRight / maxDis, 1.0f);

	_outLeftScore = std::min(1.0, std::max(0.0, 0.3f * edgeStrength + 0.4 * proximityLeft + 0.3 * cornerScore));
	_outRightScore = std::min(1.0, std::max(0.0, 0.3f * edgeStrength + 0.4 * proximityRight + 0.3 * cornerScore));
}


bool is_valid_droplet_corner(const cv::Mat& gray, const cv::Point2f& pt, int blockSize)
{
	if (gray.empty() || pt.x < 0 || pt.y < 0 || pt.x >= gray.cols || pt.y >= gray.rows)
		return false;
		
	cv::Mat norm;
	cv::Mat enhanced;
	cv::equalizeHist(gray, enhanced);
	cv::adaptiveThreshold(enhanced, norm, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
		cv::THRESH_BINARY_INV, 21, 5);
#if DEBUG_IMG
	SaveDebugImg("equalizeHist", enhanced);
#endif
	// Step 2: Define ROI
	int half = blockSize / 2;
	int x1 = std::max(0, static_cast<int>(pt.x) - half);
	int y1 = std::max(0, static_cast<int>(pt.y) - half);
	int x2 = std::min(norm.cols - 1, static_cast<int>(pt.x) + half);
	int y2 = std::min(norm.rows - 1, static_cast<int>(pt.y) + half);

	if (x2 <= x1 || y2 <= y1)
		return false;

	cv::Rect roi1(x1, y1, x2 - x1 + 1, (int)pt.y - y1);
	cv::Rect roi2(x1, (int)pt.y, x2 - x1 + 1, y2 - (int)pt.y + 1);

	if(roi1.empty() || roi2.empty())
		return false;

	cv::Rect roi(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	if (roi.empty()) return false;

	// Step 3: Analyze brightness (upper vs lower)
	int splitY = static_cast<int>(pt.y);
	if (splitY <= y1 || splitY >= y2)
		return false;

	cv::Mat roiUpper = norm(cv::Rect(x1, y1, roi.width, splitY - y1));
	cv::Mat roiLower = norm(cv::Rect(x1, splitY, roi.width, y2 - splitY + 1));

	int brightUpper = cv::countNonZero(roiUpper > 180);
	int brightLower = cv::countNonZero(roiLower > 180);
	int totalUpper = roiUpper.rows * roiUpper.cols;
	int totalLower = roiLower.rows * roiLower.cols;
	int total = totalUpper + totalLower;
	int brightTotal = brightUpper + brightLower;

	double ratioUpper = static_cast<double>(brightUpper) / totalUpper;
	double ratioLower = static_cast<double>(brightLower) / totalLower;
	double ratioTotal = static_cast<double>(brightTotal) / total;

	if (ratioTotal >= 0.43 || ratioTotal < 0.01)
		return false;
	if (ratioLower >= 0.43)
		return false;

	// Step 4: Gradient strength check
	cv::Mat gradX, gradY, magnitude;
	cv::Sobel(norm(roi), gradX, CV_32F, 1, 0, 3);
	cv::Sobel(norm(roi), gradY, CV_32F, 0, 1, 3);
	cv::magnitude(gradX, gradY, magnitude);

	double meanGradient = cv::mean(magnitude)[0];
	if (meanGradient < 25.0)  // adjustable threshold
		return false;

	return true;
}

#include "EdgeSlope.h"

SlopeIntegral::SlopeIntegral()
	: m_wndowRadius(4)
{
}

SlopeIntegral::~SlopeIntegral()
{
}

// Sub-pixel smoothing with a least-squares sliding window
std::vector<cv::Point2f> 
SlopeIntegral::subpixelSmoothFit(
	const std::vector<cv::Point>& pts,
	int windowRadius// total window = 2r + 1
) const {
	std::vector<cv::Point2f> refined;
	const int N = (int)pts.size();
	refined.reserve(N);

	for (int i = 0; i < N; ++i) {
		int start = std::max(0, i - windowRadius);
		int end = std::min(N - 1, i + windowRadius);
		int count = end - start + 1;

		// Fit x = a*y + b or y = a*x + b depending on orientation
		double sumX = 0, sumY = 0, sumX2 = 0, sumXY = 0;

		for (int j = start; j <= end; ++j) {
			double x = pts[j].x;
			double y = pts[j].y;
			sumX += x;
			sumY += y;
			sumX2 += x * x;
			sumXY += x * y;
		}

		double meanX = sumX / count;
		double meanY = sumY / count;
		double denom = sumX2 - sumX * meanX;

		double a = 0, b = 0;
		if (std::abs(denom) > 1e-6) {
			a = (sumXY - sumX * meanY) / denom;
			b = meanY - a * meanX;
		}

		// Predict smoothed point using local x
		double x = pts[i].x;
		double y = a * x + b;

		refined.emplace_back((float)x, (float)y);
	}

	return refined;
}

void SlopeIntegral::build(const std::vector<cv::Point>& pts, int wndRadius)
{
	int n = (int)pts.size();
	std::vector<cv::Point2f> smoothPts = subpixelSmoothFit(pts, wndRadius);
	m_wndowRadius = wndRadius;

	sumX.assign(n + 1, 0.0);
	sumY.assign(n + 1, 0.0);
	sumXX.assign(n + 1, 0.0);
	sumXY.assign(n + 1, 0.0);

	for (int i = 0; i < n; ++i) {
		double x = smoothPts[i].x, y = smoothPts[i].y;
		sumX[i + 1] = sumX[i] + x;
		sumY[i + 1] = sumY[i] + y;
		sumXX[i + 1] = sumXX[i] + x * x;
		sumXY[i + 1] = sumXY[i] + x * y;
	}
}

cv::Point2f SlopeIntegral::fitDirection(int i, int j) const
{
	int n = j - i;
	if (n < 2 || i < 0 || j >= (int)sumX.size()) return { 0, 0 };

	double Sx = sumX[j] - sumX[i];
	double Sy = sumY[j] - sumY[i];
	double Sxx = sumXX[j] - sumXX[i];
	double Sxy = sumXY[j] - sumXY[i];

	double denom = n * Sxx - Sx * Sx;
	if (std::abs(denom) < 1e-6)
		return { 0, 0 }; // vertical or degenerate

	double slope = (n * Sxy - Sx * Sy) / denom;
	cv::Point2f dir(1.0f, (float)slope);
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
	if (len < 1e-6)
		return { 0, 0 };
	return dir * (1.0f / len);
}

double SlopeIntegral::getSlopeAt(
	int centerIdx,
	bool getActualSlope
) {
	int i = centerIdx - m_wndowRadius;
	int j = centerIdx + m_wndowRadius;
	double angle = 0.0;
	if (i < 0 || j >= (int)sumX.size())
		return angle;

	cv::Point2f dir1 = fitDirection(i, centerIdx);
	cv::Point2f dir2 = fitDirection(centerIdx, j);

	if (cv::norm(dir1) < 1e-3 || cv::norm(dir2) < 1e-3)
		return angle;

	if (getActualSlope) {
		// For debugging: return actual slope in degrees
		double slope = dir2.y / dir2.x;
		angle = std::atan(slope) * 180.0 / CV_PI;
	}
	else {
		double dot = dir1.dot(dir2);
		dot = std::min(1.0, std::max(dot, -1.0));
		angle = std::acos(dot) * 180.0 / CV_PI;
	}

	return angle;
}

//.EOF
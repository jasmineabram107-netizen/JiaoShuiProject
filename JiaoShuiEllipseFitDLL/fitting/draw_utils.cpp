#include "draw_utils.h"


void drawPolynomial(
	const Eigen::VectorXd& coeffs,
	int center_x, 
	cv::Mat& img
) {
	for (int x = std::max(center_x - 20, 0); x < std::min(center_x + 20, img.cols); x++) {
		int yLeft = (int)polyEval(coeffs, (double)x);
		if (yLeft >= 0 && yLeft < img.rows) {
			img.at<cv::Vec3b>(cv::Point(x, yLeft)) = cv::Vec3b(0, 0, 255);
		}

	}
}

void drawPolynomialY(
	const Eigen::VectorXd& coeffs,
	int y1, 
	int y2, 
	cv::Mat& img,
	bool isLeft,
	cv::Vec3b clr
) {
	auto startY = std::max(y1, 0);
	auto endY = std::min(y2, img.rows);
#if POLYGON_LINE_MODE
	cv::Point lastPt;
	bool isLine = false;
#endif
	for (int y = startY; y < endY; y++) {
		int x = (int)polyEval(coeffs, (double)y);
		cv::Point pt(x, y);
#if POLYGON_LINE_MODE
		if (x >= 0 && x < img.cols) {
			if (isLine) {
				cv::line(img, lastPt, pt, clr, 1); // Draw line from last point to current point				
			}
			else {
				img.at<cv::Vec3b>(pt) = clr;
			}
			isLine = true;
			lastPt = pt;
		}
		else {
			isLine = false;
		}
#else
		if (x >= 0 && x < img.cols) {
			img.at<cv::Vec3b>(cv::Point(x, y)) = clr;
		}
#endif
	}
}
// Added
void getPolynomialY(
	const Eigen::VectorXd& coeffs,
	int y1,
	int y2,
	int width,
	int height,
	bool isLeft,
	std::vector<cv::Point>& vecPoints
) {
	auto startY = std::max(y1, 0);
	auto endY = std::min(y2, height);
	for (int y = startY; y < endY; y++) {
		int x = (int)polyEval(coeffs, (double)y);
		cv::Point pt(x, y);
		if (x >= 0 && x < width) {
			vecPoints.push_back(pt);
		}
	}
}

void drawPolynomialYInverse(
	const Eigen::VectorXd& coeffs, 
	int y1, 
	int y2, 
	cv::Mat& img,
	int move_y
) {
	for (int y = std::max(y1, 0); y < std::min(y2, img.rows); y++) {
		int yy = y - move_y;
		int x = (int)polyEval(coeffs, (double)yy);
		if (x >= 0 && x < img.cols) {
			img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(0, 0, 255);
		}

	}
}

void drawPolynomialYReverse(
	const Eigen::VectorXd& coeffs, 
	int y1, 
	int y2, 
	cv::Mat& img,
	bool isLeft,
	cv::Vec3b clr
) {
	auto startY = std::max(y1, 0);
	auto endY = std::min(y2, img.rows);
	int width = img.cols;
#if POLYGON_LINE_MODE
	cv::Point lastPt;
	bool isLine = false;
#endif
	for (int y = startY; y < endY; y++) {
		int x = width - (int)polyEval(coeffs, (double)y);
		cv::Point pt(x, y);
#if POLYGON_LINE_MODE
		if (x >= 0 && x < img.cols) {
			if (isLine) {
				cv::line(img, lastPt, pt, clr, 1); // Draw line from last point to current point				
			}
			else {
				img.at<cv::Vec3b>(pt) = clr;
			}
			isLine = true;
			lastPt = pt;
		}
		else {
			isLine = false;
		}
#else
		if (x >= 0 && x < img.cols) {
			img.at<cv::Vec3b>(pt) = clr;
		}
#endif		
	}
}

// Added
void getPolynomialYReverse(
	const Eigen::VectorXd& coeffs,
	int y1,
	int y2,
	int width,
	int height,
	bool isLeft, 
	std::vector<cv::Point>& vecPoints
) {
	auto startY = std::max(y1, 0);
	auto endY = std::min(y2, height);

	for (int y = startY; y < endY; y++) {
		int x = width - (int)polyEval(coeffs, (double)y);
		cv::Point pt(x, y);
		if (x >= 0 && x < width) {
			vecPoints.push_back(pt);
		}
	}
}

void drawPolynomialPolar(
	const Eigen::VectorXd& coeffs, 
	double theta1, 
	double theta2, 
	double xc, 
	double yc, 
	cv::Mat& img,
	bool isLeft,
	cv::Vec3b clr
) {
	auto step = (theta2 - theta1) / 100.0;
	auto start = isLeft ? theta1- step : theta1;
	auto end = isLeft ? theta2 : theta2 + step;
#if POLYGON_LINE_MODE
	cv::Point lastPt;
	bool isLine = false;
#endif
	for (double angle = start; angle < end; angle += step) {
		int x = (int)(xc + cos(angle) * polyEval(coeffs, angle));
		int y = (int)(yc + sin(angle) * polyEval(coeffs, angle));
		cv::Point pt(x, y);
#if POLYGON_LINE_MODE
		if (x >= 0 && x < img.cols && y >= 0 && y < img.rows) {
			if (isLine) {
				cv::line(img, lastPt, pt, clr, 1); // Draw line from last point to current point				
			}
			else {
				img.at<cv::Vec3b>(pt) = clr;
			}
			isLine = true;
			lastPt = pt;
		}
		else {
			isLine = false;
		}
#else
		if (x >= 0 && x < img.cols && y >= 0 && y < img.rows) {
			img.at<cv::Vec3b>(pt) = clr;
		}
#endif
	}
}

// Added 
void getPointsPolynomialPolar(
	const Eigen::VectorXd& coeffs,
	double theta1,
	double theta2,
	double xc,
	double yc,
	int width,
	int height,
	bool isLeft,
	std::vector<cv::Point>& vecPoints
) {
	auto step = (theta2 - theta1) / 100.0;
	auto start = isLeft ? theta1 - step : theta1;
	auto end = isLeft ? theta2 : theta2 + step;
	for (double angle = start; angle < end; angle += step) {
		int x = (int)(xc + cos(angle) * polyEval(coeffs, angle));
		int y = (int)(yc + sin(angle) * polyEval(coeffs, angle));
		cv::Point pt(x, y);
		if (x >= 0 && x < width && y >= 0 && y < height) {
			vecPoints.push_back(pt);
		}
	}
}

void drawPolynomialPolarX(
	const Eigen::VectorXd& coeffs, 
	double theta1,
	double theta2, 
	double xc, 
	double yc, 
	cv::Mat& img
) {
	for (double angle = theta1; angle < theta2; angle += (theta2 - theta1) / 100) {
		int x = (int)(xc + cos(M_PI - angle) * polyEval(coeffs, angle));
		int y = (int)(yc + sin(M_PI - angle) * polyEval(coeffs, angle));
		if (x >= 0 && x < img.cols && y >= 0 && y < img.rows)
			img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(0, 0, 255);
	}
}

void drawTangentLine2(
	cv::Mat& img, 
	const cv::Point2f& point, 
	double xx, double yy, 
	const cv::Scalar& color, 
	int length
) {
	cv::Point2f p1(4.0f * (point.x + (float)length * (float)xx), 4.0f * (point.y + (float)length * (float)yy));
	cv::Point2f p2(4.0f * (point.x - (float)length * (float)xx / 2.0f), 4.0f * (point.y - (float)length * (float)yy / 2.0f));
	cv::line(img, p1, p2, color, 1, 8, 2);
}
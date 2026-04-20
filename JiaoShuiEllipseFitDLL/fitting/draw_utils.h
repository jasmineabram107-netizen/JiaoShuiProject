#ifndef __DRAW_UTILS_H__
#define __DRAW_UTILS_H__
#include <opencv2/opencv.hpp>
#include "fitting_defines.h"
#include "ellipses_util.h"
#include "common.h"

#define POLYGON_LINE_MODE  0

void drawPolynomial(const Eigen::VectorXd& coeffs, int center_x, cv::Mat& img);
void drawPolynomialY(
	const Eigen::VectorXd& coeffs, 
	int y1, 
	int y2, 
	cv::Mat& img,
	bool isLeft = true,
	cv::Vec3b clr = cv::Vec3b(0, 0, 255)
);

void drawPolynomialYInverse(const Eigen::VectorXd& coeffs, int y1, int y2, cv::Mat& img, int move_y);
void drawPolynomialYReverse(
	const Eigen::VectorXd& coeffs, 
	int y1, 
	int y2, 
	cv::Mat& img, 
	bool isLeft = true,
	cv::Vec3b clr = cv::Vec3b(0, 0, 255)
);

void drawPolynomialPolar(
	const Eigen::VectorXd& coeffs,
	double theta1, 
	double theta2, 
	double xc, 
	double yc, 
	cv::Mat& img, 
	bool isLeft = true,
	cv::Vec3b clr = cv::Vec3b(0, 0, 255)
);
void drawPolynomialPolarX(const Eigen::VectorXd& coeffs, double theta1, double theta2, double xc, double yc, cv::Mat& img);
void drawTangentLine2(cv::Mat& img, const cv::Point2f& point, double xx, double yy, const cv::Scalar& color, int length);

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
);

void getPolynomialYReverse(
	const Eigen::VectorXd& coeffs,
	int y1,
	int y2,
	int w,
	int h,
	bool isLeft,
	std::vector<cv::Point>& vecPoints
);

void getPolynomialY(
	const Eigen::VectorXd& coeffs,
	int y1,
	int y2,
	int width,
	int height,
	bool isLeft,
	std::vector<cv::Point>& vecPoints
);
#endif//__DRAW_UTILS_H__
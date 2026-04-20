#pragma once
#include <vector>
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include "fitting_defines.h"
#include "common.h"

// Class for Ellipse
class DropletEllipse {
public:
    double h, k; // Centroid
    double a, b; // Major and minor radius
    double theta; // Inclination angle

    DropletEllipse();

    DropletEllipse(double h, double k, double a, double b, double theta);

    Eigen::Vector2d getPoint(double t) const;

    Eigen::Vector2d getGradient(double t) const;

    // Function to calculate the slope of the tangent line to an ellipse at a point (x, y), uncosider the rotation
    double tangentSlope(double x, double y) const;

    // Function to calculate the slope of the tangent line to an ellipse at a point (x, y), consider the rotation
    void getTangentVector(double x, double y, double& tx, double& ty) const;

    // Apply the rotation to convert the coordinates
    void rotate(double x, double y, double& x_prime, double& y_prime) const;
    double signedDistanceToEllipse(double x, double y) const;
};

// Class for Intersection and Tangent Calculation
class IntersectionCalculator {
public:
    DropletEllipse ellipse1, ellipse2;

    IntersectionCalculator(const DropletEllipse& e1, const DropletEllipse& e2);
    Eigen::Vector2d solveIntersection(Eigen::Vector2d initial_guess, int max_iter = 5000, double tol = 1e-6);
    double calculateAngle(double slope1,double slope2) const;

private:
    Eigen::Vector2d equations(Eigen::Vector2d params) const;
};

/**
* Find intersection points between two ellipses
* 
* ellipse 1 = 液滴, ellipse 2 = 基线
* ellipse 1's center (cx1, cy1), long radius a1, short radius b1, inclined theta1
* ellipse 2's center (cx1, cy1), long radius a1, short radius b1, inclined theta1
* 
*/
std::vector<PointAndAngle> 
findIntersectionsEllipse(
    EllipseParams ep1,
    double theta1, 
    EllipseParams ep2, 
    double theta2,
    bool checkMode = false,
    const cv::Point2f* left = NULL,
    const cv::Point2f* right = NULL
);

PointAndAngle
findIntersectionEllipseOne(
    const cv::Point2f& upcenter,
    const cv::Size2f& upRadius,
    double theta1,
    const cv::Point2f& dnCenter, 
    const cv::Size2f& dnRadius,
    double theta2, 
    const cv::Point2f& p0
);

/**
* Find intersection points between two circles
*
* circle 1's center (x1, y1), radius r1
* circle 2's center (x2, y2), radius r2
*
*/
std::vector<PointAndAngle> 
getCircleIntersections(
    double x1,
    double y1,
    double r1, 
    double x2,
    double y2, 
    double r2
);

/**
* Find intersection points between circle and line
*
* circle center (cx, cy), radius r
* (x1, y1), (x2, y2)
*
*/
std::vector<PointAndAngle> 
findIntersectionsCircleLine(
    const cv::Point2f& center,
    double r,
    const cv::Point2f& left,
    const cv::Point2f& right,
    int isSelect = 0 // 0: all, 1: left, 2: right
);

/**
* Find intersection points between ellipse and line
*
* ellipse center (cx, cy), semi-major a, semi-minor b, angle theta 
* (x1, y1), (x2, y2)
*
*/
std::vector<PointAndAngle> 
findIntersectionsEllipseLine(
    const cv::Point2f& center,
    const cv::Size2f& axes,
    double theta,
    const cv::Point2f& left,
    const cv::Point2f& right,
    int isSelect = 0 // 0: all, 1: left, 2: right
);

/**
* Find intersection points between two circles passing through 3 points
*
*/
std::vector<PointAndAngle>
findCirclePointIntersections(
    double h1, 
    double k1,
    double r1,
    double h2,
    double k2, 
    double r2
);

/** Function to calculate the coefficients of a circle given three points
*   return center_x, center_y, radius
*/
std::tuple<double, double, double>
findCircleCoefficients(
    const MPoint& p1,
    const MPoint& p2,
    const MPoint& p3
);

// Function to solve the system of linear equations for ellipse coefficients
Eigen::VectorXd 
solveEllipseCoefficients(
    const double *p_xs,
    const double *p_ys
); 

double 
calculateEllipseTangentLineSlope(
    double a, 
    double b,
    double theta,
    double x, 
    double y
);

Eigen::VectorXd 
polynomialCurveFit(
    const std::vector<double>& x, 
    const std::vector<double>& y,
    int degree
);

// x = a*y^2 + b*y + c
Eigen::VectorXd
polynomialCurveFitY(
    const std::vector<double>& x, 
    const std::vector<double>& y, 
    int degree
);

Eigen::VectorXd 
polynomialCurveFitYHouseholderQR(
    const std::vector<double>& x, 
    const std::vector<double>& y, 
    int degree
);

std::vector<PointAndAngle> 
findIntersectionPolyEllipse(
    Eigen::VectorXd coeffs, 
    const cv::Point2f& c, 
    const cv::Size2f& r,
    double theta, 
    double init_x
);

// Function to calculate the slope of the polynomial at a given angle on polar coordinate

/*
* Function to calculate the slope of the polynomial at a given angle on polar coordinate
* @param coeffs: polynomial coefficients in the form of r = a0 + a1*cos(theta) + a2*cos^2(theta) + ...
* @param angle: angle in radians
* @return: slope of the polynomial at the given angle
*/
double polySlopePolarCoordinate(Eigen::VectorXd coeffs, double angle);

// Evaluate the polynomial at theta (r = f(theta))
double evalPoly(const Eigen::VectorXd& coefs, double theta);

// Evaluate the polynomial derivative at theta(dr / dtheta)
double evalPolyDeriv(const Eigen::VectorXd& coefs, double theta);

/*
* Function to extract ellipse parameters from polynomial coefficients
* @param coeffs: polynomial coefficients in the form of r = a0 + a1*cos(theta) + a2*cos^2(theta) + ...
* @param p: output ellipse parameters (center, semi-major axis, semi-minor axis)
* @param theta: output angle of the ellipse in radians
* @return: true if successful, false if the coefficients do not represent an ellipse
*/
bool extractEllipseParams(const Eigen::VectorXd& coeffs, EllipseParams& p, double& theta);

// Function to calculate the slope of the polynomial at a given x
double polySlope(const Eigen::VectorXd& coeffs, double x);

double polyEval(const Eigen::VectorXd& coeffs, double x);

cv::Point2f polarToCartesian(double r, double theta, double xc, double yc);

/**
*
* fitMethod : 0 => Least Square, 1=> Approximate Mean Sqaure(AMS), 2=> Direct least square(Driect) Method
*/
void ellipse_regression_OpenCV(
    const std::vector<cv::Point2f>& contour,
    int fitMethod,
    const cv::Point2f& left,
    const cv::Point2f& right,
    cv::RotatedRect& _outBox
);

void ellipse_regression_Circle(
    const std::vector<cv::Point2f>& pts_up,
    int dimension,
    cv::RotatedRect& box
);

/*
* Function to calculate the determinant of a 3x3 matrix
* @param a1, b1, c1: first row of the matrix
* @param a2, b2, c2: second row of the matrix
* @param a3, b3, c3: third row of the matrix
* @return: determinant of the matrix
*/
double calc_3d_det(
    double a1, double b1, double c1,
    double a2, double b2, double c2,
    double a3, double b3, double c3
);

/*
    * 计算水平面上的接触角
    * @param theta0: 极坐标系下的起始角度
    * @param theta1: 极坐标系下的结束角度
    * @param xc: 曲线中心点x坐标
    * @param yc: 曲线中心点y坐标
    * @param coefs: 多项式系数
    * @param isLeft: 是否为左侧接触角
    * @return: 返回计算得到的接触角和相关点信息
    */
PointAndAngle calculate_polar_angles_on_horizontal(
    const cv::Point2f& left,
    const cv::Point2f& right,
    double theta0,
    double theta1,
    double xc,
    double yc,
    const Eigen::VectorXd& coefs,
    bool isLeft
);

/*
* 计算曲面上的接触角
* @param theta0: 极坐标系下的起始角度
* @param theta1: 极坐标系下的结束角度
* @param xc: 曲线中心点x坐标
* @param yc: 曲线中心点y坐标
* @param coefs: 多项式系数
* @param ellipse: 液滴椭圆拟合结果
* @param isLeft: 是否为左侧接触角
* @return: 返回计算得到的接触角和相关点信息
*/
PointAndAngle calculate_polar_angles_on_curved(
    const cv::Point2f& left,
    const cv::Point2f& right,
    double theta0,
    double theta1,
    double xc,
    double yc,
    const Eigen::VectorXd& coefs,
    const DropletEllipse& ellipse,
    bool isLeft
);

PointAndAngle calculate_bipoly_angles_on_horizontal(
    const cv::Point2f& left,
    const cv::Point2f& right,
    double y0,
    double y1,    
    const Eigen::VectorXd& coefs,
    bool isLeft,
    int width,
    double &_ypos
);

PointAndAngle calculate_bipoly_angles_on_curved(
    double y0,
    double y1,
    const Eigen::VectorXd& coefs,
    const DropletEllipse& ellipse,
    bool isLeft,
    int width,
    double& _ypos
);
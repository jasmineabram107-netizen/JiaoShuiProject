#pragma once
#include <opencv2/opencv.hpp>
#include "fitting_defines.h"
#include "ellipses_util.h"
#include "common.h"

// #define PI_Value 3.14159265358979323846
#define POINT_DIS 7
// 基线拟合计算范围为凸面
#define BASEPOINT_MARGIN 140  
#define BASEPOINT_NEIGHBOR 300
#define POLYNOMIAL_DEGREE 5
#define POLYNOMIAL_DEGREE_POLAR 3
#define POINT_CNT 100
#define MARGIN_COUNT 50
///////////////////////////////////////////////////////////////

void correct_base_params(cv::Point2f& left,  cv::Point2f& right);
void correct_base_params(cv::Point& left, cv::Point& right);

void find_contours(
	const cv::Mat& img, 
	cv::Mat& gray, 
	cv::Mat& edges, 
	std::vector<std::vector<cv::Point>>& contours
);

int find_max_contour(
	const std::vector<std::vector<cv::Point>>& contours,
	std::vector<int>& traces
);

int find_max_contour(
	std::vector<std::vector<cv::Point>>& contours,
	int side,
	int cx,
	int cy
);

int find_next_contour(
	const std::vector<std::vector<cv::Point>>& contours,
	int min_dim,
	const cv::Point& pt_r,
	std::vector<int>& traces
);

int find_next_contour(
	std::vector<std::vector<cv::Point>>& contours, 
	int min_dim, 
	cv::Point& pt_r
);

void
collection(
	std::vector<std::vector<cv::Point>>& contours,
	int max_ind, int side, bool has_pin,
	const cv::Point& pt0,
	std::vector<cv::Point>& pts,
	std::vector<cv::Point>& pts_dn,
	int margin,
	MainShapeType lower_shape
);
void find_contour_points_for_pin(
	cv::Mat& img,
	const cv::Point& pt_l, const cv::Point& pt_r,
	std::vector<cv::Point>& pts_l,
	std::vector<cv::Point>& pts_r,
	std::vector<cv::Point>& pts_down,
	MainShapeType lower_shape
);

int check_contour_top(
	const std::vector<cv::Point>& contour,
	const cv::Point& pt_l,
	const cv::Point& pt_r
);

int find_contour_include_basepoints(
	const std::vector<std::vector<cv::Point>>& contours,
	std::vector<int>& traces,
	const cv::Point& pt_l,
	const cv::Point& pt_r,
	bool findOnlyConvex 
);
int find_next_contour(
	const std::vector<std::vector<cv::Point>>& contours,
	int min_dim,
	const cv::Point& pt_r,
	std::vector<int>& traces
);


bool near_to_point0(const cv::Point& pt, const cv::Point& pt0, int dis = POINT_DIS * POINT_DIS);
bool near_to_basepoint(MPoint& pt, const cv::Point& pt0, int dis);
bool near_to_basepoint(MPoint& pt, const cv::Point2f& pt0, int dis);

int distance(const cv::Point& pt, const cv::Point& pt0);
float distance(const cv::Point2f& pt, const cv::Point2f& pt0);
float distance(const cv::Point2f& pt, const cv::Point& pt0);
float distance(const MPoint& pt, const cv::Point2f& pt0);
double disSqrt(double x1, double y1, double x2, double y2);


int
find_include_contour(
	std::vector<std::vector<cv::Point>>& contours,
	const cv::Point& pt0
);

void checkAndUpdateBaseLine(
	cv::Mat& frame, 
	cv::Point& pt_l, 
	cv::Point& pt_r, 
	std::vector<cv::Point>& pts_up
);

void updateBaselinePoints(
	cv::Vec4f& line, 
	cv::Mat& frame, 
	std::vector<cv::Point> pts_up, 
	cv::Point& pt_l,
	cv::Point& pt_r
);

bool lineCircleIntersection(
	double x0, double y0,
	double vx, double vy,
	cv::Point circleCenter,
	double radius,
	std::vector<cv::Point2f>& intersections
);

double get_point_angle(const PointAndAngle& pa);
double angle_between_vectors(MPoint vec1, MPoint vec2);

/**
* return
* -1: not find
* > 0: yedi top index
*/
int check_contour_top(
	const std::vector<cv::Point>& contour,
	const cv::Point& pt_l,
	const cv::Point& pt_r
);
int getBaselinePoints_aomian(cv::Mat& img, int* points);

// Function to calculate the distance from point (x, y) to line through points (x1, y1) and (x2, y2)
double distanceToLine(double x, double y, double x1, double y1, double x2, double y2);
double distanceToLine(const cv::Point2f& pt, const cv::Point2f& p1, const cv::Point2f& p2);
void centerOfRotatedRectangle(double x1, double y1, double x2, double y2, double height, float& cx, float& cy);

int findNearestIndex(const std::vector<cv::Point>& contour, const cv::Point& ref);
int findNearestIndex(const std::vector<cv::Point2f>& contour, const cv::Point& ref);

int getDropletContourByLTI(
	const std::vector<cv::Point>& contour, 
	const cv::Point& pt_l, 
	const cv::Point& pt_r, 
	int dist2, 
	std::vector<cv::Point>& _validContour
);

void zhangSuenThinning(cv::Mat& im);
void zhangSuenThinningFast(cv::Mat& im);

bool sortSkeletonPoints(
	const std::vector<cv::Point2f>& input,
	const cv::Point2f& pt_l,
	const cv::Point2f& pt_r,
	std::vector<cv::Point2f>& output,
	int& leftCount,                        // ← new output param
	float connectThreshold = 2.5f,
	float endThreshold = 3.0f);

int traceSkeletonTopDown_withLeftCount(
	const std::vector<cv::Point>& skeletonPoints,
	const cv::Point& pt_l,
	const cv::Point& pt_r,
	int distThresholdSq,
	std::vector<cv::Point>& outPath,
	int& leftCount
);

std::vector<cv::Point> getNeighbors(
	const cv::Mat& img, 
	const cv::Point& pt, 
	int max_gap, 
	const cv::Mat& visited
);

int traceBranch(
	const cv::Mat& img,
	cv::Point start_pt,
	int max_gap,
	const cv::Point& _leftRef,
	const cv::Point& _rightRef,
	int distThres,
	cv::Mat& visited,
	std::vector<cv::Point>& _outContour
);

bool traceTwoBranchesFromThinnedImage(
	const cv::Mat& thinned_img, 
	std::vector<cv::Point>& _outLeft, 
	std::vector<cv::Point>& _outRight, 
	const cv::Point& _leftRef,
	const cv::Point& _rightRef,
	int distThres, 
	int max_gap = 1
);

std::vector<cv::Point> extractValidContours(
	const std::vector<cv::Point>& fullContour, 
	const cv::Point& refPt, 
	int distThr
);

double signedDistanceToLine(
	const cv::Point2f& pt, const cv::Point2f& A, const cv::Point2f& B
);
int is_convex_between(const std::vector<cv::Point>& contour, int idx_l, int idx_r, double tolerance_ratio = 0.25);
bool is_nearly_straight(
	const std::vector<cv::Point>& segment,
	double tolerance = 2.0
);
bool is_none_convex(
	const std::vector<cv::Point>& segment,
	const cv::Point& pt_l,
	const cv::Point& pt_r,
	double margin = 4.0
);
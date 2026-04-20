#ifndef __DETECT_UTILS_ON_HORIZONAL_H__
#define __DETECT_UTILS_ON_HORIZONAL_H__
#include <opencv2/opencv.hpp>
#include "common.h"

enum class StepDir { None, H, V, D, OppDiag };

struct FollowConfig {
    int  maxDepth = 10;  // lookahead
    int  maxNodes = 3000;
    int  detourBudget = 3;   // zero-projection on-edge weaves allowed
    int  minProjAccept = 2;   // minimum forward progress to accept
    int  lateralTol = 2;   // widen cone to permit opposite diagonal weave
    bool edgeOnly = true; // strict mode
    int  maxGapForward = 0;   // allow up to N blank pixels forward (set 1–2 to bridge)
    int  maxGapOrtho = 0;   // allow up to N blank pixels for 1px orthogonal then forward
};

enum ScanDir {
	eScanLeftDown = 0,  // scan left-down
	eScanRightDown = 1, // scan right-down
};

typedef enum LensContourType {
    eContourTooThin = -2,
    eTopThinSpecial = -1,
    eTopBoundary = 0,
    eBottomBoundary = 1,
    eNotBoundaryFound = 2
}lensContourType;



int horizontalDistanceToLine(
    const cv::Vec4i& line, 
    int cx
);

cv::Point2f unitDirectionVector(
    const cv::Vec4i& l
);

bool correctSkewByLines(
    const cv::Mat& img, 
    cv::Mat& res, 
    double& angle
);

void rotatePoints(
    int cols, 
    int rows, 
    double angle, 
    std::vector<cv::Point2f>& pts
);

#if !_DEV_UPDATE_SUBPXL_ON_HORIZONTAL
void find_contours_simple(
    const cv::Mat& img, 
    cv::Mat& gray, 
    cv::Mat& edges, 
    std::vector<std::vector<cv::Point>>& contours
);
#endif//_DEV_UPDATE_SUBPXL_ON_HORIZONTAL

int find_max_contour(
    const std::vector<std::vector<cv::Point>>& contours, 
    int flag
);

void collections_inner(
    const cv::Mat& edges, 
    int dx, 
    int dy, 
    int& x, 
    int& y, 
    bool second
);

/*
* @brief Filter horizontal segments based on edge points.
* @param edgePoints: input edge points
* @param candidatePoints: output candidate points
* @param yBottom: the bottom y-coordinate of the segment
* @param yBaseline: the baseline y-coordinate of the segment
* @param minLength: minimum length of the segment
* @param lengthRatio: ratio to determine if the segment is valid
*/
void filterHorizontalSegments(
    const std::vector<cv::Point>& edgePoints,
    std::vector<cv::Point>& candidatePoints,
    int yBottom,
    int yBaseline,
    int minLength = 6,
    int lengthRatio = 2
);

/*
* @brief Filter vertical segments based on edge points.
* @param pts: input edge points
* @param pt_opts: output candidate points
* @param minLength: minimum length of the segment
*/
void filterVerticalSegments(
    const std::vector<cv::Point>& pts,
    std::vector<cv::Point>& pt_opts,
    int minLength = 3
);


cv::Point findContactByFlatnessLoss(
    const std::vector<cv::Point>& pts,
    int windowSize = 5,
    double angleMargin = 15.0  // only run if last segment is flat
);


cv::Point findContactByDropShadow(
    const cv::Mat& edges,
    const std::vector<cv::Point>& pts,
    int dx,
    int bandWidth = 5,
    int verticalLookahead = 10
);

// collections's updated version - Jwl
int collectEdgePoints(
    const cv::Mat& edges,
    int dx,
    int& x, int& y,
    std::vector<cv::Point>& outPts,
    bool boundary_supplement = true
);

/*
* @description: Scan lens contour to find the best contact point.
 * @param edges: Edge image
 * @param fromPt: Starting point for scanning
 * @param dir: Direction of scan (left-down or right-down)
 * @param boundary_supplement: Whether to supplement boundary points
 * @param rotated: Whether the image is rotated
 * @return: Output candiates point
*/
std::vector<cv::Point> 
scan_lens_contour(
    const cv::Mat& edges, 
    const cv::Point& fromPt,
    ScanDir dir,
    bool boundary_supplement, 
    bool rotated = false
);

int check_lens_contour_for_pin(
    cv::Mat& edges, 
    std::vector<cv::Point>& contour, 
    cv::Point* pt, 
    bool boundary_supplement
);

LensContourType classifyContourBoundary(
    const std::vector<cv::Point>& contour, 
    cv::Point& _ptLeftOut, 
    cv::Point& _ptRightOut
);

cv::Point find_top_point(
    const std::vector<cv::Point>& contour
);

/*
* @description: Scan hulu contour to find the best contact point.
* @param edges: Edge image
* @param fromPt: Starting point for scanning
* @param dir: Scan direction (left-down or right-down)
* @param pt: Output point where the contact is found
* @param boundary_supplement: Whether to supplement boundary points
* @param rotated: Whether the image is rotated
* @param bottomY: Bottom bounding Y (e.g., boundingRect.y + height)
* @param verticalThreshold: Minimum vertical distance for correction
* @return: True if backtracked to find a better point, false otherwise
*/
bool scan_hulu_contour(
    const cv::Mat& edges, 
    const cv::Point& pt0, 
    ScanDir dir,
    cv::Point* pt, 
    bool boundary_supplement,
    bool rotated = false,
    int bottomY = -1,				
    int verticalThreshold = -1		
);

int check_hulu_contour_for_pin(
    const cv::Mat& edges, 
    std::vector<cv::Point>& contour, 
    cv::Point* pt, 
    bool boundary_supplement
);

int bottom_y(
    const cv::Mat& edges, 
    int x, 
    int y
);

/*
* @description: Check if the contour represents a sphere-like shape by scanning the top and bottom edges.
* @param edges: The edge image.
* @param contours: The list of contours.
* @param iContour: The index of the contour to check.
* @param outPts: Output points[length: 3] for the left and right edges of the sphere.
* @param rotated: Whether the contour is rotated.
* @param boundary_supplement: Whether to apply boundary correction.
* @return: 0 if the contour does not represent a sphere, 1 if it does.
*/
int check_sphere_contour(
    const cv::Mat& edges, 
    const std::vector<std::vector<cv::Point>>& contours, 
    int max_ind, 
    cv::Point* pt, 
    bool rotated, 
    bool boundary_supplement
);

void calc_dimensions(
    const std::vector<std::vector<cv::Point>>& contours, 
    int& ll, 
    int& rr, 
    int& tt, 
    int& bb, 
    int min_x, 
    int min_y
);

int find_top_pixel(
    const cv::Mat& edges, 
    int x
);

#if !_DEV_UPDATE_BACKTRACE

void back_track(const cv::Mat& edges, std::vector<cv::Point>& pts, std::vector<cv::Point>& pt_opts, int dx, bool second, int min_len = 7);

// 2025年7月22日之前使用的函数(replaced by stepPrimaryEdge)
bool step_primary_edge(const cv::Mat& edges, int& x, int& y, int dx, int dy, int& y0);
void collections_pre(const cv::Mat& edges, int dx, int dy, int x, int y);

// 2025年7月22日之前使用的函数(replaced by collectEdgePoints)
void collections(
    cv::Mat& edges,
    int dx,
    int dy,
    int& x,
    int& y,
    std::vector<cv::Point>& pts,
    int flag,
    bool rotated,
    bool boundary_supplement
);

bool try_additive_recovery(cv::Mat& edges, int& x, int& y, int flag, int x_st);

bool collections_additive_case1(cv::Mat& edges, int& x, int& y);
bool collections_additive_case2(cv::Mat& edges, int& x, int& y);
bool collections_additive_case3(cv::Mat& edges, int& x, int& y);
bool collections_additive_case4(cv::Mat& edges, int& x, int& y);
bool collections_additive_case5(cv::Mat& edges, int& x, int& y);
bool collections_additive_case6(cv::Mat& edges, int& x, int& y);
bool collections_additive_case7(cv::Mat& edges, int& x, int& y);
bool collections_additive_case8(const cv::Mat& edges, int& x, int& y);
bool collections_additive_case9(cv::Mat& edges, int& x, int& y);
bool collections_additive_case10(cv::Mat& edges, int& x, int& y);
bool collections_additive_case11(cv::Mat& edges, int& x, int& y);
bool collections_additive_case12(cv::Mat& edges, int& x, int& y);
bool collections_additive_case13(cv::Mat& edges, int& x, int& y);
bool collections_additive_case14(cv::Mat& edges, int& x, int& y);
bool collections_additive_case15(cv::Mat& edges, int& x, int& y);
bool collections_additive_case16(const cv::Mat& edges, int& x, int& y, int x_st, int num1, int num2);
bool collections_additive_case17(const cv::Mat& edges, int& x, int& y, int x_st, int num1, int num2);
bool collections_additive_case18(cv::Mat& edges, int& x, int& y);

#endif
#endif//__DETECT_UTILS_ON_HORIZONAL_H__

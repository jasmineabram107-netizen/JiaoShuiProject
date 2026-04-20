#ifndef _JS_ELLIPSE_INNER_H__
#define _JS_ELLIPSE_INNER_H__
#include <opencv2/opencv.hpp>
#include "common.h"

int getIntersections(const cv::Mat& img, const cv::Point2f& left, const cv::Point2f& right, int offset_x, int offset_y, RectangleF* rect);
int find_corners(const cv::Mat& img, const cv::Point2f& left, const cv::Point2f& right, int offset_x, int offset_y, RectangleF* _outRect);

#endif//_JS_ELLIPSE_INNER_H__

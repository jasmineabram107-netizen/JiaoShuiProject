#include "internalproc.h"
#include <opencv2/opencv.hpp>
#include "fit_util.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

int getIntersections(const cv::Mat& img, const cv::Point2f& left, const cv::Point2f& right, int offset_x, int offset_y, RectangleF* _outRect)
{
    const int thresholdDark = 50;

    cv::Mat grey;
    if (img.channels() == 3)
        cv::cvtColor(img, grey, cv::COLOR_BGR2GRAY);
    else if (img.channels() == 1)
        grey = img.clone();
    cv::GaussianBlur(grey, grey, cv::Size(5, 5), 0);

    int x1 = (int)left.x- offset_x, y1 = (int)left.y - offset_y;
    int x2 = (int)right.x - offset_x, y2 = (int)right.y - offset_y;

    int dx = abs(x2 - x1), dy = abs(y2 - y1), steps = std::max(dx, dy);
    if(steps == 0)
		return 0;
    double stepX = (x2 - x1) / static_cast<double>(steps);
    double stepY = (y2 - y1) / static_cast<double>(steps);

    for (int i = 0; i <= steps; ++i) {
        int x = (int)std::round(x1 + i * stepX);
        int y = (int)std::round(y1 + i * stepY);
        if (grey.at<uchar>(y, x) < thresholdDark) {
            _outRect->left = offset_x + x;
            _outRect->top = offset_y + y;
            break;
        }
    }

    for (int i = steps; i >= 0; --i) {
        int x = (int)std::round(x1 + i * stepX);
        int y = (int)std::round(y1 + i * stepY);
        if (grey.at<uchar>(y, x) < thresholdDark) {
            _outRect->right = offset_x + x;
            _outRect->bottom = offset_y + y;
            break;
        }
    }
    return 1;
}

int find_corners(const cv::Mat& img, const cv::Point2f& left, const cv::Point2f& right, int offset_x, int offset_y, RectangleF* _outRect)
{
    if (_outRect == NULL)
        return 0;
	cv::Mat grey;
	if (img.channels() == 3)
		cv::cvtColor(img, grey, cv::COLOR_BGR2GRAY);
	else
		grey = img.clone();

	std::vector<cv::Point2f> corners;
	cv::goodFeaturesToTrack(grey, corners, 4, 0.03, 20);
	if (corners.size() < 2)
        return 0;

	cv::cornerSubPix(grey, corners, cv::Size(5, 5), cv::Size(-1, -1),
		cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 40, 0.001));

	for (auto& pt : corners) {
		pt.x += offset_x;
		pt.y += offset_y;
	}

	std::sort(corners.begin(), corners.end(), [&](const cv::Point2f& a, const cv::Point2f& b) {
		return distanceToLine(a, left, right) < distanceToLine(b, left, right);
	});

    _outRect->left = corners[0].x;
    _outRect->top = corners[0].y;
    _outRect->right = corners[1].x;
    _outRect->bottom = corners[1].y;

	return 1;
}


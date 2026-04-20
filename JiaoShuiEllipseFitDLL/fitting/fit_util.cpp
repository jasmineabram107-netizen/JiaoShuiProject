#include "fit_util.h"
#include <algorithm>
#include <set>
#include <queue>
#include <unordered_map>
#include <numeric>
#include <unordered_set>
#include "../cvLib/cvcommon.h"
//#define NOMINMAX
//#include <windows.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


// 纠错基线坐标（如果左右颠倒的话）
void correct_base_params(cv::Point2f& p1, cv::Point2f& p2)
{
	if (p2.x < p1.x) {
		auto temp = p2.x;
		p2.x = p1.x;
		p1.x = temp;

		temp = p2.y;
		p2.y = p1.y;
		p1.y = temp;
	}
}

void correct_base_params(cv::Point& p1, cv::Point& p2)
{
	if (p2.x < p1.x) {
		auto temp = p2.x;
		p2.x = p1.x;
		p1.x = temp;

		temp = p2.y;
		p2.y = p1.y;
		p1.y = temp;
	}
}

// 是否两个点很接近
bool near_to_point0(const cv::Point& pt, const cv::Point& pt0, int dis)
{
	int dx = pt.x - pt0.x;
	int dy = pt.y - pt0.y;
	return (dx * dx + dy * dy) <= dis;
}

// 距离
int distance(const cv::Point& pt, const cv::Point& pt0)
{
	int dx = pt.x - pt0.x;
	int dy = pt.y - pt0.y;
	return dx * dx + dy * dy;
}
float distance(const cv::Point2f& pt, const cv::Point& pt0)
{
	float dx = pt.x - pt0.x;
	float dy = pt.y - pt0.y;
	return dx * dx + dy * dy;
}

float distance(const MPoint& pt, const cv::Point2f& pt0)
{
	float dx = (float)pt.x - pt0.x;
	float dy = (float)pt.y - pt0.y;
	return dx * dx + dy * dy;
}

float distance(const cv::Point2f& pt, const cv::Point2f& pt0)
{
	float dx = pt.x - pt0.x;
	float dy = pt.y - pt0.y;
	return dx * dx + dy * dy;
}
double disSqrt(double x1, double y1, double x2, double y2)
{
	return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

bool near_to_basepoint(MPoint& pt, const cv::Point& pt0, int dis)
{
	int dx = (int)pt.x - pt0.x;
	int dy = (int)pt.y - pt0.y;
	return (dx * dx + dy * dy) <= dis;
}

bool near_to_basepoint(MPoint& pt, const cv::Point2f& pt0, int dis)
{
	int dx = (int)(pt.x - pt0.x);
	int dy = (int)(pt.y - pt0.y);
	return (dx * dx + dy * dy) <= dis;
}

// 获取轮廓
void find_contours(
	const cv::Mat& img,
	cv::Mat& gray,
	cv::Mat& edges,
	std::vector<std::vector<cv::Point>>& contours
) {
	//std::vector<std::vector<cv::Point>> temp_contour;

	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
#if 0
	cv::Canny(gray, edges, 50, 150);
#else
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
	cv::Canny(gray, edges, 100, 200);
	cv::Mat dilated, eroded;	
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 1));
	cv::dilate(edges, dilated, kernel2);	
	cv::erode(dilated, edges, kernel2);
#endif

	std::vector<std::vector<cv::Point>> rawContours;
	cv::findContours(edges, rawContours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	
	contours = rawContours; // Save final processed contours
#if DEBUG_IMG
	// Draw each contour with a different color
	cv::Mat tmp = img.clone();
	for (size_t i = 0; i < contours.size(); i++) {
		cv::Scalar color(rand() & 255, rand() & 255, rand() & 255);
		cv::drawContours(tmp, contours, static_cast<int>(i), color, 1);
	}
	SaveDebugImg("contours", tmp);
#endif // _DEBUG
}

// 寻找覆盖面积最大的轮廓
int find_max_contour(
	const std::vector<std::vector<cv::Point>>& contours,
	std::vector<int>& traces
) {
	int nLen = (int)contours.size();
	int max_ind = -1;
	int max_value = -1;
	for (int i = 0; i < nLen; i++) {
		traces.push_back(0);
		double cur_value1 = 0.0, cur_value2 = 0.0;
		cv::Rect rt = cv::boundingRect(contours[i]);
		int cur_value = rt.area();
		if (max_value < cur_value) {
			max_value = cur_value;
			max_ind = i;
		}
	}
	return max_ind;
}

// 寻找覆盖面积最大的轮廓
int find_max_contour(std::vector<std::vector<cv::Point>>& contours, int side, int cx, int cy)
{
	int nLen = (int)contours.size();
	int max_ind = -1;
	int max_value = -1;
	for (int i = 0; i < nLen; i++) {
		cv::Rect rt = cv::boundingRect(contours[i]);
		if (side > -1) { // 插针的处理；需要排除不合格的轮廓
			if (contours[i][0].y > cy)
				continue;
			if (side == 0) {
				if (rt.x + rt.width / 2 > cx)
					continue;
			}
			else if (side == 1) {
				if (rt.x + rt.width / 2 < cx)
					continue;
			}
		}
		int cur_value = rt.area();
		if (max_value < cur_value) {
			max_value = cur_value;
			max_ind = i;
		}
	}
	return max_ind;
}


// 断线：寻找下一个轮廓线
int find_next_contour(std::vector<std::vector<cv::Point>>& contours, int min_dim, cv::Point& pt_r)
{
	int nLen = (int)contours.size();
	int ind = -1, min_dis = -1;
	for (int i = 0; i < nLen; i++) {
		cv::Rect rt = cv::boundingRect(contours[i]);
		if (std::max(rt.width, rt.height) < min_dim) {
			continue;
		}
		int n = (int)contours[i].size();
		int cur_dis = distance(pt_r, contours[i][0]);
		for (int j = 1; j < contours[i].size(); j++) {
			int cur_dis2 = distance(pt_r, contours[i][j]);
			cur_dis = std::min(cur_dis, cur_dis2);
		}
		if (min_dis < 0 || min_dis > cur_dis) {
			min_dis = cur_dis;
			ind = i;
		}
	}
	if (min_dis > 7 * 7) ind = -1;
	return ind;
}


int
find_include_contour(
	std::vector<std::vector<cv::Point>>& contours,
	const cv::Point& pt0
) {
	int nLen = (int)contours.size();
	int max_ind = -1;
	for (int i = 0; i < nLen; i++) {
		int n = (int)contours[i].size();
		bool found = false;
		for (int j = 0; j < n; j++) {
			cv::Point pt = contours[i][j];
			if (near_to_point0(pt, pt0)) {
				found = true;
			}
		}

		if (found) {
			max_ind = i;
		}
	}
	return max_ind;
}


void
collection(
	std::vector<std::vector<cv::Point>>& contours,
	int max_ind, int side, bool has_pin,
	const cv::Point& pt0,
	std::vector<cv::Point>& pts,
	std::vector<cv::Point>& pts_dn,
	int margin,
	MainShapeType lower_shape
) {
	int dis = 10;
	dis *= dis;
	int last_x = -1, last_y = -1;

	int n = (int)contours[max_ind].size();
	if (!side) { // 左边轮廓点的处理；这样更接近于实际轮廓
		// check if have 2 top points
		if (lower_shape == 1) {
			for (int i = 2; i < (int)contours[max_ind].size() - 2; i++) {
				if (contours[max_ind][i].y == 0) {
					std::vector<cv::Point> new_contour;
					for (int j = i; j >= 0; j--) {
						new_contour.push_back(contours[max_ind][j]);
					}
					for (int j = (int)contours[max_ind].size() - 1; j > i; j--) {
						new_contour.push_back(contours[max_ind][j]);
					}
					for (int j = 0; j < (int)new_contour.size(); j++) {
						contours[max_ind][j] = new_contour[j];
					}
				}
			}
		}
		std::reverse(contours[max_ind].begin(), contours[max_ind].end());
	}
	cv::Rect rt = cv::boundingRect(contours[max_ind]);

	// 搜集轮廓上的所有点	
	int xx0 = contours[max_ind][0].x;
	int yy0 = contours[max_ind][0].y;
	// 避免重复扫描 
	int i_up = n / 2;
	if (yy0 < 10) {
		for (int i = 1; i < n; i++) {
			cv::Point pt = contours[max_ind][i];
			if (pt.x == xx0 && pt.y == yy0) {
				i_up = i / 2; // 避免重复扫描
				break;
			}
		}
	}
	else {
		for (int i = 1; i < n; i++) {
			cv::Point pt = contours[max_ind][i];
			if (pt.x == xx0 && pt.y == yy0) {
				if (i < n / 2) {
					i_up = n / 2 + i / 2 + 1;
				}
				break;
			}
		}
	}

	std::vector<cv::Point> tmp;
	bool finished = false;
	int pts_dn_count = 0;
	cv::Point last_pt = contours[max_ind][0];
	for (int i = 0; i < i_up; i++) {
		cv::Point pt = contours[max_ind][i];
		last_pt = pt;
		if (near_to_point0(last_pt, pt0, dis)) {
			finished = true;
		}
		if (finished) {
			if (!near_to_point0(last_pt, pt0, dis)) {
				pts_dn_count++;
				pts_dn.push_back(pt);
				// 检查凸边是否有左右线部分
				if (pts_dn_count > 10 && lower_shape == 1) {
					if (pts_dn[pts_dn.size() - 1].y == pts_dn[pts_dn.size() - 3].y &&
						pts_dn[pts_dn.size() - 3].y == pts_dn[pts_dn.size() - 5].y &&
						pts_dn[pts_dn.size() - 5].y == pts_dn[pts_dn.size() - 7].y) {
						break;
					}
				}
				if (side == 0) {
					if (last_pt.x < margin)
						break;
				}
				else {
					if (last_pt.x > margin)
						break;
				}
			}
		}
		else {
			tmp.push_back(pt);
		}
	}
	contours.erase(contours.begin() + max_ind);
	int min_dim = 4;// std::max(10, std::max(rt.width, rt.height) / 40);
	while (pts_dn_count < 100) { // 轮廓线断了
		int ind = find_next_contour(contours, min_dim, last_pt);
		if (ind < 0) break;
		n = (int)contours[ind].size();
		i_up = n / 2;
		int max_x = last_pt.x;
		for (int i = 0; i < i_up; i++) {
			cv::Point pt = contours[ind][i];
			if (side == 0) {
				if (pt.x < margin)
					break;

				if (pt.x < last_pt.x)
					last_pt = pt;
			}
			else {
				if (pt.x > margin)
					break;
				if (pt.x > last_pt.x)
					last_pt = pt;
			}
			pts_dn.push_back(pt);
			pts_dn_count++;
		}
		contours.erase(contours.begin() + ind);
	}

	n = (int)tmp.size();
	if (n == 0)
		return;
	bool inclined_pin = false;
	double gradient = 0.0;
	yy0 = tmp[0].y;
	xx0 = tmp[0].x;
	if (has_pin && n > 100) { // 是否插针倾斜？
		int max_cnt = 0, cnt = 1, dir = 1;
		int start_x = tmp[0].x;
		for (int i = 1; i < n; i++) {
			if (start_x == tmp[i].x) {
				cnt++;
			}
			else {
				if (max_cnt < cnt) {
					max_cnt = cnt;
					if (start_x < tmp[i].x)
						dir = 1;
					else
						dir = -1;
				}
				start_x = tmp[i].x;
				cnt = 1;
			}

		}
		if (max_cnt > 4) {
			if (max_cnt > 70) {
				gradient = 0;
				inclined_pin = false;
			}
			else {
				gradient = (double)dir / max_cnt;
				inclined_pin = true;
				cnt = 1;
				for (int i = 1; i < n; i++) {
					if (xx0 != tmp[i].x) {
						if (cnt < 3) {
							xx0 = tmp[i].x;
							yy0 = tmp[i].y;
						}
						break;
					}
					else {
						cnt++;
					}
				}
			}
		}
	}
	std::reverse(tmp.begin(), tmp.end()); // 倒序：从基线点开始寻找轮廓点——由下而上

	int addings = 0;
	for (int i = 0; i < n; i++) {
		cv::Point pt = tmp[i];
		if (has_pin) {
			if (inclined_pin) { // 插针倾斜
				double dx = (tmp[i].y - yy0) * gradient;
				if (abs(pt.x - (xx0 + dx)) < 5) {
					if (i < n - 10) {
						dx = (tmp[i + 10].y - yy0) * gradient;
						if (abs(tmp[i + 10].x - (xx0 + dx)) < 5)
							break;
					}
					else {
						break;
					}
				}
			}
			else if (abs(pt.x - xx0) < 4 && addings > 10) {
				if (i < n - 11) {
					if (abs(tmp[i + 10].x - xx0) < 4)
						break;
				}
				else
					break;

			}
		}
		pts.insert(pts.begin(), pt);
		addings++;
	}
	tmp.clear();

	pts.push_back(pt0);
}

// 处理轮廓，获取轮廓点：插针
void find_contour_points_for_pin(
	cv::Mat& img,
	const cv::Point& pt_l, const cv::Point& pt_r,
	std::vector<cv::Point>& pts_l,
	std::vector<cv::Point>& pts_r,
	std::vector<cv::Point>& pts_down,
	MainShapeType lower_shape
) {
	cv::Mat gray, edges;
	std::vector<std::vector<cv::Point>> contours;
	find_contours(img, gray, edges, contours);
	int cx = (pt_l.x + pt_r.x) / 2;
	int cy = (pt_l.y + pt_r.y) / 2;
	cy = std::max(10, cy - 10);

	// 删除不必要的部分
	int margin_l = std::max(pt_l.x - BASEPOINT_MARGIN, img.cols / 20);
	int margin_r = std::min(pt_r.x + BASEPOINT_MARGIN, img.cols - img.cols / 20);

	int max_ind = find_include_contour(contours, pt_l);
	if (max_ind < 0) {
		max_ind = find_max_contour(contours, 0, cx, cy);
		if (max_ind < 0) return;
	}

	collection(contours, max_ind, 0, true, pt_l, pts_l, pts_down, margin_l, lower_shape);

	max_ind = find_include_contour(contours, pt_r);
	if (max_ind < 0) {
		max_ind = find_max_contour(contours, 1, cx, cy);
		if (max_ind < 0) return;
	}
	collection(contours, max_ind, 1, true, pt_r, pts_r, pts_down, margin_r, lower_shape);

	std::vector<int> traces;
	for (int i = 0; i < contours.size(); i++) {
		traces.push_back(0);
	}

#if DEBUG_IMG
	cv::Mat tmp;
	img.copyTo(tmp);
	std::vector<std::vector<cv::Point>> contours_draw;
	contours_draw.push_back(pts_l);
	contours_draw.push_back(pts_r);
	contours_draw.push_back(pts_down);
	if (contours_draw[0].size())
		cv::drawContours(tmp, contours_draw, 0, cv::Scalar(0, 0, 255));
	if (contours_draw[1].size())
		cv::drawContours(tmp, contours_draw, 1, cv::Scalar(0, 255, 0));
	if (contours_draw[2].size())
		cv::drawContours(tmp, contours_draw, 2, cv::Scalar(255, 0, 0));
	SaveDebugImg("left_right_down", tmp);
#endif // DEBUG
}

/**
* return 
*  0 : not find
* > 0: yedi top index, direction right
* < 0: yedi top index, direction inverse
*/
int check_contour_top(
	const std::vector<cv::Point>& contour, 
	const cv::Point& pt_l, 
	const cv::Point& pt_r
) {
	bool left_found = false, right_found = false;
	int left_ind = -1, right_ind = -1;
	for (int i = 0; i < contour.size(); i++) {
		cv::Point pt = contour[i];
		if (near_to_point0(pt, pt_l)) {			
			if (left_found) {
				if (i - left_ind > 20) {
					return 0;
				}
				continue;
			}
			else if (right_found) {	
				left_ind = i;
				if (left_ind > contour.size() / 2 + 10) {
					return 0;
				}
				return (left_ind + right_ind) / 2;				
			}
			else {
				left_ind = i;
				left_found = true;
			}			
		}
		else if (near_to_point0(pt, pt_r)) {
			
			if (right_found) {
				if (i - right_ind > 20) {
					return 0;
				}
				continue;				
			}
			else if (left_found) {
				right_ind = i;
				if (right_ind > contour.size() / 2+ 10) {
					return 0;
				}
				return -(left_ind + right_ind) / 2;
			}
			right_ind = i;
			right_found = true;
		}
	}
	return -1;
}

bool is_nearly_straight(
	const std::vector<cv::Point>& segment,
	double tolerance
) {
	cv::Vec4f line;
	cv::fitLine(segment, line, cv::DIST_L2, 0, 0.01, 0.01);

	cv::Point2f p0 = segment.front();
	cv::Point2f dir(line[0], line[1]);
	double maxDist = 0.0;
	for (const auto& pt : segment) {
		cv::Point2f v = cv::Point2f(pt.x, pt.y) - p0;
		float proj_len = v.dot(dir);
		cv::Point2f proj = p0 + proj_len * dir;
		auto dist = cv::norm(proj - cv::Point2f(pt));
		if (maxDist < dist)
			maxDist = dist;
	}
	if (maxDist < tolerance)
		return true;
	return false;
}

bool is_none_convex(
	const std::vector<cv::Point>& segment,
	const cv::Point& pt_l,
	const cv::Point& pt_r,
	double margin
) {
	double y_avg = std::accumulate(segment.begin(), segment.end(), 0.0,
		[](double sum, const cv::Point& p) { return sum + p.y; }) / segment.size();
	double y_base = 0.5 * (pt_l.y + pt_r.y);
	double some_margin = margin; // pixels
	bool is_inverted = y_avg > y_base + some_margin;
	if (is_inverted)
		return true;
	return false;
}

int is_convex_between(
	const std::vector<cv::Point>& contour, 
	int idx_l, 
	int idx_r,
	double tolerance_ratio
) {
	std::vector<cv::Point> segment;
	int length = 0;
	int n = (int)contour.size();
	// Traverse from idx_l to idx_r (wrap around if necessary)
	for (int k = idx_l; k != idx_r; k = (k + 1) % n) {
		segment.push_back(contour[k]);
	}
	segment.push_back(contour[idx_r]); // include end point

	if (segment.size() < 3)
		return 0;
	if(is_nearly_straight(segment))
		return 0;
	if (is_none_convex(segment, contour[idx_l], contour[idx_r]))
		return 0;

	//bool res = cv::isContourConvex(segment);
	std::vector<cv::Point> hull;
	cv::convexHull(segment, hull);

	double area_segment = std::fabs(cv::contourArea(segment));
	double area_hull = std::fabs(cv::contourArea(hull));

	if (area_hull < 1e-3) // near-zero area (flat line)
		return 0;
	if (area_segment == 0)
		return 0;

	double ratio = area_segment / area_hull;
	bool _res = (ratio >= 1.0 - tolerance_ratio); // nearly convex if area loss is small
	if (!_res)
		return 0;

	length = (int)segment.size();
	return length;
}

// 寻找基线点的轮廓
int find_contour_include_basepoints(
	const std::vector<std::vector<cv::Point>>& contours, 
	std::vector<int>& traces, 
	const cv::Point& pt_l, 
	const cv::Point& pt_r,
	bool findOnlyConvex
) {	
	int best_idx = -1;
	int max_arc_length = 0;
	traces.clear();

	for (size_t i = 0; i < contours.size(); i++) {
		const std::vector<cv::Point>& contour = contours[i];
		bool left_found = false, right_found = false;
		int idx_l = -1, idx_r = -1;

		// Search for pt_l and pt_r in contour
		for (int j = 0; j < contour.size(); j++) {
			if (!left_found && near_to_point0(contour[j], pt_l, 80)) {
				idx_l = j;
				left_found = true;
			}
			if (!right_found && near_to_point0(contour[j], pt_r, 80)) {
				idx_r = j;
				right_found = true;
			}
			if (left_found && right_found) 
				break;
		}

		// Mark trace status
		traces.push_back((left_found && right_found) ? 1 : 0);

		if (left_found && right_found) {
			// Ensure indices are in range
			/*
			OutputDebugString(_T("**************\r\n"));
			for (int k = 0; k < contour.size(); k++) {
				TCHAR buf[100];
				_stprintf(buf, _T("(%d,%d),"), contour[k].x, contour[k].y);
				OutputDebugString(buf);
			}
			TCHAR buf[100];
			_stprintf(buf, _T("\r\nidxl: %d, idx_r: %d\r\n"), idx_l, idx_r);
			OutputDebugString(buf);
			*/
			int nLen = 0;
			if(findOnlyConvex)
				nLen = is_convex_between(contour, idx_l, idx_r);
			else
				nLen = std::abs(idx_r - idx_l);
			if(nLen == 0) {
				continue; // skip non-convex arc				
			}
			// Update best match
			if (nLen > max_arc_length) {
				max_arc_length = nLen;
				best_idx = static_cast<int>(i);
			}
		}
	}

	return best_idx;
}

// 断线：寻找下一个轮廓线
int find_next_contour(
	const std::vector<std::vector<cv::Point>>& contours, 
	int min_dim,
	const cv::Point& pt_r,
	std::vector<int>& traces
) {
	int nLen = (int)contours.size();
	int ind = -1, min_dis = -1;
	for (int i = 0; i < nLen; i++) {
		if (traces[i])
			continue;
		cv::Rect rt = cv::boundingRect(contours[i]);
		if (std::max(rt.width, rt.height) < min_dim) {
			traces[i] = 1;
			continue;
		}
		int n = (int)contours[i].size();
		int cur_dis = distance(pt_r, contours[i][0]);
		for (int j = 1; j < contours[i].size(); j++) {
			int cur_dis2 = distance(pt_r, contours[i][j]);
			cur_dis = std::min(cur_dis, cur_dis2);
		}		
		if (min_dis < 0 || min_dis > cur_dis) {
			min_dis = cur_dis;
			ind = i;
		}
	}
	if (min_dis > 7 * 7) 
		ind = -1;
	if (ind > -1) 
		traces[ind] = 1;
	return ind;
}

// Function to find intersection between a line and a circle
bool lineCircleIntersection(
	double x0, double y0,
	double vx, double vy, 
	cv::Point circleCenter, 
	double radius, 
	std::vector<cv::Point2f>& intersections
) {
	double cx = circleCenter.x, cy = circleCenter.y;

	// Quadratic equation coefficients: At^2 + Bt + C = 0
	double A = vx * vx + vy * vy;
	double B = 2 * (vx * (x0 - cx) + vy * (y0 - cy));
	double C = (x0 - cx) * (x0 - cx) + (y0 - cy) * (y0 - cy) - radius * radius;

	// Compute the discriminant
	double D = B * B - 4 * A * C;

	if (D < 0) {
		return false; // No intersection
	}

	// Compute the solutions for t
	double sqrtD = std::sqrt(D);
	double t1 = (-B + sqrtD) / (2 * A);
	double t2 = (-B - sqrtD) / (2 * A);

	// Compute intersection points
	intersections.push_back(cv::Point2f(x0 + t1 * vx, y0 + t1 * vy));
	if (D > 0) { // If D == 0, there's only one intersection
		intersections.push_back(cv::Point2f(x0 + t2 * vx, y0 + t2 * vy));
	}

	return true;
}

void updateBaselinePoints(
	cv::Vec4f &line, 
	cv::Mat &frame, 
	std::vector<cv::Point> pts_up, 
	cv::Point &pt_l, 
	cv::Point &pt_r
) {
	float vx = line[0], vy = line[1];  // Direction vector
	float x0 = line[2], y0 = line[3];  // A point on the line
	cv::RotatedRect box_up;
	memset(&box_up, 0, sizeof(cv::RotatedRect));
	int dimension = std::max(frame.cols, frame.rows);
	std::vector<cv::Point2f> fpts_up;
	for (int i = 0; i < pts_up.size(); i++) {
		fpts_up.push_back(cv::Point2f((float)pts_up[i].x, (float)pts_up[i].y));
	}
	ellipse_regression_Circle(fpts_up, dimension, box_up);
	std::vector<cv::Point2f> intersections;
	if (lineCircleIntersection(x0, y0, vx, vy, box_up.center, box_up.size.width, intersections)) {
		if (intersections[0].x < intersections[1].x) {
			if (disSqrt(pt_l.x, pt_l.y, intersections[0].x, intersections[0].y) < 15 && 
				disSqrt(pt_r.x, pt_r.y, intersections[1].x, intersections[1].y) < 15) {
				pt_l = cv::Point((int)round(intersections[0].x), (int)round(intersections[0].y));
				pt_r = cv::Point((int)round(intersections[1].x), (int)round(intersections[1].y));
			}			
		}
		else {
			if (disSqrt(pt_l.x, pt_l.y, intersections[1].x, intersections[1].y) < 15 && 
				disSqrt(pt_r.x, pt_r.y, intersections[0].x, intersections[0].y) < 15) {
				pt_l = cv::Point((int)round(intersections[1].x), (int)round(intersections[1].y));
				pt_r = cv::Point((int)round(intersections[0].x), (int)round(intersections[0].y));
			}
		}
	}
}

void checkAndUpdateBaseLine(
	cv::Mat& frame, 
	cv::Point& pt_l, 
	cv::Point& pt_r,
	std::vector<cv::Point>& pts_up
) {	
	int res = 0;
	// 寻找面积最大的轮廓
	cv::Mat gray, edges;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<int> traces;
	int max_ind, n = 0;

	find_contours(frame, gray, edges, contours);
	
	cv::Point pt_li = cv::Point(pt_l.x, pt_l.y);
	cv::Point pt_ri(pt_r.x, pt_r.y);

	max_ind = find_contour_include_basepoints(contours, traces, pt_li, pt_ri, true);
	if (max_ind < 0) 
		return;

	std::vector<cv::Point> cur_contour = contours[max_ind];
	int length = (int)cur_contour.size();
	cv::Rect rt = cv::boundingRect(cur_contour);
	
	cv::RotatedRect rBox = cv::minAreaRect(cur_contour);
	cv::Point2f pts[4];
	rBox.points(pts);
	bool bCheck = false;
	
	if (disSqrt(pt_l.x, pt_l.y, pts[0].x, pts[0].y) <= POINT_DIS && 
		disSqrt(pt_r.x, pt_r.y, pts[3].x, pts[3].y) <= POINT_DIS) {
		bCheck = true;
	}
	else if (disSqrt(pt_l.x, pt_l.y, pts[1].x, pts[1].y) <= POINT_DIS && 
		disSqrt(pt_r.x, pt_r.y, pts[0].x, pts[0].y) <= POINT_DIS) {
		// change order
		pts[3] = pts[0];
		pts[0] = pts[1];
		bCheck = true;
	}	

	if (!bCheck)
		return;

	bool bFind = false;
	int n_Count = 0;
	for (int i = 1; i < length - 1; i++) {
		if (cur_contour[i] == cur_contour[0]) {
			bFind = true;
			if (i < length / 2) {
				n_Count = i + 1;
			}
			break;
		}
	}

	int another_ind = -1;
	// Find another contour with baseline points
	for (int i = 0; i < (int)contours.size(); i++) {
		if (traces[i])
			continue;
		int n = (int)contours[i].size();
		bool left_found = false;
		bool right_found = false;

		for (int j = 0; j < n; j++) {
			cv::Point pt = contours[i][j];
			if (near_to_point0(pt, pt_li)) {
				left_found = true;
			}
			else if (near_to_point0(pt, pt_ri)) {
				right_found = true;
			}
		}
		if (left_found && right_found) {
			another_ind = i;
			break;
		}
	}
	if (!bFind || another_ind < 0) {
		double min_dis = 1000000;
		int min_ind = 0, start_i = 0, end_i = length - 1;
			
		for (int i = n_Count; i < length; i++) {
			double dis = disSqrt(pts[0].x, pts[0].y, cur_contour[i].x, cur_contour[i].y);
			if (distance(cur_contour[i], pt_r) <= POINT_DIS * POINT_DIS &&
				min_dis < 10) {
				break;
			}
			if (dis < min_dis) {
				min_dis = dis;
				min_ind = i;
			}
		}
		if (min_dis < 10) {
			int max_y = 0, cur_i = 0;
			for (int i = min_ind - 3; i < min_ind + 3; i++) {
				if (max_y < cur_contour[i].y) {
					max_y = cur_contour[i].y;
					start_i = i;
				}
			}

			min_dis = 1000000;
			for (int i = start_i + 1; i < length; i++) {
				double dis = disSqrt(pts[3].x, pts[3].y, cur_contour[i].x, cur_contour[i].y);
				if (dis < min_dis) {
					min_dis = dis;
					min_ind = i;
				}
			}
			if (min_dis < 10) {
				int max_y = 0, cur_i = 0;
				for (int i = min_ind + 3; i > min_ind - 3; i--) {
					if (max_y < cur_contour[i].y) {
						max_y = cur_contour[i].y;
						end_i = i;
					}
				}
			}
			std::vector<cv::Point> line_seg;
			for (int i = start_i; i < end_i; i++) {
				line_seg.push_back(cur_contour[i]);
			}
			cv::Vec4f line;  // [vx, vy, x0, y0]
			cv::fitLine(line_seg, line, cv::DIST_L2, 0, 0.01, 0.01);
			updateBaselinePoints(line, frame, pts_up, pt_l, pt_r);
		}
	}
	else {			
		if (another_ind >= 0) {
			int max_cnt = 0, cnt = 1, prev_i = 0;
			int start_i = 0, end_i = 0;
			int start_y = contours[another_ind][0].y;
			cv::Point start_pt = contours[another_ind][0];
			for (int i = 1; i < contours[another_ind].size(); i++) {
				if (contours[another_ind][i] == start_pt) {
					break;
				}
				if (start_y == contours[another_ind][i].y) {
					cnt++;
				}
				else {
					if (max_cnt < cnt) {
						max_cnt = cnt;
						start_i = prev_i;
						end_i = i - 1;
					}
					start_y = contours[another_ind][i].y;
					cnt = 1;
					prev_i = i;
				}
			}
			if (max_cnt < cnt) {
				max_cnt = cnt;
				start_i = prev_i;
				end_i = (int)contours[another_ind].size() - 1;
			}
			if (max_cnt > 10) {
				if (max_cnt < (int)contours[another_ind].size() / 2) {
					int same_count = 0;
					cnt = 1;
					for (int i = 1; i < (int)contours[another_ind].size(); i++) {
						if (start_y == contours[another_ind][i].y) {
							cnt++;
						}
						else {
							if (max_cnt - 3 < cnt) {
								same_count++;
							}
							start_y = contours[another_ind][i].y;
							cnt = 1;
						}
					}
					if (same_count >= 3) {
						// update basepoint
						// Fit a line using OpenCV's fitLine function
						cv::Vec4f line;  // [vx, vy, x0, y0]
						cv::fitLine(contours[another_ind], line, cv::DIST_L2, 0, 0.01, 0.01);
						updateBaselinePoints(line, frame, pts_up, pt_l, pt_r);
					}
				}
			}
		}
	}
}

int findNearestIndex(const std::vector<cv::Point>& contour, const cv::Point& ref) 
{
	int minIdx = 0;
	int minDist = INT_MAX;
	for (int i = 0; i < contour.size(); ++i) {
		int d = distance(contour[i], ref); // your existing distance() function
		if (d < minDist) {
			minDist = d;
			minIdx = i;
		}
	}
	return minIdx;
}

int findNearestIndex(const std::vector<cv::Point2f>& contour, const cv::Point& ref)
{
	int minIdx = 0;
	float minDist = 100000.0f;
	for (int i = 0; i < (int)contour.size(); ++i) {
		auto d = distance(contour[i], ref); // your existing distance() function
		if (d < minDist) {
			minDist = d;
			minIdx = i;
		}
	}
	return minIdx;
}



void zhangSuenThinning(cv::Mat& im) 
{
	cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
	cv::Mat diff;

	do {
		cv::Mat mFlag = cv::Mat::zeros(im.size(), CV_8UC1);

		for (int y = 1; y < im.rows - 1; y++) {
			for (int x = 1; x < im.cols - 1; x++) {
				uchar p2 = im.at<uchar>(y - 1, x);
				uchar p3 = im.at<uchar>(y - 1, x + 1);
				uchar p4 = im.at<uchar>(y, x + 1);
				uchar p5 = im.at<uchar>(y + 1, x + 1);
				uchar p6 = im.at<uchar>(y + 1, x);
				uchar p7 = im.at<uchar>(y + 1, x - 1);
				uchar p8 = im.at<uchar>(y, x - 1);
				uchar p9 = im.at<uchar>(y - 1, x - 1);
				int A = (p2 == 0 && p3 == 255) + (p3 == 0 && p4 == 255) +
					(p4 == 0 && p5 == 255) + (p5 == 0 && p6 == 255) +
					(p6 == 0 && p7 == 255) + (p7 == 0 && p8 == 255) +
					(p8 == 0 && p9 == 255) + (p9 == 0 && p2 == 255);
				int B = (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) / 255;
				if (im.at<uchar>(y, x) == 255 &&
					2 <= B && B <= 6 &&
					A == 1 &&
					(p2 * p4 * p6 == 0) &&
					(p4 * p6 * p8 == 0))
					mFlag.at<uchar>(y, x) = 1;
			}
		}
		im &= ~mFlag;

		mFlag = cv::Mat::zeros(im.size(), CV_8UC1);
		for (int y = 1; y < im.rows - 1; y++) {
			for (int x = 1; x < im.cols - 1; x++) {
				uchar p2 = im.at<uchar>(y - 1, x);
				uchar p3 = im.at<uchar>(y - 1, x + 1);
				uchar p4 = im.at<uchar>(y, x + 1);
				uchar p5 = im.at<uchar>(y + 1, x + 1);
				uchar p6 = im.at<uchar>(y + 1, x);
				uchar p7 = im.at<uchar>(y + 1, x - 1);
				uchar p8 = im.at<uchar>(y, x - 1);
				uchar p9 = im.at<uchar>(y - 1, x - 1);
				int A = (p2 == 0 && p3 == 255) + (p3 == 0 && p4 == 255) +
					(p4 == 0 && p5 == 255) + (p5 == 0 && p6 == 255) +
					(p6 == 0 && p7 == 255) + (p7 == 0 && p8 == 255) +
					(p8 == 0 && p9 == 255) + (p9 == 0 && p2 == 255);
				int B = (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) / 255;
				if (im.at<uchar>(y, x) == 255 &&
					2 <= B && B <= 6 &&
					A == 1 &&
					(p2 * p4 * p8 == 0) &&
					(p2 * p6 * p8 == 0))
					mFlag.at<uchar>(y, x) = 1;
			}
		}
		im &= ~mFlag;

		cv::absdiff(im, prev, diff);
		im.copyTo(prev);
	} while (cv::countNonZero(diff) > 0);
}

// Fast Zhang-Suen Thinning
void zhangSuenThinningFast(cv::Mat& im)
{
	CV_Assert(im.type() == CV_8UC1);
	im /= 255;

	cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
	cv::Mat diff;

	const int rows = im.rows;
	const int cols = im.cols;

	auto checkPixel = [&](int p2, int p3, int p4, int p5, int p6, int p7, int p8, int p9, int& A, int & B) {
		A = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) + (p4 == 0 && p5 == 1) +
			(p5 == 0 && p6 == 1) + (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
			(p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
		B = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;		
	};

	do {
		im.copyTo(prev);

		// Step 1
		std::vector<cv::Point> toZero;
		for (int y = 1; y < rows - 1; ++y) {
			uchar* p = im.ptr<uchar>(y);
			for (int x = 1; x < cols - 1; ++x) {
				if (p[x] != 1) continue;
				int p2 = im.at<uchar>(y - 1, x);
				int p3 = im.at<uchar>(y - 1, x + 1);
				int p4 = im.at<uchar>(y, x + 1);
				int p5 = im.at<uchar>(y + 1, x + 1);
				int p6 = im.at<uchar>(y + 1, x);
				int p7 = im.at<uchar>(y + 1, x - 1);
				int p8 = im.at<uchar>(y, x - 1);
				int p9 = im.at<uchar>(y - 1, x - 1);

				int A = 0; int B = 0;
				checkPixel(p2, p3, p4, p5, p6, p7, p8, p9, A, B);

				if (A == 1 && (B >= 2 && B <= 6) && (p2 * p4 * p6 == 0) && (p4 * p6 * p8 == 0)) {
					toZero.emplace_back(x, y);
				}
			}
		}

		for (auto& pt : toZero)
			im.at<uchar>(pt.y, pt.x) = 0;

		// Step 2
		toZero.clear();
		for (int y = 1; y < rows - 1; ++y) {
			uchar* p = im.ptr<uchar>(y);
			for (int x = 1; x < cols - 1; ++x) {
				if (p[x] != 1) continue;
				int p2 = im.at<uchar>(y - 1, x);
				int p3 = im.at<uchar>(y - 1, x + 1);
				int p4 = im.at<uchar>(y, x + 1);
				int p5 = im.at<uchar>(y + 1, x + 1);
				int p6 = im.at<uchar>(y + 1, x);
				int p7 = im.at<uchar>(y + 1, x - 1);
				int p8 = im.at<uchar>(y, x - 1);
				int p9 = im.at<uchar>(y - 1, x - 1);

				int A = 0; int B = 0;
				checkPixel(p2, p3, p4, p5, p6, p7, p8, p9, A, B);

				if (A == 1 && (B >= 2 && B <= 6) && (p2 * p4 * p8 == 0) && (p2 * p6 * p8 == 0)) {
					toZero.emplace_back(x, y);
				}
			}
		}

		for (auto& pt : toZero)
			im.at<uchar>(pt.y, pt.x) = 0;

		cv::absdiff(im, prev, diff);
	} while (cv::countNonZero(diff) > 0);

	im *= 255;
}

int traceSkeletonTopDown_withLeftCount(
	const std::vector<cv::Point>& skeletonPoints,
	const cv::Point& pt_l,
	const cv::Point& pt_r,
	int distThresholdSq,
	std::vector<cv::Point>& outPath,
	int& leftCount
) {
	if (skeletonPoints.empty()) return 0;

	const std::vector<cv::Point> directions = {
		{-1, -1}, {-1, 0}, {-1, 1},
		{0, -1},          {0, 1},
		{1, -1},  {1, 0}, {1, 1}
	};

	auto hash = [](const cv::Point& pt) { return (pt.y << 16) | pt.x; };

	std::unordered_set<int> skelSet;
	for (const auto& pt : skeletonPoints)
		skelSet.insert(hash(pt));

	// Find top-most starting point
	auto cmpTop = [](const cv::Point& a, const cv::Point& b) {
		return (a.y < b.y) || (a.y == b.y && a.x < b.x);
		};
	cv::Point start = *std::min_element(skeletonPoints.begin(), skeletonPoints.end(), cmpTop);

	std::queue<cv::Point> q;
	std::unordered_map<int, cv::Point> parent;
	std::unordered_set<int> visited;

	q.push(start);
	visited.insert(hash(start));

	bool enteredLeft = false, enteredRight = false;
	bool stopLeft = false, stopRight = false;

	int hashL = -1, hashR = -1;

	while (!q.empty()) {
		cv::Point cur = q.front(); q.pop();
		int curHash = hash(cur);

		int dL = (cur.x - pt_l.x) * (cur.x - pt_l.x) + (cur.y - pt_l.y) * (cur.y - pt_l.y);
		int dR = (cur.x - pt_r.x) * (cur.x - pt_r.x) + (cur.y - pt_r.y) * (cur.y - pt_r.y);

		if (!enteredLeft && dL <= distThresholdSq) {
			enteredLeft = true;
			hashL = curHash;
		}
		else if (enteredLeft && dL > distThresholdSq && !stopLeft) {
			stopLeft = true;
		}

		if (!enteredRight && dR <= distThresholdSq) {
			enteredRight = true;
			hashR = curHash;
		}
		else if (enteredRight && dR > distThresholdSq && !stopRight) {
			stopRight = true;
		}

		// If both have exited, we’re done
		if (stopLeft && stopRight)
			break;

		for (const auto& d : directions) {
			cv::Point next = cur + d;
			int h = hash(next);
			if (skelSet.count(h) && !visited.count(h)) {
				visited.insert(h);
				parent[h] = cur;
				q.push(next);
			}
		}
	}

	// Reconstruct path — from last visited points
	std::vector<cv::Point> fullPath;
	for (const auto& node : parent) {
		auto h = node.first;
		auto p = node.second;
		fullPath.push_back(p);
	}
	fullPath.push_back(start); // add root
	std::sort(fullPath.begin(), fullPath.end(), [](const cv::Point& a, const cv::Point& b) {
		return a.y < b.y || (a.y == b.y && a.x < b.x);
		});

	// Determine where pt_l falls — for leftCount
	leftCount = 0;
	for (size_t i = 0; i < fullPath.size(); ++i) {
		if(near_to_point0(fullPath[i], pt_l, distThresholdSq)) {
			leftCount = (int)i;
			break;
		}
	}

	outPath = std::move(fullPath);
	return (int)outPath.size();
}

bool sortSkeletonPoints(
	const std::vector<cv::Point2f>& input,
	const cv::Point2f& pt_l,
	const cv::Point2f& pt_r,
	std::vector<cv::Point2f>& output,
	int& leftCount,
	float connectThreshold,
	float endThreshold)
{
	if (input.empty()) {
		leftCount = 0;
		return false;
	}

	const int n = static_cast<int>(input.size());

	// 1. Find topmost point
	int idxTop = 0;
	float minY = input[0].y;
	for (int i = 1; i < n; ++i) {
		if (input[i].y < minY) {
			minY = input[i].y;
			idxTop = i;
		}
	}

	auto traceDirectional = [&](const cv::Point2f& stopPt, bool moveLeft) -> std::vector<cv::Point2f> {
		std::vector<cv::Point2f> result;
		std::vector<bool> visited(n, false);
		int current = idxTop;
		result.push_back(input[current]);
		visited[current] = true;

		while (true) {
			int next = -1;
			double bestDist = DBL_MAX;
			for (int i = 0; i < n; ++i) {
				if (visited[i])
					continue;
				auto dist = cv::norm(input[i] - input[current]);
				float dx = input[i].x - input[current].x;

				if (dist < connectThreshold && dist < bestDist) {
					if ((moveLeft && dx < 0) || (!moveLeft && dx > 0)) {
						bestDist = dist;
						next = i;
					}
				}
			}
			if (next == -1)
				break;

			visited[next] = true;
			result.push_back(input[next]);
			current = next;

			if (cv::norm(input[next] - stopPt) < endThreshold)
				break;
		}

		return result;
		};

	std::vector<cv::Point2f> leftPart = traceDirectional(pt_l, true);
	std::vector<cv::Point2f> rightPart = traceDirectional(pt_r, false);

	std::reverse(leftPart.begin(), leftPart.end());  // So order becomes: pt_l -> top
	output = leftPart;

	if (!rightPart.empty() && !leftPart.empty() &&
		rightPart.front() == leftPart.back()) {
		rightPart.erase(rightPart.begin());  // avoid duplicate
	}

	output.insert(output.end(), rightPart.begin(), rightPart.end());
	leftCount = (int)leftPart.size();

	return true;
}

int getDropletContourByLTI(
	const std::vector<cv::Point>& contour1,
	const cv::Point& pt_l, 
	const cv::Point& pt_r,
	int dist2,
	std::vector<cv::Point>& _validContour
) {
	// Scan from pt_l forward
	std::vector<cv::Point> pts_up_from_l, pts_up_from_r;
	bool passed = false;
	int min_dis = 10000;	
	std::vector<cv::Point> maxContour = contour1;
	int n = (int)maxContour.size();
	for (int i = 0; i < n; ++i) {
		cv::Point pt = maxContour[i];
		auto leftDist = distance(pt, pt_l);
		if (leftDist <= dist2) {
			passed = true;
			if (leftDist > min_dis)
				break;
			min_dis = leftDist;
		}
		else if (passed)
			break;
		pts_up_from_l.push_back(pt);
	}

	// Scan reverse from pt_r
	std::reverse(maxContour.begin(), maxContour.end());
	passed = false;
	min_dis = 10000;
	for (int i = 0; i < n; ++i) {
		cv::Point pt = maxContour[i];
		auto rightDist = distance(pt, pt_r);
		if (rightDist <= dist2) {
			passed = true;
			if (rightDist > min_dis) break;
			min_dis = rightDist;
		}
		else if (passed)
			break;
		pts_up_from_r.push_back(pt);
	}
	// Optional: remove overlap if needed	
	std::reverse(pts_up_from_r.begin(), pts_up_from_r.end()); // Fix order

	// Avoid overlap
	if (!pts_up_from_l.empty() && !pts_up_from_r.empty() &&
		pts_up_from_l.back() == pts_up_from_r.front()) {
		pts_up_from_r.erase(pts_up_from_r.begin());
	}

	// Merge
	_validContour = pts_up_from_l;
	int res = (int)_validContour.size();
	_validContour.insert(_validContour.end(), pts_up_from_r.begin(), pts_up_from_r.end());

	return res;
}

double get_point_angle(const PointAndAngle& pa)
{
	double res = -1.0;
	if (pa.angle < 0.0)
		return res;
	else {
		res = angle_between_vectors(
			pa.vec1,
			pa.vec2
		);
	}
	return res;
}

double angle_between_vectors(MPoint vec1, MPoint vec2)
{
	// 计算两个矢量之间的夹角
	double scalar_product = vec1.x * vec2.x + vec1.y * vec2.y;
	return acos(scalar_product) * 180 / M_PI;
}

int getMidPoint(cv::Mat& gray) {
	int res = 0;
	int start = 0, end = gray.rows - 1;
	cv::Mat out;
	cv::threshold(gray, out, 50, 255, cv::THRESH_BINARY_INV);
	for (int i = 1; i < out.rows; i++) {
		int sum = 0;
		for (int j = 0; j < out.cols; j++) {
			sum += out.at<uchar>(i, j);
		}
		if (sum < 255 * 6 && start == 0) {
			start = i;
		}
		if (sum > 255 * 6 && start > 0) {
			end = i;
			break;
		}
	}
	if(start > 0)
		return (start + end) / 2;
	else {
		return 0;
	}
}

int getBaselinePoints_aomian(cv::Mat& img, int* points)
{
	int res = 0;
	cv::Mat grey;
	int maxCorners = 5;
	std::vector<cv::Point2f> corners;
	float qualityLevel = 0.02f;
	int minDistance = 10, blockSize = 9, gradientSize = 3;
	bool useHarrisDetector = false;
	double k = 0.04; 
	cv::cvtColor(img, grey, cv::COLOR_BGR2GRAY);

	int offset_x = grey.cols / 10;
	int offset_y = getMidPoint(grey);
	cv::Mat roi = grey(cv::Rect(offset_x, offset_y, grey.cols - 2 * offset_x, grey.rows - offset_y));
	cv::goodFeaturesToTrack(roi, corners, maxCorners, qualityLevel, minDistance, cv::noArray(), blockSize, useHarrisDetector, k);

	if (corners.size() >= 2) {
		cv::cornerSubPix(
			roi, // Input image
			corners, // Vector of corners (input and output)
			cv::Size(5, 5), // Half side length of search window
			cv::Size(-1, -1), // Half side length of dead zone (-1=none)
			cv::TermCriteria(
				cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
				40, // Maximum number of iterations
				0.001 // Minimum change per iteration
			)
		);
		if ( offset_x + corners[0].x < img.cols / 2) {
			res = 1;
			points[0] = offset_x + (int)corners[0].x;
			points[1] = offset_y + (int)corners[0].y;
			for (int i = 1; i < (int)corners.size(); i++)
			{
				if (offset_x + corners[i].x > img.cols / 2) {
					res = 2;
					points[2] = offset_x + (int)corners[i].x;
					points[3] = offset_y + (int)corners[i].y;
					break;
				}
			}
			if (res == 1) {
				points[2] = img.cols - points[0];
				points[3] = points[1];
			}
		}
		else {
			res = 1;
			points[2] = offset_x + (int)corners[0].x;
			points[3] = offset_y + (int)corners[0].y;
			for (int i = 1; i < (int)corners.size(); i++)
			{
				if (offset_x + corners[i].x < img.cols / 2) {
					res = 2;
					points[0] = offset_x + (int)corners[i].x;
					points[1] = offset_y + (int)corners[i].y;
					break;
				}
			}
			if (res == 1) {
				points[0] = img.cols - points[2];
				points[1] = points[3];
			}
		}
	}	
	return res;
}

// Function to calculate the distance from point (x, y) to line through points (x1, y1) and (x2, y2)
double distanceToLine(double x, double y, double x1, double y1, double x2, double y2) 
{
	// Calculate the numerator
	double numerator = std::abs((y2 - y1) * x - (x2 - x1) * y + x2 * y1 - y2 * x1);

	// Calculate the denominator (length of the line segment vector)
	double denominator = std::sqrt((y2 - y1)* (y2 - y1) + (x2 - x1)* (x2 - x1));

	// Return the distance
	return numerator / denominator;
}

double distanceToLine(const cv::Point2f& pt, const cv::Point2f& p1, const cv::Point2f& p2)
{
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	double denominator = std::sqrt(dx * dx + dy * dy);
	if (denominator < 1e-6)
		return -1;  // Prevent divide-by-zero for very short segments

	double numerator = std::abs(dy * pt.x - dx * pt.y + p2.x * p1.y - p2.y * p1.x);
	return numerator / denominator;
}

// Function to calculate the center of a rotated rectangle
void centerOfRotatedRectangle(double x1, double y1, double x2, double y2, double height, float& cx, float& cy) 
{
	// Midpoint of the bottom side
	double mx = (x1 + x2) / 2;
	double my = (y1 + y2) / 2;

	// Vector of the bottom side
	double vx = x2 - x1;
	double vy = y2 - y1;

	// Length of the bottom side
	double length = std::sqrt(vx * vx + vy * vy);

	// Unit vector perpendicular to the bottom side
	double ux = -vy / length;
	double uy = vx / length;

	// Calculate the center
	cx = (float)(mx - (height / 2.0) * ux);
	cy = (float)(my - (height / 2.0) * uy);
}

// Helper: 8-connected neighbors
std::vector<cv::Point> getNeighbors(const cv::Mat& img, const cv::Point& pt, int max_gap, const cv::Mat& visited) 
{
	std::vector<cv::Point> neighbors;
	for (int dy = -max_gap; dy <= max_gap; ++dy) {
		for (int dx = -max_gap; dx <= max_gap; ++dx) {
			if (dx == 0 && dy == 0) continue;
			int nx = pt.x + dx, ny = pt.y + dy;
			if (0 <= nx && nx < img.cols && 0 <= ny && ny < img.rows) {
				if (img.at<uchar>(ny, nx) > 0 && visited.at<uchar>(ny, nx) == 0) {
					neighbors.emplace_back(nx, ny);
				}
			}
		}
	}
	return neighbors;
}

// Helper: trace one branch
int traceBranch(
	const cv::Mat& img, 
	cv::Point start_pt, 
	int max_gap, 
	const cv::Point& _leftRef,
	const cv::Point& _rightRef,
	int distThres,
	cv::Mat& visited,
	std::vector<cv::Point>& _outContour
)  {
	_outContour.push_back(start_pt);
	visited.at<uchar>(start_pt) = 1;
	bool leftPassed = false;
	bool rightPassed = false;
	int min_dis1 = 10000, min_dis2 = 10000;
	int direction = 0;

	cv::Point current = start_pt;
	while (true) {
		auto neighbors = getNeighbors(img, current, max_gap, visited);
		if (neighbors.empty())
			break;

		auto next_pt = *std::min_element(neighbors.begin(), neighbors.end(),
			[&](const cv::Point& a, const cv::Point& b) {
				return distance(a, current) < distance(b, current);
			});

		visited.at<uchar>(next_pt) = 1;
		_outContour.push_back(next_pt);
		current = next_pt;

		auto leftDist = distance(next_pt, _leftRef);
		min_dis1 = std::min(min_dis1, leftDist);
		if (leftDist <= distThres) {
			leftPassed = true;
			if (leftDist > min_dis1) {
				direction = -1;
				break;
			}
		}
		else if (leftPassed) {
			direction = -1;
			break;
		}

		auto rightDist = distance(next_pt, _rightRef);
		min_dis2 = std::min(min_dis2, rightDist);
		if (rightDist <= distThres) {
			rightPassed = true;
			if (rightDist > min_dis2) {
				direction = 1;
				break;
			}
		}
		else if (rightPassed) {
			direction = 1;
			break;
		}

	}
	if (direction == 0) {
		if (leftPassed && !rightPassed)
			direction = -1;
		else if (!leftPassed && rightPassed)
			direction = 1;
		else {
			if (min_dis1 < min_dis2 && min_dis1 < 5000) {
				direction = -1;
			}
			else if (min_dis2 < min_dis1 && min_dis2 < 5000) {
				direction = 1;
			}
		}
	}
	return direction;
}

// Main function: trace two connected curves from top point
bool traceTwoBranchesFromThinnedImage(
	const cv::Mat& thinned_img, 
	std::vector<cv::Point>& _outLeft, 
	std::vector<cv::Point>& _outRight, 
	const cv::Point& _leftRef,
	const cv::Point& _rightRef,
	int distThres,
	int max_gap
) {
	CV_Assert(thinned_img.type() == CV_8UC1);
	cv::Mat visited = cv::Mat::zeros(thinned_img.size(), CV_8UC1);

	// 1. Find topmost point
	cv::Point top_pt(-1, -1);

	const int margin_x = thinned_img.cols / 10;

	for (int y = 0; y < thinned_img.rows && top_pt.x == -1; ++y) {
		for (int x = margin_x; x < thinned_img.cols - margin_x; ++x) {
			if (thinned_img.at<uchar>(y, x) > 0) {
				top_pt = { x, y };
				break;
			}
		}
	}
	if (top_pt.x == -1)
		return false;

	visited.at<uchar>(top_pt) = 1;
	auto neighbors = getNeighbors(thinned_img, top_pt, max_gap, visited);
	if (neighbors.size() < 2)
		return false;

	// 2. Trace two branches
	std::vector<cv::Point> part1;
	int direction = traceBranch(thinned_img, neighbors[0], max_gap, _leftRef, _rightRef, distThres, visited, part1);
	if (direction == -1)
		_outLeft = part1;
	else if (direction == 1)
		_outRight = part1;

	std::vector<cv::Point> part2;
	int direction2 = traceBranch(thinned_img, neighbors[1], max_gap, _leftRef, _rightRef, distThres, visited, part2);
	if (direction == 0) {
		if (direction2 == -1) {
			_outLeft = part2;
			_outRight = part1;
		}
		else if (direction2 == 1) {
			_outRight = part2;
			_outLeft = part1;
		}
	}
	else {
		if (direction == -1)
			_outRight = part2;
		else if (direction == 1)
			_outLeft = part2;
	}
	_outLeft.insert(_outLeft.begin(), top_pt);
	_outRight.insert(_outRight.begin(), top_pt);
	return true;
}

std::vector<cv::Point> 
extractValidContours(
	const std::vector<cv::Point>& fullContour,
	const cv::Point& refPt,
	int distThr
) {
	bool passed = false;
	int min_dis = 10000;
	int n = (int)fullContour.size();
	std::vector<cv::Point> retContour;
	for (int i = 0; i < n; ++i) {
		cv::Point pt = fullContour[i];
		auto leftDist = distance(pt, refPt);
		if (leftDist <= distThr) {
			passed = true;
			if (leftDist > min_dis)
				break;
			min_dis = leftDist;
		}
		else if (passed)
			break;
		retContour.push_back(pt);
	}
	return retContour;
}

double signedDistanceToLine(const cv::Point2f& pt, const cv::Point2f& A, const cv::Point2f& B)
{
	cv::Point2f AB = B - A;
	cv::Point2f AP = pt - A;
	double len = std::hypot(AB.x, AB.y);
	if (len == 0.0) 
		return std::hypot(AP.x, AP.y);  // Avoid division by zero
	return (AB.x * AP.y - AB.y * AP.x) / len;     // Signed distance
}
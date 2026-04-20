#include "detect_utils_on_horizonal.h"
#include "common.h"
#include <stack>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "../cvLib/cvcommon.h"
//#include "..\utils\common_math.h"
#include "../imgproc/EdgeSlope.h"
#include "../fitting/fit_util.h"

#define DEBUG_FLATNESS_LOSS	0

#if DEBUG_FLATNESS_LOSS
#define NOMINMAX
#include <windows.h>

int print_log(const char* format, ...)
{
	static char s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);	
	OutputDebugStringA(s_printf_buf);
	return 0;
}
#endif

// When using the subpixel edge detector, extra margins are added at the top and bottom of the image during edge detection.
const int CONTOUR_Y_MARGIN = 5; 

inline bool inBounds(const cv::Mat& m, int y, int x) {
	return x >= 0 && y >= 0 && (unsigned)x < (unsigned)m.cols && (unsigned)y < (unsigned)m.rows;
}
inline bool isOnEdge(const cv::Mat& edges, int y, int x) {
	return inBounds(edges, y, x) && edges.at<uchar>(y, x) > 0;
}

inline double angleBetween(const cv::Point& a, const cv::Point& b)
{
	cv::Point2f v = b - a;
	return std::atan2(v.y, v.x) * 180.0 / CV_PI;
};


int horizontalDistanceToLine(const cv::Vec4i& line, int cx)
{
	int x1 = std::min(line[0], line[2]);
	int x2 = std::max(line[0], line[2]);

	if (cx < x1)
		return x1 - cx;
	if (cx > x2)
		return cx - x2;
	return 0;
}

cv::Point2f unitDirectionVector(const cv::Vec4i& l)
{
	int dx = l[2] - l[0];
	int dy = l[3] - l[1];
	double norm = sqrt(dx * dx + dy * dy);
	if(norm == 0) {
		return cv::Point2f(0.0f, 0.0f); // Avoid division by zero
	}
	return cv::Point2f((float)(dx / norm), (float)(dy / norm));
}

bool 
correctSkewByLines(
	const cv::Mat& img,
	cv::Mat& res,
	double& angle
) {
	cv::Mat gray, bin, edges;

	// Step 1: Convert image to grayscale and apply binary threshold
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
	cv::threshold(gray, bin, 100, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);

	// Step 2: Detect edges using Canny
	cv::Canny(bin, edges, 50, 150);

	// Step 3: Detect line segments using Probabilistic Hough Transform
	std::vector<cv::Vec4i> lines;
	int minLineLength = gray.cols / 10;
	cv::HoughLinesP(edges, lines, 1, CV_PI / 180.0, 25, minLineLength, minLineLength / 4);

	if (lines.empty()) {
		res = img.clone();
		angle = 0.0;
		return false;
	}

	std::vector<std::pair<int, cv::Vec4i>> filteredLines;
	// Step 4: Filter out nearly vertical lines (dx < 5)
	for (const auto& l : lines) {
		int dx = std::abs(l[2] - l[0]);
		int dy = std::abs(l[3] - l[1]);
		if (dx >= 5) {
			int len = dx + dy;
			filteredLines.emplace_back(len, l);
		}
	}

	if (filteredLines.empty()) {
		res = img.clone();
		angle = 0.0;
		return false;
	}

	// Step 5: Sort filtered lines by length descending
	std::sort(filteredLines.begin(), filteredLines.end(),
		[](const auto& a, const auto& b) { return a.first > b.first; });

	// Use the longest line as reference for angle direction
	const auto& baseLine = filteredLines[0].second;
	const int cx = gray.cols / 2;
	auto baseVec = unitDirectionVector(baseLine);
	int bestIndex = 0;
	int bestDistance = horizontalDistanceToLine(baseLine, cx);

	// Step 6: Choose the best aligned line (same direction and closest to center)
	for (size_t i = 1; i < filteredLines.size(); ++i) {
		double ratio = static_cast<double>(filteredLines[i].first) / filteredLines[0].first;
		if (ratio < 0.6)
			break; // Skip much shorter lines

		auto curVec = unitDirectionVector(filteredLines[i].second);
		double dotProduct = std::abs(baseVec.x * curVec.x + baseVec.y * curVec.y);
		if (dotProduct < 0.75)
			continue; // Not similarly oriented

		int curDist = horizontalDistanceToLine(filteredLines[i].second, cx);
		if (curDist < bestDistance) {
			bestDistance = curDist;
			bestIndex = static_cast<int>(i);
		}
	}

	// Step 7: Calculate skew angle from the best line
	const auto& optLine = filteredLines[bestIndex].second;
	int dx = optLine[2] - optLine[0];
	int dy = optLine[3] - optLine[1];
	angle = std::atan2(static_cast<double>(dy), static_cast<double>(dx)) * 180.0 / CV_PI;

	// Step 8: If angle exceeds threshold, rotate the image
	if (std::abs(angle) > 3.0) {
		cv::Point2f center(img.cols / 2.0f, img.rows / 2.0f);
		cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
		cv::warpAffine(img, res, rot, img.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));
		return true; // Skew corrected
	}

	res = img.clone();
	return false; // No rotation needed
}

void rotatePoints(int cols, int rows, double angle, std::vector<cv::Point2f>& pts)
{
	angle = -angle * CV_PI / 180;
	double cos_angle = cos(angle), sin_angle = sin(angle);
	double cx = cols / 2.0, cy = rows / 2.0;
	for (auto& pt : pts) {
		if (pt.x < 0 || pt.y < 0)
			continue;
		double dx = pt.x - cx, dy = pt.y - cy;
		pt.x = (float)(cx + dx * cos_angle + dy * sin_angle);
		pt.y = (float)(cy - dx * sin_angle + dy * cos_angle);
	}
}

#if !_DEV_UPDATE_SUBPXL_ON_HORIZONTAL
void 
find_contours_simple(
	const cv::Mat& img, 
	cv::Mat& gray,
	cv::Mat& edges, 
	std::vector<std::vector<cv::Point>>& contours
) {
	cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

#if 0
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
	cv::Canny(gray, edges, 50, 150);

	cv::findContours(edges, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
#else
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
	cv::Canny(gray, edges, 100, 200); // 50, 150
	cv::Mat dilated, eroded;
	cv::Mat kernel2 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 1));
	cv::dilate(edges, dilated, kernel2);
	cv::erode(dilated, edges, kernel2);
	cv::findContours(edges, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
#endif
}
#endif

// 寻找覆盖面积最大的轮廓
int 
find_max_contour(
	const std::vector<std::vector<cv::Point>>& contours, 
	int flag
) {
	int nLen = (int)contours.size();
	int max_ind = -1;
	double max_value1 = -1.0, max_value2 = -1.0;
	for (int i = 0; i < nLen; i++) {
		double cur_value1 = 0.0, cur_value2 = 0.0;
		bool update = false;
		const auto& segment = contours[i];

		cv::Rect rt = cv::boundingRect(segment);
		if (flag == 0) { // 透镜形状；宽度、面积
			cur_value1 = rt.width;
			cur_value2 = rt.area();
			if (max_value1 < cur_value1)
				update = true;
			else if (max_value1 > cur_value1)
				update = false;
			else if (max_value2 < cur_value2)
				update = true;
		}
		else if (flag == 1) { // 浸泡——球；面积
			cur_value1 = rt.area();
			if (max_value1 < cur_value1)
				update = true;
		}
		if (update) {
			max_value1 = cur_value1;
			max_value2 = cur_value2;
			max_ind = i;
		}
	}
	return max_ind;
}

void 
collections_inner(
	const cv::Mat& edges, 
	int dx, 
	int dy, 
	int& x, 
	int& y, 
	bool second
) {
	uchar pixel = edges.at<uchar>(y, x);
	int y0 = -1;
	int prev_flg = -1;
	while (x > 0 && x < edges.cols - 1 && y < edges.rows - 1) {
		if(isOnEdge(edges, y + dy, x)) {
			if (y0 < 0) 
				y0 = y;
			y += dy;
			prev_flg = 0;
			continue;
		}
		if(isOnEdge(edges, y, x + dx)) {
			y0 = -1;
			x += dx;
			prev_flg = 1;
			continue;
		}
		if(isOnEdge(edges, y + dy, x + dx)) {
			x += dx;
			y += dy;
			y0 = -1;
			prev_flg = 2;
			continue;
		}
		if (!second && dx != 0) {
			auto u = isOnEdge(edges, y - 1, x + 1);
			auto m = isOnEdge(edges, y, x + 1);
			auto d = isOnEdge(edges, y + 1, x + 1);
			if (dx < 0) {
				if (x < edges.cols - 1 && y < edges.rows - 1 && y > 0) {
					if (u && !m && d) {
						x++;
						y++;
						prev_flg = 3;
						continue;
					}
				}
				if (x < edges.cols - 1 && y < edges.rows - 2) {
					m = isOnEdge(edges, y + 1, x + 1);
					d = isOnEdge(edges, y + 2, x);
					if (m && d) {
						y += 2;
						continue;
					}
				}
			}
			else {
				break;
				if (x > 0 && y < edges.rows - 1 && y > 0) {
					if (u && !m && d) {
						x--;
						y++;
						prev_flg = 3;
						continue;
					}
				}
			}
		}
		break;
	}
	if (y0 > -1) {
		y = (y + y0 + 1) / 2;
	}
}

void filterHorizontalSegments(
	const std::vector<cv::Point>& edgePoints,
	std::vector<cv::Point>& candidatePoints,
	int yBottom,
	int yBaseline,
	int minLength,
	int lengthRatio
) {
	if (edgePoints.size() < 2)
		return;

	std::vector<cv::Point> transitions;
	int lastStartIndex = 0;
	transitions.push_back(edgePoints[0]);

	// Group into segments with y-change
	int numSegments = (int)edgePoints.size();
	for (int i = 1; i < numSegments - 1; ++i) {
		if (edgePoints[i].y == edgePoints[i - 1].y)
			continue;

		if (i - 1 != lastStartIndex)
			transitions.push_back(edgePoints[i - 1]);

		transitions.push_back(edgePoints[lastStartIndex = i]);
	}

	transitions.push_back(edgePoints.back());

	// Analyze segments
	int previousLength = -1;
	for (size_t i = 1; i < transitions.size(); ++i) {
		const cv::Point& p1 = transitions[i - 1];
		const cv::Point& p2 = transitions[i];

		// Skip if it's the baseline or too far above the bottom
		if (p2.y == yBaseline || p2.y < yBottom - 15)
			continue;

		// Must be a horizontal line (same y)
		if (p2.y != p1.y)
			continue;

		int currentLength = std::abs(p2.x - p1.x) + 1;
		if (previousLength > -1 && currentLength > minLength) {
			if (currentLength > previousLength * lengthRatio) {
				candidatePoints.push_back(p1);  // Save starting point of strong segment
				return;
			}
		}
		previousLength = currentLength;
	}
}


void filterVerticalSegments(
	const std::vector<cv::Point>& pts,
	std::vector<cv::Point>& pt_opts,
	int minLength
) {
	if (pts.size() < 2)
		return;

	int verticalLength = 1;
	int prevVerticalLength = -1;

	int bestX = -1;
	int bestY = -1;

	for (size_t i = 1; i < pts.size(); ++i) {
		if (pts[i].x == pts[i - 1].x) {
			verticalLength++;
			continue;
		}

		// Detect gap if current segment is very short and x jumps suddenly
		if (verticalLength == 1 && bestX >= 0) {
			int dx = std::abs(bestX - pts[i].x);
			if (dx > 2) {
				pt_opts.emplace_back(bestX, bestY);
				return;
			}
		}

		// End of a vertical segment
		if (verticalLength > minLength) {
			bestX = pts[i - 1].x;
			bestY = pts[i - 1].y - (verticalLength - 1) / 2;
		}

		// Heuristic: sudden growth in vertical segment
		if (prevVerticalLength > minLength && verticalLength > prevVerticalLength * 6) {
			if (bestX >= 0)
				pt_opts.emplace_back(bestX, bestY);
			return;
		}

		prevVerticalLength = verticalLength;
		verticalLength = 1;
	}

	// Final fallback
	/*if (verticalLength > minLength && bestX >= 0) {
		pt_opts.emplace_back(bestX, bestY);
	}*/
}


cv::Point findContactByDropShadow(
	const cv::Mat& edges,
	const std::vector<cv::Point>& pts,
	int dx,
	int bandWidth,
	int verticalLookahead
) {
	cv::Point bestPt = pts.back();
	int minDensity = INT_MAX;

	for (int i = static_cast<int>(pts.size()) - 1; i >= 0; --i) {
		const auto& p = pts[i];
		if (p.y + verticalLookahead >= edges.rows) 
			continue;

		int count = 0;
		for (int dy = 1; dy <= verticalLookahead; ++dy) {
			for (int d = -bandWidth; d <= bandWidth; ++d) {
				int xx = p.x + d;
				int yy = p.y - dy;
				if (isOnEdge(edges, yy, xx)) {				
						count++;
				}
			}
		}

		// Choose the point where density starts to jump
		if (count < minDensity) {
			minDensity = count;
			bestPt = p;
		}
		else if (count > minDensity + 5) {
			// We entered a drop body — step back
			break;
		}
	}

	return bestPt;
}


cv::Point findContactByFlatnessLoss(
	const std::vector<cv::Point>& pts,
	int wndSz,
	double angleMargin
) {
	int sz = (int)pts.size();
	
	int resIdx = sz - 1; // fallback to last point
	if (sz >= 2 * wndSz + 1) {		

		// if you want to see the subpixel smoothed edge points via opencv cornerSubPix
		// but it seems not work well than our own fitting, so commented out, Jewel, 2025-09-03
		/*
		std::vector<cv::Point2f> edgesF;
		edgesF.reserve(sz);
		for (const auto& p : pts) {
			edgesF.emplace_back((float)p.x, (float)p.y);
		}
		cv::cornerSubPix(
			edges, // Input image
			edgesF, // Vector of corners (input and output)
			cv::Size(5, 5), // Half side length of search window
			cv::Size(-1, -1), // Half side length of dead zone (-1=none)
			cv::TermCriteria(
				cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS,
				40, // Maximum number of iterations
				0.001 // Minimum change per iteration
			)
		);
		*/
		SlopeIntegral integral;
		integral.build(pts, wndSz);
		const int scanStart = sz - 1 - wndSz;
		const double flattenThreshold = angleMargin;  // e.g., ≤ 5°
		const double slopeChangeThreshold = angleMargin + angleMargin;     // min diff to consider flattening

		double lastAngle = integral.getSlopeAt(scanStart, false);
		double slopePrev = lastAngle;
		if (lastAngle < flattenThreshold) {
			for (int i = scanStart-1; i >= wndSz; i--) {
				//double slopePrev = integral.getSlopeAt(i - 1, wndSz, false);
				double slopeNext = integral.getSlopeAt(i, false);
#if DEBUG_FLATNESS_LOSS
				char psz[256];
				sprintf_s(psz, 256, "pts[%d] = (%d, %d), %f\n", i, pts[i].x, pts[i].y, slopeNext);
				OutputDebugStringA(psz);
#endif//DEBUG_FLATNESS_LOSS
				if (std::abs(slopeNext) > flattenThreshold &&
					std::abs(slopePrev) < flattenThreshold) {
					resIdx = i;
					break;
				}
				slopePrev = slopeNext;
			}
		}
	}
	while (resIdx > 0 && pts[resIdx].y == pts[resIdx - 1].y) {
		resIdx--;
	}
	if(resIdx >= 0)
		return pts[resIdx];
	return pts.back(); // fallback
}

// iterative deepening search for longest path of edge pixels
// iteractive dynamic programming algorithm with memoization
int collectEdgePoints(
	const cv::Mat& edges,
	int dx,
	int& x,
	int& y,
	std::vector<cv::Point>& outPts,
	bool boundary_supplement
) {
	struct PathState {
		cv::Point current;
		int index;
		int pathLen;
	};
	
	const int rows = edges.rows;
	const int cols = edges.cols;
	const int dy = 1;
	const int maxGap = boundary_supplement ? 7 : 1;
	const double maxSlopeDeviation = 1.0;  // Slope difference threshold for noise
	constexpr int slopeEvalLen = 10;
	const int similarThreshold = std::max(2, maxGap + 1);

	// --- Step 1. Define main scan directions ---
	std::vector<cv::Point> mainDir = { {dx,0}, {0,dy} , {dx,dy} };
	
	/*
	* for full extended search, but not need now
	std::vector<cv::Point> extendedDir;
	for (int r = 0; r <= maxGap; r++) {
		for (int c = 0; c <= maxGap; c++) {
			if (r == 0 && c == 0)
				continue; // skip the zero step
			if (r + c > maxGap)
				continue; // skip too long steps
			extendedDir.emplace_back(dx * c, dy * r);
		}
	}
	*/

	// --- Step 2. Initialization ---
	cv::Point start(x, y);
	outPts.clear();
	std::stack<PathState> stack;
	std::vector<cv::Point> allPoints;  // reconstruct paths only when needed
	std::vector<int> parents;
	std::unordered_map<int, int> visited; // key=y*cols+x, value=maxPathLen

	PathState initState{ start, 0, 1 };
	parents.push_back(-1);
	allPoints.push_back(start);
	stack.push(initState);
	int bestLen = 0;

	// --- Step 3. DFS Search ---
	while (!stack.empty()) {
		auto state = stack.top();
		stack.pop();
		auto cur = state.current;		
		int curKey = cur.y * cols + cur.x;
		int curPathLen = state.pathLen;

		// Skip if we already found an equal or longer path to this pixel
		auto it = visited.find(curKey);
		if (it != visited.end() && it->second >= curPathLen)
			continue;
		visited[curKey] = curPathLen;

		// --- Step 3.1 Update best path ---
		bool updated = false;
		if (outPts.empty()) {
			updated = true;
		}
		else {
			const auto& lastOut = outPts.back();
			const auto& lastCur = cur;

			// Difference threshold for "similar" length
			int lenDiff = curPathLen - (int)outPts.size();
			int similarThreshold = std::max(2, maxGap + 1);

			if (lenDiff > similarThreshold) {
				// New path is significantly longer -> always choose it
				updated = true;
			}
			else if (lenDiff < -similarThreshold) {
				// Current best path is significantly longer -> keep it
				updated = false;
			}
			else {
				// Similar lengths -> prefer forward x-progress
				bool bester = false;
				if (dx < 0)
					bester = (lastCur.x < lastOut.x);
				else if(dx > 0)
					bester = (lastCur.x > lastOut.x);

				if (bester)
					updated = true;
				else if(lastCur.x == lastOut.x) {
					// Same x -> prefer longer path
					updated = (curPathLen > outPts.size());
				}
			}
		}
		if (updated) {
			// Reconstruct path using parent chain
			outPts.clear();
			int idx = state.index;
			while(idx != -1) {
				outPts.push_back(allPoints[idx]);
				idx = parents[idx];
			}
			std::reverse(outPts.begin(), outPts.end());
			bestLen = (int)outPts.size();
		}
		
		// --- Step 3.2 Collect neighbors ---
		std::vector<cv::Point> candiates;		
		for (const auto& dir : mainDir) {
			cv::Point next = state.current + dir;

			// If immediate neighbor is edge -> add directly
			if (isOnEdge(edges, next.y, next.x)) {
				candiates.push_back(next);
				continue;
			}

			// Otherwise, try extended search in that direction			
			if (dir.x == 0) { // Vertical extension
				// Only extend point that meet first
				for (int step = 2; step <= maxGap; ++step) {
					cv::Point ext = state.current + cv::Point(0, step * dy);
					if (isOnEdge(edges, ext.y, ext.x)) {
						candiates.push_back(ext);
						break;
					}
				}
			}
			else if (dir.y == 0) { // Horizontal extension
				// Only extend point that meet first
				for (int step = 2; step <= maxGap; ++step) {
					cv::Point ext = state.current + cv::Point(step * dx, 0);
					if (isOnEdge(edges, ext.y, ext.x)) {
						candiates.push_back(ext);
						break;
					}
				}
			}
			else { // Diagonal extension
				// Try "/"-shape extensions first (closer to main direction)
				for (int step = 2; step < maxGap; ++step) {
					bool found_extension = false;
					for (int s = 0; s < step; ++s) {
						cv::Point ext1 = state.current + cv::Point((s + 1)*dx, (step - s)*dy);
						if (isOnEdge(edges, ext1.y, ext1.x)) {
							found_extension = true;
							candiates.push_back(ext1);
						}
					}
					if (found_extension)
						break;
				}
			}
		}

		// --- Step 3.3 Push candidates onto stack ---
		for(const auto& next : candiates) {
			// Avoid cycles
			int nextKey = next.y * cols + next.x;
			const auto& nextIt = visited.find(nextKey);
			if (nextIt != visited.end() && nextIt->second >= curPathLen + 1)
				continue; 
			
			// --- Step 3.3.1 Check slope noise ---
			double cosAngle = -1.0;
			if (curPathLen >= slopeEvalLen) {
				std::vector<cv::Point2f> lastPts;
				lastPts.reserve(slopeEvalLen);

				// Go backward up to slopeEvalLen points
				int idx = state.index;
				for (int i = 0; i < slopeEvalLen && idx != -1; ++i) {
					lastPts.push_back(allPoints[idx]);
					idx = parents[idx];
				}

				// We collected them in reverse → reverse to chronological order
				std::reverse(lastPts.begin(), lastPts.end());

				// Compute averages of first half and second half
				cv::Point2f sum1(0, 0), sum2(0, 0);
				int half = slopeEvalLen / 2;

				for (int i = 0; i < half; ++i)
					sum1 += lastPts[i];
				for (int i = 0; i < half; ++i)
					sum2 += lastPts[half + i];

				cv::Point2f avg1 = sum1 * (1.0f / half);
				cv::Point2f avg2 = sum2 * (1.0f / half);

				cv::Point2f dirVec = avg2 - avg1;
				cv::Point2f newVec = cv::Point2f(next.x, next.y) - avg2;

				double len1 = cv::norm(dirVec);
				double len2 = cv::norm(newVec);
				if (len1 > 1e-3 && len2 > 1e-3) {
					double cosAngle = dirVec.dot(newVec) / (len1 * len2);
					if (cosAngle < 0.5)
						continue; // block noisy sharp turns
				}
			}
			
			// --- Step 3.3.2 Accept candidate
			int myIndex = (int)allPoints.size();
			allPoints.push_back(next);
			parents.push_back(state.index);
			stack.push({ next, myIndex, state.pathLen + 1 });
		}
	}

	// --- Step 4. Final adjustment for vertical tail segments ---
	if (!outPts.empty()) {
		cv::Point last = outPts.back();
		x = last.x;
		y = last.y;

		int start = static_cast<int>(outPts.size()) - 1;
		while (start > 0) {
			auto curr = outPts[start];
			auto prev = outPts[start - 1];
			if (curr.x != prev.x)
				break;
			if (curr.y - prev.y > 1 || prev.y - curr.y > 1)
				break;
			--start;
		}
		if (start < static_cast<int>(outPts.size()) - 1) {
			y = (y + outPts[start].y) / 2;
		}
	}
	return y;
}

std::vector<cv::Point>
scan_lens_contour(
	const cv::Mat& edges, 
	const cv::Point& fromPt, 
	ScanDir dir,
	bool boundary_supplement,
	bool rotated
) {
	std::vector<cv::Point> candPts;
	std::vector<cv::Point> collected_pts1, collected_pts;
	int scan_x = fromPt.x, scan_y = fromPt.y;
	int dx = (dir == eScanLeftDown) ? -1 : 1;

	int lastY = collectEdgePoints(edges, dx, scan_x, scan_y, collected_pts, boundary_supplement);

	if (collected_pts.empty()) {		
		return candPts;
	}

	// Start refining collected points
	std::vector<cv::Point> candidate_pts = { cv::Point(scan_x, scan_y) };

	// x-方向
	filterHorizontalSegments(collected_pts, candidate_pts, scan_y, fromPt.y);
	// y-方向
	filterVerticalSegments(collected_pts, candidate_pts);
	
	// Conditional backtracking when near horizontal boundaries
	const int edge_margin = 2;
	bool near_edge = (dx > 0 && scan_x > edges.cols - 1 - edge_margin) ||
		(dx < 0 && scan_x < edge_margin);

	if (candidate_pts.size() == 1 && near_edge) {
		auto newCand = findContactByFlatnessLoss(collected_pts);
		candidate_pts.push_back(newCand);
		// back_track(edges, collected_pts, candidate_pts, dx, false);
	}

	int optimal_index = 0;
	int min_dx = std::abs(fromPt.x - candidate_pts[0].x);
	for (size_t i = 1; i < candidate_pts.size(); i++) {
		int current_dx = std::abs(fromPt.x - candidate_pts[i].x);
		if (current_dx < min_dx) {
			min_dx = current_dx;
			optimal_index = static_cast<int>(i);
		}
	}

	candPts.push_back(candidate_pts[optimal_index]);
	if(lastY != scan_y) {
		// If the last Y is different, we can add it as well
		candPts.push_back(cv::Point(scan_x, lastY));
	}

	return candPts;
}


int 
check_lens_contour_for_pin(
	cv::Mat& edges, 
	std::vector<cv::Point>& contour, 
	cv::Point* pt, 
	bool boundary_supplement
) {
	if (contour.empty())
		return 0;
	int length = (int)contour.size();
	if (contour[0].y > contour[length - 1].y) {
		std::reverse(contour.begin(), contour.end());
	}
	if (contour[0].y > CONTOUR_Y_MARGIN)
		return 0;
	cv::Rect rt = cv::boundingRect(contour);
	int cx = rt.x + rt.width / 2;
	bool is_left = (cx < edges.cols / 2);
	int nLen = (int)contour.size(), min_ind = -1;
	int dx = std::min(std::max(rt.width / 10, 20), rt.width / 2);
	for (int i = 1; i < nLen; i++) {
		if (is_left) {
			if (contour[i].x < contour[0].x - dx) {
				min_ind = i;
				break;
			}
		}
		else {
			if (contour[i].x > contour[0].x + dx) {
				min_ind = i;
				break;
			}
		}
	}
	if (min_ind < 0)
		return 0;
	cv::Point pt0(contour[min_ind]);
	int bb_up = rt.x + rt.height - 5;
	for (int i = min_ind + 1; i < nLen; i++) {
		if (is_left) {
			if (contour[i].x > pt0.x)
				break;
		}
		else {
			if (contour[i].x < pt0.x)
				break;
		}
		if (contour[i].y > bb_up)
			break;
		if (contour[i].y < contour[min_ind].y)
			min_ind = i;
	}

	pt0 = contour[min_ind];
	std::vector<cv::Point> boundPts;
	if (is_left)
		boundPts = scan_lens_contour(edges, pt0, eScanLeftDown, boundary_supplement); // 向左
	else
		boundPts = scan_lens_contour(edges, pt0, eScanRightDown, boundary_supplement); // 向右
	if (boundPts.empty()) 
		pt[0] = pt0; // fallback to the original point
	else
		pt[0] = boundPts[0];

	return 1;
}

LensContourType 
classifyContourBoundary(
	const std::vector<cv::Point>& contour, 
	cv::Point& _outPtLeft,
	cv::Point& _outPtRight
) {
	cv::Rect bounds = cv::boundingRect(contour);
	if (bounds.height == 1)
		return eNotBoundaryFound;

	int contourSize = static_cast<int>(contour.size());

	auto analyzeEdge = [&](int yTarget, int& left, int& right, int& uniqueXCount, int& pointCount) {
		std::set<int> xCoords;
		left = bounds.x + bounds.width;
		right = bounds.x - 1;
		pointCount = 0;

		for (const auto& pt : contour) {
			if (pt.y == yTarget) {
				++pointCount;
				if (xCoords.insert(pt.x).second) {
					left = std::min(left, pt.x);
					right = std::max(right, pt.x);
				}
			}
		}
		uniqueXCount = static_cast<int>(xCoords.size());
	};

	// Check top edge first
	int leftX, rightX, uniqueX, totalPoints;

	// Analyze Top Edge
	int topY = bounds.y;
	analyzeEdge(topY, leftX, rightX, uniqueX, totalPoints);

	double width = static_cast<double>(bounds.width);
	double coverage = (rightX - leftX) / width;
	double symmetry = std::abs(((leftX + rightX) * 0.5 - (bounds.x + width * 0.5)) / width * 2.0);

	if (uniqueX < 3 && (totalPoints == 1 || totalPoints == 2 || totalPoints == 4)) {
		if (symmetry < 0.7) {
			_outPtLeft = { leftX, topY };
			_outPtRight = { rightX, topY };
			return eTopBoundary;
		}
	}
	else if (uniqueX > 2) {
		if (coverage > 0.9 && symmetry < 0.01 && bounds.height < 2) {
			_outPtLeft = { leftX, topY + bounds.height / 2 };
			_outPtRight = { rightX, topY + bounds.height / 2 };
			return eTopThinSpecial;
		}

		if (coverage < 0.35 && symmetry < 0.30) {
			_outPtLeft = { leftX, topY };
			_outPtRight = { rightX, topY };
			return eTopBoundary;
		}
	}

	// Analyze Bottom Edge
	int bottomY = bounds.y + bounds.height - 1;
	analyzeEdge(bottomY, leftX, rightX, uniqueX, totalPoints);

	if (uniqueX < 3 && (totalPoints == 2 || totalPoints == 4)) {
		_outPtLeft = { leftX, bottomY };
		_outPtRight = { rightX, bottomY };
		return eBottomBoundary;
	}

	return eNotBoundaryFound;
}

cv::Point find_top_point(
	const std::vector<cv::Point>& contour
) {
	cv::Point pt(-1, -1);
	if (contour.empty())
		return pt;
	auto it = std::min_element(
		contour.begin(), contour.end(),
		[](const cv::Point& a, const cv::Point& b) {
			return a.y < b.y;
		}
	);
	int tt = it->y;
	int ll =it->x, rr = ll;
	for (const auto& p : contour) {
		if (p.y != tt) 
			continue;
		if (ll > p.x)
			ll = p.x;
		else if (rr < p.x)
			rr = p.x;
	}
	pt.x = (ll + rr) / 2;
	pt.y = tt;
	return pt;
}


bool scan_hulu_contour(
	const cv::Mat& edges, 
	const cv::Point& fromPt,	// Starting point
	ScanDir dir,				// Scan direction
	cv::Point* outPt,			// Output point
	bool boundary_supplement,
	bool rotated,
	int bottomY,				// Bottom bounding Y (e.g., boundingRect.y + height) 
	int verticalThreshold		// dy_t: min vertical distance for correction
) {
	bool backtracked = false;
	// 向右
	std::vector<cv::Point> pts;
	int x = fromPt.x;
	int y = fromPt.y;
	int dx = (dir == eScanLeftDown) ? -1 : 1; // dx: -1 for left, +1 for right
	// Step 1: Trace edge pixels in (dx,dy) direction

	int lastY = collectEdgePoints(edges, dx, x, y, pts, boundary_supplement);
	const int n = static_cast<int>(pts.size());
	if (n == 0) {
		*outPt = fromPt;		// fallback if nothing traced
		return backtracked;
	}	

	// Step 2: Pick reasonable point from the traced list
	const auto lastPt = pts[n - 1];
	if (x == fromPt.x) {
		*outPt = lastPt; // no horizontal motion: use bottom of vertical segment
	}
	else if (x == lastPt.x && y < lastPt.y) {
		// still vertical drop but x moved → trust last segment
		outPt->x = x;
		outPt->y = y;
	}
	else {
		*outPt = findContactByFlatnessLoss(pts);
		if(outPt->x != x || outPt->y != y) {
			backtracked = true; // we had to backtrack to find a better point
		}
	}
	if (!rotated)
		return backtracked;
	
	// Step 3: Analyze vertical density for rotated cases
	const int dyReach = bottomY - outPt->y;
	const int dyTravelled = outPt->y - fromPt.y;
	const int dxTravelled = std::abs(outPt->x - fromPt.x);

	if (dyReach >= dyTravelled / 3 || dxTravelled <= 2 || dyTravelled <= verticalThreshold)
		return backtracked;

	std::map<int, int> columnDensity;  // x -> count
	for (const auto& p : pts) {
		if (std::abs(p.x - fromPt.x) <= dxTravelled)
			columnDensity[p.x]++;
	}

	// Step 3: Find top 2 dense x-columns
	std::vector<std::pair<int, int>> sortedCols(columnDensity.begin(), columnDensity.end());
	std::sort(sortedCols.begin(), sortedCols.end(), [](auto& a, auto& b) {
		return a.second > b.second;
		});

	if (sortedCols.empty())
		return backtracked;

	int optX = sortedCols[0].first;
	int maxLen = sortedCols[0].second;

	if (sortedCols.size() > 1 &&
		std::abs(sortedCols[0].first - sortedCols[1].first) > 1 &&
		sortedCols[1].second >= static_cast<int>(maxLen * 0.75)) {

		int cx = (outPt->x + fromPt.x) / 2;
		int dx0 = std::abs(sortedCols[0].first - cx);
		int dx1 = std::abs(sortedCols[1].first - cx);
		if (dx1 < dx0)
			optX = sortedCols[1].first;
	}

	if (optX == outPt->x)
		return backtracked;

	// Step 4: Estimate vertical span of optimal column
	std::vector<int> ys;
	for (const auto& p : pts)
		if (p.x == optX)
			ys.push_back(p.y);

	if (ys.size() < 2)
		return backtracked;

	std::sort(ys.begin(), ys.end());
	int y1 = ys.front();
	int y2 = ys.back();
	int lenY = static_cast<int>(ys.size());

	double ratio = static_cast<double>(lenY) / dyTravelled;
	if (ratio > 0.22 && maxLen > 9) {
		outPt->x = optX;
		outPt->y = static_cast<int>(0.85 * y1 + 0.15 * y2);
	}

	return backtracked;
}


int 
check_hulu_contour_for_pin(
	const cv::Mat& edges, 
	std::vector<cv::Point>& contour, 
	cv::Point* pt, 
	bool boundary_supplement
) {
	if (contour.empty())
		return 0;
	int length = (int)contour.size();
	if (contour[0].y > contour[length - 1].y)
		std::reverse(contour.begin(), contour.end());
	if (contour[0].y > CONTOUR_Y_MARGIN)
		return 0;

	cv::Rect rt = cv::boundingRect(contour);
	int cx = rt.x + rt.width / 2;
	bool is_left = (cx < edges.cols / 2);
	int nLen = (int)contour.size(), min_ind = -1;
	int dx = std::min(rt.width / 2, std::max(rt.width / 10, 20));

	if (is_left) {
		if (contour[0].x - dx < rt.x) 
			dx = contour[0].x - rt.x - 1;
	}
	else if (contour[0].x + dx > rt.x + rt.width - 1) 
		dx = rt.x + rt.width - 1 - contour[0].x - 1;
	for (int i = 1; i < nLen; i++) {
		if (is_left) {
			if (contour[i].x < contour[0].x - dx) {
				min_ind = i;
				break;
			}
		}
		else {
			if (contour[i].x > contour[0].x + dx) {
				min_ind = i;
				break;
			}
		}
	}
	if (min_ind < 0) 
		return 0;

	cv::Point pt0(contour[min_ind]);
	int bb_up = rt.x + rt.height - 5;

	for (int i = min_ind + 1; i < nLen; i++) {
		if (is_left) {
			if (contour[i].x > pt0.x)
				break;
		}
		else {
			if (contour[i].x < pt0.x)
				break;
		}
		if (contour[i].y > bb_up)
			break;
		if (contour[i].y < contour[min_ind].y)
			min_ind = i;
	}

	pt0 = contour[min_ind];
	std::vector<cv::Point> pts1;
	if (is_left) {
		pts1 = scan_lens_contour(edges, pt0, eScanLeftDown, boundary_supplement); // 向左，向下
		if (pts1.empty())
			pt[0] = pt0;
		else
			pt[0] = pts1[0]; // Update left point

		scan_hulu_contour(edges, pt[0], eScanRightDown, pt, boundary_supplement); // 向右，向下
	}
	else {
		pts1 = scan_lens_contour(edges, pt0, eScanRightDown, boundary_supplement); // 向右，向下
		if (pts1.empty())
			pt[0] = pt0;
		else
			pt[0] = pts1[0]; // Update left point

		scan_hulu_contour(edges, pt[0], eScanLeftDown, pt, boundary_supplement); // 向左，向下
	}

	return 1;
}

int bottom_y(const cv::Mat& edges, int x, int y)
{
	int y_up = edges.rows - 1;
	while (y < y_up) {
		uchar pixel = edges.at<uchar>(y + 1, x);
		if (pixel == 0)
			break;
		y++;
	}
	return y;
}


int check_sphere_contour(
	const cv::Mat& edges, 
	const std::vector<std::vector<cv::Point>>& contours,
	int iContour, 
	cv::Point* outPts, 
	bool rotated,
	bool boundary_supplement
) {
	const std::vector<cv::Point>& contour = contours[iContour];
	cv::Rect rect = cv::boundingRect(contour);
	const int topY = rect.y;
	const int bottomY = rect.y + rect.height;
	const double TAN_1 = 0.017455; // tan(1 degree)

	// Step 1: Find all contour points at topY
	std::vector<cv::Point> topPoints;
	for (const auto& p : contour)
		if (p.y == topY)
			topPoints.push_back(p);

	if (topPoints.empty())
		return 0;

	// Step 2: Determine leftmost and rightmost X values on top
	int leftX = rect.x + rect.width + 1, rightX = -1;
	for (const auto& p : topPoints) {
		leftX = std::min(leftX, p.x);
		rightX = std::max(rightX, p.x);
	}
		
	int dyThreshold = std::max(15, rect.height / 4); // Scan threshold

	// Step 3: Scan from left-top
	cv::Point pStartL(leftX, topY);
	cv::Point ptL1 = outPts[0];	// left cheek point
	std::vector<cv::Point> leftCPts = scan_lens_contour(
		edges, pStartL, eScanLeftDown,
		boundary_supplement, rotated
	);
	if (leftCPts.empty())
		ptL1 = pStartL;
	else
		ptL1 = leftCPts[0]; // Update left cheek point

	outPts[0] = ptL1;

	cv::Point ptL2 = ptL1;		// left neck point
	bool backTrackedLeft = scan_hulu_contour(
		edges, ptL1, eScanRightDown, &ptL2, 
		boundary_supplement, rotated, bottomY, dyThreshold
	);
	float sloat = FLT_MAX;
	if (ptL2.x != ptL1.x)
		sloat = fabs(ptL2.y - ptL1.y) / fabs(ptL2.x - ptL1.x);
	if(sloat > TAN_1) // tan 1 < sloat <tan 89
		outPts[0] = ptL2;

	// Step 4: Scan from right-top
	cv::Point pStartR(rightX, topY);
	cv::Point ptR1 = outPts[1];	// right cheek point
	std::vector<cv::Point> rightCPts = scan_lens_contour(
		edges, pStartR, eScanRightDown, 
		boundary_supplement, rotated
	);
	if(rightCPts.empty())
		ptR1 = pStartR;
	else
		ptR1 = rightCPts[0]; // Update right cheek point
	outPts[1] = ptR1;
	
	cv::Point ptR2 = ptR1;		// right neck point
	bool backTrackedRight = scan_hulu_contour(
		edges, ptR1, eScanLeftDown, &ptR2, 
		boundary_supplement, rotated, bottomY, dyThreshold
	);
	sloat = FLT_MAX;
	if (ptR2.x != ptR1.x)
		sloat = fabs(ptR2.y - ptR1.y) / fabs(ptR2.x - ptR1.x);
	if (sloat > TAN_1) // tan 1 < sloat <tan 89
		outPts[1] = ptR2;
	
	// exception processing
	
	// exception1: If the angle is too large, we assume it's not a valid two points
	double angle = angleBetween(outPts[0], outPts[1]);
	if (!rotated && fabs(angle) > 8.0) { // not rotated, but if angle is greater than 10 degree
		bool processd = false;
		if(backTrackedLeft) {
			outPts[0] = ptL1; // revert to left scan result
			processd = true;
		}
		if (backTrackedRight) {
			outPts[1] = ptR1; // revert to right scan result		
			processd = true;
		}
		if (!processd) {
			if (ptL1.x + 5 > ptL2.x && ptR2.x + 5 > ptR1.x) {
				// both sides are almost vertical, revert to cheek points
				// Check all 4 possible combinations (2 left × 2 right)				
				// bestL and bestR now form the most horizontal pair

				double bestDy = 1e9; // large initial value				
				std::vector<cv::Point> leftPts = { ptL1, ptL2 };
				std::vector<cv::Point> rightPts = { ptR1, ptR2 };				
				for (const auto& L : leftPts) {
					for (const auto& R : rightPts) {
						double dy = std::abs(L.y - R.y);
						if (dy < bestDy) {
							bestDy = dy;
							outPts[0] = L;
							outPts[1] = R;
							processd = true;
						}
					}
				}
			}
		}
	}
	else if(backTrackedLeft && backTrackedRight) {
		// exception2: If both sides were backtracked, and cheek and neck points are too close,
		auto lCheek = ptL1; // lect cheek point 
		if(!leftCPts.empty())
			lCheek = leftCPts.back();
		if (fabs(lCheek.x - ptL2.x) + fabs(lCheek.y - ptL2.y) < 10.0) {			
			outPts[0] = lCheek;
		}

		auto rCheek = ptR1; // right cheek point
		if(!rightCPts.empty())
			rCheek = rightCPts.back();
		if (fabs(rCheek.x - ptR2.x) + fabs(rCheek.y - ptR2.y) < 10.0) {			
			outPts[1] = rCheek;
		}
	}

	// Step 5: Heuristic Correction if enabled
#if 0// disabled by Jewel on 2025-08-20
	if (boundary_supplement) {
		int flag = 0;
		int dyBase = std::min(ptL1.y - topY, ptR1.y - topY);
		int dyL1 = ptL2.y - ptL1.y, dyL2 = bottomY - ptL2.y;
		int dyR1 = ptR2.y - ptR1.y, dyR2 = bottomY - ptR2.y;

		if (dyBase > dyThreshold) {
			if (std::max(dyL2, dyR2) > dyThreshold) {
				if (dyL1 < dyL2 / 2 && dyR2 < dyR1 / 2) 
					flag = 1;
				else if (dyL2 < dyL1 / 2 && dyR1 < dyR2 / 2) 
					flag = 2;
			}
			else if (std::min(dyL1, dyR1) > dyThreshold &&
				dyL2 < dyL1 / 3 && dyR2 < dyR1 / 3) {
				if (rect.width < rect.height) 
					flag = 3;
				else {
					int dx1 = ptR1.x - ptL1.x, dx2 = ptR2.x - ptL2.x;
					if (dx2 < dx1 / 3 || std::abs(ptL1.y - ptR1.y) > 10)
						flag = 3;
				}
			}
		}

		// Step 6: Edge Adjustment using bottom scan
		if (flag == 0 && dyBase > dyThreshold * 2 && rect.width == edges.cols) {
			auto check_bottom_edge = [&](cv::Point p, bool left, int& yOut) -> bool {
				int y = bottom_y(edges, p.x, p.y);
				int dy = std::min(6, 2 * (y - p.y + 1));
				int threshold = static_cast<int>((left ? p.x : edges.cols - p.x) * 0.9);
				for (int i = 0; i < dy; ++i, ++y) {
					int count = cv::countNonZero(edges(cv::Range(y, y + 2),
						left ? cv::Range(0, p.x) : cv::Range(p.x, edges.cols)));
					if (count > threshold) {
						yOut = y - (i < 1 ? 0 : 1);
						return true;
					}
				}
				return false;
			};

			int yL, yR;
			bool foundL = check_bottom_edge(ptL2, true, yL);
			bool foundR = check_bottom_edge(ptR2, false, yR);

			if (foundL && foundR) {
				outPts[0].y = yL;
				outPts[1].y = yR;
				flag = 4;
			}
		}

		// Step 7: Detect external boundary line segments
		if (flag == 0) {
			const int contourCount = (int)contours.size();
			int x1 = ptL1.x, x4 = ptR1.x;
			int width = x4 - x1;
			int x2 = x1 + width / 3;
			int x3 = x4 - width / 3;
			int minLen = width / 5;
			int maxLen = static_cast<int>(width / 2.5);
			int yBottom = std::max(ptL2.y, ptR2.y);
			int minY[2] = { yBottom, yBottom };
			int indices[2] = { -1, -1 };

			for (int i = 0; i < contourCount; ++i) {
				if (i == iContour)
					continue;
				cv::Rect r = cv::boundingRect(contours[i]);
				if (r.height > 6 || r.width < minLen || r.width > maxLen)
					continue;

				int cx = r.x + r.width / 2;
				int cy = r.y + r.height / 2;

				int type = -1;
				if (cx > x1 && cx < x2 && r.x >= x1) 
					type = 0;
				else if (cx > x3 && cx < x4 && r.x >= x3 - 3)
					type = 1;

				if (type != -1 && cy >= rect.y && cy <= yBottom && cy < minY[type]) {
					minY[type] = cy;
					indices[type] = i;
				}
			}

			// Scan for strongest horizontal pixel row in each found line
			if (indices[0] >= 0 && indices[1] >= 0) {
				for (int i = 0; i < 2; ++i) {
					cv::Rect r = cv::boundingRect(contours[indices[i]]);
					int bestY = r.y;
					int maxPixels = 0;
					for (int y = r.y; y < r.y + r.height; ++y) {
						int count = cv::countNonZero(edges(cv::Range(y, y + 1), cv::Range(r.x, r.x + r.width)));
						if (count > maxPixels) {
							maxPixels = count;
							bestY = y;
						}
					}
					minY[i] = bestY;
				}
				outPts[0] = cv::Point(ptL1.x, minY[0]);
				outPts[1] = cv::Point(ptR1.x, minY[1]);
				flag = 5;
			}
		}

		// Step 8: Final point patching based on flag
		if (flag == 1) {
			int y = std::min(ptL2.y, ptR1.y);
			outPts[0] = cv::Point(ptL2.x, y);
			outPts[1] = cv::Point(ptR1.x, y);
		}
		else if (flag == 2) {
			int y = std::min(ptL1.y, ptR2.y);
			outPts[0] = cv::Point(ptL1.x, y);
			outPts[1] = cv::Point(ptR2.x, y);
		}
		else if (flag == 3) {
			int y = std::min(ptL1.y, ptR1.y);
			outPts[0] = cv::Point(ptL1.x, y);
			outPts[1] = cv::Point(ptR1.x, y);
		}
	}
#endif
	// Step 9: Set pt[1] as the center-top
	outPts[2] = cv::Point((leftX + rightX) / 2, topY);
	return 0;
}

void calc_dimensions(
	const std::vector<std::vector<cv::Point>>& contours, 
	int& ll,
	int& rr,
	int& tt,
	int& bb, 
	int min_x, 
	int min_y
) {
	ll = -1;
	int n = (int)contours.size();
	for (int i = 0; i < n; i++) {
		cv::Rect rt = cv::boundingRect(contours[i]);
		if (rt.width < min_x)
			continue;
		if (rt.height < min_y)
			continue;
		if (ll < 0) {
			ll = rt.x;
			tt = rt.y;
			rr = rt.x + rt.width;
			bb = rt.y + rt.height;
		}
		else {
			ll = std::min(ll, rt.x);
			tt = std::min(tt, rt.y);
			rr = std::max(rr, rt.x + rt.width);
			bb = std::max(bb, rt.y + rt.height);
		}
	}
}

int find_top_pixel(const cv::Mat& edges, int x)
{
	for (int i = 0; i < edges.rows; i++) {
		if (edges.at<uchar>(i, x) > 0)
			return i;
	}
	return -1;
}

#if !_DEV_UPDATE_BACKTRACE

void
collections(
	cv::Mat& edges,
	int dx,
	int dy,
	int& x,
	int& y,
	std::vector<cv::Point>& pts,
	int flag,
	bool rotated,
	bool boundary_supplement
) {
	int x_st = x;
	int y0 = -1;

	int y_st = y;
	// collections_pre(edges, dx, dy, x, y);

	uchar pixel = edges.at<uchar>(y, x);

	while (x > 0 && x < edges.cols - 1 && y < edges.rows - 1) {
		pts.push_back(cv::Point(x, y));
		if (step_primary_edge(edges, x, y, dx, dy, y0)) {
			continue;
		}

		if (boundary_supplement && try_additive_recovery(edges, x, y, flag, x_st)) {
			continue;
		}

		break; // no more progress possible
	}
	if (y0 > -1) {
		if (edges.at<uchar>(y, x - dx) == 0 ||
			edges.at<uchar>(y, x - dx - dx) == 0 ||
			edges.at<uchar>(y, x - dx - dx - dx) == 0 ||
			edges.at<uchar>(y, x - dx - dx - dx - dx) == 0) {
			y = (y + y0 + 1) / 2;
		}
	}
}

bool collections_additive_case1(cv::Mat& edges, int& x, int& y)
{
	/*
	01
	1*
	01
	*/
	int x1, y1, x2, y2, x3, y3;
	uchar pixel1, pixel2, pixel3;
	x1 = x;
	y1 = y + 1;
	x2 = x1 - 1;
	y2 = y + 1;
	x3 = x;
	y3 = y + 2;
	if (x2 < 0)
		return false;
	if (y3 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	if (pixel1 == 0 && pixel2 > 0 && pixel3 > 0) {
		*(edges.ptr<uchar>(y1, x1)) = 255;
		*(edges.ptr<uchar>(y2, x2)) = 0;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case2(cv::Mat& edges, int& x, int& y)
{
	/*
	10
	*1
	10
	*/
	int x1, y1, x2, y2, x3, y3;
	uchar pixel1, pixel2, pixel3;
	x1 = x;
	y1 = y + 1;
	x2 = x + 1;
	y2 = y + 1;
	x3 = x;
	y3 = y + 2;
	if (x2 >= edges.cols)
		return false;
	if (y3 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	if (pixel1 == 0 && pixel2 > 0 && pixel3 > 0) {
		*(edges.ptr<uchar>(y1, x1)) = 255;
		*(edges.ptr<uchar>(y2, x2)) = 0;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case3(cv::Mat& edges, int& x, int& y)
{
	/*
	1
	*
	1
	1
	*/
	int x1, y1, x2, y2, x3, y3;
	uchar pixel1, pixel2, pixel3;
	x1 = x;
	y1 = y + 1;
	x2 = x;
	y2 = y + 2;
	x3 = x;
	y3 = y + 3;
	if (y3 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	if (pixel1 == 0 && pixel2 > 0 && pixel3 > 0) {
		*(edges.ptr<uchar>(y1, x1)) = 255;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case4(cv::Mat& edges, int& x, int& y)
{
	/*
	100
	100
	0*0
	001
	001
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x;		y1 = y - 1;
	x2 = x + 1;	y2 = y + 1;
	x3 = x + 2;	y3 = y + 2;
	x4 = x + 2;	y4 = y + 3;
	if (y1 < 0)
		return false;
	if (x3 >= edges.cols)
		return false;
	if (y4 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		x++;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case5(cv::Mat& edges, int& x, int& y)
{
	/*
	11
	*0
	12
	12
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, x6, y6;
	uchar pixel1, pixel2, pixel3, pixel4, pixel5, pixel6;
	x1 = x - 1;
	y1 = y;
	x2 = x - 1;
	y2 = y + 1;
	x3 = x - 1;
	y3 = y + 2;
	x4 = x - 1;
	y4 = y + 3;
	if (x1 < 0)
		return false;
	if (y4 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		x5 = x;
		y5 = y + 2;
		x6 = x;
		y6 = y + 3;
		pixel5 = edges.at<uchar>(y5, x5);
		pixel6 = edges.at<uchar>(y6, x6);
		if (pixel5 > 0 || pixel6 > 0) {
			*(edges.ptr<uchar>(y2, x2)) = 255;
			x--;
			y++;
			return true;
		}
	}
	return false;
}

bool collections_additive_case6(cv::Mat& edges, int& x, int& y)
{
	/*
	11
	1*
	11
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
	uchar pixel1, pixel2, pixel3, pixel4, pixel5;
	x1 = x - 1;
	y1 = y;
	x2 = x - 1;
	y2 = y + 1;
	x3 = x;
	y3 = y + 1;
	x4 = x - 1;
	y4 = y + 2;
	x5 = x;
	y5 = y + 2;
	if (x1 < 0)
		return false;
	if (y5 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	pixel5 = edges.at<uchar>(y5, x5);
	if (pixel1 > 0 && pixel2 > 0 &&
		pixel3 == 0 && pixel4 > 0 &&
		pixel5 > 0) {
		*(edges.ptr<uchar>(y3, x3)) = 255;
		x;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case7(cv::Mat& edges, int& x, int& y)
{
	/*
	01
	1*11
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x + 1;
	y1 = y - 1;
	x2 = x + 1;
	y2 = y;
	x3 = x + 2;
	y3 = y;
	x4 = x + 3;
	y4 = y;
	if (y1 < 0) return false;
	if (x4 >= edges.cols) return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		x++;
		return true;
	}
	return false;
}

bool collections_additive_case8(const cv::Mat& edges, int& x, int& y)
{
	return false;
	/*
	01
	01
	01
	10
	10
	01
	01
	01
	*/
	int xx[7], yy[7];
	xx[0] = x + 1;
	yy[0] = y - 4;
	xx[1] = x + 1;
	yy[1] = y - 3;
	xx[2] = x + 1;
	yy[2] = y - 2;
	xx[3] = x;
	yy[3] = y - 1;
	xx[4] = x + 1;
	yy[4] = y + 1;
	xx[5] = x + 1;
	yy[5] = y + 2;
	xx[6] = x + 1;
	yy[6] = y + 3;
	if (xx[0] >= edges.cols)
		return false;
	if (yy[0] < 0)
		return false;
	if (yy[6] >= edges.cols)
		return false;
	for (int i = 0; i < 7; i++) {
		uchar pixel = edges.at<uchar>(yy[i], xx[i]);
		if (pixel == 0)
			return false;
	}
	x++;
	y++;
	return true;
}

bool collections_additive_case9(cv::Mat& edges, int& x, int& y)
{
	/*
	10
	10
	0*
	01
	01
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x;
	y1 = y - 1;
	x2 = x + 1;
	y2 = y + 1;
	x3 = x + 1;
	y3 = y + 2;
	x4 = x + 1;
	y4 = y + 3;
	if (y1 < 0)
		return false;
	if (y4 >= edges.rows)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		x++;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case10(cv::Mat& edges, int& x, int& y)
{
	/*
	0010
	11*1
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x - 1;
	y1 = y - 1;
	x2 = x - 1;
	y2 = y;
	x3 = x - 2;
	y3 = y;
	x4 = x - 3;
	y4 = y;
	if (y1 < 0)
		return false;
	if (x4 < 0)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		x--;
		return true;
	}
	return false;
}

bool collections_additive_case11(cv::Mat& edges, int& x, int& y)
{
	/*
	0001
	11*0
	0010
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x - 1;
	y1 = y + 1;
	x2 = x - 1;
	y2 = y + 2;
	x3 = x - 2;
	y3 = y + 1;
	x4 = x - 3;
	y4 = y + 1;
	if (y2 >= edges.rows)
		return false;
	if (x4 < 0)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 == 0 && pixel2 > 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y1, x1)) = 255;
		x--;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case12(cv::Mat& edges, int& x, int& y)
{
	/*
	11*1
	0100
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x - 1;
	y1 = y;
	x2 = x - 2;
	y2 = y;
	x3 = x - 2;
	y3 = y + 1;
	x4 = x - 3;
	y4 = y;
	if (y3 >= edges.rows)
		return false;
	if (x4 < 0)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 == 0 && pixel2 > 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y1, x1)) = 255;
		*(edges.ptr<uchar>(y3, x3)) = 0;
		x--;
		return true;
	}
	return false;
}

bool collections_additive_case13(cv::Mat& edges, int& x, int& y)
{
	/*
	011
	0*1
	110
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
	uchar pixel1, pixel2, pixel3, pixel4, pixel5;
	x1 = x + 1;
	y1 = y;
	x2 = x;
	y2 = y + 1;
	x3 = x + 1;
	y3 = y + 1;
	x4 = x;
	y4 = y + 2;
	x5 = x - 1;
	y5 = y + 2;
	if (y4 >= edges.rows)
		return false;
	if (x1 >= edges.cols)
		return false;
	if (x5 < 0)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	pixel5 = edges.at<uchar>(y5, x5);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0 && pixel5 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case14(cv::Mat& edges, int& x, int& y)
{
	/*
	10
	*1
	10
	10
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x - 1;
	y1 = y - 1;
	x2 = x - 1;
	y2 = y;
	x3 = x - 1;
	y3 = y + 1;
	x4 = x - 1;
	y4 = y + 2;
	if (y4 >= edges.rows)
		return false;
	if (x4 < 0)
		return false;
	pixel1 = edges.at<uchar>(y1, x);
	if (pixel1 > 0)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		x--;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case15(cv::Mat& edges, int& x, int& y)
{
	/*
	01
	1*
	01
	01
	*/
	int x1, y1, x2, y2, x3, y3, x4, y4;
	uchar pixel1, pixel2, pixel3, pixel4;
	x1 = x + 1;
	y1 = y - 1;
	x2 = x + 1;
	y2 = y;
	x3 = x + 1;
	y3 = y + 1;
	x4 = x + 1;
	y4 = y + 2;
	if (y4 >= edges.rows)
		return false;
	if (x1 >= edges.cols)
		return false;
	pixel1 = edges.at<uchar>(y1, x1);
	pixel2 = edges.at<uchar>(y2, x2);
	pixel3 = edges.at<uchar>(y3, x3);
	pixel4 = edges.at<uchar>(y4, x4);
	if (pixel1 > 0 && pixel2 == 0 && pixel3 > 0 && pixel4 > 0) {
		*(edges.ptr<uchar>(y2, x2)) = 255;
		x++;
		y++;
		return true;
	}
	return false;
}

bool collections_additive_case16(const cv::Mat& edges, int& x, int& y, int x_st, int num1, int num2)
{
	if (x - x_st != 1)
		return false;
	/*
	01
	01
	10	0
	...
	10	num1
	01	0
	...
	01	num2
	*/
	int xx = x, yy = y - 1;
	if (yy < 0)
		return false;
	if (y + num1 + num2 >= edges.cols)
		return false;
	uchar pixel = edges.at<uchar>(yy, xx);
	if (pixel == 0)
		return false;
	if (yy > 1) {
		pixel = edges.at<uchar>(yy - 2, xx);
		if (pixel > 0)
			return false;
	}
	xx--; yy += 2;
	for (int i = 0; i < num1; i++) {
		pixel = edges.at<uchar>(yy++, xx);
		if (pixel == 0)
			return false;
	}
	xx++;
	for (int i = 0; i < num2; i++) {
		pixel = edges.at<uchar>(yy++, xx);
		if (pixel == 0)
			return false;
	}
	x--;
	y++;
	return true;
}

bool collections_additive_case17(const cv::Mat& edges, int& x, int& y, int x_st, int num1, int num2)
{
	if (x_st - x != 1)
		return false;
	/*
	10
	10
	01	0
	...
	01	num1
	10	0
	...
	10	num2
	*/
	int xx = x, yy = y - 1;
	if (yy < 0)
		return false;
	if (y + num1 + num2 >= edges.cols)
		return false;
	uchar pixel = edges.at<uchar>(yy, xx);
	if (pixel == 0)
		return false;
	if (yy > 1) {
		pixel = edges.at<uchar>(yy - 2, xx);
		if (pixel > 0)
			return false;
	}
	xx++; yy += 2;
	for (int i = 0; i < num1; i++) {
		pixel = edges.at<uchar>(yy++, xx);
		if (pixel == 0)
			return false;
	}
	xx--;
	for (int i = 0; i < num2; i++) {
		pixel = edges.at<uchar>(yy++, xx);
		if (pixel == 0)
			return false;
	}
	x++;
	y++;
	return true;
}

bool collections_additive_case18(cv::Mat& edges, int& x, int& y)
{
	/*
	10
	10
	*1
	10
	*1
	10
	10
	*/
	int xx[5], yy[5];
	xx[0] = x + 1;
	yy[0] = y + 1;
	xx[1] = x;
	yy[1] = y + 2;
	xx[2] = x + 1;
	yy[2] = y + 3;
	xx[3] = x;
	yy[3] = y + 4;
	xx[4] = x;
	yy[4] = y + 5;
	if (xx[0] >= edges.cols)
		return false;
	if (yy[4] >= edges.cols)
		return false;
	for (int i = 0; i < 5; i++) {
		uchar pixel = edges.at<uchar>(yy[i], xx[i]);
		if (pixel == 0)
			return false;
	}
	y++;
	*(edges.ptr<uchar>(y, x)) = 255;
	*(edges.ptr<uchar>(y + 2, x)) = 255;
	return true;
}


void
back_track(
	const cv::Mat& edges,
	std::vector<cv::Point>& pts,
	std::vector<cv::Point>& pt_opts,
	int dx,
	bool second,
	int min_len
) {
	int n = (int)pts.size();
	cv::Point* pt_end = &pts[n - 1];
	cv::Point* pt_final = pt_end;
	int opt_i = -1;
	for (int i = n - 2; i > 0; i--) {
		if (abs(pts[i].y - pt_end->y) > 1) {
			opt_i = i + 1;
			break;
		}
	}
	if (opt_i > -1) {
		int dx = pts[opt_i].x - pt_end->x;
		if (abs(dx) >= min_len) {
			int dy = pts[opt_i].y - pt_end->y;
			double grad = double(dy) / dx;
			for (int i = opt_i - 1; i >= 0; i--) {
				pt_final = &pts[i];
				dx = pt_final->x - pt_end->x;
				dy = pt_final->y - pt_end->y;
				double dy_c = grad * dx;
				double dy_diff = fabs(dy_c - dy);
				if (dy_diff > 0.5)
					break;
			}
		}
	}
	cv::Point pt(*pt_final);
	if (second && pt_final != pt_end) {
		uchar pixel = edges.at<uchar>(pt.y + 1, pt.x);
		if (pixel) {
			pt.y++;
		}
		else {
			pixel = edges.at<uchar>(pt.y + 1, pt.x + dx);
			if (pixel) {
				pt.y++;
				pt.x += dx;
			}
		}
	}
	pt_opts.push_back(pt);
}


// Primary edge stepping: try moving in dx, dy, or diagonals
bool
step_primary_edge(
	const cv::Mat& edges,
	int& x,
	int& y,
	int dx,
	int dy,
	int& y0
) {
	if (isOnEdge(edges, y, x + dx)) {
		// check if isolated
		if (isOnEdge(edges, y + dy, x)) {
			y0 = -1;
			x += dx;
			return true;
		}
		if (x < 2 ||
			x > edges.cols - 2 ||
			y > edges.rows - 2) {
			y0 = -1;
			x += dx;
			return true;
		}
		if (isOnEdge(edges, y, x + dx + dx) ||
			isOnEdge(edges, y + dy, x + dx + dx) ||
			isOnEdge(edges, y + dy, x + dx)) {
			y0 = -1;
			x += dx;
			return true;
		}
	}
	if (isOnEdge(edges, y + dy, x)) {
		if (y0 < 0)
			y0 = y;
		y += dy;
		return true;
	}
	if (isOnEdge(edges, y + dy, x + dx)) {
		x += dx;
		y += dy;
		y0 = -1;
		return true;
	}
	// added by LTI
	if (isOnEdge(edges, y, x + dx + dx) &&
		isOnEdge(edges, y, x + dx + dx + dx)) {
		x += dx;
		y0 = -1;
		return true;
	}
	return false;
}


void
collections_pre(
	const cv::Mat& edges,
	int dx,
	int dy,
	int x,
	int y
) {
	uchar pixel = edges.at<uchar>(y, x);
	int x1 = -1, y1 = -1, x2, y2;
	while (x > 0 && x < edges.cols - 1 && y < edges.rows - 1) {
		x2 = x1; y2 = y1;
		x1 = x; y1 = y;
		pixel = edges.at<uchar>(y, x + dx);
		if (pixel) {
			x += dx;
			continue;
		}
		pixel = edges.at<uchar>(y + dy, x);
		if (pixel) {
			y += dy;
			continue;
		}
		pixel = edges.at<uchar>(y + dy, x + dx);
		if (pixel) {
			x += dx;
			y += dy;
			continue;
		}
		break;
	}
}


// Additive pattern recovery handler (modular group by flag)
bool
try_additive_recovery(
	cv::Mat& edges,
	int& x,
	int& y,
	int flag,
	int x_st
) {
	bool cont = false;
	switch (flag) {
	case 0:
		cont = collections_additive_case2(edges, x, y) ||
			collections_additive_case10(edges, x, y) ||
			collections_additive_case11(edges, x, y) ||
			collections_additive_case12(edges, x, y) ||
			collections_additive_case15(edges, x, y);
		break;
	case 1:
		cont = collections_additive_case3(edges, x, y) ||
			collections_additive_case5(edges, x, y) ||
			collections_additive_case6(edges, x, y) ||
			collections_additive_case9(edges, x, y) ||
			collections_additive_case14(edges, x, y);
		if (!cont) {
			cont = collections_additive_case16(edges, x, y, x_st, 2, 2) ||
				collections_additive_case16(edges, x, y, x_st, 4, 0) ||
				collections_additive_case16(edges, x, y, x_st, 3, 1);
		}
		break;
	case 2:
		cont = collections_additive_case1(edges, x, y) ||
			collections_additive_case4(edges, x, y) ||
			collections_additive_case7(edges, x, y);
		if (!cont)
			cont = collections_additive_case14(edges, x, y);
		break;
	case 3:
		cont = collections_additive_case3(edges, x, y) ||
			collections_additive_case8(edges, x, y) ||
			collections_additive_case13(edges, x, y) ||
			collections_additive_case18(edges, x, y);
		if (!cont) {
			cont = collections_additive_case17(edges, x, y, x_st, 4, 0) ||
				collections_additive_case17(edges, x, y, x_st, 2, 2) ||
				collections_additive_case17(edges, x, y, x_st, 3, 1);
		}
		break;
	}
	return cont;
}

#endif
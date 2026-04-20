#include "DetBasePtOnHorizon.h"
//#include <windows.h>
#include "common.h"
#include "..\cvLib\cvcommon.h"
#include "detect_utils_on_horizonal.h"
#include "detect_features.h"
#include "..\fitting\fit_util.h"

#if _DEV_UPDATE_SUBPXL_ON_HORIZONTAL
#	include "..\imgproc\SubpixelEdge.h"
#endif//_DEV_UPDATE_SUBPXL_ON_HORIZONTAL

#include "..\imgproc\HarrisCorner.h"

CDetBasePtOnHorizon::CDetBasePtOnHorizon()
	: net(NULL)
	, ThreeOutput_layers_name()
	, m_isBoundSupplement(true)
{
}

CDetBasePtOnHorizon::~CDetBasePtOnHorizon()
{
	if (net != NULL)
		delete net;
	net = NULL;
	ThreeOutput_layers_name.clear();
	ThreeOutput_layers_name.shrink_to_fit();
	clearState();
}

bool CDetBasePtOnHorizon::InitModel(const TCHAR* appPath)
{
	try {
		SetCurrentDirToExecutablePath(appPath);
		cv::String cfgPath = BASELINE_CFG;
		cv::String weightsPath = BASELINE_WEIGHTS;
		/*auto basepath = TCHARToCvString(appPath);
		std::string path = "..\\Bin64\\";
		cv::String cfgPath = path + BASELINE_CFG;
		cv::String weightsPath = path + BASELINE_WEIGHTS;*/
		net = new cv::dnn::Net();
		*net = cv::dnn::readNet(weightsPath, cfgPath);
		net->setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
		ThreeOutput_layers_name = net->getUnconnectedOutLayersNames();
	}
	catch (const std::exception&)
	{
		if (net != NULL)
			delete net;
		net = NULL;
		return false;
	}
	return true;
}

double CDetBasePtOnHorizon::forward_predict(const cv::Mat& frame, cv::Rect& box, int& mode)
{
    if (net == NULL || frame.empty())
        return 0.0;

	const int INPUT_SIZE = 416;
	const float SCALE_FACTOR = 1.0f / 255.0f;
	const float CONF_THRESH = (float)conf_th;  // your class member
	const float CLASS_THRESH = 0.5f;    // secondary confidence threshold

	// Preprocess
	cv::Mat blob = cv::dnn::blobFromImage(
		frame, 
		SCALE_FACTOR,
		cv::Size(INPUT_SIZE, INPUT_SIZE),
		cv::Scalar(0, 0, 0), 
		true,
		false
	);
	net->setInput(blob);

	// Forward pass
	std::vector<cv::Mat> outs;
	net->forward(outs, ThreeOutput_layers_name);

	if (outs.empty())
		return 0.0;

	const int width = frame.cols, height = frame.rows;
	std::vector<cv::Rect> boxes;
	std::vector<float> scores;
	std::vector<int> classIds;

	for (const auto& output : outs) {
		const int rows = output.rows;
		const int cols = output.cols;

		for (int i = 0; i < rows; ++i) {
			const float* data = output.ptr<float>(i);

			float object_conf = data[4];
			if (object_conf < CONF_THRESH)
				continue;

			// Get class with highest confidence
			int classId = -1;
			float class_score = -1.0f;
			for (int j = 5; j < cols; ++j) {
				if (data[j] > class_score) {
					class_score = data[j];
					classId = j - 5;
				}
			}

			float conf = object_conf * class_score;
			if (conf < CONF_THRESH)
				continue;

			// Bounding box
			float cx = data[0] * width;
			float cy = data[1] * height;
			float w = data[2] * width;
			float h = data[3] * height;

			int left = static_cast<int>(cx - w / 2);
			int top = static_cast<int>(cy - h / 2);
			cv::Rect rect = cv::Rect(
				left, 
				top, 
				static_cast<int>(w), 
				static_cast<int>(h)
			) & cv::Rect(0, 0, width, height);

			boxes.push_back(rect);
			scores.push_back(conf);
			classIds.push_back(classId);
		}
	}

	if (boxes.empty())
		return 0.0;

	// Optional: Non-Maximum Suppression
	std::vector<int> indices;
	cv::dnn::NMSBoxes(boxes, scores, CONF_THRESH, 0.4f, indices);
	if (indices.empty())
		return 0.0;

	// Select box closest to center or highest score
	int bestIdx = -1;
	float bestScore = -1.0f;
	const cv::Point center(width / 2, height / 2);
	int minDistance = width + height;

	for (int idx : indices) {
		float score = scores[idx];
		const cv::Rect& b = boxes[idx];

		int distance = std::abs(b.x + b.width / 2 - center.x) + std::abs(b.y + b.height / 2 - center.y);
		if (score > bestScore || (score == bestScore && distance < minDistance)) {
			bestScore = score;
			bestIdx = idx;
			minDistance = distance;
		}
	}

	if (bestIdx < 0)
		return 0.0;

	box = boxes[bestIdx];
	mode = classIds[bestIdx];
	return scores[bestIdx];
}

void CDetBasePtOnHorizon::clearState() 
{
	m_frame.release();
	m_cropFrame.release();
	m_isBoundSupplement = true;
	m_basePts.clear();
	m_keyPts[0] = cv::Point(-1, -1); // left
	m_keyPts[1] = cv::Point(-1, -1); // right
	m_keyPts[2] = cv::Point(-1, -1); // center-top
}

bool CDetBasePtOnHorizon::DetectBasePts(
	const cv::Mat& frame, 
	HorizonMode&mode,
	bool &hasPin,
	std::vector<cv::Point2f>& basePts,
	bool &inclined, 
	bool support_bound
) {
    if(frame.empty()) {
		return false;
	}
	bool rotated = false;
	bool result = false;

	clearState();

	m_isBoundSupplement = support_bound;

    if (inclined) {
        double angle = 0;
		
        rotated = correctSkewByLines(frame, m_frame, angle);
        if (rotated) {
			result = DetectDetails(mode, hasPin, true); // Detect base points with rotation
            if (result)
                rotatePoints(frame.cols, frame.rows, angle, m_basePts);
            else
                return result;
        }
    }
	if(!rotated) {
		m_frame = frame.clone();
		result = DetectDetails(mode, hasPin, false); // Detect base points without rotation
    }
    if (result) {
		for(const auto& pt : m_basePts) 
			basePts.push_back(pt);
    }
	inclined = rotated;

	return result;
}

bool CDetBasePtOnHorizon::DetectDetails(
	HorizonMode& mode,
    bool& hasPin, 
    bool rotated
) {
    cv::Rect detectionBox;
	bool res = DetectHorizonMode(m_frame, mode, hasPin, &detectionBox);
    if (!res) {
		return false;
	}

    // Adjust the bounding box
    detectionBox.y = std::max(detectionBox.y - 20, 0);
    detectionBox.height = std::min(detectionBox.height + 20, m_frame.rows - detectionBox.y);

    detectionBox.x = std::max(detectionBox.x - 20, 0);
    detectionBox.width = std::min(detectionBox.width + 20, m_frame.cols - detectionBox.x);

    m_cropFrame = m_frame(detectionBox);

    bool success = false;

	// Find contours from the input image
#if _DEV_UPDATE_SUBPXL_ON_HORIZONTAL
	CSubpixelEdgeDetector subpixelEdgeDetector;
	auto isDet = subpixelEdgeDetector.Detect(m_cropFrame, 1.6, 7.5, 15.0);
	if (isDet) {
		m_cropContours = subpixelEdgeDetector.getIntEdge();
		if (m_cropContours.empty()) {
			return false;
		}
		cv::cvtColor(m_cropFrame, m_cropGray, cv::COLOR_BGR2GRAY);
		
		m_cropEdge = cv::Mat::zeros(m_cropFrame.size(), CV_8UC1);
		for (const auto& contour : m_cropContours) {
			int n = (int)contour.size();
			cv::Scalar color(255);
			for (int i = 0; i < n - 1; i++) {
				cv::line(m_cropEdge,
					contour[i], contour[i + 1],
					color, 1);
			}
		}

		// Step 2: Thinning
		zhangSuenThinningFast(m_cropEdge);
#if DEBUG_IMG
		SaveDebugImg("edge_image_thin", m_cropEdge);
#endif
	}
	else {
		return false;
	}
#else//_DEV_UPDATE_SUBPXL_ON_HORIZONTAL
	find_contours_simple(m_cropFrame, m_cropGray, m_cropEdge, m_cropContours);
#if DEBUG_IMG
	SaveDebugImg("edge_image", m_cropEdge);
#endif
#endif//_DEV_UPDATE_SUBPXL_ON_HORIZONTAL

#if 0 // disabled, for test
	cv::Mat debugImg = m_cropFrame.clone();
	cv::drawContours(debugImg, m_cropContours, -1, cv::Scalar(0, 255, 0), 1);
	SaveDebugImg("edge-line", debugImg);
	SaveDebugImg("edge_image", m_cropEdge);

	CSubpixelEdgeDetector subpixelEdgeDetector;
	auto isDet = subpixelEdgeDetector.Detect(m_cropFrame, 1.6, 7.5, 15.0);
	if (isDet) {

		cv::Mat debugImg1 = m_cropFrame.clone();
		auto subContour = subpixelEdgeDetector.getIntEdge();
		for (const auto& contour : subContour) {
			int n = (int)contour.size();
			cv::Scalar color(rand() & 255, rand() & 255, rand() & 255);
			for (int i = 0; i < n - 1; i++) {
				cv::line(debugImg1,
					contour[i], contour[i + 1],
					color, 1);
			}
		}

		//cv::drawContours(debugImg1, subContour, -1, cv::Scalar(0, 255, 0), 1);
		SaveDebugImg("subContour_contours", debugImg1);
	}

	int max_ind = find_max_contour(m_cropContours, 1);
	if (max_ind >= 0) {
		auto maxCont = m_cropContours[max_ind];
		double area = cv::contourArea(maxCont);
		double perimeter = cv::arcLength(maxCont, true);
		double circularity = (4 * CV_PI * area) / (perimeter * perimeter);

		std::vector<cv::Point> hull;
		cv::convexHull(maxCont, hull);
		double hullArea = cv::contourArea(hull);
		double solidity = area / hullArea;

		char info[256];
		sprintf_s(info, "Area: %.2f, Perimeter: %.2f, Circularity: %.2f, Solidity: %.2f",
			area, perimeter, circularity, solidity);
//		OutputDebugStringA(info);
	}
#endif
    // Choose detection method based on mode
    switch (mode) {
    case eHmConvexLens:
        success = FindPointsForLens(hasPin);
        break;
    case eHmGourd:
        success = FindPointsForHulu(hasPin, rotated);
        break;
    case eHmLookingInside:
        success = FindPointsForInner();
        break;
    case eHmMushroomCap:
        success = FindPointsForMushroom(hasPin);
        break;
    default:
		success = false;
		break;
    }

    if (!success) {
        return success;
    }

    // Set dimensions based on detected points
	m_basePts.push_back(cv::Point2f(
		(float)(detectionBox.x + m_keyPts[0].x),
		(float)(detectionBox.y + m_keyPts[0].y)
	));
	m_basePts.push_back(cv::Point2f(
		(float)(detectionBox.x + m_keyPts[1].x), 
		(float)(detectionBox.y + m_keyPts[1].y)
	));

    if (hasPin || m_keyPts[2].x < 0) {
		m_basePts.push_back(cv::Point2f(-1.0f, -1.0f)); // Invalid point
		m_basePts.push_back(cv::Point2f(-1.0f, -1.0f)); // Invalid point

		m_keyPts[0].x += detectionBox.x;
		m_keyPts[0].y += detectionBox.y;
		m_keyPts[1].x += detectionBox.x;
		m_keyPts[1].y += detectionBox.y;
		m_keyPts[2] = cv::Point(-1, -1);

    }
    else {
        int top_x = detectionBox.x + m_keyPts[2].x;
        int top_y = detectionBox.y + m_keyPts[2].y;
		m_basePts.push_back(cv::Point2f(
			(float)(std::max(0, top_x - 20)),
			(float)top_y
		));
		m_basePts.push_back(cv::Point2f(
			(float)(std::min(m_frame.cols - 1, top_x + 20)), 
			(float)top_y
		));

		m_keyPts[0].x += detectionBox.x;
		m_keyPts[0].y += detectionBox.y;

		m_keyPts[1].x += detectionBox.x;
		m_keyPts[1].y += detectionBox.y;

		m_keyPts[2].x += detectionBox.x;
		m_keyPts[2].y += detectionBox.y;
    }

    return success;
}

bool CDetBasePtOnHorizon::FindPointsForMushroom(int has_pin)
{
	bool success = FindPointsForLens(has_pin);

	if (!success)
		return false;
#if _DEV_UPDATE_SUBPXL_ON_HORIZONTAL
	int y = bottom_y(m_cropEdge, m_keyPts[0].x, m_keyPts[0].y);
	m_keyPts[0].y = y;
	y = bottom_y(m_cropEdge, m_keyPts[1].x, m_keyPts[1].y);
	m_keyPts[1].y = y;
#else
	cv::Mat gray, edges;
	cv::cvtColor(m_cropFrame, gray, cv::COLOR_BGR2GRAY);
	cv::Canny(gray, edges, 50, 150);
	int y = bottom_y(edges, m_keyPts[0].x, m_keyPts[0].y);
	m_keyPts[0].y = y;
	y = bottom_y(edges, m_keyPts[1].x, m_keyPts[1].y);
	m_keyPts[1].y = y;
#endif//_DEV_UPDATE_SUBPXL_ON_HORIZONTAL
	return true;
}


bool 
CDetBasePtOnHorizon::FindPointsForInner() 
{
	// crop
	int min_x = m_cropEdge.cols / 10;
	int min_y = m_cropEdge.rows / 10;
	int max_ind = find_max_contour(m_cropContours, 1);
	if (max_ind < 0)
		return false;
	m_keyPts[2] = find_top_point(m_cropContours[max_ind]);
	int ll, rr, tt, bb;
	calc_dimensions(m_cropContours, ll, rr, tt, bb, min_x, min_y);
	std::vector<std::vector<cv::Point>> contours(m_cropContours);
	int width = rr - ll, height = bb - tt;
	int margin_l = ll, margin_r = m_cropFrame.cols - rr;
	if (margin_l == 0 || margin_r == 0) {
		int ll1, rr1, tt1, bb1, type;
		if (margin_l > 0 && margin_r == 0) {
			bb1 = find_top_pixel(m_cropEdge, rr - 1);
			type = 0;
		}
		else if (margin_l == 0 && margin_r > 0) {
			bb1 = find_top_pixel(m_cropEdge, ll);
			type = 1;
		}
		else {
			bb1 = std::min(find_top_pixel(m_cropEdge, ll), find_top_pixel(m_cropEdge, rr - 1));
			type = 2;
		}
		cv::Mat crop = m_cropEdge(cv::Range(tt, bb1 - height / 10), cv::Range::all());
		contours.clear();
		cv::findContours(crop, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		calc_dimensions(contours, ll1, rr1, tt1, bb1, min_x, min_y);
		if (type == 0) {
			rr = rr1;
		}
		else if (type == 1) {
			ll = ll1;
		}
		else {
			ll = ll1;
			rr = rr1;
		}
	}
	cv::Mat crop;
	width /= 5; height /= 4;
	int ll1 = ll + width;
	int rr1 = rr - width;
	int tt1 = tt + height;
	crop = m_cropEdge(cv::Range(tt1, bb), cv::Range(ll1, rr1));

	// top point
	contours.clear();
	cv::findContours(crop, contours, cv::noArray(), cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	int n = (int)contours.size();
	max_ind = find_max_contour(contours, 1);
	if (max_ind < 0)
		return false;
	int tt0, ll0, rr0;
	n = (int)contours[max_ind].size();
	tt0 = contours[max_ind][0].y;
	ll0 = rr0 = contours[max_ind][0].x;
	for (int i = 1; i < n; i++) {
		cv::Point pt = contours[max_ind][i];
		if (pt.y == tt0) {
			if (pt.x < ll0)
				ll0 = pt.x;
			else if (pt.x > rr0)
				rr0 = pt.x;
		}
	}
	// left point
	int x1 = ll0, y1 = tt0;
	collections_inner(crop, -1, 1, x1, y1, false);
	collections_inner(crop, +1, 1, x1, y1, true);
	// right point
	int x2 = rr0, y2 = tt0;
	collections_inner(crop, +1, 1, x2, y2, false);
	collections_inner(crop, -1, 1, x2, y2, true);
	// yanshen
	x1 += ll1; y1 += tt1;
	x2 += ll1; y2 += tt1;
	int dx = x2 - x1, dy = y2 - y1;
	int y0;
	double gradient = double(dy) / dx;
	if (abs(gradient) > 0.2) {
		gradient = 0.0;
		cv::Rect rt = cv::boundingRect(contours[max_ind]);
		int cy = rt.y + rt.height / 2 + tt1, dy_l = abs(cy - y1), dy_r = abs(cy - y2);
		if (dy_l < dy_r)
			y0 = y1;
		else
			y0 = y2;
	}
	else y0 = y1;
	int y_r = y0 + int(gradient * (rr - x1));
	int y_l = y0 + int(gradient * (ll - x1));
	m_keyPts[0].x = ll; m_keyPts[0].y = y_l;
	m_keyPts[1].x = rr; m_keyPts[1].y = y_r;
	return true;
}

bool
CDetBasePtOnHorizon::FindPointsForHulu(
	int has_pin,
	bool rotated
) {
	int max_ind = find_max_contour(m_cropContours, 1);
	std::vector<std::vector<cv::Point>> contours(m_cropContours);

	if (max_ind < 0)
		return false;
	auto pt = m_keyPts;
	if (has_pin) {
		int i = 0;
		while (true) {
			if (check_hulu_contour_for_pin(m_cropEdge, contours[max_ind], pt, m_isBoundSupplement)) {
				if (++i == 2)
					break;
				pt++;
			}
			contours.erase(contours.begin() + max_ind);
			max_ind = find_max_contour(contours, 0);
			if (max_ind < 0)
				return false;
		}
	}
	else {
		check_sphere_contour(m_cropEdge, contours, max_ind, pt, rotated, m_isBoundSupplement);
	}

	return true;
}

// Finds key points on a lens contour within an image
bool
CDetBasePtOnHorizon::FindPointsForLens(int has_pin) 
{

	// Identify the largest contour
	int max_contour_index = find_max_contour(m_cropContours, 0);
	if (max_contour_index < 0)
		return false;

	//cv::Mat mask = cv::Mat::zeros(m_cropFrame.size(), CV_8UC1);
	//cv::drawContours(mask, contours, max_contour_index, cv::Scalar(255));

	// Step 2: Thinning
	//zhangSuenThinningFast(mask);
#if DEBUG_IMG
	// Draw each contour with a different color
	//cv::Mat tmpThinning = mask.clone();
	//SaveDebugImg("zhangSuenThinning", tmpThinning);
#endif // _DEBUG

	auto pt = m_keyPts;
	std::vector<std::vector<cv::Point>> contours(m_cropContours);
	if (has_pin) {
		// Process contours considering the presence of pins
		int points_found = 0;
		while (points_found < 2) {
			if (check_lens_contour_for_pin(m_cropEdge, contours[max_contour_index], pt, m_isBoundSupplement)) {
				points_found++;
				pt++;
			}
			contours.erase(contours.begin() + max_contour_index);
			max_contour_index = find_max_contour(contours, 0);
			if (max_contour_index < 0)
				return false;
		}
	}
	else {
		cv::Point candidate_points[2];
		std::vector<cv::Point> first_max_contour;
		bool contour_saved = false;

		while (true) {
			// Check contour direction and validate it			
			LensContourType contour_direction = classifyContourBoundary(
				contours[max_contour_index], 
				candidate_points[0], 
				candidate_points[1]
			);

			if (contour_direction == eTopBoundary) { // Starting from the top
				// leftward
				std::vector<cv::Point> lPts = scan_lens_contour(
					m_cropEdge, candidate_points[0], eScanLeftDown, m_isBoundSupplement
				); 
				if (lPts.empty()) 
					pt[0] = candidate_points[0]; // Use candidate point if no leftward scan found
				else
					pt[0] = lPts[0];
				
				// rightward
				std::vector<cv::Point> rPts = scan_lens_contour(
					m_cropEdge, candidate_points[1], eScanRightDown, m_isBoundSupplement
				); 
				if (rPts.empty()) 
					pt[1] = candidate_points[1]; // Use candidate point if no rightward scan found
				else
					pt[1] = rPts[0];

				pt[2] = find_top_point(contours[max_contour_index]);
#if DEBUG_IMG
				HarrisCornerDetector cornerDet;
				if (!m_cropFrame.empty()) {
					auto corners = cornerDet.DetectShiTomasi(m_cropFrame);

					if (!corners.empty()) {
						std::vector<float> cornerStrengths(corners.size(), 0.0f);
						float maxScore = 0.0f;
						float minScore = 1e6;
						for (int i = 0; i < (int)corners.size(); i++) {
							cornerStrengths[i] = cornerDet.GetCornerStrength(i);
							maxScore = std::max(maxScore, cornerStrengths[i]);
							minScore = std::min(minScore, cornerStrengths[i]);
						}
						float scoreThreshold = 0.2f * (maxScore - minScore) + minScore; // 20% of the range

						cv::Mat debugImg1 = m_cropFrame.clone();
						// Convert to absolute coordinates
						for (auto pt : corners) {
							cv::drawMarker(debugImg1, pt, cv::Scalar(0, 0, 255), cv::MARKER_CROSS, 4, 1);
							float score = cornerDet.GetCornerStrength(&pt - &corners[0]);
						}
						SaveDebugImg("harris_corners", debugImg1);
					}
				}
#endif
				break;
			}
			else if (contour_direction == eTopThinSpecial) {
				// Contour detected clearly
				pt[0] = candidate_points[0];
				pt[1] = candidate_points[1];
				pt[2] = (candidate_points[0] + candidate_points[1]) / 2;
				break;
			}

			// Save initial contour if potentially useful
			if (!contour_saved && contour_direction != eContourTooThin) {
				contour_saved = true;
				first_max_contour = contours[max_contour_index];
			}

			// Move to next contour
			contours.erase(contours.begin() + max_contour_index);
			max_contour_index = find_max_contour(contours, 0);

			// If no contours remain, use the saved contour data
			if (max_contour_index >= 0)
				continue;
			
			if (contour_saved) {
				cv::Rect bounding_rect = cv::boundingRect(first_max_contour);
				int len = static_cast<int>(first_max_contour.size());

				int top_y = bounding_rect.y;
				int left_x = bounding_rect.x + bounding_rect.width + 1;
				int right_x = -1;

				// Identify extreme points
				for (int i = 0; i < len; i++) {
					if (first_max_contour[i].y == top_y) {
						left_x = std::min(left_x, first_max_contour[i].x);
						right_x = std::max(right_x, first_max_contour[i].x);
					}
				}

				// Refine search for top points
				int threshold_width = bounding_rect.width / 15;
				if (left_x == right_x && (left_x < threshold_width || (m_cropFrame.cols - right_x) < threshold_width)) {
					int left_bound = threshold_width * 2;
					int right_bound = m_cropFrame.cols - left_x;
					top_y = m_cropFrame.rows;

					for (int i = 0; i < len; i++) {
						if (first_max_contour[i].x < left_bound || first_max_contour[i].x > right_bound)
							continue;
						top_y = std::min(top_y, first_max_contour[i].y);
					}

					left_x = bounding_rect.x + bounding_rect.width + 1;
					right_x = -1;

					for (int i = 0; i < len; i++) {
						if (first_max_contour[i].x < left_bound || first_max_contour[i].x > right_bound)
							continue;
						if (first_max_contour[i].y == top_y) {
							left_x = std::min(left_x, first_max_contour[i].x);
							right_x = std::max(right_x, first_max_contour[i].x);
						}
					}
				}

				candidate_points[0] = { left_x, top_y };
				candidate_points[1] = { right_x, top_y };

				auto lPts = scan_lens_contour(m_cropEdge, candidate_points[0], eScanLeftDown, m_isBoundSupplement); // leftward
				if (lPts.empty()) 
					pt[0] = candidate_points[0]; // Use candidate point if no leftward scan found
				else
					pt[0] = lPts[0]; // Update left point
				
				auto rPts = scan_lens_contour(m_cropEdge, candidate_points[1], eScanRightDown, m_isBoundSupplement); // rightward
				if(rPts.empty()) 
					pt[1] = candidate_points[1]; // Use candidate point if no rightward scan found
				else
					pt[1] = rPts[0]; // Update right point

				pt[2] = { (left_x + right_x) / 2, top_y };
			}
			break;
		}
	}

	return true;
}

bool CDetBasePtOnHorizon::DetectHorizonMode(const cv::Mat& frame, HorizonMode& mode, bool& hasPin, cv::Rect* _outBox)
{
	m_frame = frame.clone();
	cv::Rect detectionBox;
	int detected_pin_status;
	double confidence = forward_predict(m_frame, detectionBox, detected_pin_status);

	if (confidence < conf_th) {
		cv::Mat gray;
		if (m_frame.channels() == 3)
			cv::cvtColor(m_frame, gray, cv::COLOR_BGR2GRAY);
		else
			gray = m_frame.clone();
		// Normalize the image for better processing
		cv::Mat norm, norm3;
		cv::equalizeHist(gray, norm);
		cv::cvtColor(norm, norm3, cv::COLOR_GRAY2BGR);

		double confidence1 = forward_predict(norm3, detectionBox, detected_pin_status);
		if (confidence1 < conf_th)
			return false;
		return false;
	}

	// Check if the detected box is valid
	if (detectionBox.x < 0 || detectionBox.y < 0 ||
		detectionBox.width <= 0 || detectionBox.height <= 0) {
		return false;
	}

	// Set mode and pin based on detected results if not specified
	if (mode == eHmUnknow) {
		mode = (HorizonMode)(detected_pin_status / 2);
		bool isPin = (detected_pin_status % 2) == 1;
		if (hasPin != isPin) {
			hasPin = isPin;
		}
	}

	if (_outBox) {
		*_outBox = detectionBox;
	}
	return true;
}

void CDetBasePtOnHorizon::GetKeyPoint(PointF* keyPts)
{
	if (keyPts == NULL)
		return;
	for (int i = 0; i < 3; ++i) {
		keyPts[i] = PointF(
			static_cast<float>(m_keyPts[i].x),
			static_cast<float>(m_keyPts[i].y)
		);
	}
}
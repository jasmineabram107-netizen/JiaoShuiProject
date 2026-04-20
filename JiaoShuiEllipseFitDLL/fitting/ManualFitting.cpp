#include "ManualFitting.h"
#include "..\cvLib\cvcommon.h"
#include "fit_util.h"
#include "draw_utils.h"

CManualFitting::CManualFitting()
	: m_lower_shape(eShapeHorizontal), m_yediFlag(eDropletCircle), m_baselineFlag(eBaseLineCircle),
	m_baseline_x(nullptr), m_baseline_y(nullptr), m_dropletX(nullptr), m_dropletY(nullptr),
	m_length(0)
{
	 
}

CManualFitting::~CManualFitting()
{

}

void CManualFitting::setParam(cv::Mat& frame,
	mainShapeType lower_shape,
	DropletFitMode yediFlag, BaseLineFitMode baselineFlag,
	double* baseline_x, double* baseline_y,
	double* yedi_x, double* yedi_y)
{
	m_frame = frame;
	m_lower_shape = lower_shape;
	m_yediFlag = yediFlag;
	m_baselineFlag = baselineFlag;
	m_baseline_x = baseline_x;
	m_baseline_y = baseline_y;
	m_dropletX = yedi_x;
	m_dropletY = yedi_y;
}

ErrorCode CManualFitting::Process(
	double* _outAngle, 
	std::vector<PointAndAngle>& res1, 
	cv::RotatedRect* _outBoxes
) {
	if (m_frame.cols < 1 || m_frame.rows < 1)
		return errFailOpen;

	// 拟合——液滴, 拟合——曲面
	memset(&m_Boxup, 0, sizeof(cv::RotatedRect));
	memset(&m_Boxup, 0, sizeof(cv::RotatedRect));

	// 交叉点，切线
	m_length = 10;
	m_res.clear();
	auto ret = errException;
	try {
		if (m_lower_shape < eShapeHorizontal) { // 凸面, 凹面
			if (m_yediFlag == eDropletCircle && m_baselineFlag == eBaseLineCircle) {
				ret = handleCircleCircleFitting();
			}
			else if (m_yediFlag == eDropletCircle && m_baselineFlag > eBaseLineCircle) {
				ret = handleCircleEllipseFitting();
			}
			else if (m_yediFlag > eDropletCircle && m_baselineFlag == eBaseLineCircle) {
				ret = handleEllipseCircleFitting();
			}
			else if (m_yediFlag > eDropletCircle && m_baselineFlag > eBaseLineCircle) {
				ret = handleEllipseEllipseFitting();
			}
		}
		else { // 水平面
			if (m_yediFlag == eDropletCircle) {
				ret = handleHorizontalCircle();
			}
			else if (m_yediFlag == eDropletWidthHeight) {// 高宽法
				ret = handleDropletWidthHeight();
			}
			else {
				ret = handleHorizontalEllipse();
			}
		}
		if (ret != errNo || m_res.empty()) {
			return ret;
		}

		if (m_res.size() == 2) {
			if (m_res[0].point.x > m_res[1].point.x) {
				std::reverse(m_res.begin(), m_res.end());
			}
		}
		for (int i = 0; i < m_res.size(); i++) {
			if (m_res[i].vec1.y > 0.0) { // 需要反转——液滴
				m_res[i].vec1.x = -m_res[i].vec1.x;
				m_res[i].vec1.y = -m_res[i].vec1.y;
			}
		}
		if (m_res[0].vec2.x < 0) {
			m_res[0].vec2.x = -m_res[0].vec2.x;
			m_res[0].vec2.y = -m_res[0].vec2.y;
		}
		if (m_res.size() > 1 && m_res[1].vec2.x > 0) {
			m_res[1].vec2.x = -m_res[1].vec2.x;
			m_res[1].vec2.y = -m_res[1].vec2.y;
		}
		m_length = std::min(m_length, m_frame.cols / 3);
		res1 = m_res;

		// 角度计算		
		if (res1.size() == 1) {
			_outAngle[0] = get_point_angle(res1[0]);
			_outAngle[1] = -1.0;
			return errFailOnlyOne;
		}
		int left_index = (res1[0].point.x < res1[1].point.x) ? 0 : 1;
		int right_index = 1 - left_index;

		_outAngle[0] = get_point_angle(res1[left_index]);		
		_outAngle[1] = get_point_angle(res1[right_index]);

		if (_outBoxes) {
			_outBoxes[0] = m_Boxup;
			_outBoxes[1] = cv::RotatedRect(cv::Point2f(0, 0), cv::Size2f(0, 0), 0);
			_outBoxes[2] = m_Boxdn;
		}
		return errNo;
	}
	catch (...)
	{
		return errException;
	}
}

ErrorCode CManualFitting::handleCircleCircleFitting()
{
	std::tuple<double, double, double> circle2 = findCircleCoefficients(
		MPoint(m_baseline_x[0], m_baseline_y[0]),
		MPoint(m_baseline_x[1], m_baseline_y[1]),
		MPoint(m_baseline_x[2], m_baseline_y[2]));
	double h2 = std::get<0>(circle2);
	double k2 = std::get<1>(circle2);
	double r2 = std::get<2>(circle2);
	if(h2 == 0 && k2 == 0 && r2 == 0) {
		return errFailFitting;
	}

	m_Boxup.center = cv::Point((int)h2, (int)k2);
	m_Boxup.size = cv::Size2f(r2, r2);
	m_Boxup.angle = 0;
	if (m_dropletX[2] == INVALID_YEDI_VALUE) {
		return errFailFitting;
	}

	std::tuple<double, double, double> circle1 = findCircleCoefficients(
		MPoint(m_dropletX[0], m_dropletY[0]), 
		MPoint(m_dropletX[1], m_dropletY[1]), 
		MPoint(m_dropletX[2], m_dropletY[2])
	);
	double h1 = std::get<0>(circle1);
	double k1 = std::get<1>(circle1);
	double r1 = std::get<2>(circle1);
	if (h1 == 0 && k1 == 0 && r1 == 0) {
		return errFailFitting;
	}

	m_Boxdn.center = cv::Point((int)h1, (int)k1);
	m_Boxdn.size = cv::Size2f(r1, r1);
	m_Boxdn.angle = 0;
	m_res = findCirclePointIntersections(h1, k1, r1, h2, k2, r2);

	m_length = (int)std::min(r1, r2);
	return errNo;
}

ErrorCode CManualFitting::handleCircleEllipseFitting()
{
	auto res = errFailDetect;
	EllipseParams ep;
	double theta;

	Eigen::VectorXd coeff = solveEllipseCoefficients(m_baseline_x, m_baseline_y);	

	if(!extractEllipseParams(coeff, ep, theta))
		return res;

	m_Boxup.center = cv::Point((int)ep.h, (int)ep.k);
	m_Boxup.size = cv::Size((int)ep.a, (int)ep.b);
	m_Boxup.angle = theta * (180.0 / M_PI);
	if (m_dropletX[2] == INVALID_YEDI_VALUE) {
		return errFailDetect;
	}

	std::tuple<double, double, double> circle = findCircleCoefficients(
		MPoint(m_dropletX[0], m_dropletY[0]), 
		MPoint(m_dropletX[1], m_dropletY[1]),
		MPoint(m_dropletX[2], m_dropletY[2]));

	EllipseParams ep2;
	ep2.h = std::get<0>(circle);
	ep2.k = std::get<1>(circle);
	ep2.a  = std::get<2>(circle);
	ep2.b = ep2.a;
	if(ep2.h == 0 && ep2.k == 0 && ep2.a == 0) {
		return errFailFitting;
	}

	m_Boxdn.center = cv::Point((int)ep2.h, (int)ep2.k);
	m_Boxdn.size = cv::Size((int)ep2.a, (int)ep2.a);
	m_Boxdn.angle = 0;

	m_length = (int)ep2.a;

	m_res = findIntersectionsEllipse(ep2, 0, ep, theta);
	return errNo;
}

ErrorCode CManualFitting::handleEllipseCircleFitting()
{
	auto res = errFailDetect;
	std::tuple<double, double, double> circle = findCircleCoefficients(
		MPoint(m_baseline_x[0], m_baseline_y[0]), 
		MPoint(m_baseline_x[1], m_baseline_y[1]),
		MPoint(m_baseline_x[2], m_baseline_y[2]));

	double cx = std::get<0>(circle);
	double cy = std::get<1>(circle);
	double r = std::get<2>(circle);
	if(cx == 0 && cy == 0 && r == 0) {
		return errFailFitting;
	}

	m_Boxup.center = cv::Point((int)cx, (int)cy);
	m_Boxup.size = cv::Size((int)r, (int)r);
	m_Boxup.angle = 0;

	if (m_dropletX[4] == INVALID_YEDI_VALUE) {
		return res;
	}
	EllipseParams ep;
	double theta;
	Eigen::VectorXd coeff = solveEllipseCoefficients(m_dropletX, m_dropletY);

	if(!extractEllipseParams(coeff, ep, theta))
		return res;
	
	m_length = (int)std::min(ep.a, ep.b);

	m_Boxdn.center = cv::Point((int)ep.h, (int)ep.k);
	m_Boxdn.size = cv::Size((int)ep.a, (int)ep.b);
	m_Boxdn.angle = theta * (180.0 / M_PI);
	m_res = findIntersectionsEllipse(ep, theta, EllipseParams(cx, cy, r, r), 0);
	return errNo;
}

ErrorCode CManualFitting::handleEllipseEllipseFitting()
{
	auto res = errFailFitting;
	bool not_found = false;

	if (m_dropletX[4] == INVALID_YEDI_VALUE) {
		return errFailFitting;
	}
	EllipseParams ep2;
	double theta2;
	Eigen::VectorXd coeff = solveEllipseCoefficients(m_baseline_x, m_baseline_y);
	if(!extractEllipseParams(coeff, ep2, theta2))
		return res;

	m_length = (int)std::min(ep2.h, ep2.k);

	m_Boxup.center = cv::Point((int)ep2.h, (int)ep2.k);
	m_Boxup.size = cv::Size((int)ep2.a, (int)ep2.b);
	m_Boxup.angle = theta2 * (180.0 / M_PI);
	double theta1;
	EllipseParams ep1;
	coeff = solveEllipseCoefficients(m_dropletX, m_dropletY);
	if(!extractEllipseParams(coeff, ep1, theta1))
		return res;
	
	m_length = (int)std::min(ep1.h, ep1.k);

	m_Boxdn.center = cv::Point((int)ep1.h, (int)ep1.k);
	m_Boxdn.size = cv::Size((int)ep1.a, (int)ep1.b);
	m_Boxdn.angle = theta1 * (180.0 / M_PI);
	m_res = findIntersectionsEllipse(ep1, theta1, ep2, theta2);
	return errNo;
}


ErrorCode CManualFitting::handleHorizontalCircle()
{
	std::tuple<double, double, double> circle = findCircleCoefficients(
		MPoint(m_dropletX[0], m_dropletY[0]),
		MPoint(m_dropletX[1], m_dropletY[1]),
		MPoint(m_dropletX[2], m_dropletY[2]));
	double cx = std::get<0>(circle);
	double cy = std::get<1>(circle);
	double r = std::get<2>(circle);
	if(cx == 0 && cy == 0 && r == 0) {
		return errFailFitting;
	}
	m_length = (int)(r / 2.0);

	m_Boxup.center = cv::Point((int)cx, (int)cy);
	m_Boxup.size = cv::Size((int)(r), (int)(r));
	m_Boxup.angle = 0;

	double tangentSlope1 = 1000000000000;
	if (m_dropletY[1] != cy) {
		tangentSlope1 = -(m_dropletX[1] - cx) / (m_dropletY[1] - cy);
	}

	double tangentSlope2 = 0;
	if (m_dropletX[1] != m_dropletX[2]) {
		tangentSlope2 = -(m_dropletY[1] - m_dropletY[2]) / (m_dropletX[1] - m_dropletX[2]);
	}

	PointAndAngle pta1;
	pta1.point.x = m_dropletX[1];
	pta1.point.y = m_dropletY[1];
	pta1.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1 + tangentSlope1 * tangentSlope2))) * 180 / M_PI;
	pta1.vec1.x = cos(atan(tangentSlope1));
	pta1.vec1.y = sin(atan(tangentSlope1));
	pta1.vec2.x = cos(atan(tangentSlope2));
	pta1.vec2.y = sin(atan(tangentSlope2));
	m_res.push_back(pta1);

	tangentSlope1 = 1000000000000;
	if (m_dropletY[2] != cy) {
		tangentSlope1 = -(m_dropletX[2] - cx) / (m_dropletY[2] - cy);
	}

	tangentSlope2 = 0;
	if (m_dropletX[1] != m_dropletX[2]) {
		tangentSlope2 = (m_dropletY[2] - m_dropletY[1]) / (m_dropletX[2] - m_dropletX[1]);
	}

	PointAndAngle pta2;
	pta2.point.x = m_dropletX[2];
	pta2.point.y = m_dropletY[2];
	pta2.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1 + tangentSlope1 * tangentSlope2))) * 180 / M_PI;
	pta2.vec1.x = cos(atan(tangentSlope1));
	pta2.vec1.y = sin(atan(tangentSlope1));
	pta2.vec2.x = cos(atan(tangentSlope2));
	pta2.vec2.y = sin(atan(tangentSlope2));
	m_res.push_back(pta2);

	return errNo;
}

ErrorCode CManualFitting::handleDropletWidthHeight()
{
	double h = m_dropletY[1] - m_dropletY[0];
	double w = m_dropletX[1] - m_dropletX[0];

	cv::Rect bounding_rect(
		(int)m_dropletX[0],             // Top-left x
		(int)m_dropletY[0],             // Top-left y
		(int)w,
		(int)h
	);
	m_Boxup = cv::RotatedRect(
		cv::Point2f((float)bounding_rect.x + (float)bounding_rect.width / 2.0f, (float)bounding_rect.y + (float)bounding_rect.height / 2.0f),
		cv::Size2f((float)bounding_rect.width, (float)bounding_rect.height),
		0.0f
	);
	PointAndAngle out;
	out.angle = 2 * atan(h / w * 2);
	out.point.x = m_dropletX[0];
	out.point.y = m_dropletY[1];

	out.vec1.x = cos(-out.angle);
	out.vec1.y = sin(-out.angle);
	out.vec2.x = 1;
	out.vec2.y = 0;
	m_res.push_back(out);
	PointAndAngle out2;
	out2.angle = 2.0 * atan(h / w * 2.0);
	out2.point.x = m_dropletX[1];
	out2.point.y = m_dropletY[1];

	out2.vec1.x = cos(out2.angle);
	out2.vec1.y = sin(out2.angle);
	out2.vec2.x = -1;
	out2.vec2.y = 0;
	m_res.push_back(out2);
	m_length = (int)h;

	return errNo;
}

ErrorCode CManualFitting::handleHorizontalEllipse()
{
	double theta;
	EllipseParams ep;
	Eigen::VectorXd coeff = solveEllipseCoefficients(m_dropletX, m_dropletY);
	if(!extractEllipseParams(coeff, ep, theta))
		return errFailFitting;

	m_length = (int)std::min(ep.a, ep.b);
	m_Boxup.center = cv::Point((int)ep.h, (int)ep.k);
	m_Boxup.size = cv::Size((int)ep.a, (int)ep.b);
	m_Boxup.angle = theta * (180.0 / M_PI);

	double tangentSlope1 = calculateEllipseTangentLineSlope(ep.a, ep.b, theta, m_dropletX[3] - ep.h, m_dropletY[3] - ep.k);

	double tangentSlope2 = 0;
	if (m_dropletX[3] != m_dropletX[4]) {
		tangentSlope2 = (m_dropletY[3] - m_dropletY[4]) / (m_dropletX[3] - m_dropletX[4]);
	}

	PointAndAngle pta1;
	pta1.point.x = m_dropletX[3];
	pta1.point.y = m_dropletY[3];
	pta1.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1 + tangentSlope1 * tangentSlope2))) * 180 / M_PI;
	pta1.vec1.x = cos(atan(tangentSlope1));
	pta1.vec1.y = sin(atan(tangentSlope1));
	pta1.vec2.x = cos(atan(tangentSlope2));
	pta1.vec2.y = sin(atan(tangentSlope2));
	m_res.push_back(pta1);

	tangentSlope1 = calculateEllipseTangentLineSlope(ep.a, ep.b, theta, m_dropletX[4] - ep.h, m_dropletY[4] - ep.k);

	PointAndAngle pta2;
	pta2.point.x = m_dropletX[4];
	pta2.point.y = m_dropletY[4];
	pta2.angle = atan(abs((tangentSlope2 - tangentSlope1) / (1 + tangentSlope1 * tangentSlope2))) * 180 / M_PI;
	pta2.vec1.x = cos(atan(tangentSlope1));
	pta2.vec1.y = sin(atan(tangentSlope1));
	pta2.vec2.x = cos(atan(tangentSlope2));
	pta2.vec2.y = sin(atan(tangentSlope2));
	m_res.push_back(pta2);

	return errNo;
}

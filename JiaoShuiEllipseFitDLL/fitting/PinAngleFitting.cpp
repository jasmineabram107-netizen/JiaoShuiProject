#include "PinAngleFitting.h"
#include "..\cvLib\cvcommon.h"
#include "fit_util.h"
#include "draw_utils.h"
///

CPinAngleFitting::CPinAngleFitting(CDropletContext* pCtx)
	: CDropletFittingBase(pCtx)
{	
	if (pCtx) {
		pCtx->ClearResultVariables();
	}
}

CPinAngleFitting::~CPinAngleFitting() 
{

}

ErrorCode CPinAngleFitting::process() 
{
	if (m_pContext == NULL || m_pContext->IsEmptyImage())
		return errFailOpen;

	if (m_pContext->isEqualLRPoints())
		return errFailFitting;

	try {
		// 获取轮廓
		std::vector<cv::Point2f> fpts_l, fpts_r, fpts_dn;
		if (!prepareContours(fpts_l, fpts_r, fpts_dn))
			return errNotFoundContour;

		// 交叉点，切线
		cv::RotatedRect box_dn;
		if(!fitBaseline(fpts_dn, box_dn)) // 拟合基线和液滴轮廓
			return errFailedFitBaseline;
		m_pContext->SetBox(2, box_dn);
		// 拟合——液滴
		cv::RotatedRect box_up;
		std::vector<PointAndAngle> _angleRes;
		double _angles[2];
		if(!fitDroplet(box_dn, fpts_l, fpts_r, box_up, _angleRes)) // 拟合液滴轮廓
			return errFailedFitDroplet;

		if (_angleRes.empty()) {
			return errFailFitting;
		}

		if (_angleRes.size() == 2) {
			if (_angleRes[0].point.x > _angleRes[1].point.x) {
				// Reversing the vector
				std::reverse(_angleRes.begin(), _angleRes.end());
			}
		}
		int length = (int)std::min(box_up.size.width, box_up.size.height) / 2;
		if (m_pContext->DropletMode() == eDropletWidthHeight)
			length = length * 2;
		length = std::min(length, m_pContext->ImageWidth() / 3);
		length = std::max(length, 30);
		for (int i = 0; i < _angleRes.size(); i++)
		{
			if (_angleRes[i].vec1.y > 0.0) { // 需要反转——液滴
				_angleRes[i].vec1.x = -_angleRes[i].vec1.x;
				_angleRes[i].vec1.y = -_angleRes[i].vec1.y;
			}
		}
		if (_angleRes[0].vec2.x < 0) {
			_angleRes[0].vec2.x = -_angleRes[0].vec2.x;
			_angleRes[0].vec2.y = -_angleRes[0].vec2.y;
		}
		if (_angleRes.size() > 1) {
			if (_angleRes[1].vec2.x > 0) {
				_angleRes[1].vec2.x = -_angleRes[1].vec2.x;
				_angleRes[1].vec2.y = -_angleRes[1].vec2.y;
			}
		}
		// 角度计算		
		ErrorCode res_code = errNo;
		if (_angleRes.size() == 1) {
			_angles[0] = angle_between_vectors(
				_angleRes[0].vec1,
				_angleRes[0].vec2
			);
			_angles[1] = -1.0;
			res_code = errFailOnlyOne;
		}
		else {
			int left_index = (_angleRes[0].point.x < _angleRes[1].point.x) ? 0 : 1;
			_angles[0] = angle_between_vectors(
				_angleRes[left_index].vec1,
				_angleRes[left_index].vec2
			);
			int right_index = 1 - left_index;
			_angles[1] = angle_between_vectors(
				_angleRes[right_index].vec1,
				_angleRes[right_index].vec2
			);
		}

		m_pContext->SetAnglePoints(_angleRes);
		m_pContext->SetAngles(_angles);
		m_pContext->SetBox(2, box_dn);
		return res_code;
	}
	catch (...)
	{
		return errException;
	}
}

bool CPinAngleFitting::prepareContours(
	std::vector<cv::Point2f>& fpts_l, 
	std::vector<cv::Point2f>& fpts_r,
	std::vector<cv::Point2f>& fpts_dn
) {
	auto left = m_pContext->leftPoint();
	auto right = m_pContext->rightPoint();
	std::vector<cv::Point> pts_l, pts_r;
	int left_cnt = 0; // 液滴的左边的点数

	auto& ptsUp = m_pContext->DropletPtRef();
	auto& ptsDown = m_pContext->BaelinePtRef();

	find_contour_points_for_pin(
		m_pContext->Image(),
		left, right,
		pts_l, pts_r, ptsDown,
		m_pContext->mainShape()
	);

	if (pts_l.empty() || pts_r.empty())
		return false;
	if (m_pContext->mainShape() < eShapeHorizontal && ptsDown.size() < 10)
		return false;

	ptsUp.reserve(pts_l.size() + pts_r.size());
	fpts_l.reserve(pts_l.size());
	fpts_r.reserve(pts_r.size());
	fpts_dn.reserve(ptsDown.size());

	for (int i = 0; i < (int)pts_l.size(); i++) {
		ptsUp.emplace_back(pts_l[i]);
		fpts_l.emplace_back((float)pts_l[i].x, (float)pts_l[i].y);
	}
	for (int i = 0; i < (int)pts_r.size(); i++) {
		ptsUp.emplace_back(pts_r[i]);
		fpts_r.emplace_back((float)pts_r[i].x, (float)pts_r[i].y);
	}

	for (int i = 0; i < (int)ptsDown.size(); i++) {
		fpts_dn.emplace_back((float)ptsDown[i].x, (float)ptsDown[i].y);
	}

	if (m_pContext->IsSubpixel()) {
		cv::Mat gray;
		cv::cvtColor(m_pContext->Image(), gray, cv::COLOR_BGR2GRAY);
		//cv::Canny(gray, edges, 50, 150);
		cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
		cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.01);
		cv::cornerSubPix(gray, fpts_l, cv::Size(5, 5), cv::Size(-1, -1), criteria);
		cv::cornerSubPix(gray, fpts_r, cv::Size(5, 5), cv::Size(-1, -1), criteria);
	}
    return true;
}


bool 
CPinAngleFitting::fitBaseline(
	const std::vector<cv::Point2f>& fpts_dn, 
	cv::RotatedRect& box_dn
) {
	// 拟合——液滴
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());
	bool res = false;
	// 拟合——曲面
	memset(&box_dn, 0, sizeof(cv::RotatedRect));
	auto left = m_pContext->leftPoint();
	auto right = m_pContext->rightPoint();
	auto fitMode = m_pContext->fitMode();
	auto mainShape = m_pContext->mainShape();
	BaseLineFitMode basefitMode = m_pContext->BaselineMode();

	if (fitMode == eFitAuto) {
		if (mainShape < eShapeHorizontal) {
			if (basefitMode == eBaseLineCircle)
				ellipse_regression_Circle(fpts_dn, dimension, box_dn);
			else
				ellipse_regression_OpenCV(fpts_dn, basefitMode - 1, left, right, box_dn);
			if (box_dn.size.width == 0.0 || box_dn.size.height == 0.0)
				return res;
			res = true;
		}
		else {
			box_dn.center = left;
			box_dn.angle = atanf((left.y - right.y) / (left.x - right.x)) * 180.0f / M_PI;
			box_dn.size.width = right.x - left.x;
			box_dn.size.height = right.y - left.y;
			res = true;
		}
	}
	else {
		if (m_pContext->BaselineMode() == eBaseLineCircle) {
			std::tuple<double, double, double> circle2 = findCircleCoefficients(
				MPoint(m_baselineX[0], m_baselineY[0]),
				MPoint(m_baselineX[1], m_baselineY[1]),
				MPoint(m_baselineX[2], m_baselineY[2]));
			double h2 = std::get<0>(circle2);
			double k2 = std::get<1>(circle2);
			double r2 = std::get<2>(circle2);
			if(h2 == 0.0 || k2 == 0.0 || r2 == 0.0) {
				return res;
			}
			box_dn.center.x = (float)h2;
			box_dn.center.y = (float)k2;
			box_dn.angle = 0.0f;
			box_dn.size.width = (float)r2;
			box_dn.size.height = (float)r2;
			res = true;
		}
		else {
			EllipseParams ep;
			double theta;
			Eigen::VectorXd coeff = solveEllipseCoefficients(m_baselineX, m_baselineY);
			if (extractEllipseParams(coeff, ep, theta)) {
				box_dn.center.x = (float)ep.h;
				box_dn.center.y = (float)ep.k;
				box_dn.angle = (float)(theta * 180.0 / M_PI);
				box_dn.size.width = (float)ep.a;
				box_dn.size.height = (float)ep.b;
				res = true;
			}
			else {
				return res;
			}
		}
	}

	if (mainShape != eShapeHorizontal) {
		auto maxSize = std::max(box_dn.size.width, box_dn.size.height);
		if (maxSize < 5.0f)
			res = false;
	}

	return res;
}

bool 
CPinAngleFitting::fitDroplet(
	const cv::RotatedRect& box_dn, 
	std::vector<cv::Point2f>& fpts_l, 
	std::vector<cv::Point2f>& fpts_r, 
	cv::RotatedRect& box_up,
	std::vector<PointAndAngle>& _outRes
) {
	bool res = false;
	auto left = m_pContext->leftPoint();
	auto right = m_pContext->rightPoint();
	auto dropfitMode = m_pContext->DropletMode();
	auto basefitMode = m_pContext->BaselineMode();
	auto mainShape = m_pContext->mainShape();
	auto fitMode = m_pContext->fitMode();

	box_up = {};
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());

	auto addIntersection = [&](const cv::RotatedRect& dropletBox, const cv::Point2f& basePt) {
		if (m_pContext->BaselineMode() == eBaseLineCircle) {
			auto out = getCircleIntersections(
				dropletBox.center.x, dropletBox.center.y, dropletBox.size.width,
				box_dn.center.x, box_dn.center.y, box_dn.size.width);
			if (!out.empty()) {
				if (out.size() == 1 || near_to_basepoint(out[0].point, basePt, 80))
					_outRes.push_back(out[0]);
				else
					_outRes.push_back(out[1]);
			}
		}
		else {
			auto out = findIntersectionEllipseOne(
				dropletBox.center, dropletBox.size, dropletBox.angle * M_PI / 180.0,
				box_dn.center, box_dn.size, box_dn.angle * M_PI / 180.0, basePt);
			if (out.point.x != 0 && out.point.y != 0) {
				_outRes.push_back(out);
			}
		}
	};
	switch (dropfitMode)
	{
	case eDropletDoubleCircle:
		ellipse_regression_Circle(fpts_l, dimension, box_up);
		m_pContext->SetBox(0, box_up);

		if (mainShape == eShapeHorizontal) {
			auto out = findIntersectionsCircleLine(
				box_up.center,
				box_up.size.width,
				left, right, 1
			);
			_outRes.insert(_outRes.end(), out.begin(), out.end());
		} 
		else
			addIntersection(box_up, left);

		ellipse_regression_Circle(fpts_r, dimension, box_up);
		m_pContext->SetBox(1, box_up);

		if (mainShape == eShapeHorizontal) {
			auto out = findIntersectionsCircleLine(
				box_up.center,
				box_up.size.width,
				left, right, 2
			);
			_outRes.insert(_outRes.end(), out.begin(), out.end());
		}
		else
			addIntersection(box_up, right);
		break;
	case eDropletDoubleEllipse:
		{			
			PointAndAngle out;

			ellipse_regression_OpenCV(fpts_l, 1, left, right, box_up);
			m_pContext->SetBox(0, box_up);
			if (mainShape == eShapeHorizontal) {
				auto out = findIntersectionsEllipseLine(
					box_up.center, box_up.size, box_up.angle * M_PI / 180.0,
					left, right, 1
				);
				_outRes.insert(_outRes.end(), out.begin(), out.end());
			}
			else {
				out = findIntersectionEllipseOne(
					box_up.center, box_up.size, box_up.angle * M_PI / 180.0,
					box_dn.center, box_dn.size, box_dn.angle * M_PI / 180.0, left);
				if (out.point.x != 0 && out.point.y != 0) {
					_outRes.push_back(out);
				}
			}

			ellipse_regression_OpenCV(fpts_r, 1, left, right, box_up);
			m_pContext->SetBox(1, box_up);
			if (mainShape == eShapeHorizontal) {
				auto out = findIntersectionsEllipseLine(
					box_up.center, box_up.size, box_up.angle * M_PI / 180.0,
					left, right, 2
				);
				_outRes.insert(_outRes.end(), out.begin(), out.end());
			}
			else {
				out = findIntersectionEllipseOne(
					box_up.center, box_up.size, box_up.angle * M_PI / 180.0,
					box_dn.center, box_dn.size, box_dn.angle * M_PI / 180.0, right);
				if (out.point.x != 0 && out.point.y != 0) {
					_outRes.push_back(out);
				}
			}
		}		
		break;
	case eDropletCircle:
		break;
	case eDropletEllipse:
	case eDropletEllipseAMS:
	case eDropletEllipseDirect:
	{
		fpts_l.insert(fpts_l.end(), fpts_r.begin(), fpts_r.end());
		ellipse_regression_OpenCV(fpts_l, dropfitMode - 1, left, right, box_up);
		m_pContext->SetBox(0, box_up);
		if (mainShape == eShapeHorizontal)
			_outRes = findIntersectionsEllipseLine(
				box_up.center,
				box_up.size,
				box_up.angle * M_PI / 180.0,
				left, right
			);
		else {
			EllipseParams ep1(box_up.center.x, box_up.center.y, box_up.size.width, box_up.size.height);
			EllipseParams ep2(box_dn.center.x, box_dn.center.y, box_dn.size.width, box_dn.size.height);
			_outRes = findIntersectionsEllipse(
				ep1, box_up.angle * M_PI / 180.0,
				ep2, box_dn.angle * M_PI / 180.0,
				(fitMode != eFitManual && fitMode != eFitSemiAuto),
				&left, &right);
		}		
		break;
	}
	case eDropletDoublePolynomial:	
		_outRes = doublePolynomialFit(fpts_l, fpts_r, box_up, box_dn);
		break;
	case eDropletPolynomial:
		_outRes = polynomialFit(fpts_l, fpts_r, box_up, box_dn);
		break;	
	case eDropletWidthHeight: // illegal case
	default:
		break;
	}
	
	return true;
}


std::vector<PointAndAngle>
CPinAngleFitting::doublePolynomialFit(
	const std::vector<cv::Point2f>& fpts_l,
	const std::vector<cv::Point2f>& fpts_r,
	cv::RotatedRect& box_up,		    // 上半部分拟合结果
	const cv::RotatedRect& box_dn		// 下半部分拟合结果
) {
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	std::vector<double> rLeft, thetaLeft, rRight, thetaRight;
	std::vector<PointAndAngle> _outRes;
	int width = m_pContext->ImageWidth();
	int height = m_pContext->ImageHeight();

	double xc, ycLeft, ycRight;
	int leftPointCnt = 0;
	int rightPointCnt = 0;
	bool horizontal = (m_pContext->mainShape() == eShapeHorizontal);

	if (horizontal) {
		leftPointCnt = std::max(20, std::min((int)fpts_l.size() * 3 / 5, POINT_CNT * 2));
		rightPointCnt = std::max(20, std::min(((int)fpts_r.size()) * 3 / 5, POINT_CNT * 2));
	}
	else {
		leftPointCnt = std::max(10, std::min((int)fpts_l.size() * 3 / 5, POINT_CNT));
		rightPointCnt = std::max(10, std::min((int)fpts_r.size() * 3 / 5, POINT_CNT));
	}

	xc = (left.x + right.x) / 2;
	ycLeft = std::max(left.y, right.y);
	ycRight = ycLeft;

	// left
	float lmin_y = 10000.f;
	float lmax_y = -1.f;
	float largmaxy_x = -1.f;
	float rmin_y = 10000.f;
	float rmax_y = -1.f;
	float rargmaxy_x = -1.f;
	std::vector<double> xxLeft, yyLeft, xxRight, yyRight;

	get_xy_array_on_contour(
		fpts_l, true, leftPointCnt, xxLeft, yyLeft, lmin_y, lmax_y, largmaxy_x
	);
	Eigen::VectorXd coefsLeft = polynomialCurveFitYHouseholderQR(
		xxLeft, yyLeft, POLYNOMIAL_DEGREE
	);
#if DEBUG_IMG
	cv::Mat tmp = m_pContext->Image().clone();
	for (int i = 0; i < (int)xxLeft.size(); i++) {
		cv::drawMarker(
			tmp, 
			cv::Point(width - (int)xxLeft[i], (int)yyLeft[i]),
			cv::Scalar(0, 255, 0), 
			cv::MARKER_CROSS, 
			1, 1);
	}
	SaveDebugImg("left_contour-nonpin", tmp);
#endif
	// right	
	get_xy_array_on_contour(
		fpts_r, false, rightPointCnt, xxRight, yyRight, rmin_y, rmax_y, rargmaxy_x
	);

#if DEBUG_IMG
	cv::Mat tmp1 = m_pContext->Image().clone();
	for (int i = 0; i < (int)xxRight.size(); i++) {
		cv::drawMarker(
			tmp1,
			cv::Point((int)xxRight[i], (int)yyRight[i]),
			cv::Scalar(0, 255, 0),
			cv::MARKER_CROSS,
			1, 1);
	}
	SaveDebugImg("right_contour-nonpin", tmp1);
#endif
	auto coefsRight = polynomialCurveFitYHouseholderQR(xxRight, yyRight, POLYNOMIAL_DEGREE);

	double ly0 = lmin_y;
	double ly1 = lmax_y;
	double ry0 = rmin_y;
	double ry1 = rmax_y;

	if (largmaxy_x < 0)
		largmaxy_x = left.x;
	cv::Point leftReal((int)largmaxy_x, (int)lmax_y);
	if (lmax_y + 10.f >= left.y) {
		leftReal.y = (int)left.y;
		leftReal.x = width - (int)polyEval(coefsLeft, (double)left.y);
	}
	ly1 = left.y + 10.f;	
	// drawPolynomialYReverse(coefsLeft, (int)lmin_y, leftReal.y + 1, m_debugImg);
	getPolynomialYReverse(coefsLeft, (int)lmin_y, leftReal.y + 1, width, height, true, m_pContext->LeftPolyPtRef());

	if (rargmaxy_x < 0)
		rargmaxy_x = right.x;
	cv::Point rightReal((int)rargmaxy_x, (int)rmax_y);
	if (rmax_y + 10.f >= right.y) {
		rightReal.y = (int)right.y;
		rightReal.x = (int)polyEval(coefsRight, (double)right.y);
	}
	ry1 = right.y + 10.f;
	// drawPolynomialY(coefsRight, (int)rmin_y, (int)rightReal.y + 1, m_debugImg, false);
	getPolynomialY(coefsRight, (int)rmin_y, (int)rightReal.y + 1, width, height, false, m_pContext->RightPolyPtRef());

	DropletEllipse ellipse(
		box_dn.center.x, box_dn.center.y,
		box_dn.size.width, box_dn.size.height,
		box_dn.angle* M_PI / 180.0); // TODO	

	PointAndAngle outLeft, outRight;
	double lym = -1.f, rym = -1.f;
	if (horizontal) {
		outLeft = calculate_bipoly_angles_on_horizontal(
			left, right, ly0, ly1, coefsLeft, true, width, lym);
		outRight = calculate_bipoly_angles_on_horizontal(
			left, right, ry0, ry1, coefsRight, false, width, rym);
	}
	else {
		outLeft = calculate_bipoly_angles_on_curved(
			ly0, ly1, coefsLeft, ellipse, true, width, lym);
		outRight = calculate_bipoly_angles_on_curved(
			ry0, ry1, coefsRight, ellipse, false, width, rym);
	}
	if (lym < 0) {
		auto slantLeft = polySlope(coefsLeft, leftReal.y);
		outLeft.angle = M_PI - atan(slantLeft);
		outLeft.point.x = leftReal.x;
		outLeft.point.y = leftReal.y;
		double tangentSlope2 = 10000000;
		if (horizontal) {
			if (leftReal.x != rightReal.x) {
				tangentSlope2 = (rightReal.y - leftReal.y) / (rightReal.x - leftReal.x);
			}
		}
		else {
			// tangentSlope2 = ellipse.tangentSlope(leftReal.x, leftReal.y);
			double dx = 0, dy = 0;
			ellipse.getTangentVector(leftReal.x, leftReal.y, dx, dy);
			tangentSlope2 = (std::abs(dx) < 1e-10) ? 10000000 : dy / dx;
			tangentSlope2 *= -1.0;
		}
		outLeft.vec1.x = cos(outLeft.angle + M_PI / 2);
		outLeft.vec1.y = -sin(outLeft.angle + M_PI / 2);
		outLeft.vec2.x = cos(atan(tangentSlope2));
		if (horizontal)
			outLeft.vec2.y = sin(atan(tangentSlope2));
		else
			outLeft.vec2.y = -sin(atan(tangentSlope2));
	}
	if (rym < 0) {
		auto slantRight = polySlope(coefsRight, rightReal.y);
		double tangentSlope2 = 10000000;
		outRight.angle = atan(slantRight);
		outRight.point.x = rightReal.x;
		outRight.point.y = rightReal.y;
		tangentSlope2 = 10000000;

		if (horizontal) {
			if (leftReal.x != rightReal.x) {
				tangentSlope2 = (leftReal.y - rightReal.y) / (leftReal.x - rightReal.x);
			}
		}
		else {
			// tangentSlope2 = ellipse.tangentSlope(rightReal.x, rightReal.y);
			double dx = 0, dy = 0;
			ellipse.getTangentVector(rightReal.x, rightReal.y, dx, dy);
			tangentSlope2 = (std::abs(dx) < 1e-10) ? 10000000 : dy / dx;
			tangentSlope2 *= -1.0;
		}

		outRight.vec1.x = cos(outRight.angle + M_PI / 2);
		outRight.vec1.y = -sin(outRight.angle + M_PI / 2);
		outRight.vec2.x = cos(atan(tangentSlope2));
		if (horizontal)
			outRight.vec2.y = sin(atan(tangentSlope2));
		else
			outRight.vec2.y = -sin(atan(tangentSlope2));
	}
	_outRes.push_back(outLeft);
	_outRes.push_back(outRight);

	box_up.size.width = box_up.size.height = right.x - left.x;
	m_pContext->SetBox(0, box_up);

	return _outRes;
}


std::vector<PointAndAngle> 
CPinAngleFitting::polynomialFit(
	const std::vector<cv::Point2f>& fpts_l,
	const std::vector<cv::Point2f>& fpts_r,
	cv::RotatedRect& box_up,
	const cv::RotatedRect& box_dn
) {
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	std::vector<double> rLeft, thetaLeft, rRight, thetaRight;
	std::vector<PointAndAngle> _outRes;
	int width = m_pContext->ImageWidth();
	int height = m_pContext->ImageHeight();

	double xc, ycLeft, ycRight;
	float min_x = 10000.f, min_y = -1.f, max_x = -1.f, max_y = -1.f;
	int leftPointCnt = 0;
	int rightPointCnt = 0;
	bool horizontal = (m_pContext->mainShape() == eShapeHorizontal);
	if (horizontal) {
		leftPointCnt = std::max(20, std::min((int)fpts_l.size() * 3 / 5, POINT_CNT * 2));
		rightPointCnt = std::max(20, std::min(((int)fpts_r.size()) * 3 / 5, POINT_CNT * 2));
	}
	else {
		leftPointCnt = std::max(10, std::min((int)fpts_l.size() * 3 / 5, POINT_CNT));
		rightPointCnt = std::max(10, std::min((int)fpts_r.size() * 3 / 5, POINT_CNT));
	}

	xc = (left.x + right.x) / 2;
	ycLeft = std::max(left.y, right.y);
	ycRight = ycLeft;

	double theta_minLeft = 100000, theta_maxLeft = -10000000;
	double theta_minRight = 100000, theta_maxRight = -10000000;

	// left
	get_boundary_xpos_on_contour(fpts_l, min_x, min_y, true);
	if (min_y < ycLeft)
		ycLeft = min_y;
	get_polar_array_on_contour(
		fpts_l, true, 
		xc, ycLeft, leftPointCnt, 
		rLeft, thetaLeft
	);
	if (rLeft.empty())
		return _outRes;
	auto leftMinMax = std::minmax_element(thetaLeft.begin(), thetaLeft.end());
	theta_minLeft = *leftMinMax.first;
	theta_maxLeft = *leftMinMax.second;
	Eigen::VectorXd coefsLeft = polynomialCurveFit(thetaLeft, rLeft, POLYNOMIAL_DEGREE_POLAR);
	// drawPolynomialPolar(coefsLeft, theta_minLeft, theta_maxLeft, xc, ycLeft, m_debugImg, true);
	getPointsPolynomialPolar(coefsLeft, theta_minLeft, theta_maxLeft, xc, ycLeft, width, height, true, m_pContext->LeftPolyPtRef());

	// right
	get_boundary_xpos_on_contour(
		fpts_r, max_x, max_y, false
	);
	if (max_y < ycRight - 1)
		ycRight = max_y;
	get_polar_array_on_contour(
		fpts_r, false, 
		xc, ycRight, 
		rightPointCnt,
		 rRight, thetaRight
	);
	if (rRight.empty())
		return _outRes;
	auto rightMinMax = std::minmax_element(thetaRight.begin(), thetaRight.end());
	theta_minRight = *rightMinMax.first;
	theta_maxRight = *rightMinMax.second;
	auto coefsRight = polynomialCurveFit(thetaRight, rRight, POLYNOMIAL_DEGREE_POLAR);
	// drawPolynomialPolar(coefsRight, theta_minRight, theta_maxRight, xc, ycRight, m_debugImg, false);
	getPointsPolynomialPolar(coefsRight, theta_minRight, theta_maxRight, xc, ycRight, width, height, false, m_pContext->RightPolyPtRef());

	// calculate angles

	PointAndAngle outLeft, outRight;
	const double MARGIN_THETA = 10.0 * M_PI / 180.0;
	double lt0 = theta_minLeft, lt1 = theta_maxLeft;
	double rt0 = theta_minRight, rt1 = theta_maxRight;

	auto computeTheta = [&](float x, float y, double yc, bool isLeft) -> double {
		double dx = x - xc;
		double dy = y - yc;
		double theta = atan2(dy, dx);
		if (isLeft && theta < 0)
			theta += 2 * M_PI;
		return theta;
	};
	double lt2 = computeTheta(left.x, left.y + 10.f, ycLeft, true);
	double lt3 = computeTheta(left.x + 10.f, left.y, ycLeft, true);
	lt0 = std::min(lt2, lt3);// lt0 - MARGIN_THETA;
	lt1 = lt1;// lt1 + MARGIN_THETA;

	double rt2 = computeTheta(right.x, right.y + 10.f, ycRight, false);
	double rt3 = computeTheta(right.x - 10.f, right.y, ycRight, false);
	rt0 = rt0;// -MARGIN_THETA;
	rt1 = std::max(rt2, rt3); // rt1 + MARGIN_THETA;	

	if (horizontal) {
		outLeft = calculate_polar_angles_on_horizontal(
			left, right,
			lt0, lt1,
			xc, ycLeft,
			coefsLeft, true
		);
		outRight = calculate_polar_angles_on_horizontal(
			left, right,
			rt0, rt1,
			xc, ycRight,
			coefsRight, false
		);
	}
	else {
		DropletEllipse ellipse(
			box_dn.center.x, box_dn.center.y,
			box_dn.size.width, box_dn.size.height,
			box_dn.angle * M_PI / 180.0); // TODO
		outLeft = calculate_polar_angles_on_curved(
			left, right,
			lt0, lt1,
			xc, ycLeft,
			coefsLeft, ellipse,
			true
		);
		outRight = calculate_polar_angles_on_curved(
			left, right,
			rt0, rt1,
			xc, ycRight,
			coefsRight, ellipse,
			false
		);
	}

	if (outLeft.point.x >= 0.0 && outLeft.point.y >= 0.0)
		_outRes.push_back(outLeft);
	if (outRight.point.x >= 0.0 && outRight.point.y >= 0.0)
		_outRes.push_back(outRight);

	box_up.size.width = box_up.size.height = right.x - left.x;
	m_pContext->SetBox(0, box_up);

	return _outRes;
}

void
CPinAngleFitting::get_boundary_xpos_on_contour(
	const std::vector<cv::Point2f>& fpts,
	float& _xpos,
	float& _ypos,
	bool isLeft
) {
	_ypos = -1.f;
	if (isLeft)
		_xpos = 10000.f;
	else
		_xpos = -1.f;

	for (int i = 0; i < fpts.size(); i++) {
		if (isLeft) {
			if (_xpos > fpts[i].x) {
				_xpos = fpts[i].x;
				_ypos = fpts[i].y;
			}
		}
		else {
			if (_xpos < fpts[i].x) {
				_xpos = fpts[i].x;
				_ypos = fpts[i].y;
			}
		}
	}
}


void CPinAngleFitting::get_xy_array_on_contour(
	const std::vector<cv::Point2f>& fpts_up,
	bool isLeft,
	int maxPtCnt,
	std::vector<double>& x,
	std::vector<double>& y,
	float& min_y,
	float& max_y,
	float& argmaxy_x,
	int maxDist
) {
	double dist1 = 0.0, dist2 = 0.0;
	const auto& leftPt = m_pContext->leftPoint();
	const auto& rightPt = m_pContext->rightPoint();
	const auto& refPt = (isLeft ? leftPt : rightPt);
	double width = (double)m_pContext->ImageWidth();
	min_y = 10000.f;
	max_y = -1.f;
	argmaxy_x = -1.f;
	int pointCnt = (int)fpts_up.size();

	if (!fpts_up.empty()) {
		dist1 = distance(fpts_up[0], refPt);
		dist2 = distance(fpts_up[pointCnt - 1], refPt);
	}

	if (dist1 < dist2) {
		for (int i = 0; i < pointCnt; i++) {
			if (refPt.y - fpts_up[i].y < maxDist && refPt.y >= fpts_up[i].y) {
				if(isLeft)
					x.push_back(width - fpts_up[i].x);
				else
					x.push_back(fpts_up[i].x);
				y.push_back(fpts_up[i].y);
				min_y = std::min(min_y, fpts_up[i].y);
				if (max_y < fpts_up[i].y) {
					max_y = fpts_up[i].y;
					argmaxy_x = fpts_up[i].x;
				}
			}
			if (x.size() > maxPtCnt)
				break;
		}
	}
	else {
		for (int i = pointCnt - 1; i >= 0; i--) {
			if (refPt.y - fpts_up[i].y < maxDist && refPt.y >= fpts_up[i].y) {
				if (isLeft)
					x.push_back(width - fpts_up[i].x);
				else
					x.push_back(fpts_up[i].x);
				y.push_back(fpts_up[i].y);
				min_y = std::min(min_y, fpts_up[i].y);
				if (max_y < fpts_up[i].y) {
					max_y = fpts_up[i].y;
					argmaxy_x = fpts_up[i].x;
				}
			}
			if (x.size() > maxPtCnt)
				break;
		}
	}
}

void CPinAngleFitting::get_polar_array_on_contour(
	const std::vector<cv::Point2f>& fpts,
	bool isLeft,
	double xc,
	double yc,
	int pointCnt,
	std::vector<double>& r,
	std::vector<double>& theta,
	int maxDist
) {
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	if (isLeft) {
		for (int i = (int)fpts.size() - 1; i >= 0; i--) {
			if (left.y - fpts[i].y < maxDist) {
				r.push_back(disSqrt(fpts[i].x, fpts[i].y, xc, yc));
				double angle = atan2((fpts[i].y - yc), (fpts[i].x - xc));
				if (angle <= 0)
					angle += 2 * M_PI;
				theta.push_back(angle);
			}
			if (r.size() > pointCnt)
				break;
		}
	}
	else {
		for (int i = (int)fpts.size() - 1; i >= 0; i--) {
			if (right.y - fpts[i].y < maxDist) {
				r.push_back(disSqrt(fpts[i].x, fpts[i].y, xc, yc));
				double angle = atan2((fpts[i].y - yc), (fpts[i].x - xc));
				theta.push_back(angle);
			}
			if (r.size() > pointCnt)
				break;
		}
	}
}

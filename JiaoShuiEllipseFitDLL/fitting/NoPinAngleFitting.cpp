#include "NoPinAngleFitting.h"
#include "..\cvLib\cvcommon.h"
#include "fit_util.h"
#include "draw_utils.h"

CNoPinAngleFitting::CNoPinAngleFitting(CDropletContext* pCtx)
	: CDropletFittingBase(pCtx)
{
	if (pCtx) {
		pCtx->ClearResultVariables();
	}
}

CNoPinAngleFitting::~CNoPinAngleFitting()
{
}

ErrorCode CNoPinAngleFitting::process() 
{
	if (m_pContext == NULL || m_pContext->IsEmptyImage())
		return errFailOpen;
		
	if (m_pContext->isEqualLRPoints())
		return errFailFitting;
	try {
		std::vector<cv::Point2f> fpts_up, fpts_dn;		
		auto& _outDropletPts = m_pContext->DropletPtRef();
		auto& _outBaselinePts = m_pContext->BaelinePtRef();

		std::vector<PointAndAngle> _outRes;
		double _outAngles[2] = { -1.0, -1.0 }; // left, right

		int left_cnt = 0;
		if (!prepareContours(_outDropletPts, _outBaselinePts, fpts_up, fpts_dn, left_cnt))
			return errNotFoundContour;

		cv::RotatedRect box_up;
		if (!fitDroplet(fpts_up, box_up)) {
			return errFailedFitDroplet;
		}
		m_pContext->SetBox(0, box_up);

		cv::RotatedRect box_dn;
		if (!fitBaseline(fpts_dn, box_dn)) {
			return errFailedFitBaseline;
		}
		m_pContext->SetBox(2, box_dn);

		bool resIntersection = findIntersections(box_dn, fpts_up, box_up, left_cnt, _outRes);

		if (!resIntersection || _outRes.size() < 1) {
			return errNotFoundIntersection;
		}
		if (_outRes.size() == 2) {
			if (_outRes[0].point.x > _outRes[1].point.x) {
				// Reversing the vector
				std::reverse(_outRes.begin(), _outRes.end());
			}
		}
		int length = (int)(std::min(box_up.size.width, box_up.size.height) / 2.0f);
		if (m_pContext->DropletMode() == eDropletWidthHeight)
			length = length * 2;
		length = std::min(length, m_pContext->ImageWidth() / 3);
		for (int i = 0; i < _outRes.size(); i++) {
			if (_outRes[i].vec1.y > 0.0) { // 需要反转——液滴
				_outRes[i].vec1.x = -_outRes[i].vec1.x;
				_outRes[i].vec1.y = -_outRes[i].vec1.y;
			}
		}
		if (!_outRes.empty() && _outRes[0].vec2.x < 0) {
			_outRes[0].vec2.x = -_outRes[0].vec2.x;
			_outRes[0].vec2.y = -_outRes[0].vec2.y;
		}
		if (_outRes.size() > 1) {
			if (_outRes[1].vec2.x > 0) {
				_outRes[1].vec2.x = -_outRes[1].vec2.x;
				_outRes[1].vec2.y = -_outRes[1].vec2.y;
			}
		}
		// 角度计算		
		ErrorCode result_code = errNo;
		if (_outRes.size() == 1) {
			_outAngles[0] = get_point_angle(_outRes[0]);
			_outAngles[1] = -1.0;
			result_code = errFailOnlyOne;
		}
		else
		{
			int left_index = (_outRes[0].point.x < _outRes[1].point.x) ? 0 : 1;
			_outAngles[0] = angle_between_vectors(_outRes[left_index].vec1, _outRes[left_index].vec2);
			int right_index = 1 - left_index;
			_outAngles[1] = angle_between_vectors(_outRes[right_index].vec1, _outRes[right_index].vec2);
		}
		m_pContext->SetAnglePoints(_outRes);
		m_pContext->SetAngles(_outAngles);
		return result_code;
	}
	catch (...)
	{
		return errException;
	}
}



bool CNoPinAngleFitting::prepareContours(
	std::vector<cv::Point>& _outDropletPts,
	std::vector<cv::Point>& _outBaselinePts,
	std::vector<cv::Point2f>& fpts_up, 
	std::vector<cv::Point2f>& fpts_dn,
	int& left_cnt
) {
	left_cnt = 0; // 液滴的左边的点数
	auto leftPt = m_pContext->leftPoint();
	auto rightPt = m_pContext->rightPoint();
	cv::Point pt_li = leftPt, pt_ri = rightPt;
	switch (m_pContext->mainShape())
	{
	case eShapeConvex:
		left_cnt = find_contour_points_tumian(_outDropletPts, _outBaselinePts);
		break;
	case eShapeConcave:
		left_cnt = find_contour_points_aomian(_outDropletPts, _outBaselinePts);
		break;
	case eShapeHorizontal:
		left_cnt = find_contour_points_shuipingmian(_outDropletPts);
		if (!m_pContext->IsSubpixel()) {
			checkAndUpdateBaseLine(m_pContext->Image(), pt_li, pt_ri, _outDropletPts);
			if (leftPt.x != pt_li.x || leftPt.y != pt_li.y) {
				leftPt = pt_li;
			}
			if (rightPt.x != pt_ri.x || rightPt.y != pt_ri.y) {
				rightPt = pt_ri;
			}
			m_pContext->SetLeftRightPoints(leftPt, rightPt);
		}
		break;
	default:
		return false;
		break;
	}
#if DEBUG_IMG
	cv::Mat dbgImg = m_pContext->Image().clone();
	for (const auto& pt : _outDropletPts) {
		cv::circle(dbgImg, pt, 2, cv::Scalar(0, 0, 255), -1);
	}
	for (const auto& pt : _outBaselinePts) {
		cv::circle(dbgImg, pt, 2, cv::Scalar(255, 0, 0), -1);
	}
	SaveDebugImg("prepare_contour-dropletanalyzer", dbgImg);
#endif
	if (_outDropletPts.empty())
		return false;

	for (const auto& pt : _outDropletPts)
		fpts_up.emplace_back(pt);
	for (const auto& pt : _outBaselinePts)
		fpts_dn.emplace_back(pt);

	if (m_pContext->IsSubpixel()) {
		cv::Mat gray;
		cv::cvtColor(m_pContext->Image(), gray, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);
		cv::cornerSubPix(gray, fpts_up, cv::Size(5, 5), cv::Size(-1, -1),
			cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.01));
	}

	return true;
}


bool 
CNoPinAngleFitting::fitBaseline(
	const std::vector<cv::Point2f>& fpts_dn,
	cv::RotatedRect& box_dn
) {
	// 拟合——曲面
	const auto left = m_pContext->leftPoint(), right = m_pContext->rightPoint();
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());
	memset(&box_dn, 0, sizeof(cv::RotatedRect));
	const auto fitMode = m_pContext->fitMode();
	const auto mainShape = m_pContext->mainShape();
	const auto baselineFitmode = m_pContext->BaselineMode();
	if (fitMode == eFitAuto && mainShape < eShapeHorizontal) { // for concave, convex
		if (baselineFitmode == eBaseLineCircle)
			ellipse_regression_Circle(fpts_dn, dimension, box_dn);
		else
			ellipse_regression_OpenCV(fpts_dn, baselineFitmode - 1, left, right, box_dn);
		if (box_dn.size.width == 0.0 || box_dn.size.height == 0.0)
			return false;
	}
	else if (fitMode == eFitAuto) {	// for horizontal
		box_dn.center = left;
		box_dn.angle = (left.x == right.x) ? 90.0 : (double)atanf((left.y - right.y) / (left.x - right.x)) * 180.0 / M_PI;
		box_dn.size.width = fabs(right.x - left.x);
		box_dn.size.height = fabs(right.y - left.y);
	}
	else { // 半自动
		if (baselineFitmode == eBaseLineCircle) {
			std::tuple<double, double, double> circle2 = findCircleCoefficients(
				MPoint(m_baselineX[0], m_baselineY[0]),
				MPoint(m_baselineX[1], m_baselineY[1]),
				MPoint(m_baselineX[2], m_baselineY[2]));
			if(std::get<0>(circle2) == 0.0 &&
				std::get<1>(circle2) == 0.0 &&
				std::get<2>(circle2) == 0.0)
				return false; // 没有找到圆

			double h2 = std::get<0>(circle2);
			double k2 = std::get<1>(circle2);
			double r2 = std::get<2>(circle2);

			box_dn.center = cv::Point2f((float)h2, (float)k2);
			box_dn.size = cv::Size2f((float)r2, (float)r2);
			box_dn.angle = 0;
		}
		else {
			EllipseParams ep;
			double theta;
			Eigen::VectorXd coeff = solveEllipseCoefficients(m_baselineX, m_baselineY);
			if (extractEllipseParams(coeff, ep, theta)) {
				box_dn.center = cv::Point2f((float)ep.h, (float)ep.k);
				box_dn.size = cv::Size2f((float)ep.a, (float)ep.b);
				box_dn.angle = (float)theta * 180.f / (float)M_PI;
			}
		}
	}
	if (mainShape != eShapeHorizontal) {
		auto maxSize = std::max(box_dn.size.width, box_dn.size.height);
		if (maxSize < 5.0f)
			return false;
	}
	return true;
}



bool 
CNoPinAngleFitting::fitDroplet(
	const std::vector<cv::Point2f>& fpts_up,
	cv::RotatedRect& box_up
) {
	// 拟合——液滴
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());
	const auto left = m_pContext->leftPoint(), right = m_pContext->rightPoint();
	DropletFitMode dropfitMode = m_pContext->DropletMode();
	if (dropfitMode == eDropletCircle) // for circle
		ellipse_regression_Circle(fpts_up, dimension, box_up);
	else if (dropfitMode <= eDropletDoubleEllipse) // for ellipse
		ellipse_regression_OpenCV(fpts_up, dropfitMode - 1, left, right, box_up);

	if (dropfitMode <= eDropletDoubleEllipse &&
		(box_up.size.width == 0.0 || box_up.size.height == 0.0))
		return false;
	return true;
}

bool CNoPinAngleFitting::findIntersections(
	const cv::RotatedRect& box_dn,
	const std::vector<cv::Point2f>& fpts_up,
	cv::RotatedRect& box_up, int& left_cnt,
	std::vector<PointAndAngle>& _outRes)
{
	// 交叉点，切线		
	bool foundIntersection = false;
	const auto left = m_pContext->leftPoint(), right = m_pContext->rightPoint();
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());
	const auto fitMode = m_pContext->fitMode();
	const auto mainShape = m_pContext->mainShape();
	const auto baselineFitmode = m_pContext->BaselineMode();
	const auto dropletFitmode = m_pContext->DropletMode();

	if (mainShape < eShapeHorizontal) { // for concave, convex
		if (dropletFitmode == eDropletCircle && baselineFitmode == eBaseLineCircle) {
			_outRes = getCircleIntersections(
				box_up.center.x, box_up.center.y, box_up.size.width,
				box_dn.center.x, box_dn.center.y, box_dn.size.width
			);
			foundIntersection = !_outRes.empty();
		}
		else { // m_baselineFitMode is ellipse
			if (dropletFitmode == eDropletDoubleEllipse) { // 双椭圆		
				auto processSegment = [&](const auto& points, bool isLeft) {
					ellipse_regression_OpenCV(points, 0, left, right, box_up);	
					if(isLeft)
						m_pContext->SetBox(0, box_up);
					else
						m_pContext->SetBox(1, box_up);
					auto refpt = (isLeft ? left : right);
					PointAndAngle intersection = findIntersectionEllipseOne(
						box_up.center, box_up.size,
						box_up.angle * (CV_PI / 180.0),
						box_dn.center, box_dn.size,
						box_dn.angle * (CV_PI / 180.0),
						refpt
					);

					if (intersection.point.x != 0 && intersection.point.y != 0) {
						_outRes.push_back(intersection);
					}
				};

				// Process left segment
				std::vector<cv::Point2f> left_segment(fpts_up.begin(), fpts_up.begin() + fpts_up.size() * 2 / 3);
				processSegment(left_segment, true);

				// Process right segment with overlap
				std::vector<cv::Point2f> right_segment(fpts_up.begin() + fpts_up.size() / 2, fpts_up.end());
				right_segment.insert(right_segment.end(),
					fpts_up.begin(),
					fpts_up.begin() + (fpts_up.size() * 2 / 3 - right_segment.size()));
				processSegment(right_segment, false);
			}
			else if (dropletFitmode == eDropletDoublePolynomial) { // 双多项式 - not horizontal
				_outRes = doublePolynomialFit(fpts_up, left_cnt, box_up, box_dn, false);
			}
			else if (dropletFitmode == eDropletPolynomial) { // 多项式（极坐) - not horizontal
				_outRes = polynomialFit(fpts_up, left_cnt, box_up, box_dn, false);
			}
			else if (dropletFitmode == eDropletDoubleCircle) { // 双圆 - not horizontal
				_outRes = doubleCircleFitOnCurved(fpts_up, left_cnt, box_up, box_dn);
			}
			else { // 椭圆
				EllipseParams ep1(box_up.center.x, box_up.center.y, box_up.size.width, box_up.size.height);
				EllipseParams ep2(box_dn.center.x, box_dn.center.y, box_dn.size.width, box_dn.size.height);
				_outRes = findIntersectionsEllipse(
					ep1, 
					box_up.angle * M_PI / 180.0,
					ep2, 
					box_dn.angle * M_PI / 180.0,
					(fitMode != eFitSemiAuto && fitMode != eFitManual),
					&left, &right
				);
			}
		}
	}
	else { // for horizontal
		if (dropletFitmode == eDropletCircle) {
			_outRes = findIntersectionsCircleLine(
				box_up.center, 
				box_up.size.width, 
				left, right
			);
		}
		else if (dropletFitmode == eDropletDoubleEllipse) { // 双椭圆 
			std::vector<PointAndAngle> out;
			std::vector<cv::Point2f> tmp_pts;
			// left
			if (left_cnt == 0)
				left_cnt = (int)fpts_up.size() / 2;
			for (int i = 0; i < left_cnt; i++) {
				tmp_pts.push_back(fpts_up[i]);
			}
			int tmp_cnt = (int)tmp_pts.size();
			int fpts_num = (int)fpts_up.size() * 2 / 3;
			for (int i = 0; i < fpts_num - tmp_cnt; i++) {
				tmp_pts.push_back(fpts_up[left_cnt + i]);
			}
			ellipse_regression_OpenCV(tmp_pts, 0, left, right, box_up);
			m_pContext->SetBox(0, box_up);
			out = findIntersectionsEllipseLine(
				box_up.center, box_up.size,
				box_up.angle * M_PI / 180.0, left, right, 1
			);
			if (!out.empty()) {
				for(const auto& it : out)
					_outRes.push_back(it);
			}

			tmp_pts.clear();
			// right
			for (int i = left_cnt; i < fpts_up.size(); i++) {
				tmp_pts.push_back(fpts_up[i]);
			}
			fpts_num = (int)fpts_up.size() * 2 / 3;
			tmp_cnt = (int)tmp_pts.size();
			for (int i = 0; i < fpts_num - tmp_cnt; i++) {
				tmp_pts.push_back(fpts_up[i]);
			}
			ellipse_regression_OpenCV(tmp_pts, 0, left, right, box_up);
			m_pContext->SetBox(1, box_up);
			out = findIntersectionsEllipseLine(
				box_up.center, box_up.size,
				box_up.angle * M_PI / 180.0, left, right, 2);
			if (!out.empty()) {
				for (const auto& it : out)
					_outRes.push_back(it);
			}
		}
		else if (dropletFitmode == eDropletPolynomial) {// 多项式（极坐）- horizontal
			_outRes = polynomialFit(fpts_up, left_cnt, box_up, box_dn, true);
		}
		else if (dropletFitmode == eDropletDoublePolynomial) { // 双多项式	- horizontal
			_outRes = doublePolynomialFit(fpts_up, left_cnt, box_up, box_dn, true);
		}
		else if (dropletFitmode == eDropletWidthHeight) { // 高宽法 - horizontal				
			double h = -100;
			for (int i = 0; i < fpts_up.size(); i++) {
				double dis = distanceToLine(
					fpts_up[i].x, fpts_up[i].y, 
					left.x, left.y, 
					right.x, right.y
				);
				if (h < dis)
					h = dis;
			}

			double angleRad = atan2(right.y - left.y, right.x - left.x);
			box_up.angle = (float)(angleRad * 180.0 / M_PI);

			centerOfRotatedRectangle(
				left.x, left.y, 
				right.x, right.y,
				h, 
				box_up.center.x, box_up.center.y
			);
			box_up.size.width = sqrt(
				(left.x - right.x) * (left.x - right.x) + 
				(left.y - right.y) * (left.y - right.y)
			);
			box_up.size.height = (float)h;

			m_pContext->SetBox(0, box_up);
			PointAndAngle out;
			if (box_up.size.width != 0)
				out.angle = 2.0 * atan(h / box_up.size.width * 2.0);
			out.point.x = left.x;
			out.point.y = left.y;
			double base_angle = atan2(right.y - left.y, right.x - left.x);
			out.vec1.x = cos(base_angle - out.angle);
			out.vec1.y = sin(base_angle - out.angle);
			out.vec2.x = cos(base_angle);
			out.vec2.y = sin(base_angle);
			_outRes.push_back(out);

			PointAndAngle out2;
			if (box_up.size.width != 0)
				out2.angle = 2.0 * atan(h / box_up.size.width * 2.0);
			out2.point.x = right.x;
			out2.point.y = right.y;
			base_angle = atan2(left.y - right.y, left.x - right.x);
			out2.vec1.x = cos(base_angle + out.angle);
			out2.vec1.y = sin(base_angle + out.angle);
			out2.vec2.x = cos(base_angle);
			out2.vec2.y = sin(base_angle);
			_outRes.push_back(out2);
		}
		else if (dropletFitmode == eDropletDoubleCircle) { // 双圆拟合
			_outRes = doubleCircleFitOnHorizontal(fpts_up, left_cnt, box_up, box_dn);			
		}
		else { // 椭圆拟合
			_outRes = findIntersectionsEllipseLine(
				box_up.center, 
				box_up.size, 
				box_up.angle * M_PI / 180.0,
				left, right
			);
		}
	}
	return true;
}

std::vector<PointAndAngle>
CNoPinAngleFitting::doublePolynomialFit(
	const std::vector<cv::Point2f>& fpts_up,	// 上半部分轮廓点
	int&							left_cnt,	// 左边点数
	cv::RotatedRect&				box_up,		// 上半部分拟合结果
	const cv::RotatedRect&			box_dn,		// 下半部分拟合结果
	bool							horizontal	// 是否水平拟合
) {
	std::vector<double> xxLeft, yyLeft, xxRight, yyRight;
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	std::vector<PointAndAngle> _outRes;
	if (left_cnt == 0)
		left_cnt = (int)fpts_up.size() / 2;

	int leftPointCount = std::max(10, std::min(left_cnt * 4 / 5, POINT_CNT));
	if (horizontal) {
		leftPointCount = std::max(20, std::min(left_cnt * 4 / 5, POINT_CNT * 2));
	}
	int rightPointCnt = std::max(10, std::min(((int)fpts_up.size() - left_cnt) * 3 / 5, POINT_CNT));
	if (horizontal) {
		rightPointCnt = std::max(20, std::min(left_cnt * 3 / 5, POINT_CNT * 2));
	}

	// left
	float lmin_y = 10000.f;
	float lmax_y = -1.f;
	float largmaxy_x = -1.f;
	float rmin_y = 10000.f;
	float rmax_y = -1.f;
	float rargmaxy_x = -1.f;
	int width = m_pContext->ImageWidth();
	int height = m_pContext->ImageHeight();

	get_xy_array_on_contour(
		fpts_up, true, left_cnt, leftPointCount, xxLeft, yyLeft, lmin_y, lmax_y, largmaxy_x
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
		fpts_up, false, left_cnt, rightPointCnt, xxRight, yyRight, rmin_y, rmax_y, rargmaxy_x
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
	//drawPolynomialYReverse(coefsLeft, (int)lmin_y, leftReal.y + 1, m_debugImg);
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
			double dx = 0, dy = 0;
			ellipse.getTangentVector(leftReal.x, leftReal.y, dx, dy);
			tangentSlope2 = (std::abs(dx) < 1e-10) ? 10000000 : dy / dx;
			tangentSlope2 *= -1.0;
			// tangentSlope2 = ellipse.tangentSlope(leftReal.x, leftReal.y);
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

void 
CNoPinAngleFitting::get_boundary_xpos_on_contour(
	int start, 
	int end, 
	const std::vector<cv::Point2f>& fpts_up, 
	float& _xpos, 
	float& _ypos,
	bool left
) {
	if (left) {
		_xpos = 10000.f, _ypos = -1.f;
		for (int i = start; i < end; i++) {
			if (_xpos > fpts_up[i].x) {
				_xpos = fpts_up[i].x;
				_ypos = fpts_up[i].y;
			}
		}
	}
	else {		
		_xpos = -1.f; _ypos = 10000.f;
		for (int i = start; i < end; i++) {
			if (_xpos < fpts_up[i].x) {
				_xpos = fpts_up[i].x;
				_ypos = fpts_up[i].y;
			}
		}
	}
}
void CNoPinAngleFitting::get_xy_array_on_contour(
	const std::vector<cv::Point2f>& fpts_up,
	bool isLeft,
	int left_cnt,
	int pointCnt,
	std::vector<double>& x,
	std::vector<double>& y,
	float& min_y,
	float& max_y,
	float& argmaxy_x,
	int maxDist
) {
	double dist1 = 0.0, dist2 = 0.0;
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	double width = (double)m_pContext->ImageWidth();
	min_y = 10000.f;
	max_y = -1.f;
	argmaxy_x = -1.f;

	if (isLeft) {
		if (!fpts_up.empty() && left_cnt > 0) {
			dist1 = distance(fpts_up[0], left);
			dist2 = distance(fpts_up[left_cnt - 1], left);
		}

		if (dist1 < dist2) {
			for (int i = 0; i < left_cnt; i++) {
				if (left.y - fpts_up[i].y < maxDist && left.y >= fpts_up[i].y) {
					x.push_back(width - fpts_up[i].x);
					y.push_back(fpts_up[i].y);
					min_y = std::min(min_y, fpts_up[i].y);
					if (max_y < fpts_up[i].y) {
						max_y = fpts_up[i].y;
						argmaxy_x = fpts_up[i].x;
					}
				}
				if (x.size() > pointCnt)
					break;
			}
		}
		else {
			for (int i = left_cnt - 1; i >= 0; i--) {
				if (left.y - fpts_up[i].y < 100 && left.y >= fpts_up[i].y) {
					x.push_back(width - fpts_up[i].x);
					y.push_back(fpts_up[i].y);
					min_y = std::min(min_y, fpts_up[i].y);
					if (max_y < fpts_up[i].y) {
						max_y = fpts_up[i].y;
						argmaxy_x = fpts_up[i].x;
					}
				}
				if (x.size() > pointCnt)
					break;
			}
		}
	}
	else {
		dist1 = 0.f, dist2 = 0.f;
		if (fpts_up.size() > 1 && left_cnt >= 0 && left_cnt < (int)fpts_up.size()) {
			dist1 = distance(fpts_up[left_cnt], right);
			dist2 = distance(fpts_up[fpts_up.size() - 1], right);
		}
		if (dist1 < dist2) {
			for (int i = left_cnt; i < fpts_up.size(); i++) {
				if (right.y - fpts_up[i].y < 100 && right.y >= fpts_up[i].y) {
					x.push_back(fpts_up[i].x);
					y.push_back(fpts_up[i].y);
					min_y = std::min(min_y, fpts_up[i].y);
					if (max_y < fpts_up[i].y) {
						max_y = fpts_up[i].y;
						argmaxy_x = fpts_up[i].x;
					}
				}
				if (x.size() > pointCnt)
					break;
			}
		}
		else {
			for (int i = (int)fpts_up.size() - 1; i > left_cnt; i--) {
				if (right.y - fpts_up[i].y < 100 && right.y >= fpts_up[i].y) {
					x.push_back(fpts_up[i].x);
					y.push_back(fpts_up[i].y);

					min_y = std::min(min_y, fpts_up[i].y);
					if (max_y < fpts_up[i].y) {
						max_y = fpts_up[i].y;
						argmaxy_x = fpts_up[i].x;
					}
				}
				if (x.size() > pointCnt)
					break;
			}
		}
	}
}

void CNoPinAngleFitting::get_polar_array_on_contour(
	const std::vector<cv::Point2f>& fpts_up,
	bool isLeft,
	int left_cnt,
	double xc,
	double yc,
	int pointCnt,
	std::vector<double>& r,
	std::vector<double>& theta,
#if DEBUG_IMG
	std::vector<cv::Point>& debugPts,
#endif
	int maxDist
) {
	float dist1 = 0.f, dist2 = 0.f;
	int iStart = 0, iEnd = 0;
	cv::Point2f refPt;
	if (isLeft) {
		iStart = 0;
		iEnd = left_cnt;
		refPt = m_pContext->leftPoint();
	}
	else {
		iStart = left_cnt;
		iEnd = (int)fpts_up.size();
		refPt = m_pContext->rightPoint();
	}
	if (!fpts_up.empty() && left_cnt > 0 && left_cnt < fpts_up.size()) {
		dist1 = distance(fpts_up[iStart], refPt);
		dist2 = distance(fpts_up[iEnd - 1], refPt);
	}
	r.clear();
	theta.clear();
	if (dist1 < dist2) {
		for (int i = iStart; i < iEnd; i++) {
			if (refPt.y - fpts_up[i].y < maxDist && refPt.y >= fpts_up[i].y) {
				r.push_back(disSqrt(fpts_up[i].x, fpts_up[i].y, xc, yc));
				double angle = atan2((fpts_up[i].y - yc), (fpts_up[i].x - xc));
#if DEBUG_IMG
				debugPts.push_back(cv::Point((int)fpts_up[i].x, (int)fpts_up[i].y));
#endif
				if (isLeft && angle <= 0)
					angle += 2 * M_PI;
				theta.push_back(angle);
			}
			if (r.size() > pointCnt)
				break;
		}
	}
	else {
		for (int i = iEnd - 1; i >= iStart; i--) {
			if (refPt.y - fpts_up[i].y < maxDist && refPt.y >= fpts_up[i].y) {
				r.push_back(disSqrt(fpts_up[i].x, fpts_up[i].y, xc, yc));
				double angle = atan2((fpts_up[i].y - yc), (fpts_up[i].x - xc));
#if DEBUG_IMG
				debugPts.push_back(cv::Point((int)fpts_up[i].x, (int)fpts_up[i].y));
#endif
				if (isLeft && angle <= 0)
					angle += 2 * M_PI;
				theta.push_back(angle);
			}
			if (r.size() > pointCnt)
				break;
		}
	}
}

std::vector<PointAndAngle>
CNoPinAngleFitting::polynomialFit(
	const std::vector<cv::Point2f>& fpts_up,
	int& left_cnt,
	cv::RotatedRect& box_up,
	const cv::RotatedRect& box_dn,
	bool horizontal
) {
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	std::vector<PointAndAngle> _outRes;
	std::vector<double> rLeft, thetaLeft, rRight, thetaRight;
	int width = m_pContext->ImageWidth();
	int height = m_pContext->ImageHeight();

	if (left_cnt == 0)
		left_cnt = (int)fpts_up.size() / 2;
	int leftPointCnt = 0; 
	int rightPointCnt = 0;
	if (horizontal) {
		leftPointCnt = std::max(20, std::min(left_cnt * 3 / 5, POINT_CNT * 2));
		rightPointCnt = std::max(20, std::min(((int)fpts_up.size() - left_cnt) * 3 / 5, POINT_CNT * 2));
	}
	else {
		leftPointCnt = std::max(10, std::min(left_cnt * 3 / 5, POINT_CNT));
		rightPointCnt = std::max(10, std::min(((int)fpts_up.size() - left_cnt) * 3 / 5, POINT_CNT));
	}
	double xc = (left.x + right.x) / 2;	

	// left	interpolation
	float min_x = 10000.f, max_y = -1.f;
	double ycLeft = std::max(left.y, right.y);
	get_boundary_xpos_on_contour(0, left_cnt, fpts_up, min_x, max_y, true);	
	if (max_y < ycLeft)
		ycLeft = max_y;
#if DEBUG_IMG
	std::vector<cv::Point> leftPts;
#endif // _DEBUG		
	get_polar_array_on_contour(
		fpts_up, true, left_cnt, 
		xc, ycLeft, leftPointCnt,
		rLeft, thetaLeft
#if DEBUG_IMG
		,leftPts
#endif // _DEBUG	
	);	
	if (thetaLeft.empty()) 
		return _outRes;
#if DEBUG_IMG
	cv::Mat tmp = m_pContext->Image().clone();
	for (auto lpt : leftPts) {
		cv::drawMarker(tmp, lpt, cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 1, 1);
	}
	SaveDebugImg("left_contour-nonpin", tmp);
#endif
	Eigen::VectorXd coefsLeft = polynomialCurveFit(thetaLeft, rLeft, POLYNOMIAL_DEGREE_POLAR);
	auto leftMinMax = std::minmax_element(thetaLeft.begin(), thetaLeft.end());
	double theta_minLeft = *leftMinMax.first;
	double theta_maxLeft = *leftMinMax.second;	
	// drawPolynomialPolar(coefsLeft, theta_minLeft, theta_maxLeft, xc, ycLeft, m_debugImg);
	getPointsPolynomialPolar(coefsLeft, theta_minLeft, theta_maxLeft, xc, ycLeft, width, height, true, m_pContext->LeftPolyPtRef());

	// right interpolation
	double ycRight = std::max(left.y, right.y);
	float max_x = -1.f;
	max_y = 10000.f;
	get_boundary_xpos_on_contour(
		left_cnt, (int)fpts_up.size(),
		fpts_up, max_x, max_y, false
	);
	if (max_y < ycRight - 1)
		ycRight = max_y;
#if DEBUG_IMG
	std::vector<cv::Point> rightPts;
#endif // _DEBUG
	get_polar_array_on_contour(
		fpts_up, false,
		left_cnt, xc, ycRight,
		rightPointCnt,
		rRight, thetaRight
#if DEBUG_IMG
		,rightPts
#endif
	);
	if (thetaRight.empty())
		return _outRes;
#if DEBUG_IMG
	cv::Mat tmp1 = m_pContext->Image().clone();
	for (auto rpt : rightPts) {
		cv::drawMarker(tmp1, rpt, cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 1, 1);
	}
	SaveDebugImg("right_contour-nonpin", tmp1);
#endif
	auto coefsRight = polynomialCurveFit(thetaRight, rRight, POLYNOMIAL_DEGREE_POLAR);
	auto rightMinMax = std::minmax_element(thetaRight.begin(), thetaRight.end());
	auto theta_minRight = *rightMinMax.first;
	auto theta_maxRight = *rightMinMax.second;
	
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

	auto leftPt = m_pContext->leftPoint();
	auto rightPt = m_pContext->rightPoint();

	if (horizontal) {
		outLeft = calculate_polar_angles_on_horizontal(
			leftPt, rightPt,
			lt0, lt1, 
			xc, ycLeft, 
			coefsLeft, true
		);
		outRight = calculate_polar_angles_on_horizontal(
			leftPt, rightPt,
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
			leftPt, rightPt,
			lt0, lt1,
			xc, ycLeft, 
			coefsLeft, ellipse, 
			true
		);
		outRight = calculate_polar_angles_on_curved(
			leftPt, rightPt,
			rt0, rt1,
			xc, ycRight,
			coefsRight, ellipse,
			false
		);
	}

	if(outLeft.point.x >= 0.0 && outLeft.point.y >= 0.0)
		_outRes.push_back(outLeft);
	if (outRight.point.x >= 0.0 && outRight.point.y >= 0.0)
		_outRes.push_back(outRight);

	box_up.size.width = box_up.size.height = right.x - left.x;
	m_pContext->SetBox(0, box_up);

	return _outRes;
}

std::vector<PointAndAngle> 
CNoPinAngleFitting::doubleCircleFitOnCurved(
	const std::vector<cv::Point2f>& fpts_up,
	int& left_cnt,
	cv::RotatedRect& box_up,
	const cv::RotatedRect& box_dn
) {
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	std::vector<PointAndAngle> _outRes;
	std::vector<cv::Point2f> tmp_pts;
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());

	// left
	if (left_cnt == 0)
		left_cnt = (int)fpts_up.size() / 2;
	for (int i = 0; i < left_cnt; i++) {
		tmp_pts.push_back(fpts_up[i]);
	}
	ellipse_regression_Circle(tmp_pts, dimension, box_up);
	m_pContext->SetBox(0, box_up);

	auto baselineFitmode = m_pContext->BaselineMode();
	if (baselineFitmode == eBaseLineCircle) {
		std::vector<PointAndAngle> out = getCircleIntersections(
			box_up.center.x, box_up.center.y, box_up.size.width,
			box_dn.center.x, box_dn.center.y, box_dn.size.width
		);
		if (out.size() == 2) {
			if (distance(out[0].point, left) < distance(out[1].point, left)) {
				_outRes.push_back(out[0]);
			}
			else {
				_outRes.push_back(out[1]);
			}
		}
		else if (out.size() == 1) {
			_outRes.push_back(out[0]);
		}
	}
	else {
		PointAndAngle out;
		out = findIntersectionEllipseOne(
			box_up.center, box_up.size,
			box_up.angle * M_PI / 180.0,
			box_dn.center, box_dn.size,
			box_dn.angle * M_PI / 180.0, left
		);

		if (out.point.x != 0 && out.point.y != 0) {
			_outRes.push_back(out);
		}
	}

	// right
	tmp_pts.clear();
	for (int i = left_cnt; i < fpts_up.size(); i++) {
		tmp_pts.push_back(fpts_up[i]);
	}
	ellipse_regression_Circle(tmp_pts, dimension, box_up);
	m_pContext->SetBox(1, box_up);
	if (baselineFitmode == eBaseLineCircle) {
		std::vector<PointAndAngle> out = getCircleIntersections(
			box_up.center.x, box_up.center.y, box_up.size.width,
			box_dn.center.x, box_dn.center.y, box_dn.size.width
		);
		if (out.size() == 2) {
			if (distance(out[0].point, right) < distance(out[1].point, right)) {
				_outRes.push_back(out[0]);
			}
			else {
				_outRes.push_back(out[1]);
			}
		}
		else if (out.size() == 1) {
			_outRes.push_back(out[0]);
		}
	}
	else {
		PointAndAngle out;
		out = findIntersectionEllipseOne(
			box_up.center, box_up.size,
			box_up.angle * M_PI / 180.0,
			box_dn.center, box_dn.size,
			box_dn.angle * M_PI / 180.0, right
		);
		if (out.point.x != 0 && out.point.y != 0) {
			_outRes.push_back(out);
		}
	}
	return _outRes;
}

std::vector<PointAndAngle> 
CNoPinAngleFitting::doubleCircleFitOnHorizontal(
	const std::vector<cv::Point2f>& fpts_up,	// 上半部分轮廓点
	int& left_cnt,								// 左边点数
	cv::RotatedRect& box_up,					// 上半部分拟合结果
	const cv::RotatedRect& box_dn				// 下半部分拟合结果
) {
	const auto& left = m_pContext->leftPoint();
	const auto& right = m_pContext->rightPoint();
	std::vector<PointAndAngle> _outRes;
	std::vector<cv::Point2f> tmp_pts;
	int dimension = std::max(m_pContext->ImageWidth(), m_pContext->ImageHeight());
	// left
	if (left_cnt == 0)
		left_cnt = (int)fpts_up.size() / 2;
	for (int i = 0; i < left_cnt; i++) {
		tmp_pts.push_back(fpts_up[i]);
	}
	ellipse_regression_Circle(tmp_pts, dimension, box_up);
	m_pContext->SetBox(0, box_up);

	std::vector<PointAndAngle> out = findIntersectionsCircleLine(
		box_up.center,
		box_up.size.width,
		left, right,
		1
	);
	for(const auto pt : out)
		_outRes.push_back(pt);
	
	tmp_pts.clear();
	// right
	for (int i = left_cnt; i < fpts_up.size(); i++) {
		tmp_pts.push_back(fpts_up[i]);
	}
	ellipse_regression_Circle(tmp_pts, dimension, box_up);
	m_pContext->SetBox(1, box_up);

	out = findIntersectionsCircleLine(
		box_up.center,
		box_up.size.width,
		left, right,
		2
	);
	for (auto pt : out)
		_outRes.push_back(pt);

	return _outRes;
}

int CNoPinAngleFitting::get_max_contour(
	std::vector<std::vector<cv::Point>>& _contours,
	std::vector<int>& _traces,
	bool findOnlyConvex
) {
	int maxIndex = -1;
	cv::Mat gray, edges;		

	find_contours(m_pContext->Image(), gray, edges, _contours);

	_traces.clear();
	if (_contours.empty()) {
		return -1;
	}
	auto left = m_pContext->leftPoint();
	auto right = m_pContext->rightPoint();
	maxIndex = find_contour_include_basepoints(_contours, _traces, left, right, findOnlyConvex);
	if (maxIndex < 0) {
		_traces.clear();
		maxIndex = find_max_contour(_contours, _traces);
	}
#if DEBUG_IMG
	// Draw each contour with a different color
	cv::Mat tmpmaxind = m_pContext->Image().clone();
	cv::Scalar color(255, 0, 0);
	cv::drawContours(tmpmaxind, _contours, maxIndex, color, 1);
	cv::drawMarker(tmpmaxind, m_pContext->leftPoint(), cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 10, 1);
	cv::drawMarker(tmpmaxind, m_pContext->rightPoint(), cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 10, 1);
	SaveDebugImg("max_contour", tmpmaxind);
#endif // _DEBUG
	if (maxIndex >= 0)
		_traces[maxIndex] = 1;
	return maxIndex;
}

int 
CNoPinAngleFitting::find_contour_points_shuipingmian(
	std::vector<cv::Point>& pts_up
) {
	int res = 0;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<int> traces;
	int max_ind = get_max_contour(contours, traces, true);
	if (max_ind < 0) {
		return res;
	}
	const int dis2 = (m_pContext->ImageWidth() / 40 + 4) * (m_pContext->ImageWidth() / 40 + 4);
	const int dis1 = (m_pContext->ImageWidth() / 100 + 1) * (m_pContext->ImageWidth() / 100 + 1);

	cv::Mat mask = cv::Mat::zeros(m_pContext->Image().size(), CV_8UC1);
	cv::drawContours(mask, contours, max_ind, cv::Scalar(255));

	// Step 2: Thinning
	zhangSuenThinningFast(mask);

	std::vector<cv::Point> left, right;
	auto leftBasePt = m_pContext->leftPoint();
	auto rightBasePt = m_pContext->rightPoint();
	bool extractcontours = traceTwoBranchesFromThinnedImage(mask, left, right, leftBasePt, rightBasePt, dis2, 1);
	if (!extractcontours)
		return 0;

	res = (int)left.size();
	std::reverse(left.begin(), left.end());
	pts_up.insert(pts_up.end(), left.begin(), left.end());
	pts_up.insert(pts_up.end(), right.begin(), right.end());

#if DEBUG_IMG
	cv::Mat tmp1;
	m_pContext->Image().copyTo(tmp1);
	std::vector<std::vector<cv::Point>> contours_draw;
	contours_draw.push_back(pts_up);
	if (contours_draw[0].size())
		cv::drawContours(tmp1, contours_draw, 0, cv::Scalar(0, 0, 255));
	cv::drawMarker(tmp1, m_pContext->leftPoint(), cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 10, 1);
	cv::drawMarker(tmp1, m_pContext->rightPoint(), cv::Scalar(0, 255, 0), cv::MARKER_CROSS, 10, 1);
	SaveDebugImg("found_basepoints", tmp1);
#endif
	return res;
}

// 处理轮廓，获取轮廓点：
int CNoPinAngleFitting::find_contour_points_aomian(
	std::vector<cv::Point>& pts_up,
	std::vector<cv::Point>& pts_dn
) {
	int res = 0;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<int> traces;
	int max_ind = get_max_contour(contours, traces);
	int n = 0;	
	if (max_ind < 0) {
		return res;
	}
	auto & maxContour = contours[max_ind];
	if (maxContour.size() < 7)
		return res;
	n = (int)maxContour.size();
	// check top start point
	auto leftPt = m_pContext->leftPoint();
	auto rightPt = m_pContext->rightPoint();
	int top_ind = check_contour_top(maxContour, leftPt, rightPt);
	if (top_ind > 0) {
		std::vector<cv::Point> new_contour;
		for (int i = 0; i < n; i++) {
			int ind = (i + top_ind) % n;
			new_contour.push_back(maxContour[ind]);
		}
		for (int i = 0; i < n; i++) {
			maxContour[i] = new_contour[i];
		}
	}
	else if (top_ind < 0) {
		top_ind = -top_ind;
		std::vector<cv::Point> new_contour;
		for (int i = 0; i < n; i++) {
			int ind = (i + top_ind) % n;
			new_contour.push_back(maxContour[ind]);
		}
		for (int i = 0; i < n; i++) {
			maxContour[i] = new_contour[i];
		}
		std::reverse(maxContour.begin(), maxContour.end());
	}

	cv::Rect rt = cv::boundingRect(maxContour);
	int ll = rt.x, rr = rt.x + rt.width;
	int min_dim = std::max(10, std::max(rt.width, rt.height) / 40);
	// 轮廓——左半部分
	int dis = POINT_DIS;

	// Fix 
	int i_up = n / 2;
	int x0 = maxContour[0].x;
	int y0 = maxContour[0].y;
	for (int i = 1; i < n; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x == x0 && pt.y == y0) {
			i_up = i / 2; // 避免重复扫描
			break;
		}
	}
	int dis_sq = dis * dis;
	bool passed_point = false;
	// 删除不必要的部分
	int margin_l = std::max((int)leftPt.x - BASEPOINT_MARGIN, m_pContext->ImageWidth() / 20);
	int margin_r = std::min((int)rightPt.x + BASEPOINT_MARGIN, m_pContext->ImageWidth() - m_pContext->ImageWidth() / 20);

	int counts = 0;

	cv::Point pt0 = leftPt;
	for (int i = 0; i < i_up; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x < margin_l) 
			break;
		if (near_to_point0(pt, leftPt, dis_sq)) {
			passed_point = true;
		}
		else if (passed_point) {
			pts_dn.push_back(pt);
			counts++;

			if (pt.x - ll < dis) 
				break;
			if (pt.x < pt0.x) 
				pt0 = pt;
		}
		else {
			pts_up.push_back(pt);
		}
	}
	while (counts < 100) { // 轮廓线断了
		int ind = find_next_contour(contours, min_dim, pt0, traces);
		if (ind < 0) 
			break;
		n = (int)contours[ind].size();
		i_up = n / 2;
		for (int i = 0; i < i_up; i++) {
			cv::Point pt = contours[ind][i];
			if (pt.x < margin_l)
				continue;
			if (pt.x > margin_r) 
				continue;
			if (pt.x > leftPt.x)
				continue;
			pts_dn.push_back(pt);
			counts++;
		}
	}

	res = (int)pts_up.size();
	// 轮廓——右半部分
	std::reverse(maxContour.begin(), maxContour.end());
	passed_point = false;
	n = (int)maxContour.size();
	i_up = n / 2;
	x0 = maxContour[0].x;
	y0 = maxContour[0].y;
	counts = 0;
	for (int i = 1; i < n; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x == x0 && pt.y == y0) {
			i_up = i / 2; // 避免重复扫描
			break;
		}
	}
	pt0 = rightPt;
	for (int i = 0; i < i_up; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x > margin_r) 
			break;
		if (near_to_point0(pt, rightPt, dis_sq)) {
			passed_point = true;
		}
		else if (passed_point) {
			pts_dn.push_back(pt);
			counts++;

			if (rr - pt.x < dis)
				break;
			if (pt.x > pt0.x)
				pt0 = pt;
		}
		else {
			pts_up.push_back(pt);

		}
	}
	while (counts < 100) { // 轮廓线断了
		int ind = find_next_contour(contours, min_dim, pt0, traces);
		if (ind < 0) 
			break;
		n = (int)contours[ind].size();
		i_up = n / 2;
		for (int i = 0; i < i_up; i++) {
			cv::Point pt = contours[ind][i];
			if (pt.x < margin_l) 
				continue;
			if (pt.x > margin_r)
				continue;
			if (pt.x < rightPt.x)
				continue;
			pts_dn.push_back(pt);
		}
	}
	return res;
}

// 处理轮廓，获取轮廓点：凸面
int CNoPinAngleFitting::find_contour_points_tumian(
	std::vector<cv::Point>& pts_up,
	std::vector<cv::Point>& pts_dn
) {
	int res = 0;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<int> trace;
	// 寻找面积最大的轮廓
	int max_ind = get_max_contour(contours, trace);
	if (max_ind < 0) {	
		return res;
	}
	auto & maxContour = contours[max_ind];
	int n = (int)maxContour.size();

	if (n < 7)
		return res;
	auto leftPt = m_pContext->leftPoint();
	auto rightPt = m_pContext->rightPoint();
	// check top start point
	int top_ind = check_contour_top(maxContour, leftPt, rightPt);
	if (top_ind > 0) {
		std::vector<cv::Point> new_contour;
		for (int i = 0; i < n; i++) {
			int ind = (i + top_ind) % n;
			new_contour.push_back(maxContour[ind]);
		}
		for (int i = 0; i < n; i++) {
			maxContour[i] = new_contour[i];
		}
	}
	else if (top_ind < 0) {
		top_ind = -top_ind;
		std::vector<cv::Point> new_contour;
		for (int i = 0; i < n; i++) {
			int ind = (i + top_ind) % n;
			new_contour.push_back(maxContour[ind]);
		}
		for (int i = 0; i < n; i++) {
			maxContour[i] = new_contour[i];
		}
		std::reverse(maxContour.begin(), maxContour.end());
	}

	cv::Rect rt = cv::boundingRect(maxContour);
	int ll = rt.x, rr = rt.x + rt.width;
	int min_dim = std::max(10, std::max(rt.width, rt.height) / 40);
	// 轮廓——左半部分
	int dis = POINT_DIS;

	// Fix 
	int i_up = n / 2;
	int x0 = maxContour[0].x;
	int y0 = maxContour[0].y;
	for (int i = 1; i < n; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x == x0 && pt.y == y0) {
			i_up = i / 2; // 避免重复扫描
			break;
		}
	}
	int dis_sq = dis * dis;
	bool passed_point = false;
	// 删除不必要的部分
	int margin_l = std::max((int)leftPt.x - BASEPOINT_MARGIN, m_pContext->ImageWidth() / 20);
	int margin_r = std::min((int)rightPt.x + BASEPOINT_MARGIN, m_pContext->ImageWidth() - m_pContext->ImageWidth() / 20);

	int counts = 0;

	cv::Point pt0 = leftPt;
	for (int i = 0; i < i_up; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x < margin_l && counts > MARGIN_COUNT)
			break;
		if (near_to_point0(pt, leftPt, dis_sq)) {
			passed_point = true;
		}
		else if (passed_point) {
			pts_dn.push_back(pt);
			counts++;

			if (pt.x - ll < dis)
				break;
			if (pt.x < pt0.x)
				pt0 = pt;
		}
		else {
			pts_up.push_back(pt);
		}
	}
	while (counts < 100) { // 轮廓线断了
		int ind = find_next_contour(contours, min_dim, pt0, trace);
		if (ind < 0)
			break;
		n = (int)contours[ind].size();
		i_up = n / 2;
		int max_x = pt0.x;
		for (int i = 0; i < i_up; i++) {
			cv::Point pt = contours[ind][i];
			if (pt.x < margin_l)
				break;
			if (pt.x > margin_r)
				break;
			if (pt.x < pt0.x)
				pt0 = pt;
			pts_dn.push_back(pt);
			counts++;
		}
	}

	res = (int)pts_up.size();
	// 轮廓——右半部分
	std::reverse(maxContour.begin(), maxContour.end());
	passed_point = false;
	n = (int)maxContour.size();
	i_up = n / 2;
	x0 = maxContour[0].x;
	y0 = maxContour[0].y;
	counts = 0;
	for (int i = 1; i < n; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x == x0 && pt.y == y0) {
			i_up = i / 2; // 避免重复扫描
			break;
		}
	}
	pt0 = rightPt;
	for (int i = 0; i < i_up; i++) {
		cv::Point pt = maxContour[i];
		if (pt.x > margin_r && counts > MARGIN_COUNT)
			break;
		if (near_to_point0(pt, rightPt, dis_sq)) {
			passed_point = true;
		}
		else if (passed_point) {
			pts_dn.push_back(pt);
			counts++;
			if (rr - pt.x < dis)
				break;
			if (pt.x > pt0.x)
				pt0 = pt;
		}
		else {
			pts_up.push_back(pt);
		}
	}
	while (counts < 100) { // 轮廓线断了
		int ind = find_next_contour(contours, min_dim, pt0, trace); 
		if (ind < 0)
			break;
		n = (int)contours[ind].size();
		i_up = n / 2;
		for (int i = 0; i < i_up; i++) {
			cv::Point pt = contours[ind][i];
			if (pt.x < margin_l)
				break;
			if (pt.x > margin_r)
				break;
			if (pt.x > pt0.x)
				pt0 = pt;
			pts_dn.push_back(pt);
		}
	}

	return res;
}

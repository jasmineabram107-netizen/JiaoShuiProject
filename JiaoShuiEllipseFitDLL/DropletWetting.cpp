#include "common.h"
#include "DropletWetting.h"
#include "fitting\ManualFitting.h"
#include "fitting\internalproc.h"
#include "fitting\PinAngleFitting.h"
#include "fitting\NoPinAngleFitting.h"
#include "utils\internal.h"

json makeBoxJson(const cv::RotatedRect& box)
{
	return json{
		{"cx", box.center.x},
		{"cy", box.center.y},
		{"angle", box.angle},
		{"width", box.size.width},
		{"height", box.size.height}
	};
}

json makeContourJson(const std::vector<cv::Point>& points) 
{
	json contour = json::array();
	for (const auto& pt : points) {
		contour.push_back({ {"x", pt.x}, {"y", pt.y} });
	}
	return contour;
};

json makeContourJson(const std::vector<cv::Point2f>& points)
{
	json contour = json::array();
	for (const auto& pt : points) {
		contour.push_back({ {"x", pt.x}, {"y", pt.y} });
	}
	return contour;
};

json makeSideJson(const PointAndAngle& angle) 
{
	return json{
		{JSNAME_RESANGLES, angle.angle},
		{JSNAME_RESPOINTS, { {"x", angle.point.x}, {"y", angle.point.y} }},
		{JSNAME_RESVEC1, { {"x", angle.vec1.x}, {"y", angle.vec1.y} }},
		{JSNAME_RESVEC2, { {"x", angle.vec2.x}, {"y", angle.vec2.y} }},
	};
}

CDropletWetting::CDropletWetting()
	: m_bInited(false)
	, m_pBaseShape(NULL)
	, m_pBasePoints(NULL)
	, m_pDetector(NULL)
	, m_errorCode(errNo)
	, m_Context()
{
	initVariables(true);
}

CDropletWetting::~CDropletWetting()
{
	if (m_pBaseShape) {
		delete m_pBaseShape;
		m_pBaseShape = NULL;
	}
	if (m_pBasePoints) {
		delete m_pBasePoints;
		m_pBasePoints = NULL;
	}
	if (m_pDetector) {
		delete m_pDetector;
		m_pDetector = NULL;
	}
	m_jsInput.clear();
	m_jsOutput.clear();	
}

bool CDropletWetting::SetImage(const TCHAR* imagePath)
{
	if (!m_bInited) {
		m_bInited = InitModel(imagePath);
	}
	if (m_bInited) {
		initVariables(true);
		if (!m_Context.LoadImage(imagePath)) {
			errorReport(errFailOpen);
			return false;
		}		
#if UNICODE
		auto imgpath = wchar_to_utf8(imagePath);
#else
		auto imgpath = imagePath;
#endif
		m_jsInput[JSNAME_IMAGEPATH] = imgpath;
	}
	else {
		errorReport(errInitModel);
	}
	return true;
}

bool CDropletWetting::InitModel(const TCHAR* appPath)
{
	if (m_bInited) {
		return true;
	}
	m_pBaseShape = new CBaselineShape();
	m_pBasePoints = new CBasePoints();
	m_pDetector = new CDetBasePtOnHorizon();
	if (!m_pBaseShape->InitModel(appPath)) {
		errorReport(errInitModel1);
		return false;
	}
	if (!m_pBasePoints->InitModel(appPath)) {
		errorReport(errInitModel2);
		return false;
	}
	if (!m_pDetector->InitModel(appPath)) {
		errorReport(errInitModel3);
		return false;
	}
	m_bInited = true;
	return true;
}

void CDropletWetting::initVariables(bool initImage)
{	
	if (initImage) {
		m_Context.InitVariables();
		m_jsInput.clear();
		m_jsOutput.clear();
	}
	m_errorCode = errNo;

	m_dropletPtsByManual.clear();
	m_basePtsByManual.clear();
}

errCode CDropletWetting::prepareEngineVariables()
{
	initVariables();
	if (!m_bInited) {
		errorReport(errInitModel);
		return errException;
	}
	if (m_Context.IsEmptyImage()) {
		errorReport(errFailOpen);
		return errFailOpen;
	}
	if (!foo3381_check()) {
		errorReport(errException);
		return errException;
	}
	return errNo;
}

void CDropletWetting::errorReport(ErrorCode errCode)
{
	m_jsOutput[JSNAME_RESCODE] = errCode;
	m_errorCode = errCode;
}

BaseLineShapeKind
CDropletWetting::GetBaselineShape()
{
	if (!m_bInited) {
		errorReport(errInitModel);
		return eBS_Unknown;
	}
	if (m_Context.IsEmptyImage()) {
		errorReport(errFailOpen);
		return eBS_Unknown;
	}
	if (!foo3381_check()) {
		errorReport(errException);
		return eBS_Unknown;
	}
	auto res = m_pBaseShape->getShape(m_Context.Image());
	if (res < 0 || res >= eBS_Count) {
		errorReport(errFailedDetectShape);
		return eBS_Unknown;
	}
	else {
		m_Context.SetBaselineShape(res);
		m_jsOutput[JSNAME_BSLINESHAPE] = res;
	}
	return res;
}


#if _DEV_OLDSTYLE_API

errCode 
CDropletWetting::RecognizeDropletAngleByAuto(
	DropletFitMode dropletFitMode, 
	BaseLineFitMode baselineFitMode, 
	MainShapeType mainShapeType,
	bool isSubpixel,
	bool rotated,
	HorizonMode horMode,
	bool hasPin
) {
	ErrorCode err = prepareEngineVariables();
	if(err != errNo)
		return err;

	clearFittingOutputJson();
	m_jsInput[JSNAME_FITMODE] = eFitAuto;
	m_jsInput[JSNAME_MAINSHAPE] = mainShapeType;
	m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
	m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;
	m_jsInput[JSNAME_ROTATED] = rotated;	
	m_jsInput[JSNAME_HORIZONMODE] = horMode;
	m_jsInput[JSNAME_HASPIN] = hasPin;

	m_Context.SetModes(dropletFitMode, baselineFitMode, isSubpixel, eFitAuto);
	
	if (mainShapeType == eShapeUnknown) {
		if (m_Context.baselineShape() == eBS_Unknown) {
			GetBaselineShape();
			if (m_Context.baselineShape() == eBS_Unknown) {
				errorReport(errFailedDetectShape);
				return errFailedDetectShape;
			}
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
		hasPin = m_Context.HasPin();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
		m_Context.SetPin(hasPin);
		m_Context.SetHorizontalMode(horMode);
	}	
	
	if (mainShapeType == eShapeHorizontal) {
		// Recognition for horizontal shape
		bool hasPin0 = m_Context.HasPin();
		std::vector<cv::Point2f> basePts;
		HorizonMode horMode0 = horMode;

		auto res = m_pDetector->DetectBasePts(m_Context.Image(), horMode0, hasPin0, basePts, rotated);
		m_Context.SetPin(hasPin0);

		if(horMode != horMode0)
			m_Context.SetHorizontalMode(horMode0);

		if (res && basePts.size() >= 2) {
			RectangleF _newBaserect;
			if (isSubpixel) {
				_newBaserect = { basePts[0].x, basePts[0].y,
							   basePts[1].x, basePts[1].y };
				m_pBasePoints->updateBasePoints(m_Context.Image(), m_Context.mainShape(), &_newBaserect, true);
			}
			else {
				_newBaserect = RectangleF{ (double)basePts[0].x, (double)basePts[0].y,
								  (double)basePts[1].x, (double)basePts[1].y };
			}
			m_Context.UpdateBaserect(_newBaserect);
			m_errorCode = errNo;
		}
		else {
			errorReport(errFailedDetectBasePoint);
			return errFailedDetectBasePoint;
		}
	}
	else {
		// Non-horizontal shape: get base points directly
		RectangleF rect;
		auto result = m_pBasePoints->getBaselinePointsByPath(m_Context.Image(), m_Context.mainShape(), &rect);
		if (result) {
			RectangleF _newBaserect = m_Context.IsSubpixel() ? rect : RectangleF{
				(double)(int)rect.left, (double)(int)rect.top,
				(double)(int)rect.right, (double)(int)rect.bottom
			};
			m_Context.UpdateBaserect(_newBaserect);
			m_errorCode = errNo;
		}
		else {
			errorReport(errFailedDetectBasePoint);
			return errFailedDetectBasePoint;
		}
	}
	if (m_errorCode == errNo) {
		auto calcRes = calculateDropletAngle();
		if (!calcRes) {
			if(m_errorCode == errNo)
				errorReport(errFailedDetectAngle);
		}
	}
	
	return m_errorCode;
}

ErrorCode
CDropletWetting::RecognizeDropletAngleByBaseline(
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	MainShapeType mainShapeType,
	bool isSubpixel,
	const std::vector<cv::Point2f>& linePts
) {
	ErrorCode err = prepareEngineVariables();
	if (err != errNo)
		return err;
	if (linePts.size() != 2) {
		errorReport(errInvalidBasePoints);
		return errInvalidBasePoints;
	}
	// Set input JSON
	clearFittingOutputJson();
	m_jsInput[JSNAME_FITMODE] = eFitByBasePoint;
	m_jsInput[JSNAME_MAINSHAPE] = mainShapeType;
	m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
	m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;
	m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(linePts);

	m_Context.SetModes(dropletFitMode, baselineFitMode, isSubpixel, eFitAuto);

	if (mainShapeType == eShapeUnknown) {
		GetBaselineShape();
		if (m_Context.baselineShape() == eBS_Unknown) {
			errorReport(errFailedDetectShape);
			return errFailedDetectShape;
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
	}

	RectangleF tmpRect(linePts[0].x, linePts[0].y, linePts[1].x, linePts[1].y);
	bool resGetPts = getBasePointsFromLine(linePts[0], linePts[1], tmpRect);
	if (!m_Context.IsSubpixel()) {
		tmpRect = RectangleF{
			(double)(int)tmpRect.left, (double)(int)tmpRect.top,
			(double)(int)tmpRect.right, (double)(int)tmpRect.bottom
		};
	}
	m_Context.UpdateBaserect(tmpRect);

	return calculateDropletAngle() ? errNo : errFailedDetectAngle;
}

ErrorCode
CDropletWetting::RecognizeDropletAngleByBasePoints(
	FitMode fitMode,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	MainShapeType mainShapeType,
	bool isSubpixel,
	const std::vector<cv::Point2f>& basePts
) {
	ErrorCode err = prepareEngineVariables();
	if (err != errNo)
		return err;
	if (basePts.size() < 2) {
		errorReport(errInvalidBasePoints);
		return errInvalidBasePoints;
	}
	// Set input JSON
	clearFittingOutputJson();
	m_jsInput[JSNAME_FITMODE] = fitMode;
	m_jsInput[JSNAME_MAINSHAPE] = mainShapeType;
	m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
	m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;
	m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(basePts);

	m_Context.SetModes(dropletFitMode, baselineFitMode, isSubpixel, fitMode);

	if (mainShapeType == eShapeUnknown) {
		GetBaselineShape();
		if (m_Context.baselineShape() == eBS_Unknown) {
			errorReport(errFailedDetectShape);
			return errFailedDetectShape;
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
	}

	RectangleF _newRect;
	if (isSubpixel || mainShapeType == eShapeHorizontal) {
		_newRect = { basePts[0].x, basePts[0].y,
					   basePts[1].x, basePts[1].y };
		m_pBasePoints->updateBasePoints(m_Context.Image(), m_Context.mainShape(), &_newRect, true);
	}
	else {
		_newRect = RectangleF{ (double)basePts[0].x, (double)basePts[0].y,
						  (double)basePts[1].x, (double)basePts[1].y };
	}
	m_Context.UpdateBaserect(_newRect);

	return calculateDropletAngle() ? errNo : errFailedDetectAngle;
}

ErrorCode
CDropletWetting::RecognizeDropletAngleByBaseManual(
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	FitMode fitMode,
	MainShapeType mainShapeType,
	bool isSubpixel,
	const std::vector<cv::Point2f>& dropletPts,
	const std::vector<cv::Point2f>& baselinePts
) {
	ErrorCode err = prepareEngineVariables();
	if (err != errNo)
		return err;
	if (fitMode <= eFitByLine || fitMode > eFitManual) {
		errorReport(errInvalidFitMode);
		return errInvalidFitMode;
	}

	m_Context.SetModes(dropletFitMode, baselineFitMode, isSubpixel, fitMode);

	// Json input
	clearFittingOutputJson();
	m_jsInput[JSNAME_FITMODE] = fitMode;
	m_jsInput[JSNAME_MAINSHAPE] = mainShapeType;
	m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
	m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;
	if (!baselinePts.empty())
		m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(baselinePts);
	if (!dropletPts.empty())
		m_jsInput[JSNAME_MANUALDROPLETPTS] = makeContourJson(dropletPts);

	// Set main shape type and check if it is empty
	if (mainShapeType == eShapeUnknown) {
		GetBaselineShape();
		if (m_Context.baselineShape() == eBS_Unknown) {
			errorReport(errFailedDetectShape);
			return errFailedDetectShape;
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
	}

	// Set base points and droplet points
	m_dropletPtsByManual.clear();
	m_basePtsByManual.clear();

	if (m_Context.DropletMode() == eDropletWidthHeight) {
		for (int i = 0; i < baselinePts.size(); i++) {
			m_basePtsByManual.push_back(baselinePts[i]);
		}
		if (dropletPts.size() < 2) {
			errorReport(errInvalidManualPointCount);
			return errInvalidManualPointCount;
		}
		for (int i = 0; i < 2; i++) {
			m_dropletPtsByManual.push_back(dropletPts[i]);
		}
	}
	else {
		int neededBasePts = (baselineFitMode > eBaseLineCircle ? 5 : 3);
		if (m_Context.mainShape() == eShapeHorizontal) {
			neededBasePts = 2;
		}
		if (baselinePts.size() < neededBasePts) {
			errorReport(errInvalidManualPointCount);
			return errInvalidManualPointCount;
		}

		neededBasePts = (dropletFitMode > eDropletCircle ? 5 : 3);
		if (m_Context.fitMode() == eFitManual && dropletPts.size() < neededBasePts) {
			errorReport(errInvalidManualPointCount);
			return errInvalidManualPointCount;
		}
		for (int i = 0; i < baselinePts.size(); i++) {
			m_basePtsByManual.push_back(baselinePts[i]);
		}
		for (int i = 0; i < dropletPts.size(); i++) {
			m_dropletPtsByManual.push_back(dropletPts[i]);
		}
	}
	if (m_Context.fitMode() == eFitSemiAuto) {
		RectangleF _baseRect;
		auto result = m_pBasePoints->getBaselinePointsByPath(m_Context.Image(), mainShapeType, &_baseRect);
		if (result) {
			m_pBasePoints->updateBasePoints(m_Context.Image(), m_Context.mainShape(), &_baseRect, false);
			m_Context.UpdateBaserect(_baseRect);
		}
	}
	return calculateDropletAngle() ? errNo : errFailedDetectAngle;
}

bool CDropletWetting::calculateDropletAngle()
{
	double out_anglesself[2] = { 0, 0 };
	std::vector<PointAndAngle> res;
	cv::RotatedRect boxes[2];
	std::vector<cv::Point> pts_up, pts_dn;
	bool result = false;
	std::vector<cv::Point2f> corners;
	// detect_corner_points(m_frame, corners);

	ErrorCode resCode = errNo;
	if (m_Context.fitMode() == eFitAuto) {
		if (m_Context.HasPin()) {
			CPinAngleFitting pinAngleFitting(&m_Context);
			pinAngleFitting.setParameters();
			resCode = pinAngleFitting.process();
		}
		else {  // auto & no-pin
			CNoPinAngleFitting dropletAnalyzer(&m_Context);
			double* baseline_x = NULL;
			double* baseline_y = NULL;
			readyManulBasePts(&baseline_x, &baseline_y);
			dropletAnalyzer.setParameters(baseline_x, baseline_y);
			resCode = dropletAnalyzer.process();
			if (baseline_x) delete[] baseline_x;
			if (baseline_y) delete[] baseline_y;
		}
	}
	else if (m_Context.fitMode() == eFitManual) {
		CManualFitting manualFitting;
		double* baseline_x = NULL;
		double* baseline_y = NULL;
		double* yedi_x = NULL;
		double* yedi_y = NULL;
		readyManualDropletPts(&yedi_x, &yedi_y);
		readyManulBasePts(&baseline_x, &baseline_y, true);
		manualFitting.setParam(
			m_Context.Image(),
			m_Context.mainShape(),
			m_Context.DropletMode(),
			m_Context.BaselineMode(),
			baseline_x,
			baseline_y,
			yedi_x,
			yedi_y
		);
		resCode = manualFitting.Process(out_anglesself, res);
		if (baseline_x) delete[] baseline_x;
		if (baseline_y) delete[] baseline_y;
		if (yedi_x) delete[] yedi_x;
		if (yedi_y) delete[] yedi_y;
		if (resCode == errNo) {
			m_Context.SetAnglePoints(res);
			m_Context.SetAngles(out_anglesself);
			result = true;
		}
	}
	else { // semi-auto & by point
		CNoPinAngleFitting dropletAnalyzer(&m_Context);
		double* baseline_x = NULL;
		double* baseline_y = NULL;
		readyManulBasePts(&baseline_x, &baseline_y);
		dropletAnalyzer.setParameters(baseline_x, baseline_y);
		resCode = dropletAnalyzer.process();
		if (baseline_x) delete[] baseline_x;
		if (baseline_y) delete[] baseline_y;
	}
	if (resCode == errNo || resCode == errFailOnlyOne) {
		result = true;
	}
	else {
		errorReport(resCode);
	}
	if (result) {
		auto _angleDetails = m_Context.GetAnglePoints();
		auto _angles = m_Context.GetAngles();
		auto _outBoxes = m_Context.GetBoxes();

		int left_index = 0, right_index = 1;

		m_jsOutput[JSNAME_RESCODE] = errNo;
		if (_angleDetails.size() >= left_index)
			m_jsOutput[JSNAME_RESULTLEFT] = makeSideJson(_angleDetails[left_index]);
		if (_angleDetails.size() >= right_index) {
			m_jsOutput[JSNAME_RESULTRIGHT] = makeSideJson(_angleDetails[right_index]);
		}
		m_jsOutput[JSNAME_BASELINEBOX] = m_Context.baselineShape();
		m_jsOutput[JSNAME_DROPBOX] = makeBoxJson(_outBoxes[0]);
		if (m_Context.mainShape() != eShapeHorizontal &&
			_outBoxes[1].size.width > 0 &&
			_outBoxes[1].size.height > 0
			)
			m_jsOutput[JSNAME_BASELINEBOX] = makeBoxJson(_outBoxes[1]);

		m_jsOutput[JSNAME_DROPCONTOUR] = makeContourJson(m_Context.DropletPtRef());
		m_jsOutput[JSNAME_BASELINECONTOUR] = makeContourJson(m_Context.BaelinePtRef());
	}
	return result;
}


#endif//_DEV_OLDSTYLE_API
bool
CDropletWetting::GetResult(
	double* _outAngle,
	PointAndAngle* _outAngleDetail
) {
	bool res = false;
	if (m_errorCode != errNo) {
		return res;
	}
	auto _outAngles = m_Context.GetAngles();
	auto _outAnglesDetail = m_Context.GetAnglePoints();
	if (_outAngles[0] == 0 && _outAngles[1] == 0) {
		return res;
	}
	int cnt = 0;
	if (_outAngle) {
		_outAngle[0] = _outAngles[0];
		_outAngle[1] = _outAngles[1];
		res = true;
	}
	if (_outAngleDetail && _outAnglesDetail.size() > 1) {
		_outAngleDetail[0] = _outAnglesDetail[0];
		_outAngleDetail[1] = _outAnglesDetail[1];
		res = true;
	}

	return res;
}

bool CDropletWetting::GetMidResult(
	MainShapeType& _mainShape,
	bool& _hasPin,
	bool& _isEmpty,
	HorizonMode& _horizonMode,
	RectangleF& _basePts,
	std::vector<cv::RotatedRect>& _outBox)
{
	if (m_errorCode != errNo || m_Context.fitMode() == eFitManual) {
		return false;
	}
	_mainShape = m_Context.mainShape();
	_hasPin = m_Context.HasPin();
	_isEmpty = m_Context.IsEmptyDroplet();
	_basePts = m_Context.baseRect();
	_horizonMode = m_Context.GetHorizontalMode();
	_outBox.clear();
	auto _outBoxes = m_Context.GetBoxes();
	for (int i = 0; i < 3; i++) {
		if (_outBoxes[i].size.width > 0 && _outBoxes[i].size.height > 0) {
			_outBox.push_back(_outBoxes[i]);
		}
	}

	return true;
}

bool
CDropletWetting::getBasePointsFromLine(
	const cv::Point2f& leftPt, 
	const cv::Point2f& rightPt, 
	RectangleF& _outRect
) {
	if(m_Context.IsEmptyImage()) {
		errorReport(errFailOpen);
		return false;
	}
	auto left = leftPt;
	auto right = rightPt;
	if (right.x < left.x) {
		std::swap(left, right);
	}

	_outRect = { left.x, left.y, right.x, right.y };

	cv::Mat grey;
	auto& frame = m_Context.Image();
	cv::cvtColor(frame, grey, cv::COLOR_BGR2GRAY);

	int padding = 10;
	int x0 = (int)fmax(0, left.x - padding);
	int x1 = (int)fmin(frame.cols, right.x + padding);
	int y0 = (int)fmax(0, fmin(left.y, right.y) - padding);
	int y1 = (int)fmin(frame.rows, fmax(left.y, right.y) + padding);

	cv::Mat roi = grey(cv::Rect(x0, y0, x1 - x0, y1 - y0));
	int offset_x = x0, offset_y = y0;

	switch (m_Context.mainShape()) {
	case eShapeConvex:
		return getIntersections(roi, left, right, offset_x, offset_y, &_outRect);
	case eShapeConcave:
		return find_corners(roi, left, right, offset_x, offset_y, &_outRect);
	default:
		if (getIntersections(roi, left, right, offset_x, offset_y, &_outRect)) {
			if ((left.x == _outRect.left && left.y == _outRect.top) ||
				(right.x == _outRect.right && right.y == _outRect.bottom))
				return find_corners(roi, left, right, offset_x, offset_y, &_outRect);
			return true;
		}
		return false;
	}
	return true;
}


bool 
CDropletWetting::GetContourPointsCount(
	int& _outDropletCount, 
	int& _outBaselineCount
) {
	if (m_errorCode != errNo || m_Context.fitMode() == eFitManual) {
		return false;
	}
	_outDropletCount = (int)m_Context.DropletPtRef().size();
	_outBaselineCount = (int)m_Context.BaelinePtRef().size();

	return true;
}

bool CDropletWetting::GetContourPoints(
	std::vector<cv::Point>& _outDropletPts, 
	std::vector<cv::Point>& _outBaselinePts
){
	if (m_errorCode != errNo || m_Context.fitMode() == eFitManual) {
		return false;
	}
	
	_outDropletPts = m_Context.DropletPtRef();
	_outBaselinePts = m_Context.BaelinePtRef();

	return true;
}

jstring CDropletWetting::GetResultByJsonFormat()
{
	jstring res;
	json j;
	j[JSNAME_INPUT] = m_jsInput;
	j[JSNAME_OUTPUT] = m_jsOutput;

	std::string json_str = j.dump(2);
#ifdef UNICODE
	// Convert UTF-8 JSON string to Unicode wchar_t*
	res = utf8_to_wchar(json_str);
#else
	res = json_str;
#endif
	return res;
}

bool CDropletWetting::getHorizontalMode(HorizonMode& _outMode, bool& _outHasPin) 
{
	bool res = false;
	
	clearHorizontalModeJson();

	if (m_bInited) {
		if(m_Context.IsEmptyImage()) {
			errorReport(errFailOpen);
			return res;
		}
		res = m_pDetector->DetectHorizonMode(m_Context.Image(), _outMode, _outHasPin);
		if (res) {
			m_jsOutput[JSNAME_HORIZONMODE] = _outMode;
			m_jsOutput[JSNAME_HASPIN] = _outHasPin;
		}
	} 
	else {
		errorReport(errInitModel);
	}
	
	return res;
}

ErrorCode
CDropletWetting::getBasepointsByAuto(
	bool isHorizontal,
	bool &hasPin,
	bool &rotated,
	HorizonMode &horMode,
	bool isSubpixel,
	bool isBoundSupplement,
	RectangleF &rectBasePts, 
	PointF* keyPts
) {
	clearBasePointJson();
	m_jsInput[JSNAME_FITMODE] = eFitAuto;
	m_jsInput[JSNAME_HASPIN] = hasPin;
	m_jsInput[JSNAME_ROTATED] = rotated;
	m_jsInput[JSNAME_HORIZONMODE] = horMode;
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;

	ErrorCode res_code = prepareEngineVariables();

	if(res_code != errNo)
		return res_code;

	MainShapeType mainShapeType = m_Context.mainShape();

	m_Context.SetModes(isSubpixel, eFitAuto);
	if (mainShapeType == eShapeUnknown) {
		if (m_Context.baselineShape() == eBS_Unknown) {
			GetBaselineShape();
			if (m_Context.baselineShape() == eBS_Unknown) {
				errorReport(errFailedDetectShape);
				return errFailedDetectShape;
			}
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
		hasPin = m_Context.HasPin();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
		m_Context.SetPin(hasPin);
		m_Context.SetHorizontalMode(horMode);
	}

	if ((isHorizontal && mainShapeType != eShapeHorizontal)
		|| (!isHorizontal && mainShapeType == eShapeHorizontal)) {
		errorReport(errFailedDetectBasePoint);
		return errFailedDetectBasePoint;
	}

	RectangleF _newRect;

	if (mainShapeType == eShapeHorizontal) {
		// Recognition for horizontal shape
		bool hasPin0 = m_Context.HasPin();
		std::vector<cv::Point2f> basePts;
		HorizonMode horMode0 = horMode;

		auto res = m_pDetector->DetectBasePts(m_Context.Image(), horMode0, hasPin0, basePts, rotated, isBoundSupplement);
		m_Context.SetPin(hasPin0);		
		if (horMode != horMode0)
			m_Context.SetHorizontalMode(horMode0);
		
		if (res && basePts.size() >= 2) {			
			if (isSubpixel) {
				_newRect = { basePts[0].x, basePts[0].y,
							   basePts[1].x, basePts[1].y };
				m_pBasePoints->updateBasePoints(m_Context.Image(), mainShapeType, &_newRect, true);
			}
			else {
				_newRect = RectangleF{ (double)basePts[0].x, (double)basePts[0].y,
								  (double)basePts[1].x, (double)basePts[1].y };
			}
			m_Context.UpdateBaserect(_newRect);
			if (keyPts) {
				m_pDetector->GetKeyPoint(keyPts);
			}
			m_errorCode = errNo;
		}

		else {
			errorReport(errFailedDetectBasePoint);
			return errFailedDetectBasePoint;
		}
	}
	else {
		RectangleF rect;
		auto result = m_pBasePoints->getBaselinePointsByPath(m_Context.Image(), mainShapeType, &rect);
		if (result) {
			_newRect = isSubpixel ? rect : RectangleF{
				(double)(int)rect.left, (double)(int)rect.top,
				(double)(int)rect.right, (double)(int)rect.bottom
			};
			m_Context.UpdateBaserect(rect);
			m_errorCode = errNo;
		}
		else {
			errorReport(errFailedDetectBasePoint);
			return errFailedDetectBasePoint;
		}
	}

	rectBasePts = _newRect;
	horMode = m_Context.GetHorizontalMode();
	hasPin = m_Context.HasPin();

	m_jsOutput[JSNAME_HORIZONMODE] = horMode;
	m_jsOutput[JSNAME_ROTATED] = rotated;
	m_jsOutput[JSNAME_HASPIN] = hasPin;
	m_jsOutput[JSNAME_BASEPTS] = {
		{"x1", rectBasePts.left}, {"y1", rectBasePts.top},
		{"x2", rectBasePts.right}, {"y2", rectBasePts.bottom}
	};
	if (keyPts) {
		json keyPtArray = json::array();
		for (int i = 0; i < 3; i++) {
			keyPtArray.push_back({ {"x", keyPts[i].x}, {"y", keyPts[i].y}});
		}
		m_jsOutput[JSNAME_KEYPTS] = keyPtArray;
	}

	return m_errorCode;
}

ErrorCode
CDropletWetting::getBasepointsByBaseline(
	bool isHorizontal,
	bool isSubpixel,
	const std::vector<cv::Point2f>& linePts,
	RectangleF& rectBasePts
) {
	clearBasePointJson();
	m_jsInput[JSNAME_FITMODE] = eFitByLine;
	m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(linePts);
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;

	auto res_code = prepareEngineVariables();
	if(res_code != errNo)
		return res_code;
	
	if (linePts.size() != 2) {
		errorReport(errInvalidBasePoints);
		return errInvalidBasePoints;
	}

	MainShapeType mainShapeType = m_Context.mainShape();

	m_Context.SetModes(isSubpixel, eFitAuto);

	if (mainShapeType == eShapeUnknown) {
		if (m_Context.baselineShape() == eBS_Unknown) {
			GetBaselineShape();
			if (m_Context.baselineShape() == eBS_Unknown) {
				errorReport(errFailedDetectShape);
				return errFailedDetectShape;
			}
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
	}

	if ((isHorizontal && mainShapeType != eShapeHorizontal)
		|| (!isHorizontal && mainShapeType == eShapeHorizontal)) {
		errorReport(errFailedDetectBasePoint);
		return errFailedDetectBasePoint;
	}

	RectangleF tmpRect;
	bool resGetPts = getBasePointsFromLine(linePts[0], linePts[1], tmpRect);
	if (resGetPts) {
		RectangleF _newRect = isSubpixel ? tmpRect : RectangleF{
				(double)(int)tmpRect.left, (double)(int)tmpRect.top,
				(double)(int)tmpRect.right, (double)(int)tmpRect.bottom
		};
		m_Context.UpdateBaserect(_newRect);
		rectBasePts = _newRect;		
	}
	else {
		errorReport(errFailedDetectBasePoint);
		return errFailedDetectBasePoint;
	}

	m_jsOutput[JSNAME_BASEPTS] = {
		{"x1", rectBasePts.left}, {"y1", rectBasePts.top},
		{"x2", rectBasePts.right}, {"y2", rectBasePts.bottom}
	};

	return errNo;
}

ErrorCode 
CDropletWetting::getBasepointsByBasepoint(
	bool isHorizontal,
	bool isSubpixel,
	const std::vector<cv::Point2f>& basePts,
	RectangleF& rectBasePts
) {
	clearBasePointJson();
	m_jsInput[JSNAME_FITMODE] = eFitByBasePoint;
	m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(basePts);
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;

	auto resCode = prepareEngineVariables();
	if (resCode != errNo)
		return resCode;

	if (basePts.size() < 2) {
		errorReport(errInvalidBasePoints);
		return errInvalidBasePoints;
	}
	MainShapeType mainShapeType = m_Context.mainShape();

	m_Context.SetModes(isSubpixel, eFitAuto);

	if (mainShapeType == eShapeUnknown) {
		if (m_Context.baselineShape() == eBS_Unknown) {
			GetBaselineShape();
			if (m_Context.baselineShape() == eBS_Unknown) {
				errorReport(errFailedDetectShape);
				return errFailedDetectShape;
			}
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
	}

	if ((isHorizontal && mainShapeType != eShapeHorizontal)
		|| (!isHorizontal && mainShapeType == eShapeHorizontal)) {
		errorReport(errFailedDetectBasePoint);
		return errFailedDetectBasePoint;
	}

	bool res = false;
	RectangleF _newRect;
	if (isSubpixel || mainShapeType == eShapeHorizontal) {
		_newRect = { basePts[0].x, basePts[0].y,
					   basePts[1].x, basePts[1].y };
		res = m_pBasePoints->updateBasePoints(m_Context.Image(), mainShapeType, &_newRect, true);
	}
	else {
		_newRect = RectangleF{ (double)basePts[0].x, (double)basePts[0].y,
						  (double)basePts[1].x, (double)basePts[1].y };
		res = true;
	}
	if (res) {
		rectBasePts = _newRect;
		m_Context.UpdateBaserect(_newRect);
	}
	else {
		errorReport(errFailedDetectBasePoint);
		return errFailedDetectBasePoint;
	}
	
	m_jsOutput[JSNAME_BASEPTS] = {
		{"x1", rectBasePts.left}, {"y1", rectBasePts.top},
		{"x2", rectBasePts.right}, {"y2", rectBasePts.bottom}
	};

	return errNo;
}

ErrorCode 
CDropletWetting::getBasepointsBySemiauto(
	MainShapeType mainShapeType,
	bool isSubpixel,
	const std::vector<cv::Point2f>& baselinePts,
	RectangleF& rectBasePts
) {
	clearBasePointJson();
	m_jsInput[JSNAME_FITMODE] = eFitSemiAuto;
	m_jsInput[JSNAME_MAINSHAPE] = mainShapeType;
	m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(baselinePts);
	m_jsInput[JSNAME_USESUBPIXEL] = isSubpixel;

	auto resCode = prepareEngineVariables();
	if(resCode != errNo)
		return resCode;

	m_Context.SetModes(isSubpixel, eFitSemiAuto);

	// Set main shape type and check if it is empty
	if (mainShapeType == eShapeUnknown) {
		if (m_Context.baselineShape() == eBS_Unknown) {
			GetBaselineShape();
			if (m_Context.baselineShape() == eBS_Unknown) {
				errorReport(errFailedDetectShape);
				return errFailedDetectShape;
			}
		}
		m_Context.BuildShapeTypeFromDetailedShape();
		mainShapeType = m_Context.mainShape();
	}
	else {
		m_Context.SetMainShape(mainShapeType);
	}

	RectangleF _newRect;
	bool res = false;
	auto result = m_pBasePoints->getBaselinePointsByPath(m_Context.Image(), mainShapeType, &_newRect);
	if (result) {
		res = m_pBasePoints->updateBasePoints(m_Context.Image(), mainShapeType, &_newRect, false);
	}
	if(res) {
		m_Context.UpdateBaserect(_newRect);
		rectBasePts = _newRect;
	}
	else {
		errorReport(errFailedDetectBasePoint);
		return errFailedDetectBasePoint;
	}

	m_jsOutput[JSNAME_BASEPTS] = {
		{"x1", rectBasePts.left}, {"y1", rectBasePts.top},
		{"x2", rectBasePts.right}, {"y2", rectBasePts.bottom}
	};
	
	return errNo;
}

FittingGraphicsType CDropletWetting::getFittingGraphicsType(DropletFitMode dropletFitMode) const 
{
	FittingGraphicsType graphicsType = eFgNone;
	if (dropletFitMode == eDropletWidthHeight)
		graphicsType = eFgRectangle;
	else if (dropletFitMode >= eDropletCircle && dropletFitMode <= eDropletEllipseDirect)
		graphicsType = eFgOneEllipse;
	else if (dropletFitMode == eDropletDoubleCircle || dropletFitMode == eDropletDoubleEllipse)
		graphicsType = eFgTwoEllipses;
	else if (dropletFitMode == eDropletPolynomial || dropletFitMode == eDropletDoublePolynomial)
		graphicsType = eFgPolyline;
	return graphicsType;
}

void CDropletWetting::readyManualDropletPts(double** x, double**y) const
{
	if (x == NULL || y == NULL)
		return;
	int nManualCnt = (int)m_dropletPtsByManual.size();
	if (nManualCnt < 1)
		return;
	
	int ptcount = std::max(nManualCnt, 5);
	*x = new double[ptcount];
	*y = new double[ptcount];
	memset(*x, 0, sizeof(double) * ptcount);
	memset(*y, 0, sizeof(double) * ptcount);
	for (int i = 0; i < nManualCnt; i++) {
		(*x)[i] = m_dropletPtsByManual[i].x;
		(*y)[i] = m_dropletPtsByManual[i].y;
	}
}

void CDropletWetting::readyManulBasePts(double** x, double** y, bool bSupport) const 
{
	if (x == NULL || y == NULL)
		return;
	int nManualCnt = (int)m_basePtsByManual.size();
	if (nManualCnt < 1)
		return;

	int ptcount = std::max(nManualCnt, 5);
	*x = new double[ptcount];
	*y = new double[ptcount];
	memset(*x, 0, sizeof(double) * ptcount);
	memset(*y, 0, sizeof(double) * ptcount);
	for (int i = 0; i < nManualCnt; i++) {
		(*x)[i] = m_basePtsByManual[i].x;
		(*y)[i] = m_basePtsByManual[i].y;
	}

	if (nManualCnt == 2 && bSupport) {
		(*x)[2] = (m_basePtsByManual[0].x + m_basePtsByManual[1].x) / 2.0;
		(*y)[2] = (m_basePtsByManual[0].y + m_basePtsByManual[1].y) / 2.0;
	}
}

bool 
CDropletWetting::fitting(
	FitMode fitMode,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	RectangleF baseRect, 
	FittingInfo* outInfo,
	bool isCallByExternal
) {
	double out_anglesself[2] = { 0, 0 };
	std::vector<PointAndAngle> res;
	cv::RotatedRect boxes[3]; // 0, 1: droplet (double circle, double ellipse), 2: baseline
	std::vector<cv::Point> pts_up, pts_dn;
	bool result = false;
	std::vector<cv::Point2f> corners;
	
	// json input
	if (isCallByExternal) {
		clearFittingOutputJson();
		m_jsInput[JSNAME_FITMODE] = fitMode;
		m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
		m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
		m_jsInput[JSNAME_BASEPTS] = {
			{"x1", baseRect.left}, {"y1", baseRect.top},
			{"x2", baseRect.right}, {"y2", baseRect.bottom}
		};
	}

	m_Context.ClearResultVariables();
	m_Context.SetModes(dropletFitMode, baselineFitMode);
	m_Context.UpdateBaserect(baseRect);

	FittingGraphicsType graphicsType = getFittingGraphicsType(dropletFitMode);

	if (!foo3381_check()) {
		errorReport(errException);
		return false;
	}
	bool hasPin = m_Context.HasPin();
	auto mainShape = m_Context.mainShape();
	ErrorCode res_code = errNo;

	if (fitMode == eFitAuto) {
		if (hasPin) {
			CPinAngleFitting pinAngleFitting(&m_Context);
			pinAngleFitting.setParameters(NULL, NULL);			
			res_code = pinAngleFitting.process();			
		}
		else {
			CNoPinAngleFitting dropletAnalyzer(&m_Context);
						
			double* baseline_x = NULL; 
			double* baseline_y = NULL; 

			readyManulBasePts(&baseline_x, &baseline_y);
			
			dropletAnalyzer.setParameters(baseline_x, baseline_y);
			res_code = dropletAnalyzer.process();

			if(baseline_x)
				delete[] baseline_x;
			if(baseline_y)
				delete[] baseline_y;			
		}
	}
	else if (fitMode == eFitManual) {
		CManualFitting manualFitting;

		double* yedi_x = NULL;
		double* yedi_y = NULL;
		double* baseline_x = NULL;
		double* baseline_y = NULL;

		readyManualDropletPts(&yedi_x, &yedi_y);
		readyManulBasePts(&baseline_x, &baseline_y, true);

		cv::RotatedRect boxes[3];
		manualFitting.setParam(
			m_Context.Image(),
			mainShape, dropletFitMode, baselineFitMode,
			baseline_x, baseline_y, yedi_x, yedi_y);
		res_code = manualFitting.Process(out_anglesself, res, boxes);

		if(baseline_x) 		delete[] baseline_x;
		if(baseline_y) 		delete[] baseline_y;
		if(yedi_x) 			delete[] yedi_x;
		if(yedi_y) 			delete[] yedi_y;

		if (res_code == errNo) {
			m_Context.SetAnglePoints(res);
			m_Context.SetAngles(out_anglesself);
			m_Context.SetBox(0, boxes[0]);
			m_Context.SetBox(1, boxes[1]);
			if (mainShape == eShapeHorizontal) {
				cv::RotatedRect emptyBox;
				emptyBox.size.width = 0; emptyBox.size.height = 0;
				emptyBox.center.x = 0; emptyBox.center.y = 0; emptyBox.angle = 0;
				m_Context.SetBox(2, emptyBox);
			}
			else
				m_Context.SetBox(2, boxes[2]);	
		}
	}
	else { // semi-auto & by point
		CNoPinAngleFitting dropletAnalyzer(&m_Context);		
		double* baseline_x = NULL;
		double* baseline_y = NULL;
		readyManulBasePts(&baseline_x, &baseline_y);
		
		dropletAnalyzer.setParameters(baseline_x, baseline_y);
		res_code = dropletAnalyzer.process();
		if(baseline_x)
			delete[] baseline_x;
		if(baseline_y)
			delete[] baseline_y;
	}

	if (res_code == errNo || res_code == errFailOnlyOne) {
		result = true;
	}
	else {
		errorReport(res_code);
	}

	fillFittingInfo(outInfo, (mainShape == eShapeHorizontal));

	return result;
}

void CDropletWetting::fillFittingInfo(FittingInfo* outInfo, bool isHorizontal)
{
	m_jsOutput[JSNAME_RESCODE] = m_errorCode;
	m_jsOutput[JSNAME_BSLINESHAPE] = m_Context.baselineShape();
	auto _outDropletPts = m_Context.DropletPtRef();
	if (!_outDropletPts.empty())
		m_jsOutput[JSNAME_DROPCONTOUR] = makeContourJson(_outDropletPts);

	auto _outBaselinePts = m_Context.BaelinePtRef();
	if (!_outBaselinePts.empty())
		m_jsOutput[JSNAME_BASELINECONTOUR] = makeContourJson(_outBaselinePts);

	if (outInfo == NULL)
		return;
	FittingGraphicsType graphicsType = getFittingGraphicsType(m_Context.DropletMode());
	outInfo->fgType = graphicsType;
	auto boxes = m_Context.GetBoxes();
	if (outInfo->boxes) {
		for (int i = 0; i < 3; i++) {
			outInfo->boxes[i].center.x = boxes[i].center.x;
			outInfo->boxes[i].center.y = boxes[i].center.y;
			outInfo->boxes[i].angle = boxes[i].angle;
			outInfo->boxes[i].a = boxes[i].size.width;
			outInfo->boxes[i].b = boxes[i].size.height;			
		}
	}
	// json writing of boxes
	if (outInfo->fgType == eFgTwoEllipses) {
		m_jsOutput[JSNAME_DROPLEFTBOX] = makeBoxJson(boxes[0]);
		m_jsOutput[JSNAME_DROPRIGHTBOX] = makeBoxJson(boxes[1]);		
	}
	else if (outInfo->fgType == eFgOneEllipse || outInfo->fgType == eFgRectangle) {
		m_jsOutput[JSNAME_DROPBOX] = makeBoxJson(boxes[0]);
	}
	if (!isHorizontal)
		m_jsOutput[JSNAME_BASELINEBOX] = makeBoxJson(boxes[2]);

	auto angles = m_Context.GetAngles();
	auto detailedAngle = m_Context.GetAnglePoints();
	if (detailedAngle.size() == 2 && detailedAngle[1].point.x < detailedAngle[0].point.x)
		std::swap(detailedAngle[0], detailedAngle[1]);

	outInfo->angle[0] = outInfo->angle[1] = -1.0;
	for (int i = 0; i < 2; i++) {
		outInfo->angle[i] = angles[i];
		if (i < detailedAngle.size()) // todo:
			outInfo->pointAngle[i] = detailedAngle[i];
	}
	if (detailedAngle.size() > 0 && outInfo->angle[0] >= 0.0) {
		m_jsOutput[JSNAME_RESULTLEFT] = makeSideJson(detailedAngle[0]);
	}
	if (detailedAngle.size() > 1 && outInfo->angle[1] >= 0.0) {
		m_jsOutput[JSNAME_RESULTRIGHT] = makeSideJson(detailedAngle[1]);
	}

	auto _leftPolyline = m_Context.LeftPolyPtRef();
	outInfo->leftPolylineSize = static_cast<int>(_leftPolyline.size());
	if (outInfo->leftPolylineSize > 0) {
		outInfo->leftPolyline = new PointF[outInfo->leftPolylineSize];
		for (int i = 0; i < outInfo->leftPolylineSize; ++i) {
			outInfo->leftPolyline[i].x = static_cast<float>(_leftPolyline[i].x);
			outInfo->leftPolyline[i].y = static_cast<float>(_leftPolyline[i].y);
		}
		if (outInfo->fgType == eFgPolyline)
			m_jsOutput[JSNAME_LEFTPOLYLINE] = makeContourJson(_leftPolyline);
	}
	else {
		outInfo->leftPolyline = nullptr;
	}

	auto _rightPolyline = m_Context.RightPolyPtRef();
	outInfo->rightPolylineSize = static_cast<int>(_rightPolyline.size());
	if (outInfo->rightPolylineSize > 0) {
		outInfo->rightPolyline = new PointF[outInfo->rightPolylineSize];
		for (int i = 0; i < outInfo->rightPolylineSize; ++i) {
			outInfo->rightPolyline[i].x = static_cast<float>(_rightPolyline[i].x);
			outInfo->rightPolyline[i].y = static_cast<float>(_rightPolyline[i].y);
		}
		if (outInfo->fgType == eFgPolyline)
			m_jsOutput[JSNAME_RIGHTPOLYLINE] = makeContourJson(_rightPolyline);
	}
	else {
		outInfo->rightPolyline = nullptr;
	}

}

bool
CDropletWetting::fittingManual(
	MainShapeType mainShapeType,
	FitMode fitMode,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	RectangleF baseRect,
	const std::vector<cv::Point2f>& baselinePts,
	const std::vector<cv::Point2f>& dropletPts,
	FittingInfo* outInfo
) {
	bool ret = false;
	// json input
	clearFittingOutputJson();
	m_jsInput[JSNAME_FITMODE] = fitMode;
	m_jsInput[JSNAME_MAINSHAPE] = mainShapeType;
	m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
	m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
	m_jsInput[JSNAME_BASEPTS] = {
		{"x1", baseRect.left}, {"y1", baseRect.top},
		{"x2", baseRect.right}, {"y2", baseRect.bottom}
	};
	if (!baselinePts.empty()) {
		m_jsInput[JSNAME_MANUALBSLINEPTS] = makeContourJson(baselinePts);
	}
	if (!dropletPts.empty()) {
		m_jsInput[JSNAME_MANUALDROPLETPTS] = makeContourJson(dropletPts);
	}

	if (fitMode == eFitManual) {
		initVariables();
		if (!m_bInited) {
			errorReport(errInitModel);
			return ret;
		}
		if (m_Context.IsEmptyImage()) {
			errorReport(errFailOpen);
			return ret;
		}
		m_Context.SetModes(dropletFitMode, baselineFitMode, false, fitMode);

		if (!foo3381_check()) {
			errorReport(errException);
			return ret;
		}

		if (mainShapeType == eShapeUnknown) {
			GetBaselineShape();
			if (m_Context.baselineShape() == eBS_Unknown) {
				errorReport(errFailedDetectShape);
				return ret;
			}
			m_Context.BuildShapeTypeFromDetailedShape();
			mainShapeType = m_Context.mainShape();
		}
		else {
			m_Context.SetMainShape(mainShapeType);
		}
	}

	m_dropletPtsByManual.clear();
	m_basePtsByManual.clear();

	if (dropletFitMode == eDropletWidthHeight) {
		for (int i = 0; i < baselinePts.size(); i++) {
			m_basePtsByManual.push_back(baselinePts[i]);
		}
		if (dropletPts.size() < 2) {
			errorReport(errInvalidManualPointCount);
			return ret;
		}
		for (int i = 0; i < 2; i++) {
			m_dropletPtsByManual.push_back(dropletPts[i]);
		}
	}
	else {
		int neededBasePts = (baselineFitMode > eBaseLineCircle ? 5 : 3);
		if (mainShapeType == eShapeHorizontal) {
			neededBasePts = 0;
		}
		if (baselinePts.size() < neededBasePts) {
			errorReport(errInvalidManualPointCount);
			return ret;
		}

		neededBasePts = (dropletFitMode > eDropletCircle ? 5 : 3);
		if (fitMode == eFitManual && dropletPts.size() < neededBasePts) {
			errorReport(errInvalidManualPointCount);
			return ret;
		}
		for (int i = 0; i < baselinePts.size(); i++) {
			m_basePtsByManual.push_back(baselinePts[i]);
		}
		for (int i = 0; i < dropletPts.size(); i++) {
			m_dropletPtsByManual.push_back(dropletPts[i]);
		}
	}

	ret = fitting(fitMode, dropletFitMode, baselineFitMode, baseRect, outInfo, false);
	return ret;
}

bool
CDropletWetting::fittingBasePointsAndLine(
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode, // will not be used.
	RectangleF baseRect,
	FittingInfo* outInfo
) {
	// json input
	clearFittingOutputJson();
	m_jsInput[JSNAME_FITMODE] = eFitByBasePoint;
	m_jsInput[JSNAME_DROPLETFITMODE] = dropletFitMode;
	m_jsInput[JSNAME_BASELINEFITMODE] = baselineFitMode;
	m_jsInput[JSNAME_BASEPTS] = {
		{"x1", baseRect.left}, {"y1", baseRect.top},
		{"x2", baseRect.right}, {"y2", baseRect.bottom}
	};

	return fitting(eFitAuto, dropletFitMode, baselineFitMode, baseRect, outInfo, false);
}

void CDropletWetting::clearFittingOutputJson()
{
	std::vector<std::string> keys = {
		JSNAME_RESCODE, 
		JSNAME_MAINSHAPE,	// ? 
		JSNAME_RESANGLES, 
		JSNAME_DROPBOX, 
		JSNAME_DROPLEFTBOX,
		JSNAME_DROPRIGHTBOX,
		JSNAME_BASELINEBOX, 
		JSNAME_DROPCONTOUR,
		JSNAME_BASELINECONTOUR,
		JSNAME_LEFTPOLYLINE,
		JSNAME_RIGHTPOLYLINE, 
		JSNAME_RESULTLEFT,
		JSNAME_RESULTRIGHT
	};
	for (const auto& key : keys) {
		if (m_jsOutput.contains(key)) {
			m_jsOutput.erase(key);
		}
	}
}

void CDropletWetting::clearHorizontalModeJson()
{
	std::vector<std::string> keys = {
		JSNAME_RESCODE,
		JSNAME_HASPIN,
		JSNAME_HORIZONMODE
	};
	for (const auto& key : keys) {
		if (m_jsOutput.contains(key)) {
			m_jsOutput.erase(key);
		}
	}
}

void CDropletWetting::clearBasePointJson()
{
	std::vector<std::string> keys = {
		JSNAME_RESCODE,
		JSNAME_HORIZONMODE,
		JSNAME_ROTATED,
		JSNAME_ROTATED,
		JSNAME_BASEPTS,
		JSNAME_KEYPTS
	};
	for (const auto& key : keys) {
		if (m_jsOutput.contains(key)) {
			m_jsOutput.erase(key);
		}
	}
}
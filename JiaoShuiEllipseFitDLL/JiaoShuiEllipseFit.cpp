#include "JiaoShuiEllipseFit.h"
#include "../cvLib/cvcommon.h"
#include "DropletWetting.h"

#ifdef _DEBUG
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#	define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

CDropletWetting* dropletMain = NULL;

// Version histories

#define VERSION_STR _T("1.4.0.20250920")

std::wstring errorString[] = {
	_T("成功"),
	_T("全局异常处理"),
	_T("有效期过期"),
	_T("打不开图片"),
	_T("模型加载失败"),
	_T("模型加载失败(baseline.cfg，baseline.weights)"),
	_T("模型加载失败(9-shapenet.ini，9-shapenet.dic)"),
	_T("模型加载失败(det3.ini，det3.dic)"),
	_T("基线点无效"),
	_T("拟合模式无效"),
	_T("手动点数无效"),
	_T("无法找到轮廓"),
	_T("液滴拟合失败"),
	_T("基线拟合失败"),
	_T("无法找到交点"),
	_T("形状检测失败"),
	_T("基线点检测失败"),
	_T("圆拟合失败"),
	_T("无法检测交叉点"),
	_T("只检测到一个交叉点"),
	_T("检测不到有效液滴"),
	_T("拟合分析失败"),
	_T("角度检测失败"),
};

bool dropletInitLib(const TCHAR* appPath)
{
	bool res = false;
	if (dropletMain == NULL) {
		dropletMain = new CDropletWetting();
		res = dropletMain->InitModel(appPath);
	}
	else {
		return res;
	}

	if(!res) 
		dropletReleaseLib();

	return res;
}

void dropletReleaseLib()
{
	if(dropletMain != NULL) {
		delete dropletMain;
		dropletMain = NULL;
	}

}

bool dropletSetImage(const TCHAR* imagePath)
{
	if(!imagePath) 
		return false;
	if(!dropletMain) 
		return false;
	auto res = dropletMain->SetImage(imagePath);
	return res;
}

#if _DEV_OLDSTYLE_API
bool dropletAutoFit(
	DropletFitMode dropletFitMode, 
	BaseLineFitMode baselineFitMode, 
	MainShapeType mainShape, 
	bool isSubpixel,
	bool rotated,
	HorizonMode horMode,
	bool hasPin
) {
	if(!dropletMain) 
		return errException;
	auto res = dropletMain->RecognizeDropletAngleByAuto(
		dropletFitMode,
		baselineFitMode, 
		mainShape, 
		isSubpixel,
		rotated, 
		horMode, 
		hasPin
	);
	return (res == errNo);
}

bool dropletFitByBaseLine(
	const PointF* pts, 
	DropletFitMode dropletFitMode, 
	BaseLineFitMode baselineFitMode,
	MainShapeType mainShape, 
	bool isSubpixel
) {
	if(!dropletMain || pts == NULL) 
		return false;
	std::vector<cv::Point2f> linePoints;
	linePoints.push_back(cv::Point2f((float)pts[0].x, (float)pts[0].y));
	linePoints.push_back(cv::Point2f((float)pts[1].x, (float)pts[1].y));
	auto res = dropletMain->RecognizeDropletAngleByBaseline(
		dropletFitMode, 
		baselineFitMode,
		mainShape, 
		isSubpixel, 
		linePoints
	);
	return (res == errNo);
}

bool dropletFitByBasePoint(
	const PointF* pts,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode, 
	MainShapeType mainShape,
	bool isSubpixel
) {
	if(!dropletMain || pts == NULL)
		return false;
	std::vector<cv::Point2f> basePts;
	basePts.push_back(cv::Point2f((float)pts[0].x, (float)pts[0].y));
	basePts.push_back(cv::Point2f((float)pts[1].x, (float)pts[1].y));
	auto res = dropletMain->RecognizeDropletAngleByBasePoints(
		eFitAuto, 
		dropletFitMode, 
		baselineFitMode,
		mainShape, 
		isSubpixel,
		basePts
	);
	return (res == errNo);
}

bool dropletManualFit(
	int nDroplets,
	PointF* droplets,
	int nBaselines,
	PointF* baselines,
	DropletFitMode dropletFitMode, 
	BaseLineFitMode baselineFitMode,
	MainShapeType mainShape,
	bool isSubpixel	
) {
	if (!dropletMain)
		return false;
	std::vector<cv::Point2f> linePoints;
	std::vector<cv::Point2f> dropletPoints;
	for(int i = 0; i < nDroplets; i++)
		dropletPoints.push_back(cv::Point2f((float)droplets[i].x, (float)droplets[i].y));
	for(int i = 0; i < nBaselines; i++)
		linePoints.push_back(cv::Point2f((float)baselines[i].x, (float)baselines[i].y));

	auto res = dropletMain->RecognizeDropletAngleByBaseManual(
		dropletFitMode, 
		baselineFitMode, 
		eFitManual, 
		mainShape, 
		isSubpixel, 
		dropletPoints,
		linePoints
	);

	return (res == errNo);
}

bool dropletSemiAutoFit(
	int nBaselines, PointF* baselines,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	MainShapeType mainShape,
	bool isSubpixel)
{
	bool res = false;
	if (!dropletMain || baselines == NULL)
		return false;
	if (mainShape == eShapeHorizontal && nBaselines == 2) {
		res = dropletFitByBaseLine(
			baselines,
			dropletFitMode, 
			baselineFitMode, 
			mainShape,
			isSubpixel
		);
	}
	else {
		std::vector<cv::Point2f> linePoints;
		std::vector<cv::Point2f> dropletPoints;
		for (int i = 0; i < nBaselines; i++)
			linePoints.push_back(cv::Point2f((float)baselines[i].x, (float)baselines[i].y));

		auto resCode = dropletMain->RecognizeDropletAngleByBaseManual(
			dropletFitMode,
			baselineFitMode,
			eFitSemiAuto,
			mainShape,
			isSubpixel,
			dropletPoints,
			linePoints
		);

		res = (resCode == errNo);
	}
	
	return res;
}

BaseLineShapeKind dropletGetShape()
{
	if (!dropletMain)
		return eBS_Unknown;
	auto res = dropletMain->GetBaselineShape();
	return res;
}

#endif//_DEV_OLDSTYLE_API
bool dropletGetResult(
	double* angles,
	PointAndAngle* angleDetails
) {
	if (!dropletMain)
		return false;
	auto res = dropletMain->GetResult(angles, angleDetails);
	return res;
}

bool dropletMidResult(
	MainShapeType* _mainShape,
	bool* _hasPin,
	bool* _isEmpty,
	HorizonMode* _horizonMode,
	RectangleF* _basePts,
	EllipseFitBox* _outBox
) {
	if (!dropletMain)
		return false;
	std::vector<cv::RotatedRect> boxes;
	MainShapeType mainShape;
	bool hasPin;
	bool isEmpty;
	RectangleF basePts;
	HorizonMode horizonMode;

	auto res = dropletMain->GetMidResult(
		mainShape,
		hasPin,
		isEmpty,
		horizonMode,
		basePts,
		boxes
	);
	if (res) {
		if (_mainShape)
			*_mainShape = mainShape;
		if (_basePts)
			*_basePts = basePts;
		if (_hasPin)
			*_hasPin = hasPin;
		if (_isEmpty)
			*_isEmpty = isEmpty;
		if (_horizonMode)
			*_horizonMode = horizonMode;

		if (_outBox) {
			if (boxes.size() > 0) {
				_outBox[0].center.x = boxes[0].center.x;
				_outBox[0].center.y = boxes[0].center.y;
				_outBox[0].angle = boxes[0].angle;
				_outBox[0].a = boxes[0].size.width;
				_outBox[0].b = boxes[0].size.height;
			}
			if (boxes.size() > 1) {
				_outBox[1].center.x = boxes[1].center.x;
				_outBox[1].center.y = boxes[1].center.y;
				_outBox[1].angle = boxes[1].angle;
				_outBox[1].a = boxes[1].size.width;
				_outBox[1].b = boxes[1].size.height;
			}
		}
	}
	return res;
}

ErrorCode dropletGetLastError()
{
	if(!dropletMain) 
		return errNo;
	auto res = dropletMain->GetLastErrorCode();
	return res;
}

BSTR dropletGetResultString(ErrorCode code)
{	
	auto res = SysAllocString(errorString[code].c_str());
	return res;
}

BSTR dropletGetResultByJsonFormat()
{
	if (!dropletMain)
		return NULL;
	auto res = dropletMain->GetResultByJsonFormat();
	BSTR bstr = SysAllocString(res.c_str());
	return bstr;
}


bool dropletGetContourCounts(
	int* dropletCount, 
	int* baselineCount
) {
	if(!dropletMain) 
		return false;
	int dropCount = 0, baseCount = 0;
	auto res = dropletMain->GetContourPointsCount(dropCount, baseCount);
	if (res) {
		if(dropletCount)
			*dropletCount = dropCount;
		if(baselineCount)
			*baselineCount = baseCount;
	}
	return res;
}
bool dropletGetContours(
	int dropletCount,
	int baselineCount, 
	PointF* dropletPts, 
	PointF* baselinePts
) {
	if(!dropletMain) 
		return false;
	std::vector<cv::Point> dropletPoints, baselinePoints;
	auto res = dropletMain->GetContourPoints(dropletPoints, baselinePoints);
	if (res) {
		if(dropletCount > 0 && dropletPts) {
			dropletCount = std::min(dropletCount, ( int )dropletPoints.size() );
			for(int i = 0; i < dropletCount; i++) {
				dropletPts[i].x = dropletPoints[i].x;
				dropletPts[i].y = dropletPoints[i].y;
			}
		}
		if(baselineCount > 0 && baselinePts) {
			baselineCount = std::min(baselineCount, ( int )baselinePoints.size() );
			for(int i = 0; i < baselineCount; i++) {
				baselinePts[i].x = baselinePoints[i].x;
				baselinePts[i].y = baselinePoints[i].y;
			}
		}
	}
	return res;
}

/*
* Added  2025-08-15
*/

bool dropletGetShapeEx(BaseLineShapeKind *shape, HorizonMode *horizon, bool* hasPin) 
{
	bool res = false;
	
	if (!dropletMain)
		return res;
	auto baseLineShapeKind = dropletMain->GetBaselineShape();
	if (baseLineShapeKind == eBS_Unknown)
		return res;
	res = true;
	HorizonMode horizonMode = eHmUnknow;
	bool isHorizontal = (baseLineShapeKind % 3 == 2);
	bool pin = (baseLineShapeKind >= eBS_ConvexWithPins) && (baseLineShapeKind < eBS_ConvexWithoutPins);
	if (shape) {
		*shape = baseLineShapeKind;
	}
	if (hasPin)
		*hasPin = pin;
	if (isHorizontal) {
		auto ret = dropletMain->getHorizontalMode(horizonMode, pin);
		if (ret) {
			res = true;
			if (horizon)
				*horizon = horizonMode;
		}
	}
	return res;
}

bool dropletGetBasepointsByAutoOnHorizontal(
	bool& hasPin, 
	bool& rotated,
	HorizonMode& horMode,	
	RectangleF* basePts, 
	PointF* keyPts,
	bool isSubpixel,
	bool isBoundSupplement
) {
	if (!dropletMain)
		return errException;

	RectangleF _basePts;
	
	auto res = dropletMain->getBasepointsByAuto(
		true,
		hasPin,
		rotated,
		horMode,
		isSubpixel,
		isBoundSupplement,
		_basePts, 
		keyPts
	);

	if (res == errNo) {
		if (basePts) 
			*basePts = _basePts;
	}
	return (res == errNo);
}

bool dropletGetBasepointsByAutoOnSurface(
	bool hasPin,	
	RectangleF* basePts,
	bool isSubpixel,
	bool isBoundSupplement
) {
	if (!dropletMain)
		return errException;

	RectangleF _basePts;
	bool rotated = false;
	HorizonMode horMode = HorizonMode::eHmUnknow;
	auto res = dropletMain->getBasepointsByAuto(
		false,
		hasPin,
		rotated,
		horMode,
		isSubpixel,
		isBoundSupplement,
		_basePts, 
		nullptr
	);

	if (res == errNo) {
		if (basePts) 
			*basePts = _basePts;
	}
	return (res == errNo);
}

bool dropletGetBasepointsBylineOnHorizontal(
	const PointF* pts, 
	RectangleF* basePts,
	bool isSubpixel
) {
	if (!dropletMain)
		return errException;

	std::vector<cv::Point2f> linePts;
	linePts.push_back(cv::Point2f((float)pts[0].x, (float)pts[0].y));
	linePts.push_back(cv::Point2f((float)pts[1].x, (float)pts[1].y));
	RectangleF rectBasePts;
	bool isHorizontal = true;
	auto res = dropletMain->getBasepointsByBaseline(
		isHorizontal,
		isSubpixel,
		linePts,
		rectBasePts
	);
	if (res == errNo) {
		if (basePts) 
			*basePts = rectBasePts;
	}
	return (res == errNo);
}

bool dropletGetBasepointsBylineOnSurface(
	const PointF* pts,
	RectangleF* basePts,
	bool isSubpixel
) {
	if (!dropletMain)
		return errException;

	std::vector<cv::Point2f> linePts;
	linePts.push_back(cv::Point2f((float)pts[0].x, (float)pts[0].y));
	linePts.push_back(cv::Point2f((float)pts[1].x, (float)pts[1].y));
	RectangleF rectBasePts;
	bool isHorizontal = false;
	auto res = dropletMain->getBasepointsByBaseline(
		isHorizontal,
		isSubpixel,
		linePts,
		rectBasePts
	);
	if (res == errNo) {
		if (basePts) *basePts = rectBasePts;
	}
	return (res == errNo);
}

bool dropletGetBasepointsByPointsOnHorizontal(
	const PointF* pts,
	RectangleF* basePts,
	bool isSubpixel
) {
	if (!dropletMain)
		return errException;

	std::vector<cv::Point2f> linePts;
	linePts.push_back(cv::Point2f((float)pts[0].x, (float)pts[0].y));
	linePts.push_back(cv::Point2f((float)pts[1].x, (float)pts[1].y));
	RectangleF rectBasePts;
	bool isHorizontal = true;
	auto res = dropletMain->getBasepointsByBasepoint(
		isHorizontal,
		isSubpixel,
		linePts,
		rectBasePts
	);
	if (res == errNo) {
		if (basePts) *basePts = rectBasePts;
	}
	return (res == errNo);
}

bool dropletGetBasepointsByPointsOnSurface(
	const PointF* pts,
	RectangleF* basePts,
	bool isSubpixel
) {
	if (!dropletMain)
		return errException;

	std::vector<cv::Point2f> linePts;
	linePts.push_back(cv::Point2f((float)pts[0].x, (float)pts[0].y));
	linePts.push_back(cv::Point2f((float)pts[1].x, (float)pts[1].y));
	RectangleF rectBasePts;
	bool isHorizontal = false;
	auto res = dropletMain->getBasepointsByBasepoint(
		isHorizontal,
		isSubpixel,
		linePts,
		rectBasePts
	);
	if (res == errNo) {
		if (basePts) *basePts = rectBasePts;
	}
	return (res == errNo);
}

bool dropletGetBasepointsBySemiautoOnSurface(
	MainShapeType mainShapeType,
	int nBaselines, const PointF* baselines,
	RectangleF* basePts,
	bool isSubpixel
) {
	if (!dropletMain || baselines == NULL)
		return false;

	std::vector<cv::Point2f> linePoints;
	std::vector<cv::Point2f> dropletPoints;
	for (int i = 0; i < nBaselines; i++)
		linePoints.push_back(cv::Point2f((float)baselines[i].x, (float)baselines[i].y));
	RectangleF rectBasePts;

	auto res = dropletMain->getBasepointsBySemiauto(
		mainShapeType,
		isSubpixel,
		dropletPoints, 
		rectBasePts
	);
	if (res == errNo) {
		if (basePts)
			*basePts = rectBasePts;
	}
	return (res == errNo);
}

bool dropletAutoFittingOnHorizontal(
	DropletFitMode dropletFitMode,
	const RectangleF* pts,
	FittingInfo* outInfo
) {
	if (!dropletMain)
		return false;	

	bool res = false;
	auto mainShape = dropletMain->getMainShape();
	if (mainShape == MainShapeType::eShapeHorizontal) {
		res = dropletMain->fitting(
			eFitAuto,
			dropletFitMode,
			BaseLineFitMode::eBaseLineCircle, // will not be used.
			*pts,
			outInfo);
	}
	return res;
}

bool dropletAutoFittingOnSurface(
	MainShapeType mainShape,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	const RectangleF* pts,
	FittingInfo* outInfo
)
{
	bool res = false;
	if (mainShape < MainShapeType::eShapeHorizontal) {
		res = dropletMain->fitting(
			eFitAuto,
			dropletFitMode,
			baselineFitMode, // will not be used.
			*pts,
			outInfo);
	}

	return res;
}

bool dropletManualFitting(
	MainShapeType mainShape,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	int nDroplets,
	const PointF* droplets,
	int nBaselines,
	const PointF* baselines,	
	bool hasPin, 
	FittingInfo* outInfo,
	bool isSubpixel
) {
	RectangleF rectBasePts;

	std::vector<cv::Point2f> linePoints;
	std::vector<cv::Point2f> dropletPoints;
	for (int i = 0; i < nBaselines; i++)
		linePoints.push_back(cv::Point2f((float)baselines[i].x, (float)baselines[i].y));

	for (int i = 0; i < nDroplets; i++)
		dropletPoints.push_back(cv::Point2f((float)droplets[i].x, (float)droplets[i].y));

	auto res = dropletMain->fittingManual(
		mainShape,
		eFitManual,
		dropletFitMode,
		baselineFitMode, // will not be used.
		rectBasePts,
		linePoints, 
		dropletPoints,
		outInfo);

	return res;
}

bool dropletSemiautoFitting(
	MainShapeType mainShape,
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	const RectangleF* pts,
	int nBaselines,
	PointF* baselines,
	FittingInfo* outInfo
) {
	RectangleF rectBasePts = *pts;

	std::vector<cv::Point2f> linePoints;
	std::vector<cv::Point2f> dropletPoints;
	for (int i = 0; i < nBaselines; i++)
		linePoints.push_back(cv::Point2f((float)baselines[i].x, (float)baselines[i].y));

	auto res = dropletMain->fittingManual(
		mainShape,
		eFitSemiAuto,
		dropletFitMode,
		baselineFitMode, // will not be used.
		rectBasePts,
		linePoints,
		dropletPoints,
		outInfo);

	return res;
}

bool dropletPointsFitting(
	DropletFitMode dropletFitMode,
	BaseLineFitMode baselineFitMode,
	const RectangleF* pts,
	FittingInfo* outInfo
) {
	auto res = dropletMain->fittingBasePointsAndLine(
		dropletFitMode,
		baselineFitMode, // will not be used.
		*pts,
		outInfo
	);

	return res;
}

void dropletFreeFittingInfo(FittingInfo* info)
{
	if (info == nullptr)
		return;

	if(info->leftPolyline != nullptr)
		delete[] info->leftPolyline;

	if (info->rightPolyline != nullptr)
		delete[] info->rightPolyline;

	info->leftPolyline = nullptr;
	info->leftPolylineSize = 0;

	info->rightPolyline = nullptr;
	info->rightPolylineSize = 0;
}

BSTR dropletGetVersion()
{
	auto res = SysAllocString(VERSION_STR);
	return res;
}
//.EOF
 #include "DropletContext.h"
#include "..\cvLib\cvcommon.h"

CDropletContext::CDropletContext()
	: m_frame(cv::Mat())
	, m_ptLeft(cv::Point2f(0, 0))
	, m_ptRight(cv::Point2f(0, 0))
	, m_mainShape(eShapeUnknown)
	, m_dropletFitMode(eDropletCircle)
	, m_baseLineFitMode(eBaseLineCircle)
	, m_fitMode(eFitAuto)
	, m_bUseSubpixel(false)
	, m_baseLineShapeKind(eBS_Unknown)	
	, m_hasPin(false)
	, m_isEmpty(false)
	, m_horizonMode(eHmUnknow)
{
	m_outAngles[0] = m_outAngles[1] = 0.0;
}

CDropletContext::~CDropletContext()
{
	InitVariables();
}

void CDropletContext::InitVariables()
{
	m_baseLineFitMode = eBaseLineCircle;
	m_dropletFitMode = eDropletCircle;
	m_baseLineShapeKind = eBS_Unknown;
	m_bUseSubpixel = false;
	m_mainShape = eShapeUnknown;
	m_fitMode = eFitAuto;
	m_hasPin = false;
	m_isEmpty = false;
	if (!m_frame.empty()) {
		m_frame.release();
	}
	m_baseRect.left = m_baseRect.top = m_baseRect.right = m_baseRect.bottom = 0.0;
	ClearResultVariables();
}

void CDropletContext::ClearResultVariables()
{
	memset(m_outAngles, 0, sizeof(m_outAngles));
	m_angles.clear();
	memset(m_outBoxes, 0, sizeof(m_outBoxes));
	m_outDropletPts.clear();
	m_outBaselinePts.clear();
	m_leftPolyline.clear();
	m_rightPolyline.clear();
}

bool CDropletContext::LoadImage(const TCHAR* path)
{
	bool res = true;
	if (!m_frame.empty()) {
		m_frame.release();		
	}
	m_frame = loadImageFromUnicodePath(path);
	if (m_frame.empty()) {
		res = false;
	}
	res = true;
	return res;
}

bool CDropletContext::IsEmptyImage() const
{
	return m_frame.empty();
}

void CDropletContext::SetModes(DropletFitMode dFit, BaseLineFitMode bFit, bool subPixel, FitMode fitMode)
{
	m_dropletFitMode = dFit;
	m_baseLineFitMode = bFit;
	m_bUseSubpixel = subPixel;
	m_fitMode = fitMode;
}

void CDropletContext::SetModes(DropletFitMode dFit, BaseLineFitMode bFit)
{
	m_dropletFitMode = dFit;
	m_baseLineFitMode = bFit;
}

void CDropletContext::SetModes(bool subPixel, FitMode fitMode)
{
	m_bUseSubpixel = subPixel;
	m_fitMode = fitMode;
}

void CDropletContext::SetBaselineShape(BaseLineShapeKind _val)
{
	m_baseLineShapeKind = _val; 
}

void CDropletContext::BuildShapeTypeFromDetailedShape()
{
	m_mainShape = static_cast<MainShapeType>(m_baseLineShapeKind % 3);
	m_hasPin = (m_baseLineShapeKind >= eBS_ConvexWithPins) &&
		(m_baseLineShapeKind < eBS_ConvexWithoutPins);
	m_isEmpty = (m_baseLineShapeKind < eBS_ConvexWithPins);
}

void CDropletContext::SetMainShape(MainShapeType _val)
{
	m_mainShape = _val;
}

void CDropletContext::UpdateBaserect(const RectangleF& _val)
{
	m_baseRect = _val;
	m_ptLeft.x = _val.left;
	m_ptLeft.y = _val.top;
	m_ptRight.x = _val.right;
	m_ptRight.y = _val.bottom;
}

void CDropletContext::SetLeftRightPoints(const cv::Point2f& l, const cv::Point2f& r)
{
	m_ptLeft = l;
	m_ptRight = r;
	m_baseRect.left = l.x;
	m_baseRect.top = l.y;
	m_baseRect.right = r.x;
	m_baseRect.bottom = r.y;
}
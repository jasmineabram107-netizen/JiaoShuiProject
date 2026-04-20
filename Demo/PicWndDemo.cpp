/******************************************************************************
\author	Jewel
\date	10/17/2019
******************************************************************************/

#include "pch.h"
#include "Demo.h"
#include "PicWndDemo.h"
#include <gdiplus.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CPicEditWnd

BOOL IsNearPoints(const PointF& a, const PointF& b, int diff = DIFF)
{
	return abs(a.x - b.x) < diff && abs(a.y - b.y) < diff;
}
IMPLEMENT_DYNAMIC(CPicEditWnd, CWnd)
CPicEditWnd::CPicEditWnd()
	: m_iWidth(0)
	, m_iHeight(0)
	, m_sScale(1.0f)
	, m_dx(0)
	, m_dy(0)
	, m_id(0)
	, m_processed(false)
	, m_success(false)
	, m_drawMode(eDrawNone)
	, m_bLeftButtonDown(false)
	, m_bRightButtonDown(false)
	, m_bLeftDragging(false)
	, m_bRightDragging(false)
	, m_bTracking(false)
	, m_drawModeOld(eDrawNone)
	, m_iLeftMoveID(-1)
	, m_iRightMoveID(-1)
	, m_bResult(false)
	, m_bHasPin(false)
	, m_bEllipseMode(false)
	, m_iFitMode(eFitAuto)
	, m_upFlag(eDropletCircle)
	, m_dnFlag(eBaseLineCircle)
{
}

CPicEditWnd::~CPicEditWnd()
{
	if (m_image.IsValid()) {
		m_image.Destroy();
	}
	ClearState();
}

BEGIN_MESSAGE_MAP(CPicEditWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()	
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CPicWnd message handlers
BOOL CPicEditWnd::CreateWnd(CWnd* pParent, const RECT& rc, UINT nID, int id)
{
	m_id = id;
	return CWnd::Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, rc, pParent, nID);
}

PointF CPicEditWnd::Screen2Real(const CPoint& pt) const
{
	PointF ptReal;
	if(m_sScale == 0.0)
		return ptReal;

	ptReal.x = (int)((double)(pt.x - m_dx) / m_sScale);
	ptReal.y = (int)((double)(pt.y - m_dy) / m_sScale);
	return ptReal;
}

CPoint CPicEditWnd::Real2Screen(const PointF& pt) const
{
	CPoint ptScreen;
	if(m_sScale == 0.0)
		return ptScreen;

	ptScreen.x = (int)(pt.x * m_sScale) + m_dx;
	ptScreen.y = (int)(pt.y * m_sScale) + m_dy;
	return ptScreen;
}

void CPicEditWnd::SetImage(const CString& strFile, bool bUpdate)
{
	m_processed = false;
	if (m_sFilename.CompareNoCase(strFile) == 0)
		return;
	m_sFilename = strFile;

	if (m_image.IsValid()) {
		m_image.Destroy();
		m_iWidth = m_iHeight = 0;
	}
	if (m_image.Load(strFile)) {
		m_iWidth = m_image.GetWidth();
		m_iHeight = m_image.GetHeight();
		ClearState();
	}
	else {
		m_iWidth = m_iHeight = 0;
	}
	if (GetSafeHwnd() && IsWindow(GetSafeHwnd())) {
		CRect rc;
		GetClientRect(&rc);
		calculateScale(rc.Width(), rc.Height());
		if(bUpdate)
			Invalidate();
	}
}

void CPicEditWnd::ClearState()
{
	m_processed = false;
	m_success = false;
	m_baseline.clear();
	m_dropletPoints.clear();
	m_iLeftMoveID = -1;
	m_drawMode = eDrawNone;
	m_drawModeOld = eDrawNone;
	m_rcTracker.m_rect.SetRect(50, 50, 200, 200);
	m_rcTracker.m_nStyle = CRectTracker::resizeInside ^ CRectTracker::dottedLine;
	m_rcTracker.m_nHandleSize = 5;

	ClearDrawItems();
}

void CPicEditWnd::ShowResult(bool success, const CString& msg)
{
	m_drawModeOld = m_drawMode;
	m_drawMode = eDrawSuccess;
	m_bResult = success;
	m_sResult = msg;
	Invalidate();
}


void CPicEditWnd::OnSize(UINT nType, int cx, int cy)
{
	if(m_iWidth == 0 || m_iHeight == 0)
		return;
	float oldScale = m_sScale;
	float dx0 = (float)m_dx, dy0 = (float)m_dy;
	calculateScale(cx, cy);

	if (oldScale > 0 && oldScale != m_sScale) {
		// Rescale the tracker rectangle
		CRect rect = m_rcTracker.m_rect;
		rect.left = static_cast<int>((rect.left - dx0) / oldScale * m_sScale) + m_dx;
		rect.top = static_cast<int>((rect.top - dy0) / oldScale * m_sScale) + m_dy;
		rect.right = static_cast<int>((rect.right - dx0) / oldScale * m_sScale) + m_dx;
		rect.bottom = static_cast<int>((rect.bottom - dy0) / oldScale * m_sScale) + m_dy;
		m_rcTracker.m_rect = rect;
	}

	if (GetSafeHwnd() && IsWindow(GetSafeHwnd()))
		Invalidate();
}

void CPicEditWnd::calculateScale(int cx, int cy)
{
	if (cx == 0 || cy == 0)
		return;
	if (m_iWidth == 0 || m_iHeight == 0)
		return;
	float scaleX = (float)cx / (float)m_iWidth;
	float scaleY = (float)cy / (float)m_iHeight;
	m_sScale = min(scaleX, scaleY);

	int w = (int)(m_iWidth * m_sScale);
	int h = (int)(m_iHeight * m_sScale);
	m_dx = (cx > w ? (cx - w) / 2 : 0);
	m_dy = (cy > h ? (cy - h) / 2 : 0);
}

void CPicEditWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages
	CRect rc;
	GetClientRect(&rc);
	CMemDC memDC(dc, this);
	DrawContents(&memDC.GetDC(), rc);
}

void CPicEditWnd::DrawPoint(CDC* pDC, const PointF& pt, COLORREF color, int index, int radius)
{	
	CPen pen(PS_SOLID, 1, color);
	CPen* pOldPen = pDC->SelectObject(&pen);
	CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	int x = (int)((double)pt.x * m_sScale) + m_dx;
	int y = (int)((double)pt.y * m_sScale) + m_dy;
	pDC->Ellipse(x - radius, y - radius, x + radius, y + radius);

	// Cross mark inside
	pDC->MoveTo(x - radius / 2, y);
	pDC->LineTo(x + radius / 2, y);
	pDC->MoveTo(x, y - radius / 2);
	pDC->LineTo(x, y + radius / 2);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	if(index > 0) {
		auto oldColor = pDC->SetTextColor(color);
		auto oldBkColor = pDC->SetBkColor(RGB(255, 255, 255));
		CString str;
		str.Format(_T("%d"), index);
		pDC->TextOut(x + radius, y + radius, str); // Draw index number
		pDC->SetTextColor(oldColor);
		pDC->SetBkColor(oldBkColor);
	}
}

void CPicEditWnd::DrawContents(CDC* pDC, const CRect& rc)
{
	pDC->FillSolidRect(&rc, RGB(49, 49, 49));
	if (!m_image.IsValid())
		return;
	
	int w = (int)(m_iWidth * m_sScale);
	int h = (int)(m_iHeight * m_sScale);
	CRect rcImg(m_dx, m_dy, m_dx + w, m_dy + h);

	if (m_drawMode == eDrawSuccess) {
		m_image.Draw(pDC->GetSafeHdc(), rcImg);
		COLORREF col = RGB(0, 255, 0);
		if (!m_bResult)
			col = RGB(255, 0, 0);
		COLORREF oldCol = pDC->SetTextColor(col);
		if(m_bResult)
			pDC->TextOut(m_dx + 10, m_dy + 10, _T("OK"));
		else
			pDC->TextOut(m_dx + 10, m_dy + 10, _T("Error: ") + m_sResult);
		pDC->SetTextColor(oldCol);
	}
	else
		m_image.Draw(pDC->GetSafeHdc(), rcImg);

	auto drawMode = m_drawMode;
	if(drawMode == eDrawSuccess) {
		drawMode = m_drawModeOld;
	}

	switch (drawMode) {
	case eDrawBaseLine: 
	{		
		if(m_baseline.size() == 1)
			DrawPoint(pDC, m_baseline[0], RGB(255, 0, 0));
		else if (m_baseline.size() == 2) {
			CPen linePen(PS_SOLID, 1, RGB(0, 0, 255));	
			CBrush brush(RGB(0, 0, 255));
			CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brush);
			CPen* pOldPen = (CPen*)pDC->SelectObject(&linePen);
			const int radius = 3;
			CPoint pt0 = Real2Screen(m_baseline[0]);
			CPoint pt1 = Real2Screen(m_baseline[1]);
			pDC->Rectangle(pt0.x - radius, pt0.y - radius, pt0.x + radius, pt0.y + radius);
			pDC->MoveTo(pt0);
			pDC->LineTo(pt1);
			pDC->Rectangle(pt1.x - radius, pt1.y - radius, pt1.x + radius, pt1.y + radius);
			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
		break;
	}
	case eDrawBasePoint:
	{
		int n = (int)m_ptBasePoint.size();
		for (int i = 0; i < n; ++i) {
			DrawPoint(pDC, m_ptBasePoint[i], RGB(255, 0, 0));
		}
		break;
	}
	case eDrawTrackRect:
	{
		CPen linePen(PS_SOLID, 1, RGB(0, 0, 255));
		CPen* pOldPen = (CPen*)pDC->SelectObject(&linePen);
		CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		m_rcTracker.Draw(pDC);	
		if (!m_TrackerRect.IsRectEmpty()) {
			CPen dashPen(PS_DOT, 1, RGB(0, 0, 255));			
			pDC->SelectObject(&dashPen);
			pDC->SelectStockObject(NULL_BRUSH);
			pDC->Rectangle(&m_TrackerRect);
		}
		int n = (int)m_baseline.size();
		for (int i = 0; i < n; i++) {
			DrawPoint(pDC, m_baseline[i], RGB(255, 0, 0), i+1);
		}
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
		break;
	}
	case eDrawDropletBaseline:
	{		
		if (m_iFitMode == eFitManual) {
			int n = (int)m_dropletPoints.size();
			for (int i = 0; i < n; i++) {
				DrawPoint(pDC, m_dropletPoints[i], RGB(0, 0, 255), i + 1);
			}
		}
		int n = (int)m_baseline.size();
		for (int i = 0; i < n; i++) {
			DrawPoint(pDC, m_baseline[i], RGB(255, 0, 0), i+1);
		}
	}
	break;
	default:
		break;
	}		
	int n = (int)m_drawItems.size();
	for (int i = 0; i < n; ++i) {
		if(m_drawItems[i].isVisible())
			m_drawItems[i].Draw(pDC, m_dx, m_dy, m_sScale);
	}
}

BOOL CPicEditWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		int iKey = -1;
		if (pMsg->wParam == VK_LEFT) {
			iKey = 0;
		}
		else if (pMsg->wParam == VK_RIGHT) {
			iKey = 1;
		}
		else if (pMsg->wParam == VK_HOME) {
			iKey = 2;
		}
		else if (pMsg->wParam == VK_END) {
			iKey = 3;
		}
		if (iKey >= 0) {

		}
	}

	return CWnd::PreTranslateMessage(pMsg);
}

bool CPicEditWnd::isInRect(const CPoint& pt) const
{
	if(m_iWidth == 0 || m_iHeight == 0)
		return false;
	CRect rc;
	GetClientRect(&rc);	
	CRect rcImg(m_dx, m_dy, rc.right - m_dx, rc.bottom - m_dy);
	if(rcImg.IsRectEmpty())
		return false;
	if (rcImg.PtInRect(pt)) {
		return true;
	}
	return false;
}

int CPicEditWnd::TrySelectPoint(const PointF& pt, const std::vector<PointF>& ptArray)
{
	for (int i = 0; i < ptArray.size(); ++i) {
		if (IsNearPoints(pt, ptArray[i])) {
			return i;
		}
	}
	return -1;
}

void CPicEditWnd::OnLButtonDown(UINT nFlags, CPoint point)
{	
	SetFocus();
	if (!isInRect(point)) {
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}	

	if(m_drawMode == eDrawSuccess) {
		m_drawMode = m_drawModeOld;		
	}

	BOOL bUpdate = FALSE;
	m_lastMousePos = point; // Save initial point for tracking
	m_bLeftDragging = FALSE;
	auto realPt = Screen2Real(point);

	switch (m_drawMode) {
	case eDrawBaseLine:
	{
		if (!m_baseline.empty()) {
			m_baseline.clear();
		}
		m_baseline.push_back(realPt);
		m_bLeftDragging = TRUE;
		bUpdate = TRUE;
		break;
	}
	case eDrawTrackRect:
	{
		if (m_rcTracker.HitTest(point) >= 0) {
			if (m_rcTracker.Track(this, point, TRUE)) {
				m_bLeftDragging = TRUE;
				m_iLeftMoveID = -1;
				EvaluationProc(); // ✅ Add here immediately after Track
			}			
		}
		else // TODO: redraw tracker
		{
			m_TrackerRect.left = point.x;
			m_TrackerRect.top = point.y;
			m_TrackerRect.right = point.x;
			m_TrackerRect.bottom = point.y;
			m_lastMousePos = point;
			m_bLeftDragging = TRUE;
			bUpdate = TRUE;
		}
		break;
	}
	case eDrawBasePoint:
	{
		int idx = TrySelectPoint(realPt, m_ptBasePoint);
		if (idx < 0) {
			if(m_ptBasePoint.size() == 2)
				m_ptBasePoint.clear();
			m_ptBasePoint.push_back(realPt);
			bUpdate = TRUE;
		}
		else {
			m_iLeftMoveID = idx;
			m_bLeftDragging = TRUE;
		}
		break;
	}
	case eDrawDropletBaseline:
	{
		int idx = TrySelectPoint(realPt, m_baseline);
		int needsPoints = 2;
		if (m_bEllipseMode) {
			if (m_dnFlag > eBaseLineCircle)
				needsPoints = 5;
			else
				needsPoints = 3;
		}

		if (idx < 0) {
			if (m_baseline.size() >= needsPoints)
				m_baseline.clear();
			
			m_baseline.push_back(realPt);
			bUpdate = TRUE;
		}
		else {
			m_iLeftMoveID = idx;
			m_bLeftDragging = TRUE;
		}
		break;
	}
	case eDrawSuccess:
		break;
	}
	
	m_bLeftButtonDown = TRUE;

	if(bUpdate)
		Invalidate();
	if(!m_bLeftDragging)
		EvaluationProc();

	CWnd::OnLButtonDown(nFlags, point);
}

void CPicEditWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!isInRect(point) || GetFocus() != this) {
		CWnd::OnMouseMove(nFlags, point);
		return;
	}
	BOOL bUpdate = FALSE;	
	auto realPt = Screen2Real(point);
	if (m_bLeftButtonDown) {
		if (m_bLeftDragging) {
			if (m_drawMode == eDrawBaseLine) {
				if(m_baseline.size() == 1) {
					m_baseline.push_back(realPt);
				}
				else
					m_baseline[1] = realPt;
				bUpdate = TRUE;
			}
			else if (m_drawMode == eDrawBasePoint) {
				if (m_iLeftMoveID >= 0 && m_iLeftMoveID < m_ptBasePoint.size()) {
					m_ptBasePoint[m_iLeftMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
			else if (m_drawMode == eDrawTrackRect) { // TODO: update tracker rect
				m_TrackerRect.left = min(m_lastMousePos.x, point.x);
				m_TrackerRect.top = min(m_lastMousePos.y, point.y);
				m_TrackerRect.right = max(m_lastMousePos.x, point.x);
				m_TrackerRect.bottom = max(m_lastMousePos.y, point.y);
				bUpdate = TRUE;
			}
			else if (m_drawMode == eDrawDropletBaseline) {
				if (m_iLeftMoveID >= 0 && m_iLeftMoveID < m_baseline.size()) {
					m_baseline[m_iLeftMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
		}		
	}
	else if (m_bRightButtonDown) {
		if (m_bRightDragging) {
			if (m_drawMode == eDrawTrackRect) {
				if (m_iRightMoveID >= 0 && m_iRightMoveID < m_baseline.size()) {
					m_baseline[m_iRightMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
			else if (m_drawMode == eDrawDropletBaseline) {
				if (m_iRightMoveID >= 0 && m_iRightMoveID < m_dropletPoints.size()) {
					m_dropletPoints[m_iRightMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
		}
	}
	
	if (bUpdate)
		Invalidate();
	CWnd::OnMouseMove(nFlags, point);
}


void CPicEditWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (!isInRect(point) || GetFocus() != this) {
		CWnd::OnLButtonUp(nFlags, point);
		return;
	}
	BOOL bUpdate = FALSE;
	auto realPt = Screen2Real(point);
	if (m_bLeftButtonDown) {
		if(m_bLeftDragging) {
			if (m_drawMode == eDrawBaseLine) {
				if (m_baseline.size() == 1) {
					m_baseline.push_back(realPt);
				}
				else
					m_baseline[1] = realPt;
				bUpdate = TRUE;
			}
			else if (m_drawMode == eDrawBasePoint) {
				if (m_iLeftMoveID >= 0 && m_iLeftMoveID < m_ptBasePoint.size()) {
					m_ptBasePoint[m_iLeftMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
			else if (m_drawMode == eDrawTrackRect) {
				if(m_TrackerRect.Width() * m_TrackerRect.Height() > 25) {
					m_rcTracker.m_rect = m_TrackerRect;					
				}
				m_TrackerRect.SetRectEmpty();
				bUpdate = TRUE;
				m_lastMousePos = { 0,0 };
			}
			else if (m_drawMode == eDrawDropletBaseline) {
				if (m_iLeftMoveID >= 0 && m_iLeftMoveID < m_baseline.size()) {
					m_baseline[m_iLeftMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
		}
		m_bLeftButtonDown = FALSE;
		m_bLeftDragging = FALSE;
	}
	if (bUpdate) {
		Invalidate();
		EvaluationProc();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CPicEditWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CString cmd_str(_T("explorer /select, ") + m_sFilename);
	CT2A pss(cmd_str);
	WinExec(pss.m_psz, SW_SHOW);

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CPicEditWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	if (!isInRect(point)) {
		CWnd::OnRButtonDown(nFlags, point);
		return;
	}

	BOOL bUpdate = FALSE;
	m_bRightDragging = FALSE;

	if (m_drawMode == eDrawSuccess) {
		m_drawMode = m_drawModeOld;
	}
	auto realPt = Screen2Real(point);
	switch (m_drawMode) {	
	/*
	case eDrawTrackRect:
	{
		int needsPoints = 2;
		if (m_bEllipseMode) {
			if (m_dnFlag > eBaseLineCircle)
				needsPoints = 5;
			else
				needsPoints = 3;
		}
		int idx = TrySelectPoint(realPt, m_baseline);
		if (idx < 0) {
			if (m_baseline.size() == needsPoints)
				m_baseline.clear();
			m_baseline.push_back(realPt);
			bUpdate = TRUE;
		}
		else {
			m_iRightMoveID = idx;
			m_bRightDragging = TRUE;
		}
		break;
	}	
	*/
	case eDrawDropletBaseline:
	{	
		int needsPoints = 2;
		
		if (m_upFlag > eDropletCircle && m_upFlag < eDropletWidthHeight)
			needsPoints = 5;
		else
			needsPoints = 3;
		if (m_upFlag == eDropletWidthHeight) {
			break;
		}
		int idx = TrySelectPoint(realPt, m_dropletPoints);
		if (idx < 0) {
			if (m_dropletPoints.size() >= needsPoints)
				m_dropletPoints.clear();

			m_dropletPoints.push_back(realPt);
			bUpdate = TRUE;
		}
		else {
			m_iRightMoveID = idx;
			m_bRightDragging = TRUE;
		}
		break;
	}
	case eDrawSuccess:
		break;
	}

	m_bRightButtonDown = TRUE;

	if (bUpdate)
		Invalidate();
	if (!m_bRightDragging)
		EvaluationProc();

	CWnd::OnRButtonDown(nFlags, point);
}

void CPicEditWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!isInRect(point) || GetFocus() != this) {
		CWnd::OnRButtonUp(nFlags, point);
		return;
	}
	BOOL bUpdate = FALSE;
	auto realPt = Screen2Real(point);
	if (m_bRightButtonDown) {
		if (m_bRightDragging) {
			if (m_drawMode == eDrawTrackRect) {
				if (m_iRightMoveID >= 0 && m_iRightMoveID < m_baseline.size()) {
					m_baseline[m_iRightMoveID] = realPt;
					bUpdate = TRUE;
				}
			}			
			else if (m_drawMode == eDrawDropletBaseline) {
				if (m_iRightMoveID >= 0 && m_iRightMoveID < m_dropletPoints.size()) {
					m_dropletPoints[m_iRightMoveID] = realPt;
					bUpdate = TRUE;
				}
			}
		}
		m_bRightButtonDown = FALSE;
		m_bRightDragging = FALSE;
	}
	if (bUpdate) {
		Invalidate();
		EvaluationProc();
	}

	CWnd::OnRButtonUp(nFlags, point);
}

BOOL CPicEditWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{	
	if (pWnd == this && m_drawMode == eDrawTrackRect && m_rcTracker.SetCursor(this, nHitTest))
		return TRUE;	

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CPicEditWnd::SetBasePoint(double x0, double y0, double x1, double y1)
{
	m_ptBasePoint.clear();
	m_ptBasePoint.resize(2);
	m_ptBasePoint[0].x = x0;
	m_ptBasePoint[0].y = y0;
	m_ptBasePoint[1].x = x1;
	m_ptBasePoint[1].y = y1;
	m_drawMode = eDrawBasePoint;

	Invalidate();
}

bool CPicEditWnd::GetBasePoint(double* x0, double* y0, double* x1, double* y1) const
{
	bool res = false;
	if (x0 && y0 && x1 && y1 && m_ptBasePoint.size() > 1 && m_sScale != 0) {
		*x0 = m_ptBasePoint[0].x;
		*y0 = m_ptBasePoint[0].y;
		*x1 = m_ptBasePoint[1].x;
		*y1 = m_ptBasePoint[1].y;
		res = true;
	}
	return res;
}

bool CPicEditWnd::GetLineSegment(double* x0, double* y0, double* x1, double* y1) const
{
	bool res = false;
	if (x0 && y0 && x1 && y1 && m_baseline.size() > 1 && m_sScale != 0) {
		*x0 = m_baseline[0].x;
		*y0 = m_baseline[0].y;
		*x1 = m_baseline[1].x;
		*y1 = m_baseline[1].y;
		res = true;
	}
	return res;
}

bool CPicEditWnd::GetDropletPoints(int* nCount, PointF** pts) const
{
	bool res = false;
	if (nCount == NULL || m_sScale == 0)
		return res;

	*nCount = (int)m_dropletPoints.size();

	if (pts == NULL || m_dropletPoints.size() == 0)
		return res;
	*pts = new PointF[*nCount];
	for (int i = 0; i < *nCount; ++i) {
		(*pts)[i] = {m_dropletPoints[i].x, m_dropletPoints[i].y};		
	}
	res = true;
	return res;
}

bool CPicEditWnd::GetBaseLinePoints(int* nCount, PointF** pts) const
{
	bool res = false;
	if (nCount == NULL || m_sScale == 0)
		return res;

	*nCount = (int)m_baseline.size();

	if (pts == NULL || m_baseline.size() == 0)
		return res;
	*pts = new PointF[*nCount];
	for (int i = 0; i < *nCount; ++i) {
		(*pts)[i] = {m_baseline[i].x, m_baseline[i].y};		
	}
	res = true;
	return res;
}

bool CPicEditWnd::GetTrackerRect(CRect* rc) const
{
	if (rc == NULL || m_sScale == 0.0)
		return false;

	CRect rt = m_rcTracker.m_rect;
	bool res = false;
	CRect rcImg(
		(int)((double)(rt.left - m_dx) / m_sScale),
		(int)((double)(rt.top - m_dy) / m_sScale),
		(int)((double)(rt.right - m_dx) / m_sScale),
		(int)((double)(rt.bottom - m_dy) / m_sScale));
	if (rc) {
		*rc = rcImg;
		res = true;
	}
	return res;
}

void CPicEditWnd::SetDrawingMode(FitMode _fitMode, BOOL _bEllipseMode, BOOL _bHasPin, DropletFitMode _upFlag, BaseLineFitMode _dnFlag)
{
	BOOL bChanged =
		(m_iFitMode != _fitMode) ||
		(m_bEllipseMode != _bEllipseMode) ||
		(m_bHasPin != _bHasPin) ||
		(m_upFlag != _upFlag) ||
		(m_dnFlag != _dnFlag);
	
	m_iFitMode = _fitMode;
	m_bEllipseMode = _bEllipseMode;
	m_bHasPin = _bHasPin;
	m_upFlag = _upFlag;
	m_dnFlag = _dnFlag;

	switch(_fitMode) {
	case eFitAuto:
		m_drawMode = eDrawNone;
		break;
	case eFitByBasePoint:
		m_drawMode = eDrawBasePoint; // Manual mode (2 points)
		break;
	case eFitByLine:
		m_drawMode = eDrawBaseLine; // Semi-auto mode (2 points)
		break;
	case eFitSemiAuto:
	case eFitManual:
		if (_upFlag == eDropletWidthHeight)
			m_drawMode = eDrawTrackRect; // Width/Height mode
		else
			m_drawMode = eDrawDropletBaseline; // Manual draw both droplet + baseline
		break;
	default:
		m_drawMode = eDrawNone;
		break;
	}

	if (bChanged && GetSafeHwnd() && IsWindow(GetSafeHwnd())) {
		Invalidate();
	}
}

bool CPicEditWnd::IsReadyToEvaluate() const
{
	switch (m_drawMode) {
	case eDrawBaseLine: 
		return m_baseline.size() >= 2;
	case eDrawBasePoint: 
		return m_ptBasePoint.size() >= 2;
	case eDrawTrackRect: {
		if (m_bEllipseMode)
			return m_baseline.size() >= (m_dnFlag > eBaseLineCircle ? 5 : 3);
		return !m_rcTracker.m_rect.IsRectEmpty();
	}
	case eDrawDropletBaseline: {
		int baseCount = m_bEllipseMode ? (m_dnFlag > eBaseLineCircle ? 5 : 3) : 2;
		int dropCount = (m_upFlag > eDropletCircle ? 5 : 3);
		bool ready = (m_baseline.size() >= baseCount);
		if (m_iFitMode == eFitManual)
			ready = ready && (m_dropletPoints.size() >= dropCount);
		return ready;
	}
	default:
		return false;
	}
}

void CPicEditWnd::EvaluationProc()
{	
	auto bReady = IsReadyToEvaluate();
	if (bReady && GetParent()) {
		GetParent()->PostMessage(WM_READY_RECORG_MSG, m_id, 0); // Notify parent window
	}
}

void CPicEditWnd::AddPoint(const PointF& pt, bool visible)
{
	CDrawItems item;
	item.AddPoint(pt, eLayerBasePt);
	item.setVisible(visible);
	m_drawItems.push_back(item);
}

void CPicEditWnd::AddAngleLine(const PointAndAngle& pt1, bool visible)
{
	CDrawItems item;
	item.AddAngleLine(pt1, eLayerAngle);
	item.setVisible(visible);
	m_drawItems.push_back(item);
}

void CPicEditWnd::AddEllipse(const EllipseFitBox& ellipse, DrawObjLayer layer, COLORREF col, bool visible)
{
	CDrawItems item;
	item.AddEllipse(ellipse, layer, col);
	item.setVisible(visible);
	m_drawItems.push_back(item);
}

void CPicEditWnd::AddPolyline(const std::vector<PointF>& pts, DrawObjLayer layer, COLORREF col, bool visible)
{
	CDrawItems item;
	item.AddPolyline(pts, layer, col);
	item.setVisible(visible);
	m_drawItems.push_back(item);
}

void CPicEditWnd::AddRectangle(const EllipseFitBox& rc, DrawObjLayer layer, COLORREF col, bool visible)
{
	CDrawItems item;
	item.AddRectangle(rc, layer, col);
	item.setVisible(visible);
	m_drawItems.push_back(item);
}

void CPicEditWnd::ClearDrawItems()
{
	for(auto& item : m_drawItems)
		item.Clear();
	m_drawItems.clear();
}

void CPicEditWnd::UpdateLayerVisibility(DrawObjLayer layer, bool bVisible)
{
	int n = (int)m_drawItems.size();
	for (int i = 0; i < n; ++i) {
		if (m_drawItems[i].getLayer() == layer) {
			m_drawItems[i].setVisible(bVisible);
		}
	}
	if (GetSafeHwnd() && IsWindow(GetSafeHwnd())) {
		Invalidate();
	}
}

//.EOF
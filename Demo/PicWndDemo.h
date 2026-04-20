/******************************************************************************
\author	Jewel
\date	10/17/2019
******************************************************************************/

#pragma once
#include <vector>
//#include <afxwin.h>
#include <afxext.h>
//#define _MSC_STDINT_H_		//  [9/26/2019 Jewel]

#include "ximage/ximage.h"
#pragma comment(lib, "cximagecrtu.lib")
#include "common.h"
#include <vector>
#include "drawitem.h"
#define DIFF			6

const UINT WM_READY_RECORG_MSG = ::RegisterWindowMessage(_T("WM_READY_RECORG_MSG"));

// CPicWnd
typedef enum DrawMode {
	eDrawNone,
	eDrawBaseLine,
	eDrawTrackRect,
	eDrawBasePoint,
	eDrawDropletBaseline,
	eDrawSuccess,
	eDrawModeCount
}eDrawMode;

class CPicEditWnd : public CWnd
{
	DECLARE_DYNAMIC(CPicEditWnd)

public:
	CPicEditWnd();
	virtual ~CPicEditWnd();

	BOOL CreateWnd(CWnd* pParent, const RECT& rc, UINT nID, int id);
	void SetImage(const CString& strFile, bool bUpdate = true);

	int GetRealWidth() const { return m_iWidth; }
	int GetRealHeight() const { return m_iHeight; }
		
	void ShowResult(bool success, const CString& msg);
	void SetDrawingMode(FitMode _fitMode, BOOL _bEllipseMode, BOOL _bHasPin, DropletFitMode _upFlag, BaseLineFitMode _dnFlag);

	bool GetTrackerRect(CRect* rc) const;

	void SetBasePoint(double x0, double y0, double x1, double y1);
	bool GetBasePoint(double* x0, double* y0, double* x1, double* y1) const;
	bool GetLineSegment(double* x0, double* y0, double* x1, double* y1) const;

	bool GetDropletPoints(int* nCount, PointF** pts) const;
	bool GetBaseLinePoints(int* nCount, PointF** pts) const;

	void AddPoint(const PointF& pt, bool visible = true);
	void AddAngleLine(const PointAndAngle& pt1, bool visible = true);
	void AddEllipse(const EllipseFitBox& ellipse, DrawObjLayer layer, COLORREF col, bool visible = true);
	void AddPolyline(const std::vector<PointF>& pts, DrawObjLayer layer, COLORREF col, bool visible = true);
	void AddRectangle(const EllipseFitBox& rc, DrawObjLayer layer, COLORREF col, bool visible = true);
	void ClearDrawItems();
	void UpdateLayerVisibility(DrawObjLayer layer, bool visible);

private:
	void calculateScale(int cx, int cy);
	bool isInRect(const CPoint& pt) const;
	void ClearState();
	void DrawPoint(CDC* pDC, const PointF& pt, COLORREF color, int index = 0, int radius = 6);
	int TrySelectPoint(const PointF& pt, const std::vector<PointF>& ptArray);
	void EvaluationProc();
	PointF Screen2Real(const CPoint& pt) const;
	CPoint Real2Screen(const PointF& pt) const;
	bool IsReadyToEvaluate() const;
private:
	std::vector<CDrawItems> m_drawItems;
	CString	m_sFilename;
	CxImage m_image;

	int		m_iWidth;
	int		m_iHeight;
	float	m_sScale;
	int		m_dx;
	int		m_dy;	
	int		m_id;
	bool	m_processed;
	bool	m_success;
	
	CRectTracker		m_rcTracker;	
	CRect				m_TrackerRect;
	CPoint				m_lastMousePos;

	std::vector<PointF>	m_dropletPoints;
	std::vector<PointF>	m_baseline;
	std::vector<PointF>	m_ptBasePoint;

	DrawMode		m_drawMode;
	DrawMode		m_drawModeOld;
	int				m_iLeftMoveID;
	BOOL			m_bLeftButtonDown;
	BOOL			m_bTracking;
	BOOL			m_bLeftDragging;
	BOOL			m_bRightDragging;
	BOOL			m_bRightButtonDown;
	int				m_iRightMoveID;

	FitMode			m_iFitMode;
	BOOL 			m_bEllipseMode;
	BOOL 			m_bHasPin;
	DropletFitMode	m_upFlag;
	BaseLineFitMode m_dnFlag;

	bool			m_bResult;
	CString			m_sResult;
protected:
	void DrawContents(CDC* pDC, const CRect& rc);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
//.EOF
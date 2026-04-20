#ifndef __JIAOSHUI_APP_DRAWITEM_H__
#define __JIAOSHUI_APP_DRAWITEM_H__
#include <vector>
#include "common.h"
#include <variant>
#include <utility>

const COLORREF COLOR_POINT = RGB(255, 0, 0);
const COLORREF COLOR_ANGLE = RGB(0, 255, 0);
const COLORREF COLOR_LEFT_FIT = RGB(0, 0, 255);
const COLORREF COLOR_RIGHT_FIT = RGB(0, 255, 255);
const COLORREF COLOR_BASELINE = RGB(255, 0, 255);

typedef enum DrawObjType {
	eObjNone = -1,
	eObjPoint = 0,
	eObjCircle = 1,
	eObjEllipse = 2,
	eObjRectangle = 3,
	eObjPoliline = 4,
	eObjAngleLine = 5,
	eObjCount,
}eDrawObjectType;

typedef enum DrawObjLayer {
	eLayerNone = -1,
	eLayerAngle = 0,
	eLayerFit = 1,
	eLayerBasePt = 2,
	eLayerCount
}drawObjectLayer;

using drawObject = std::variant<
	PointF,
	RectangleF,
	EllipseFitBox,
	std::pair<PointF, double>,         // Circle: center + radius
	PointAndAngle,
	std::vector<PointF>                // Polyline
>;

class CDrawItems {
public:
	CDrawItems();
	virtual ~CDrawItems();
	void Clear();
	void AddPoint(const PointF& pt, DrawObjLayer layer);
	void AddAngleLine(const PointAndAngle& pt1, DrawObjLayer layer);
	void AddCircle(const PointF& center, double radius, DrawObjLayer layer, COLORREF col);
	void AddEllipse(const EllipseFitBox& ellipse, DrawObjLayer layer, COLORREF col);
	void AddPolyline(const std::vector<PointF>& pts, DrawObjLayer layer, COLORREF col);
	void AddRectangle(const EllipseFitBox& rc, DrawObjLayer layer, COLORREF col);
	void Draw(CDC* pDC, int dx, int dy, float scale);
	DrawObjLayer getLayer() const { return _layer; }
	void setVisible(bool visible) { _isVisible = visible; }
	bool isVisible() const { return _isVisible; }
private:
	DrawObjType type;
	DrawObjLayer _layer;
	COLORREF color;
	drawObject obj;
	bool _isVisible;
};


#endif//__JIAOSHUI_APP_DRAWITEM_H__
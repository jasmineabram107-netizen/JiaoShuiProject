#include "pch.h"
#include "drawitem.h"
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

CDrawItems::CDrawItems()	
{
	Clear();
}

CDrawItems::~CDrawItems()
{
	Clear();
}

void CDrawItems::Clear()
{
	type = eObjNone;
	color = RGB(0, 0, 0);
	obj = {};
	_layer = eLayerNone;
	_isVisible = true;
}
void CDrawItems::AddPoint(const PointF& pt, DrawObjLayer layer)
{
	type = eObjPoint;
	color = COLOR_POINT;
	obj = pt;
	_layer = layer;
}

void CDrawItems::AddAngleLine(const PointAndAngle& pt1, DrawObjLayer layer)
{
	type = eObjAngleLine;
	color = COLOR_ANGLE;
	obj = pt1;
	_layer = layer;
}

void CDrawItems::AddCircle(const PointF& center, double radius, DrawObjLayer layer, COLORREF col)
{
	type = eObjCircle;
	color = col;
	obj = std::make_pair(center, radius);
	_layer = layer;
}

void CDrawItems::AddEllipse(const EllipseFitBox& ellipse, DrawObjLayer layer, COLORREF col)
{
	type = eObjEllipse;
	color = col;
	obj = ellipse;
	_layer = layer;
}

void CDrawItems::AddPolyline(const std::vector<PointF>& pts, DrawObjLayer layer, COLORREF col)
{
	type = eObjPoliline;
	color = col;
	obj = pts;
	_layer = layer;
}

void CDrawItems::AddRectangle(const EllipseFitBox& rc, DrawObjLayer layer, COLORREF col)
{
	type = eObjRectangle;
	color = col;
	obj = rc;
	_layer = layer;
}

void CDrawItems::Draw(CDC* pDC, int dx, int dy, float scale)
{
	if (type == eObjNone)
		return;

	Gdiplus::Graphics graphics(pDC->GetSafeHdc());
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	Gdiplus::Pen pen(
		Gdiplus::Color(255, GetRValue(color), GetGValue(color), GetBValue(color)), 
		2.0f
	);
	pen.SetStartCap(Gdiplus::LineCapRound);
	pen.SetEndCap(Gdiplus::LineCapRound);

	switch (type)
	{
	case eObjPoint:
	{
		if (auto pt = std::get_if<PointF>(&obj)) {
			Gdiplus::PointF p((*pt).x * scale + dx, (*pt).y * scale + dy);
			graphics.DrawLine(&pen, p.X - 3, p.Y - 3, p.X + 3, p.Y + 3);
			graphics.DrawLine(&pen, p.X - 3, p.Y + 3, p.X + 3, p.Y - 3);
			// graphics.FillEllipse(&Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 0, 0)), p.x - 2, p.y - 2, 4, 4);
		}
		break;
	}
	case eObjCircle:
	{
		if (auto circle = std::get_if<std::pair<PointF, double>>(&obj)) {
			PointF center = circle->first;
			double r = circle->second;
			Gdiplus::RectF rect(
				(center.x - r) * scale + dx,
				(center.y - r) * scale + dy,
				(float)(2 * r * scale),
				(float)(2 * r * scale)
			);
			graphics.DrawEllipse(&pen, rect);
		}
		break;
	}
	case eObjEllipse:
	{
		if (auto ellipse = std::get_if<EllipseFitBox>(&obj)) {
			const auto& e = *ellipse;
			Gdiplus::Matrix m;
			m.RotateAt(
				(Gdiplus::REAL)e.angle,
				Gdiplus::PointF(
					(Gdiplus::REAL)(e.center.x * scale + dx), 
					(Gdiplus::REAL)(e.center.y * scale + dy)
				)
			);
			graphics.SetTransform(&m);
			Gdiplus::RectF rect(
				(e.center.x - e.a) * scale + dx,
				(e.center.y - e.b) * scale + dy,
				(float)(2 * e.a * scale),
				(float)(2 * e.b * scale)
			);
			graphics.DrawEllipse(&pen, rect);
			graphics.ResetTransform();
		}
		break;
	}
	case eObjRectangle:
	{
		if (auto rc = std::get_if<EllipseFitBox>(&obj)) {
			const auto& e = *rc;
			Gdiplus::Matrix m;
			m.RotateAt(
				(Gdiplus::REAL)e.angle,
				Gdiplus::PointF(
					(Gdiplus::REAL)(e.center.x * scale + dx),
					(Gdiplus::REAL)(e.center.y * scale + dy)
				)
			);
			graphics.SetTransform(&m);
			Gdiplus::RectF rect(
				(e.center.x - e.a / 2.0) * scale + dx,
				(e.center.y - e.b / 2.0) * scale + dy,
				(e.a) * scale,
				(e.b) * scale
			);			
			
			graphics.DrawRectangle(&pen, rect);
			graphics.ResetTransform();
		}
		break;
	}
	case eObjPoliline:
	{
		if (auto poly = std::get_if<std::vector<PointF>>(&obj)) {
			if (poly->size() >= 2) {
				std::vector<Gdiplus::PointF> scaled;
				for (const auto& p : *poly)
					scaled.emplace_back(p.x * scale + dx, p.y * scale + dy);
				graphics.DrawLines(&pen, scaled.data(), (INT)scaled.size());
			}
		}
		break;
	}
	case eObjAngleLine:
	{
		const double radius = 40.0; // 20 pixels
		if (auto line = std::get_if<PointAndAngle>(&obj)) {
			Gdiplus::PointF p0 = Gdiplus::PointF(line->point.x * scale + dx, line->point.y * scale + dy);
			Gdiplus::PointF p1 = Gdiplus::PointF(line->vec1.x * radius * scale, line->vec1.y * radius * scale);
			Gdiplus::PointF p2 = Gdiplus::PointF(line->vec2.x * radius * scale, line->vec2.y * radius * scale);
			graphics.DrawLine(&pen, p0, p0 + p1);
			graphics.DrawLine(&pen, p0, p0 + p2);
		}
		break;
	}
	default:
		break;
	}
}

//.EOF
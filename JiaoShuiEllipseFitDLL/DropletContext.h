#ifndef __JIAUOSHUILIB_DROPLETCTX_H__
#define __JIAUOSHUILIB_DROPLETCTX_H__
#include <opencv2/opencv.hpp>
#include "common.h"
#include <tchar.h>

class CDropletContext {
public:
	CDropletContext();
	virtual ~CDropletContext();

	void InitVariables();

	void SetBaselineShape(BaseLineShapeKind _val);
	BaseLineShapeKind baselineShape() const { return m_baseLineShapeKind; }

	void SetMainShape(MainShapeType _val);
	MainShapeType mainShape() const { return m_mainShape; }

	FitMode fitMode() const { return m_fitMode; }

	bool LoadImage(const TCHAR* path);
	inline cv::Mat& Image() { return m_frame; }
	bool IsEmptyImage() const;

	void SetModes(DropletFitMode dFit, BaseLineFitMode bFit, bool subPixel, FitMode fitMode);
	void SetModes(bool subPixel, FitMode fitMode);
	void SetModes(DropletFitMode dFit, BaseLineFitMode bFit);
	void SetHorizontalMode(HorizonMode _newMode) { m_horizonMode = _newMode; }
	HorizonMode GetHorizontalMode() const { return m_horizonMode; }


	void BuildShapeTypeFromDetailedShape();

	bool IsSubpixel() const { return m_bUseSubpixel; }

	bool HasPin() const { return m_hasPin; }
	void SetPin(bool _val) {
		if (_val != m_hasPin) 
			m_hasPin = _val; 
	}

	DropletFitMode DropletMode() const {
		return m_dropletFitMode;
	}

	BaseLineFitMode BaselineMode() const {
		return m_baseLineFitMode;
	}
	bool IsEmptyDroplet() const { return m_isEmpty; }

	void UpdateBaserect(const RectangleF& _val);
	RectangleF& baseRect() { return m_baseRect; }

	cv::Point2f leftPoint() const { return m_ptLeft; }
	cv::Point2f rightPoint() const { return m_ptRight; }
	void SetLeftRightPoints(const cv::Point2f& l, const cv::Point2f& r);

	bool isEqualLRPoints() const {
		return (m_ptLeft.x == m_ptRight.x);
	}

	int ImageWidth() const {
		return m_frame.cols;
	}

	int ImageHeight() const {
		return m_frame.rows;
	}

	void ClearResultVariables();
	std::vector<cv::Point>& DropletPtRef() {
		return m_outDropletPts;
	}
	std::vector<cv::Point>& BaelinePtRef() {
		return m_outBaselinePts;
	}

	// 0： 液滴左边（椭圆， 圆， 高宽法矩形）， 1： 液滴右边(双椭圆，双圆)， 2： 基线
	void SetBox(int idx, const cv::RotatedRect& box) {
		if (idx >= 0 && idx < 3) {
			m_outBoxes[idx] = box;
			if (idx == 2 && m_mainShape == eShapeHorizontal) {
				m_outBoxes[2].size.height = m_outBoxes[2].size.width = 0;
			}
		}
	}
	const cv::RotatedRect* GetBoxes() const { return m_outBoxes; }

	void SetAnglePoints(const std::vector<PointAndAngle>& pts) 
	{		
		m_angles = pts;
		if(m_angles.size() == 2) {
			if (m_angles[1].point.x < m_angles[0].point.x) {
				std::swap(m_angles[0], m_angles[1]);
			}			
		}
	}
	void SetAngles(const double* angles)
	{
		if (angles) {
			m_outAngles[0] = angles[0];
			m_outAngles[1] = angles[1];
		}
	}
	const double* GetAngles() const { return m_outAngles; }
	const std::vector<PointAndAngle>& GetAnglePoints() const { return m_angles; }

	std::vector<cv::Point>& LeftPolyPtRef() {
		return m_leftPolyline;
	}

	std::vector<cv::Point>& RightPolyPtRef() {
		return m_rightPolyline;
	}
protected:
	cv::Mat m_frame;
	cv::Point2f m_ptLeft;
	cv::Point2f m_ptRight;
	RectangleF	m_baseRect;

	MainShapeType m_mainShape;
	DropletFitMode m_dropletFitMode;
	BaseLineFitMode m_baseLineFitMode;
	BaseLineShapeKind	m_baseLineShapeKind;

	FitMode m_fitMode;
	bool m_bUseSubpixel;
	bool m_hasPin;
	bool m_isEmpty;
	HorizonMode				m_horizonMode;

	double						m_outAngles[2];
	std::vector<PointAndAngle>	m_angles;
	cv::RotatedRect				m_outBoxes[3];	// 0： 液滴左边（椭圆， 圆， 高宽法矩形）， 1： 液滴右边(双椭圆，双圆)， 2： 基线

	std::vector<cv::Point>		m_outDropletPts;
	std::vector<cv::Point>		m_outBaselinePts;

	std::vector<cv::Point> m_leftPolyline;		// left points on polymonial fitting
	std::vector<cv::Point> m_rightPolyline;		// right points on polymonial fitting
};

#endif//__JIAUOSHUILIB_DROPLETCTX_H__
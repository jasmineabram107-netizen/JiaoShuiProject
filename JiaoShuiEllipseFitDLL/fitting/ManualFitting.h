#ifndef __FITTING_MANUAL_H__
#define __FITTING_MANUAL_H__
#include "opencv2\opencv.hpp"
#include "common.h"
#include "fitting_defines.h"


constexpr double INVALID_YEDI_VALUE = -10.0;

/**
*	手动接触角计算函数*
*	up_flag：液滴的拟合算法（0=圆，1=椭圆，2=opencv）
*   dn_flag：基线的拟合算法（0=圆，1=椭圆，2=opencv）
*	lower_shape： 曲面的形状 （ 0=凸面， 1=凹面，2=水平面）
*	baseline_x: 基线点 x坐标指针
*   baseline_y: 基线点 y坐标指针
*   yedi_x: 液滴点 x坐标指针
*   yedi_y: 液滴点 y坐标指针
*	outputs: 接触角（左，右）
*
*   返回值
	0：成功
	1：全局报错
	2：打不开图片
	3：圆拟合失败
	4：无法检测交叉点
	5：只检测到一个交叉点
*/
class CManualFitting
{
public: 
	CManualFitting();
	virtual ~CManualFitting();
	void setParam(cv::Mat& frame,
		mainShapeType lower_shape,
		DropletFitMode yediFlag, BaseLineFitMode baselineFlag,
		double* baseline_x, double* baseline_y,
		double* yedi_x, double* yedi_y);

	ErrorCode Process(double* _outAngle, std::vector<PointAndAngle>& res, cv::RotatedRect* _outBoxes = NULL);
private:
	ErrorCode handleCircleCircleFitting();
	ErrorCode handleCircleEllipseFitting();
	ErrorCode handleEllipseCircleFitting();
	ErrorCode handleEllipseEllipseFitting();
	ErrorCode handleHorizontalCircle();
	ErrorCode handleDropletWidthHeight();
	ErrorCode handleHorizontalEllipse();
private:
	cv::Mat m_frame;
	mainShapeType m_lower_shape;
	DropletFitMode m_yediFlag;
	BaseLineFitMode m_baselineFlag;
	double* m_baseline_x;
	double* m_baseline_y;
	double* m_dropletX;
	double* m_dropletY;

	std::vector<PointAndAngle> m_res;
	int		m_length;

	cv::RotatedRect m_Boxup;
	cv::RotatedRect m_Boxdn;
};
#endif//__FITTING_MANUAL_H__
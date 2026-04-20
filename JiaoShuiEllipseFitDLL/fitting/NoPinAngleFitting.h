#ifndef __JIAOSHUI_DROPLETANAL_H__
#define __JIAOSHUI_DROPLETANAL_H__

#include "DropletFittingBase.h"

/**
*	接触角计算函数
*
*	up_flag：液滴的拟合算法（0=>圆, 1=>CV椭圆拟合, 2=>CV椭圆拟合(AMS), 3=>CV椭圆拟合(Direct), 4=>双椭圆, 5=>双多项式, 6=>高宽法, 7=>多项式（极坐））
*   dn_flag：基线的拟合算法（0=圆，1=椭圆，2=opencv）
*	lower_shape： 曲面的形状 （ 0=凸面， 1=凹面，2=水平面）
*   fit_mode: 拟合模式 (0=>自动拟合，1=>手动拟合, 2=>半自动(液滴自动，基线手动))
*	baseline_x: 基线点 x坐标指针
*   baseline_y: 基线点 y坐标指针
*   yedi_x: 液滴点 x坐标指针
*   yedi_y: 液滴点 y坐标指针
*	outputs: 接触角（左，右）
*   bUseSubpixel：用亚像素为边缘检测
*
*   返回值
	0：成功
	1：全局报错
	2：打不开图片
	3：圆拟合失败
	4：无法检测交叉点
	5：只检测到一个交叉点
*/

class CNoPinAngleFitting : public CDropletFittingBase
{
public:
	CNoPinAngleFitting(CDropletContext* pCtx);
	virtual ~CNoPinAngleFitting();

	virtual ErrorCode process();
private:
	bool prepareContours(
		std::vector<cv::Point>& _outDropletPts,
		std::vector<cv::Point>& _outBaselinePts,
		std::vector<cv::Point2f>& fpts_up, 
		std::vector<cv::Point2f>& fpts_dn,
		int& left_cnt);
	bool fitBaseline(
		const std::vector<cv::Point2f>& fpts_dn,
		cv::RotatedRect& box_dn
	);
	bool fitDroplet(
		const std::vector<cv::Point2f>& fpts_up,
		cv::RotatedRect& box_up
	);

	bool findIntersections(
		const cv::RotatedRect& box_dn,
		const std::vector<cv::Point2f>& fpts_up,
		cv::RotatedRect& box_up, int& left_cnt,
		std::vector<PointAndAngle>& _outRes
	);
	int find_contour_points_shuipingmian(std::vector<cv::Point>& pts_up);
	int find_contour_points_aomian(
		std::vector<cv::Point>& pts_up,
		std::vector<cv::Point>& pts_dn
	);
	int find_contour_points_tumian(
		std::vector<cv::Point>& pts_up,
		std::vector<cv::Point>& pts_dn
	);

	int get_max_contour(
		std::vector<std::vector<cv::Point>>& _contours,
		std::vector<int>& _traces,
		bool findOnlyConvex = false
	);

	std::vector<PointAndAngle> doubleCircleFitOnCurved(
		const std::vector<cv::Point2f>& fpts_up,
		int& left_cnt,
		cv::RotatedRect& box_up,
		const cv::RotatedRect& box_dn
	);

	std::vector<PointAndAngle> doubleCircleFitOnHorizontal(
		const std::vector<cv::Point2f>& fpts_up,
		int& left_cnt,
		cv::RotatedRect& box_up,
		const cv::RotatedRect& box_dn
	);

	std::vector<PointAndAngle> doublePolynomialFit(
		const std::vector<cv::Point2f>& fpts_up,
		int& left_cnt,
		cv::RotatedRect& box_up,
		const cv::RotatedRect& box_dn,
		bool horizontal
	);

	std::vector<PointAndAngle> polynomialFit(
		const std::vector<cv::Point2f>& fpts_up,
		int& left_cnt,
		cv::RotatedRect& box_up,
		const cv::RotatedRect& box_dn,
		bool horizontal
	);

	void get_boundary_xpos_on_contour(
		int start,
		int end,
		const std::vector<cv::Point2f>& fpts_up,
		float& _xpos,
		float& _ypos,
		bool left
	);
	void get_polar_array_on_contour(
		const std::vector<cv::Point2f>& fpts_up,
		bool isLeft,
		int left_cnt,		
		double xc, 
		double yc, 
		int pointCnt,
		std::vector<double>& r,
		std::vector<double>& theta,
#if DEBUG_IMG
		std::vector<cv::Point> &dbgPts, 		
#endif
		int maxDist = 100
	);	
	void get_xy_array_on_contour(
		const std::vector<cv::Point2f>& fpts_up,
		bool isLeft,
		int left_cnt,
		int pointCnt,
		std::vector<double>& x,
		std::vector<double>& y,
		float& min_y,
		float& max_y,
		float& argmaxy_x,
		int maxDist = 100
	);

};

#endif//__JIAOSHUI_DROPLETANAL_H__
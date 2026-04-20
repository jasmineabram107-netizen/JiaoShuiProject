#ifndef __PIN_ANGLE_FITTING_H__
#define __PIN_ANGLE_FITTING_H__
#include "DropletFittingBase.h"
//#include "fitting_defines.h"
//#include "fit_util.h"

/**
*	接触角计算函数-插针
*
*	up_flag：液滴的拟合算法（4=>双椭圆, 7=>多项式（极坐））
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
class CPinAngleFitting : public CDropletFittingBase{
public:
    CPinAngleFitting(CDropletContext* pCtx);
    virtual ~CPinAngleFitting();

    virtual ErrorCode process();
private:
    // Member variables for parameters
//    std::vector<PointAndAngle> m_outAngleRes;
//   double m_outputs[2];

//    std::vector<cv::Point> m_ptsUp, m_ptsDn;
private:
    // Internal helper methods
    bool prepareContours(
        std::vector<cv::Point2f>& fpts_l, 
        std::vector<cv::Point2f>& fpts_r, 
        std::vector<cv::Point2f>& fpts_dn
    );
    bool fitBaseline(
        const std::vector<cv::Point2f>& fpts_dn, 
        cv::RotatedRect& box_dn
    );
    bool fitDroplet(
        const cv::RotatedRect& box_dn, 
        std::vector<cv::Point2f>& fpts_l, 
        std::vector<cv::Point2f>& fpts_r, 
        cv::RotatedRect& box_up,
        std::vector<PointAndAngle>& _outRes
    );

    std::vector<PointAndAngle> polynomialFit(
        const std::vector<cv::Point2f>& fpts_l,
        const std::vector<cv::Point2f>& fpts_r,
        cv::RotatedRect& box_up,
        const cv::RotatedRect& box_dn
    );
    std::vector<PointAndAngle> doublePolynomialFit(
        const std::vector<cv::Point2f>& fpts_l,
        const std::vector<cv::Point2f>& fpts_r,
        cv::RotatedRect& box_up,		    // 上半部分拟合结果
        const cv::RotatedRect& box_dn		// 下半部分拟合结果
    );
    void get_boundary_xpos_on_contour(
        const std::vector<cv::Point2f>& fpts_up,
        float& _xpos,
        float& _ypos,
        bool left
    );
    void get_polar_array_on_contour(
        const std::vector<cv::Point2f>& fpts,
        bool isLeft,
        double xc,
        double yc,
        int pointCnt,
        std::vector<double>& r,
        std::vector<double>& theta,
        int maxDist = 100
    );
    void get_xy_array_on_contour(
        const std::vector<cv::Point2f>& fpts_up,
        bool isLeft,
        int maxPtCnt,
        std::vector<double>& x,
        std::vector<double>& y,
        float& min_y,
        float& max_y,
        float& argmaxy_x,
        int maxDist = 100
    );
};

#endif // __PIN_ANGLE_FITTING_H__

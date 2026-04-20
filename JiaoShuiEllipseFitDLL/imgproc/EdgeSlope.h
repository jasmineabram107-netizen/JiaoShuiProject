#ifndef __JIAOSHUILIB_EDGESCOPE_H__
#define __JIAOSHUILIB_EDGESCOPE_H__
#include <vector>
#include <opencv2/opencv.hpp>

class SlopeIntegral
{
public:    
    SlopeIntegral();
    virtual ~SlopeIntegral();
    void build(const std::vector<cv::Point>& pts, int wndRadius);
    double getSlopeAt(
        int centerIdx,
        bool getActualSlope = true
    );
private:
    cv::Point2f fitDirection(int i, int j) const;
    std::vector<cv::Point2f> subpixelSmoothFit(
        const std::vector<cv::Point>& pts,
        int windowRadius = 4  // total window = 2r + 1
    )const ;
    int m_wndowRadius;
    std::vector<double> sumX, sumY, sumXX, sumXY;
};

#endif//__JIAOSHUILIB_EDGESCOPE_H__
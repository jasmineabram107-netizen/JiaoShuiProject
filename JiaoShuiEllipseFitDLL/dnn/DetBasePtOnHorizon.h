#ifndef __FENXI_DETECTOR_HOR_SUR_H__
#define __FENXI_DETECTOR_HOR_SUR_H__
#include "tchar.h"
#include "common.h"
#include <opencv2/opencv.hpp>

/*
* @brief Detect base points on horizontal surface
*/
class CDetBasePtOnHorizon
{
public:
	CDetBasePtOnHorizon();
	virtual ~CDetBasePtOnHorizon();

	bool InitModel(const TCHAR* appPath);
	bool DetectBasePts(const cv::Mat& frame, HorizonMode& mode, bool& hasPin,  std::vector<cv::Point2f>& basePts, bool &inclined, bool support_bound = true);
	bool DetectHorizonMode(const cv::Mat& frame, HorizonMode& mode, bool& hasPin, cv::Rect* _outBox = NULL);
	void GetKeyPoint(PointF* keyPts);
private:
	double forward_predict(const cv::Mat& frame, cv::Rect& box, int& mode);
	bool DetectDetails(HorizonMode& mode, bool& hasPin, bool rotated);
	void clearState();
	bool FindPointsForMushroom(int has_pin);
	bool FindPointsForInner();
	bool FindPointsForHulu(int has_pin, bool rotated);
	bool FindPointsForLens(int has_pin);
private:
	cv::dnn::Net* net = NULL;
	std::vector<cv::String> ThreeOutput_layers_name;
	cv::Mat m_frame;
	
	cv::Mat m_cropFrame;
	cv::Mat m_cropGray;
	cv::Mat m_cropEdge;
	std::vector<std::vector<cv::Point>> m_cropContours;

	bool m_isBoundSupplement;
	std::vector<cv::Point2f> m_basePts;
	cv::Point m_keyPts[3]; // left, right, center-top
};

#endif//__FENXI_DETECTOR_HOR_SUR_H__
#pragma once
#include <opencv2/imgproc.hpp>
#include "opencv2/dnn.hpp"
#include "common.h"
#include "../cvLib/cvcommon.h"
#include <tchar.h>

class CBaselineShape
{

public:
	CBaselineShape();
	virtual ~CBaselineShape();

	cv::dnn::Net m_net;	

	/*
	* 识别基线形状
	* img：输入图像
	*
	* 返回 - 0：凸面	1：凹面	2：水平面
	*/
	baseLineShapeKind getShape(const cv::Mat &img);

	/*
	* 识别基线形状
	* imagePath：输入图像路径
	*
	* 返回 - 0：凸面	1：凹面	2：水平面
	*/
	baseLineShapeKind getShapeByPath(const TCHAR* imagePath);
	baseLineShapeKind getShapeByPath(const cv::Mat& imagePath);
	bool InitModel(const TCHAR* appPath);
private:
	bool isInitd = false;
};
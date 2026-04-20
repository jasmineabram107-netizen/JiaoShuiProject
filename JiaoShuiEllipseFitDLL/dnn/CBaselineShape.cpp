#include "CBaselineShape.h"
#include <opencv2/opencv.hpp>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

using namespace cv;
using namespace cv::dnn;

CBaselineShape::CBaselineShape() 
{
}

CBaselineShape::~CBaselineShape()
{
	if (isInitd)
	{
		m_net.~Net();
	}
}

bool CBaselineShape::InitModel(const TCHAR* appPath)
{
	isInitd = false;
	SetCurrentDirToExecutablePath(appPath);
	std::string ini = ELLIPSEFIT_CFG; // basepath + ELLIPSEFIT_CFG;
	std::string dic = ELLIPSEFIT_WEIGHTS; // basepath + ELLIPSEFIT_WEIGHTS;
	//auto basepath = TCHARToCvString(appPath);
	//std::string path = "..\\Bin64\\";
	//std::string ini = path + ELLIPSEFIT_CFG; // basepath + ELLIPSEFIT_CFG;
	//std::string dic = path + ELLIPSEFIT_WEIGHTS; // basepath + ELLIPSEFIT_WEIGHTS;

	try
	{
		m_net = readNetFromCaffe(ini, dic);
		m_net.setPreferableBackend(DNN_BACKEND_DEFAULT);
		m_net.setPreferableTarget(DNN_TARGET_CPU);
		isInitd = true;
	}
	catch (const std::exception&)
	{
		isInitd = false;
	}
	
	return isInitd;
}

/*
* 识别基线形状
* img：输入图像
*
* 返回 - 0：空白的凸面，1：空白的凹面，2：空白的水平面，3：插针凸面，4：插针凹面，5：插针水平面， 6：没针的凸面，7：没针的凹面，8：没针的水平面
*/
baseLineShapeKind CBaselineShape::getShape(const cv::Mat& img)
{
	float scale = 1.0;
	Scalar mean = Scalar(104, 117, 123);
	bool swapRB = false;
	int ww = 224;
	int hh = 224;

	if(img.data == NULL)
	{
		// LOG: failed load image
		return eBS_Unknown;
	}
	Mat blob;
	//! [Create a 4D blob from a frame]
	blobFromImage(img, blob, scale, Size(ww, hh), mean, swapRB, false);
	//! [Create a 4D blob from a frame]
	//! [Set input blob]
	m_net.setInput(blob);
	//! [Set input blob]
	//! [Make forward pass]
	Mat prob = m_net.forward();
	//! [Make forward pass]

	//! [Get a class with a highest score]
	Point classIdPoint;
	double confidence;
	minMaxLoc(prob.reshape(1, 1), 0, &confidence, 0, &classIdPoint);
	int classId = classIdPoint.x;
	if(classId < eBS_Unknown || classId >= eBS_Count)
	{
		// LOG: class ID error
		return eBS_Unknown;
	}
	return (baseLineShapeKind)classId;
}

/*
* 识别基线形状
* img：输入图像
*
* 返回 - 0：空白的凸面，1：空白的凹面，2：空白的水平面，3：插针凸面，4：插针凹面，5：插针水平面， 6：没针的凸面，7：没针的凹面，8：没针的水平面
*/
baseLineShapeKind CBaselineShape::getShapeByPath(const TCHAR* imagePath)
{
	Mat img = loadImageFromUnicodePath(imagePath);
	if (img.data == NULL) {
		// LOG: failed load image
		return eBS_Unknown;
	}
	return getShape(img);
}

baseLineShapeKind CBaselineShape::getShapeByPath(const cv::Mat& image)
{
	if (image.data == NULL) {
		return eBS_Unknown;
	}
	return getShape(image);
}

#ifndef __DROPLET_WETTING_REC_H__
#define __DROPLET_WETTING_REC_H__

#include "common.h"
#include "tchar.h"
#include <opencv2/opencv.hpp>
#include "json.hpp"
#include "dnn\DetBasePtOnHorizon.h"
#include "dnn\CBasePoints.h"
#include "dnn\CBaselineShape.h"
#include "DropletContext.h"

using json = nlohmann::json;

#ifdef _UNICODE
#define jstring std::wstring
#else
#define jstring std::string
#endif

class CDropletWetting
{

public:	
	CDropletWetting();
	virtual ~CDropletWetting();	

	bool InitModel(const TCHAR* appPath);
	bool SetImage(const TCHAR* imagePath);

	ErrorCode GetLastErrorCode() const { return m_errorCode; }
	BaseLineShapeKind GetBaselineShape();
#if _DEV_OLDSTYLE_API
	/*
	* @brief Recognize droplet angle based on the current settings.
	 * @param dropletFitMode: Mode for fitting the droplet shape.
	 * @param baselineFitMode: Mode for fitting the baseline.
	 * @param mainShapeType: Type of the main shape to recognize.
	 * @param isSubpixel: Whether to use subpixel accuracy.
	 * @param rotated: Whether the image is rotated.
	 * @param horMode: Horizon mode to use.
	 * @param hasPin: Whether the droplet has a pin.
	 * @return ErrorCode: indicating success or failure.	 
	*/
	ErrorCode RecognizeDropletAngleByAuto(
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode, 
		MainShapeType mainShapeType, 
		bool isSubpixel,
		bool rotated,
		HorizonMode horMode,
		bool hasPin);
	/*
	* @brief Recognize droplet angle using a baseline line.
	 * @param dropletFitMode: Mode for fitting the droplet shape.
	 * @param baselineFitMode: Mode for fitting the baseline.
	 * @param mainShapeType: Type of the main shape to recognize.
	 * @param isSubpixel: Whether to use subpixel accuracy.
	 * @param linePts: Points defining the baseline line.
	 * @return ErrorCode: indicating success or failure.
	*/
	ErrorCode RecognizeDropletAngleByBaseline(
		DropletFitMode dropletFitMode, 
		BaseLineFitMode baselineFitMode, 
		MainShapeType mainShapeType, 
		bool isSubpixel,
		const std::vector<cv::Point2f>& linePts);

	/*
	* @brief Recognize droplet angle using base points.
	 * @param fitMode: Mode for fitting the droplet shape.
	 * @param dropletFitMode: Mode for fitting the droplet shape.
	 * @param baselineFitMode: Mode for fitting the baseline.
	 * @param mainShapeType: Type of the main shape to recognize.
	 * @param isSubpixel: Whether to use subpixel accuracy.
	 * @param basePts: Points defining the base line.
	 * @return ErrorCode: indicating success or failure.
	*/
	ErrorCode RecognizeDropletAngleByBasePoints(
		FitMode fitMode, 
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		MainShapeType mainShapeType, 
		bool isSubpixel,
		const std::vector<cv::Point2f>& basePts);

	/*
	* @brief Recognize droplet angle using manual input for droplet and base points.
	 * @param dropletFitMode: Mode for fitting the droplet shape.
	 * @param baselineFitMode: Mode for fitting the baseline.
	 * @param fitMode: Mode for fitting the droplet shape.
	 * @param mainShapeType: Type of the main shape to recognize.
	 * @param isSubpixel: Whether to use subpixel accuracy.
	 * @param dropletPts: Points defining the droplet shape.
	 * @param basePts: Points defining the base line.
	 * @return ErrorCode: indicating success or failure.
	*/
	ErrorCode RecognizeDropletAngleByBaseManual(
		DropletFitMode dropletFitMode, 
		BaseLineFitMode baselineFitMode, 
		FitMode fitMode, 
		MainShapeType mainShapeType,
		bool isSubpixel,
		const std::vector<cv::Point2f>& dropletPts,
		const std::vector<cv::Point2f>& basePts);
		
#endif//_DEV_OLDSTYLE_API
	/*
	* @brief Get the result of the droplet angle recognition.
	 * @param _outAngle: Pointer to store the recognized angle. length of 2
	 * @param _outAngleDetail: Pointer to store detailed angle information. length of 2
	 * @return bool: true if successful, false otherwise.
	 *
	 * This function retrieves the recognized droplet angle and its details.
	 * It should be called after a successful recognition operation.
	*/
	bool GetResult(double* _outAngle, PointAndAngle* _outAngleDetail);

	/*
	* @brief Get the bounding boxes of the recognized droplet and baseline.
	 * @param _outBox: Vector to store the bounding boxes of the droplet and baseline.
	 * @return bool: true if successful, false otherwise.
	 *
	 * This function retrieves the bounding boxes of the recognized droplet and baseline.
	 * It should be called after a successful recognition operation.
	*/
	bool GetMidResult(
		MainShapeType& _mainShape,
		bool& _hasPin,
		bool& _isEmpty,
		HorizonMode& _horizonMode,
		RectangleF& _basePts,
		std::vector<cv::RotatedRect>& _outBox
	);

	/*
	* @brief Get the contours of the recognized droplet and baseline.
	 * @param _outDropletPts: Vector to store the contour points of the droplet.
	 * @param _outBaselinePts: Vector to store the contour points of the baseline.
	 * @return bool: true if successful, false otherwise.
	 * 
	 * This function retrieves the contour points of the recognized droplet and baseline.
	 * It should be called after a successful recognition operation.	
	*/
	bool GetContourPoints(
		std::vector<cv::Point>& _outDropletPts, 
		std::vector<cv::Point>& _outBaselinePts
	);

	/*
	* @brief Get the count of contours for the droplet and baseline.
	 * @param _outDropletCount: Reference to store the count of droplet contours.
	 * @param _outBaselineCount: Reference to store the count of baseline contours.
	 * @return bool: true if successful, false otherwise.
	 * 
	 * This function retrieves the count of contours for the recognized droplet and baseline.
	 * It should be called after a successful recognition operation.	
	*/
	bool GetContourPointsCount(int& _outDropletCount, int& _outBaselineCount);

	/*
	* @brief Get the result of the droplet angle recognition in JSON format.
	 * @return jstring: JSON string containing the recognition results.
	 * 
	 * This function returns the recognition results in a JSON format, which includes
	 * angles, bounding boxes, and other relevant information.
	*/
	jstring GetResultByJsonFormat();
	bool getHorizontalMode(HorizonMode& _outMode, bool& _outHasPin);
	ErrorCode getBasepointsByAuto(
		bool isHorizontal,
		bool &hasPin,
		bool &rotated,
		HorizonMode &horMode,
		bool isSubpixel,
		bool isBoundSupplement,
		RectangleF &rectBasePts, 
		PointF* keyPts
	);

	ErrorCode getBasepointsByBaseline(
		bool isHorizontal,
		bool isSubpixel,
		const std::vector<cv::Point2f>& linePts,
		RectangleF& rectBasePts
	);

	ErrorCode getBasepointsByBasepoint(
		bool isHorizontal,
		bool isSubpixel,
		const std::vector<cv::Point2f>& basePts,
		RectangleF& rectBasePts
	);

	ErrorCode getBasepointsBySemiauto(
		MainShapeType mainShapeType,
		bool isSubpixel,
		const std::vector<cv::Point2f>& baselinePts, 
		RectangleF& rectBasePts
	);

	bool fitting(
		FitMode fitMode,
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		RectangleF baseRect, 
		FittingInfo* outInfo,
		bool isCallByExternal = true
	);

	MainShapeType getMainShape() const { return m_Context.mainShape(); }
	bool fittingManual(
		MainShapeType mainShape,
		FitMode fitMode,
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		RectangleF baseRect,
		const std::vector<cv::Point2f>& baselinePts,
		const std::vector<cv::Point2f>& dropletPts,
		FittingInfo* outInfo
	);

	bool fittingBasePointsAndLine(
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode, // will not be used.
		RectangleF rectBasePts,
		FittingInfo* outInfo
	);
private:
	void initVariables(bool initImage = false);
	ErrorCode prepareEngineVariables();
#if _DEV_OLDSTYLE_API
	bool calculateDropletAngle();
#endif//_DEV_OLDSTYLE_API
	void errorReport(ErrorCode errCode);
	bool getBasePointsFromLine(
		const cv::Point2f& leftPt,
		const cv::Point2f& rightPt, 
		RectangleF& _outRect);
	FittingGraphicsType getFittingGraphicsType(DropletFitMode dropletFitMode) const;
	void fillFittingInfo(FittingInfo* outInfo, bool isHorizontal);

	void readyManualDropletPts(double** x, double** y) const;
	void readyManulBasePts(double** x, double** y, bool bSupport = false) const;

	void clearFittingOutputJson();	
	void clearHorizontalModeJson();
	void clearBasePointJson();
private:

	bool					m_bInited;
	// engine variables
	CBaselineShape*			m_pBaseShape;	
	CBasePoints*			m_pBasePoints;
	CDetBasePtOnHorizon*	m_pDetector;	

	CDropletContext			m_Context;

	ErrorCode				m_errorCode;	

	std::vector<cv::Point2f> m_dropletPtsByManual;
	std::vector<cv::Point2f> m_basePtsByManual;	

	json		m_jsInput;
	json		m_jsOutput;
};

#endif//__DROPLET_WETTING_REC_H__

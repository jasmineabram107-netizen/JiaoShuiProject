#ifndef __JIAOSHUI_ELLIPSEFIT_H__
#define __JIAOSHUI_ELLIPSEFIT_H__

#include "common.h"
#include <tchar.h>
#include <comutil.h>
#define FENXIELLIPSE_API __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif
	/*
	* @brief 初始化
	* @param appPath 应用程序路径
	* @return true:成功，false:失败	
	*/
	FENXIELLIPSE_API bool dropletInitLib(const TCHAR* appPath);
	/*
	* @brief 释放	
	*/
	FENXIELLIPSE_API void dropletReleaseLib();

	/*
	* @brief 设置图片
	* @param imagePath 图片路径
	* @return true:成功，false:失败		
	*/
	FENXIELLIPSE_API bool dropletSetImage(const TCHAR* imagePath);
#if _DEV_OLDSTYLE_API
	/*
	* @brief 自动拟合, 接触角计算
	* @param dropletFitMode 液滴拟合模式(CV椭圆拟合)
	* @param baselineFitMode 基线拟合模式(CV椭圆拟合)
	* @param mainShape 主形状(未知)
	* @param isSubpixel 是否亚像素拟合(false)
	* @param rotated 是否旋转(false)
	* @param horMode 水平模式(未知)
	* @param hasPin 是否有针(false)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletAutoFit(		
		DropletFitMode dropletFitMode = eDropletEllipse, 
		BaseLineFitMode baselineFitMode = eBaseLineEllipse,
		MainShapeType mainShape = eShapeUnknown,
		bool isSubpixel = false,
		bool rotated = false,
		HorizonMode horMode = eHmUnknow,
		bool hasPin = false
	);

	/*
	* @brief 直线找基线位置, 接触角计算
	* @param pts 两个点的坐标为直线
	* @param dropletFitMode 液滴拟合模式(CV椭圆拟合)
	* @param baselineFitMode 基线拟合模式(CV椭圆拟合)
	* @param mainShape 主形状(未知)
	* @param isSubpixel 是否亚像素拟合(false)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletFitByBaseLine(
		const PointF* pts, 		
		DropletFitMode dropletFitMode = eDropletEllipse,
		BaseLineFitMode baselineFitMode = eBaseLineEllipse,
		MainShapeType mainShape = eShapeUnknown,
		bool isSubpixel = false);

	/*
	* @brief 手动输入基线点, 接触角计算
	* @param pts 两个基线点的坐标
	* @param dropletFitMode 液滴拟合模式(CV椭圆拟合)
	* @param baselineFitMode 基线拟合模式(CV椭圆拟合)
	* @param mainShape 主形状(未知)
	* @param isSubpixel 是否亚像素拟合(false)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletFitByBasePoint(
		const PointF* pts, 	
		DropletFitMode dropletFitMode = eDropletEllipse,
		BaseLineFitMode baselineFitMode = eBaseLineEllipse,
		MainShapeType mainShape = eShapeUnknown,
		bool isSubpixel = false);

	/*
	* @brief 手动输入液滴和基线点, 接触角计算	
	* @param nDroplets 液滴点个数
	* @param droplets 液滴点坐标
	* @param nBaselines 基线点个数
	* @param baselines 基线点坐标
	* @param dropletFitMode 液滴拟合模式(CV椭圆拟合)
	* @param baselineFitMode 基线拟合模式(CV椭圆拟合)
	* @param mainShape 主形状(未知)
	* @param isSubpixel 是否亚像素拟合(false)
	* @return true:成功，false:失败	
	*/
	FENXIELLIPSE_API bool dropletManualFit(
		int nDroplets, PointF* droplets, 
		int nBaselines, PointF* baselines, 
		DropletFitMode dropletFitMode = eDropletEllipse,
		BaseLineFitMode baselineFitMode = eBaseLineEllipse,
		MainShapeType mainShape = eShapeUnknown,
		bool isSubpixel = false);

	/*
	* @brief 半自动拟合(液滴自动，基线手动), 接触角计算
	* @param nBaselines 基线点个数
	* @param baselines 基线点坐标
	* @param dropletFitMode 液滴拟合模式(CV椭圆拟合)
	* @param baselineFitMode 基线拟合模式(CV椭圆拟合)
	* @param mainShape 主形状(未知)
	* @param isSubpixel 是否亚像素拟合(false)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletSemiAutoFit(		
		int nBaselines, PointF* baselines,
		DropletFitMode dropletFitMode = eDropletEllipse,
		BaseLineFitMode baselineFitMode = eBaseLineEllipse,
		MainShapeType mainShape = eShapeUnknown,
		bool isSubpixel = false);

	/*
	* @brief 获取基线形状
	* @return 基线形状
	*/
	FENXIELLIPSE_API baseLineShapeKind dropletGetShape();
#endif//_DEV_OLDSTYLE_API
	/**
	 * @brief 获取接触角和详细信息（两个角点）
	 * @param angles 输出的两个接触角（单位：弧度），double[2]
	 * @param angleDetails 输出的两个角点信息（PointAndAngle[2]），可选
	 * @return true: 有效输出；false: 结果无效
	 */
	FENXIELLIPSE_API bool dropletGetResult(
		double* angles,
		PointAndAngle* angleDetails = NULL
	);

	/*
	* @brief 获取接触角和详细信息
	* @param _outMainShape 形状
	* @param _outHasPin 是否有针
	* @param _outIsEmpty 是否为空
	* @param _horizonMode 水平模式
	* @param _outBasePts 基线点坐标
	* @param _outBoxs 液滴基线拟合框(_outBasePts[2])
	* @return true: 有效输出；false: 结果无效
	*/
	FENXIELLIPSE_API bool dropletMidResult(
		MainShapeType* _outMainShape,
		bool* _outHasPin,
		bool* _outIsEmpty,
		HorizonMode* _horizonMode,
		RectangleF* _outBasePts,
		EllipseFitBox* _outBoxs
	);

	/*
	* @brief 获取液滴和基线轮廓点个数
	* @param _outDropletCount 液滴点个数
	* @param _outBaselineCount 基线点个数
	* @return true: 有效输出；false: 结果无效
	*/
	FENXIELLIPSE_API bool dropletGetContourCounts(
		int* _outDropletCount, 
		int* _outBaselineCount
	);

	/*
	* @brief 获取液滴和基线轮廓点坐标
	* @param dropletCount 液滴点个数
	* @param baselineCount 基线点个数
	* @param _outDropletPts 液滴点坐标
	* @param _outBaselinePts 基线点坐标
	* @return true: 有效输出；false: 结果无效
	*/
	FENXIELLIPSE_API bool dropletGetContours(
		int dropletCount, 
		int baselineCount, 
		PointF* _outDropletPts,
		PointF* _outBaselinePts);

	/*
	* @brief 获取最后错误
	*/
	FENXIELLIPSE_API ErrorCode dropletGetLastError();

	/*
	* @brief 获取错误信息
	* @param code 错误码
	* @return 错误信息	
	*/
	FENXIELLIPSE_API BSTR dropletGetResultString(ErrorCode code);
	
	/*
	* @brief 获取结果JSON格式
	* @return 结果JSON格式	
	*/
	FENXIELLIPSE_API BSTR dropletGetResultByJsonFormat();
	
	

	/*
	* @brief 获取基线形状, 水平模式，是否有针
	* @param shape 基线形状
	* @param horizon 水平模式
	* @param hasPin 是否有针
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetShapeEx(
		BaseLineShapeKind* shape, 
		HorizonMode* horizon, 
		bool* hasPin
	);

	/*
	* @brief 获取基线位置(水平面，自动)
	* @param hasPin 是否有针
	* @param rotated 是否旋转
	* @param horMode 水平模式	
	* @param basePts 输出基线位置
	* @param keyPts 输出关键点位置(左、右、液滴上中点：3个点)
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @param isBoundSupplement 是否边界补充(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsByAutoOnHorizontal(
		bool& hasPin,
		bool& rotated,
		HorizonMode& horMode,		
		RectangleF* basePts, 
		PointF* keyPts = NULL,
		bool isSubpixel = true,
		bool isBoundSupplement = true
	);

	/*
	* @brief 获取基线位置(凸凹面，自动)
	* @param hasPin 是否有针	
	* @param basePts 输出基线位置	
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @param isBoundSupplement 是否边界补充(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsByAutoOnSurface(
		bool hasPin,		
		RectangleF* basePts,
		bool isSubpixel = true,
		bool isBoundSupplement = true
	);

	/*
	* @brief 获取基线位置(水平面，找基线)
	* @param pts 两个点的坐标为直线	
	* @param basePts 输出基线位置
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsBylineOnHorizontal(
		const PointF* pts,
		RectangleF* basePts,
		bool isSubpixel = true
	);

	/*
	* @brief 获取基线位置(凸凹面，找基线)
	* @param pts 两个点的坐标为直线	
	* @param basePts 输出基线位置
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsBylineOnSurface(
		const PointF* pts,		
		RectangleF* basePts,
		bool isSubpixel = true
	);

	/*
	* @brief 获取基线位置(水平面，找基点)
	* @param pts 两个基线点的坐标	
	* @param basePts 输出基线位置
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsByPointsOnHorizontal(
		const PointF* pts,
		RectangleF* basePts,
		bool isSubpixel = true
	);

	/*
	* @brief 获取基线位置(凸凹面，找基点)
	* @param pts 两个基线点的坐标	
	* @param basePts 输出基线位置
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsByPointsOnSurface(
		const PointF* pts,		
		RectangleF* basePts,
		bool isSubpixel = true
	);

	/*
	* @brief 获取基线位置(半自动)
	* @param mainShapeType 主形状
	* @param nBaselines 基线点个数
	* @param baselines 基线点坐标	
	* @param basePts 输出基线位置
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletGetBasepointsBySemiautoOnSurface(
		MainShapeType mainShapeType,
		int nBaselines, 
		const PointF* baselines,		
		RectangleF* basePts,
		bool isSubpixel = true
	);

	/*
	* @brief 自动拟合(水平面), 接触角计算(输出详细信息)
	* @param dropletFitMode 液滴拟合模式
	* @param pts 两个基线点的坐标
	* @param outInfo 输出拟合信息
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletAutoFittingOnHorizontal(
		DropletFitMode dropletFitMode,
		const RectangleF* pts,
		FittingInfo* outInfo
	);

	/*
	* @brief 自动拟合(凸凹面), 接触角计算(输出详细信息)
	* @param mainShape 主形状
	* @param dropletFitMode 液滴拟合模式
	* @param baselineFitMode 基线拟合模式
	* @param pts 两个基线点的坐标
	* @param outInfo 输出拟合信息
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletAutoFittingOnSurface(
		MainShapeType mainShape,
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		const RectangleF* pts,
		FittingInfo* outInfo
	);

	/*
	* @brief 手动输入液滴和基线点, 接触角计算(输出详细信息)
	* @param mainShape 主形状
	* @param dropletFitMode 液滴拟合模式
	* @param baselineFitMode 基线拟合模式
	* @param nDroplets 液滴点个数
	* @param droplets 液滴点坐标
	* @param nBaselines 基线点个数
	* @param baselines 基线点坐标	
	* @param hasPin 是否有针
	* @param outInfo 输出拟合信息
	* @param isSubpixel 是否亚像素拟合(true, 不再支持)
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletManualFitting(
		MainShapeType mainShape,
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		int nDroplets,
		const PointF* droplets,
		int nBaselines,
		const PointF* baselines,		
		bool hasPin,
		FittingInfo* outInfo,
		bool isSubpixel = true
	);

	/*
	* @brief 半自动拟合(液滴自动，基线手动), 接触角计算(输出详细信息)
	* @param mainShape 主形状(未知)
	* @param dropletFitMode 液滴拟合模式(CV椭圆拟合)
	* @param baselineFitMode 基线拟合模式(CV椭圆拟合)
	* @param pts 两个基线点的坐标
	* @param nBaselines 基线点个数
	* @param baselines 基线点坐标
	* @param outInfo 输出拟合信息
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletSemiautoFitting(
		MainShapeType mainShape,
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		const RectangleF* pts,
		int nBaselines,
		PointF* baselines,
		FittingInfo* outInfo
	);

	/*
	* @brief 液滴点拟合, 接触角计算(输出详细信息)
	* @param dropletFitMode 液滴拟合模式
	* @param baselineFitMode 基线拟合模式
	* @param pts 两个基线点的坐标
	* @param outInfo 输出拟合信息
	* @return true:成功，false:失败
	*/
	FENXIELLIPSE_API bool dropletPointsFitting(
		DropletFitMode dropletFitMode,
		BaseLineFitMode baselineFitMode,
		const RectangleF* pts,
		FittingInfo* outInfo
	);

	/*
	* @brief 释放拟合信息(FittingInfo)
	* @param info 拟合信息
	*/
	FENXIELLIPSE_API void dropletFreeFittingInfo(FittingInfo* info);

	FENXIELLIPSE_API BSTR dropletGetVersion();

#ifdef __cplusplus
}
#endif

#endif//__JIAOSHUI_ELLIPSEFIT_H__
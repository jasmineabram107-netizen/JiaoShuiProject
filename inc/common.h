#ifndef __JIAOSHUI_COMMON_H__
#define __JIAOSHUI_COMMON_H__

//////////////////////////////////////////////////////////////////
// macros for versioning
#define _DEV_OLDSTYLE_API	0

//////////////////////////////////////////////////////////////////
// macros for debugging
#define DEBUG_IMG	1			// for debugging of image processing

//////////////////////////////////////////////////////////////////
// in testing dev macros
#define _DEV_UPDATE_BACKTRACE				1
#define _DEV_UPDATE_SUBPXL_ON_HORIZONTAL	1

//////////////////////////////////////////////////////////////////
// constants
const double conf_th = 0.1;

#define BASELINE_CFG		"baseline.cfg"
#define BASELINE_WEIGHTS	"baseline.weights"

#define ELLIPSEFIT_CFG		"9-shapenet.ini"			// 识别基线形状-配置文件
#define ELLIPSEFIT_WEIGHTS	"9-shapenet.dic"			// 识别基线形状-模型文件

#define ELLIPSEBASEPOINT_CFG		"det3.ini"			// 基线点检测配置文件
#define ELLIPSEBASEPOINT_WEIGHTS	"det3.dic"			// 基线点检测模型文件

//////////////////////////////////////////////////////////////////
// JSON field names
#define JSNAME_INPUT				"input"
#define JSNAME_OUTPUT				"result"

// input
#define JSNAME_IMAGEPATH			"image_path"				// image path (in)
#define JSNAME_FITMODE				"fit_mode"					// fitting mode(auto, bypoint, byline, semi-auto, manual) (in)
#define JSNAME_MAINSHAPE			"main_shape"				// main shape(convex, concave, horizontal) (in)

#define JSNAME_USESUBPIXEL			"use_subpixel"				// use subpixel (in)
#define JSNAME_ROTATED				"rotated"					// rotated (in/out)
#define JSNAME_HASPIN				"has_pin"					// has pin (in/out)
#define JSNAME_HORIZONMODE			"horizon_mode"				// horizontal mode (in/out)
#define JSNAME_BASEPTS				"base_points"				// 2 base points (in/out)
#define JSNAME_MANUALBSLINEPTS		"manual_baseline_points"	// manual baseline points (in)
#define JSNAME_MANUALDROPLETPTS		"manual_droplet_points"		// manual droplet points (in)

#define JSNAME_DROPLETFITMODE		"droplet_fit_mode"			// droplet fitting mode (in)
#define JSNAME_BASELINEFITMODE		"baseline_fit_mode"			// baseline fitting mdoe (in)

// output
#define JSNAME_RESCODE				"code"

#define JSNAME_KEYPTS				"key_points"
#define JSNAME_BSLINESHAPE			"baseline_shape"

#define JSNAME_DROPBOX				"droplet_box"
#define JSNAME_DROPLEFTBOX			"left_drop_box"
#define JSNAME_DROPRIGHTBOX			"right_drop_box"
#define JSNAME_BASELINEBOX			"baseline_box"
#define JSNAME_DROPCONTOUR			"droplet_contour"
#define JSNAME_BASELINECONTOUR		"baseline_contour"
#define JSNAME_LEFTPOLYLINE			"left_polyline"
#define JSNAME_RIGHTPOLYLINE		"right_polyline"

#define JSNAME_RESULTLEFT			"left_angle"
#define JSNAME_RESULTRIGHT			"right_angle"
#	define JSNAME_RESPOINTS				"points"
#	define JSNAME_RESANGLES				"angles"
#	define JSNAME_RESVEC1				"vec1"
#	define JSNAME_RESVEC2				"vec2"

//////////////////////////////////////////////////////////////////
// definitions of data structures

// error codes
typedef enum ErrorCode {
	errNo = 0,                // 成功
	errException,         // 全局异常处理 & expired
	errExpired,		  // 有效期过期
	errFailOpen,          // 打不开图片(Unable to open the image)
	errInitModel,	   // 模型加载失败(all models)
	errInitModel1,       // 模型加载失败(baseline.cfg，baseline.weights)
	errInitModel2,       // 模型加载失败(9-shapenet.ini，9-shapenet.dic)
	errInitModel3,       // 模型加载失败(det3.ini，det3.dic)	
	errInvalidBasePoints, // 基线点无效(Invalid base points)
	errInvalidFitMode, // 拟合模式无效(Invalid fit mode)
	errInvalidManualPointCount, // 手动点数无效(Invalid manual point count)
	errNotFoundContour, // 无法找到轮廓(Unable to find contour)
	errFailedFitDroplet, // 液滴拟合失败(Droplet fitting failed)
	errFailedFitBaseline, // 基线拟合失败(Baseline fitting failed)
	errNotFoundIntersection, // 无法找到交点(Unable to find intersection)
	errFailedDetectShape, // 形状检测失败(Shape detection failed)
	errFailedDetectBasePoint, // 基线点检测失败(Base point detection failed)
	errFailFitting,       // 圆拟合失败(Circle fitting failed)
	errFailDetect,        // 无法检测交叉点(Unable to detect intersection)
	errFailOnlyOne,       // 只检测到一个交叉点(Only one intersection was detected)
	errFailNoGlue,        // 检测不到有效液滴
	errFailNotFound,      // 拟合分析失败
	errFailedDetectAngle, // 角度检测失败(Angle detection failed)
	errLast
}errCode;

// deep learning model class ID ( don't change order and value )
typedef enum BaseLineShapeKind {
	eBS_Unknown = -1,
	eBS_BlankConvex = 0,					 // 空白的凸面，
	eBS_BlankConcave = 1,					 // 空白的凹面
	eBS_BlankHorizontal = 2,				 // 空白的水平面
	eBS_ConvexWithPins = 3,					 // 插针凸面
	eBS_ConcaveWithPins = 4,				 // 插针凹面
	eBS_HorizontalWithPins = 5,				 // 插针水平面
	eBS_ConvexWithoutPins = 6,				 // 没针的凸面
	eBS_ConcaveWithoutPins = 7,				 // 没针的凹面
	eBS_HorizontalWithoutPins = 8,			 // 没针的水平面
	eBS_Count
} baseLineShapeKind;

// 基线形态: 
typedef enum MainShapeType {
	eShapeUnknown = -1,	// 未知
	eShapeConvex = 0,		// 凸面
	eShapeConcave = 1,		// 凹面
	eShapeHorizontal = 2	// 水平面
}mainShapeType;

// 拟合模式
typedef enum FitMode {
	eFitAuto = 0,		// 自动拟合	
	eFitByBasePoint,	// 找基点
	eFitByLine,			// 找基线
	eFitSemiAuto,		// 半自动(液滴自动，基线手动)	
	eFitManual,			// 手动拟合
}fitMode;

// 处理方法
typedef enum HorizonMode {
	eHmUnknow = -1,			// 未知
	eHmConvexLens,			// 凸透镜
	eHmGourd,				// 葫芦
	eHmLookingInside,		// 查看内部
	eHmMushroomCap,			// 蘑菇菌盖
	eHmCount
}horizonMode;

// 液滴拟合模式
typedef enum DropletFitMode {
	eDropletCircle = 0,		// 圆
	eDropletEllipse = 1,	// OpenCV椭圆拟合
	eDropletEllipseAMS = 2,	// OpenCV椭圆拟合(AMS)
	eDropletEllipseDirect = 3,	// OpenCV椭圆拟合(Direct)
	eDropletDoubleEllipse = 4,	// 双椭圆
	eDropletDoublePolynomial = 5,	// 双多项式
	eDropletWidthHeight = 6,	// 高宽法
	eDropletPolynomial = 7,	// 多项式（极坐）
	eDropletDoubleCircle = 8	// 双圆拟合
}dropletFitMode;

// 基线拟合模式
typedef enum BaseLineFitMode {
	eBaseLineCircle = 0,		// 圆
	eBaseLineEllipse = 1,		// OpenCV椭圆拟合
	eBaseLineEllipseAMS = 2,	// OpenCV椭圆拟合(AMS)
	eBaseLineEllipseDirect = 3	// OpenCV椭圆拟合(Direct)
}baseLineFitMode;

// 矩形
typedef struct RectangleF {
	double left;
	double top;
	double right;
	double bottom;
	RectangleF() : left(0), top(0), right(0), bottom(0) {}
	RectangleF(double l, double t, double r, double b) : left(l), top(t), right(r), bottom(b) {}
}rectangleF;

inline bool operator==(const RectangleF& lhs, const RectangleF& rhs)
{
	return lhs.left == rhs.left &&
		lhs.top == rhs.top &&
		lhs.right == rhs.right &&
		lhs.bottom == rhs.bottom;
}

inline bool operator!=(const RectangleF& lhs, const RectangleF& rhs)
{
	return !(lhs == rhs);
}

// 点
typedef struct PointF {
	double x;
	double y;
	PointF(double xCoord, double yCoord) : x(xCoord), y(yCoord) {}
	PointF() : x(0), y(0) {}
}MPoint;

// 液滴表面与基线之间的接触角。
typedef struct PointAndAngle {
	PointF point;	// 接触点
	double angle;	// 接触角（单位：弧度）
	PointF vec1;	// 液滴切线
	MPoint vec2;	// 基线切线
	PointAndAngle() : point(-1.0, -1.0), angle(-1.0), vec1(0, 0), vec2(0, 0) {}
}PtAngle;


// 椭圆拟合的结果参数
typedef struct EllipseFitBox {
	PointF center;	// 圆心
	double a;		// 半长轴
	double b;		// 半短轴
	double angle;	// 旋转角度(degree)
	EllipseFitBox() : center(0, 0), a(0), b(0), angle(0) {}
}tagEllipseFitBox;

// 基线信息
typedef struct BaselineInfo {
	BaseLineShapeKind shape;	// 基线形态
	HorizonMode horizon;		// 水平基线的时候处理方法
	bool haspin;				// 是否有插针, todo: not need 
	BaselineInfo() : shape(eBS_Unknown), horizon(eHmUnknow), haspin(false) {}
}tagBaselineInfo;

// 拟合图形类型
typedef enum FittingGraphicsType {
	eFgNone = 0,	// 0: none
	eFgOneEllipse = 1,            // 单椭圆, 单圆
	eFgTwoEllipses = 2,	   // 双椭圆, 双圆
	eFgPolyline = 3,	// 多项式
	eFgRectangle = 4,	// 高宽法矩形
	eFgCount
}fittingGraphicsType;

// 拟合结果
typedef struct FittingInfo {
	FittingGraphicsType fgType;		// 拟合图形类型
	double  angle[2];				// todo: duplicated, not need
	PointAndAngle  pointAngle[2];	// 0： 左角， 1： 右角
	EllipseFitBox  boxes[3];		// 0： 液滴左边（椭圆， 圆， 高宽法矩形）， 1： 液滴右边(双椭圆，双圆)， 2： 基线
	int     leftPolylineSize;		// 多项式拟合左边点数
	PointF* leftPolyline;			// 多项式拟合左边点
	int     rightPolylineSize;		// 多项式拟合右边点数
	PointF* rightPolyline;			// 多项式拟合右边点
	FittingInfo() 		
	{
		leftPolyline = nullptr;
		rightPolyline = nullptr;
		leftPolylineSize = 0;
		rightPolylineSize = 0;
		angle[0] = angle[1] = -1.0;
		fgType = eFgNone;
	}
}tagFittingInfo;

#endif
//__JIAOSHUI_COMMON_H__
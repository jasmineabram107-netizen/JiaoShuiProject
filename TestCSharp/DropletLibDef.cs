using System;
using System.Runtime.InteropServices;

namespace DropletLibCSharp
{
    

    /// <summary>
    /// Represents the shape classification of the baseline. // 基线形状
    /// </summary>
    public enum BaseLineShapeKind : int
    {
        eBS_Unknown = -1,                           // 未知形状
        eBS_BlankConvex = 0,                     // 空白的凸面，
        eBS_BlankConcave = 1,                    // 空白的凹面
        eBS_BlankHorizontal = 2,                 // 空白的水平面
        eBS_ConvexWithPins = 3,                  // 插针凸面
        eBS_ConcaveWithPins = 4,                 // 插针凹面
        eBS_HorizontalWithPins = 5,              // 插针水平面
        eBS_ConvexWithoutPins = 6,               // 没针的凸面
        eBS_ConcaveWithoutPins = 7,              // 没针的凹面
        eBS_HorizontalWithoutPins = 8,           // 没针的水平面
        eBS_Count
    }

    /// <summary>
    /// 角度计算返回值 
    /// </summary>
    public enum ErrorCode : int
    {
        errNo = 0,                // 成功
        errException,         // 全局异常处理 & expired
        errExpired,       // 有效期过期
        errFailOpen,          // 打不开图片(Unable to open the image)
        errInitModel,      // 模型加载失败(all models)
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
    }

    // 基线形态: 
    public enum MainShapeType : int
    {
        eShapeUnknown = -1, // 未知
        eShapeConvex = 0,       // 凸面
        eShapeConcave = 1,      // 凹面
        eShapeHorizontal = 2    // 水平面
    }

    // 拟合模式
    public enum FitMode : int
    {
        eFitAuto = 0,       // 自动拟合
        eFitByBasePoint = 1,     // 手动拟合
        eFitByLine = 2, // 找基点
        eFitSemiAuto = 3,   // 半自动(液滴自动，基线手动)
        eFitManual = 4  //全手动
    }

    // 处理方法
    public enum HorizonMode : int
    {
        eHmUnknow = -1,         // 未知
        eHmConvexLens,          // 凸透镜
        eHmGourd,               // 葫芦
        eHmLookingInside,       // 查看内部
        eHmMushroomCap,         // 蘑菇菌盖
        eHmCount
    }

    // 液滴拟合模式
    public enum DropletFitMode : int
    {
        eDropletCircle = 0,     // 圆
        eDropletEllipse = 1,    // OpenCV椭圆拟合
        eDropletEllipseAMS = 2, // OpenCV椭圆拟合(AMS)
        eDropletEllipseDirect = 3,  // OpenCV椭圆拟合(Direct)
        eDropletDoubleEllipse = 4,  // 双椭圆
        eDropletDoublePolynomial = 5,   // 双多项式
        eDropletWidthHeight = 6,    // 高宽法
        eDropletPolynomial = 7, // 多项式（极坐）
        eDropletDoubleCircle = 8    // 双圆拟合
    }

    // 基线拟合模式
    public enum BaseLineFitMode : int
    {
        eBaseLineCircle = 0,        // 圆
        eBaseLineEllipse = 1,       // OpenCV椭圆拟合
        eBaseLineEllipseAMS = 2,    // OpenCV椭圆拟合(AMS)
        eBaseLineEllipseDirect = 3  // OpenCV椭圆拟合(Direct)
    }


    // 拟合图形类型
    public enum FittingGraphicsType : int
    {
        eFgNone = 0,        // 无
        eFgOneEllipse,      // 单椭圆, 单圆
        eFgTwoEllipses,     // 双椭圆, 双圆
        eFgPolyline,        // 折线(多项式)
        eFgRectangle,       // 矩形(高宽法)
        eFgCount
    }

    /// <summary>
    /// Defines a rectangle using double-precision floating-point coordinates. // 矩形结构体定义
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct RectangleF
    {
        /// <summary>
        /// The X-coordinate of the left edge of the rectangle. // 左边界
        /// </summary>
        public double left;

        /// <summary>
        /// The Y-coordinate of the top edge of the rectangle. // 上边界
        /// </summary>
        public double top;

        /// <summary>
        /// The X-coordinate of the right edge of the rectangle. // 右边界
        /// </summary>
        public double right;

        /// <summary>
        /// The Y-coordinate of the bottom edge of the rectangle. // 下边界
        /// </summary>
        public double bottom;
    }

    // 点结构体定义
    [StructLayout(LayoutKind.Sequential)]
    public struct PointF
    {
        public double x;
        public double y;
    }

    // 角点和角度结构体定义
    [StructLayout(LayoutKind.Sequential)]
    public struct PointAndAngle
    {
        public PointF point;    // 角点坐标
        public double angle;   // 角度(弧度)
        public PointF vec1;    // 液滴切线
        public PointF vec2;    // 基线切线
    };

    // 椭圆拟合框结构体定义
    [StructLayout(LayoutKind.Sequential)]
    public struct EllipseFitBox
    {
        public PointF center;  // 圆心
        public double a;       // 半长轴
        public double b;       // 半短轴
        public double angle;   // 旋转角度
    }

    // 拟合信息结构体定义
    [StructLayout(LayoutKind.Sequential)]
    public struct FittingInfo
    {
        public FittingGraphicsType fgType;      // 拟合图形类型(兩个)
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]

        public double[] angle; // 接触角(弧度)， 两个
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]

        public PointAndAngle[] pointAngle; // 角点和角度， 两个
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]

        public EllipseFitBox[] boxes;   // 拟合框， 两个液滴+一个基线

        public int leftPolylineSize; // 左侧折线点个数
        public IntPtr leftPolyline;     // 左侧折线点坐标(PointF*)

        public int rightPolylineSize;   // 右侧折线点个数
        public IntPtr rightPolyline;    // 右侧折线点坐标(PointF*)
    }

    // C# 版本的拟合信息
    public class FittingInfoCS
    {
        public FittingGraphicsType fgType;      // 拟合图形类型
        public double[] angle;                  // 接触角(弧度)， 两个
        public PointAndAngle[] pointAngle;      // 角点和角度， 两个
        public EllipseFitBox[] boxes;           // 拟合框， 两个液滴+一个基线
        public PointF[] leftPoints;             // 左侧折线点坐标
        public PointF[] rightPoints;            // 右侧折线点坐标

        // 从本地结构体转换为C#结构体
        public static FittingInfoCS FromNative(FittingInfo native)
        {
            var info = new FittingInfoCS
            {
                fgType = native.fgType,
                angle = native.angle,
                pointAngle = native.pointAngle,
                boxes = native.boxes,
                leftPoints = ConvertPointerArray(native.leftPolyline, native.leftPolylineSize),
                rightPoints = ConvertPointerArray(native.rightPolyline, native.rightPolylineSize)
            };
            DropletLib.dropletFreeFittingInfo(ref native);
            return info;
        }

        // 将指针数组转换为PointF数组
        private static PointF[] ConvertPointerArray(IntPtr ptr, int count)
        {
            if (ptr == IntPtr.Zero || count <= 0)
                return Array.Empty<PointF>();

            PointF[] result = new PointF[count];
            int size = Marshal.SizeOf(typeof(PointF));
            for (int i = 0; i < count; i++)
            {
                IntPtr p = IntPtr.Add(ptr, i * size);
                result[i] = Marshal.PtrToStructure<PointF>(p);
            }
            return result;
        }
    }
}
//.EOF
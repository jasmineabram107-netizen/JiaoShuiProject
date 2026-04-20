using System;
using System.Runtime.InteropServices;
using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace DropletLibCSharp
{    
    public class DropletLib
    {
#if DEBUG
        const string dllPath = "CEPHd.dll";
#else
        const string dllPath = "CEPH.dll";
#endif

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletInitLib(string appPath);
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern void dropletReleaseLib();
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletSetImage(string imagePath);

#if OLD_API
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletAutoFit(
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            MainShapeType mainShape,
            bool isSubpixel,
            bool rotated,
            HorizonMode horMode,
            bool hasPin
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletFitByBaseLine(
            PointF[] points,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            MainShapeType mainShape,
            bool isSubpixel);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletFitByBasePoint(
            PointF[] points,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            MainShapeType mainShape,
            bool isSubpixel);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletManualFit(
            int nDroplets,
            PointF[] droplets,
            int nBaselines,
            PointF[] baselines,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            MainShapeType mainShape,
            bool isSubpixel);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletSemiAutoFit(
            int nBaselines,
            PointF[] baselines,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            MainShapeType mainShape,
            bool isSubpixel);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern BaseLineShapeKind dropletGetShape();
#endif//OLD_API

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetResult(
            [In, Out] double[] angles,
            [In, Out] PointAndAngle[] angleDetails
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletMidResult(
            ref MainShapeType _outMainShape,
            ref bool _outHasPin,
            ref bool _outIsEmpty,
            ref HorizonMode _outhorizonMode,
            ref RectangleF _outBasePts,
            [In, Out] EllipseFitBox[] _outBoxs);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetContourCounts(
            out int _outDropletCount,
            out int _outBaselineCount);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetContours(
            int dropletCount,
            int baselineCount,
            [In, Out] PointF[] _outDropletPts,
            [In, Out] PointF[] _outBaselinePts);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr dropletGetResultString(ErrorCode code);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern ErrorCode dropletGetLastError();

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr dropletGetResultByJsonFormat();

        /////////////////////////////////////////////////////////////////////////////////////////

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetShapeEx(
            out BaseLineShapeKind shape,
            out HorizonMode horizon,
            out bool hasPin
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsByAutoOnHorizontal(
            [In, Out] ref bool hasPin,
            [In, Out] ref bool rotated,
            [In, Out] ref HorizonMode horMode,            
            [Out] out RectangleF basePts,
            [Out] PointF[] keyPts,
            [In] bool isSubpixel,
            [In] bool isBoundSupplement
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsByAutoOnSurface(
            bool hasPin,            
            out RectangleF basePts,
            bool isSubpixel,
            bool isBoundSupplement
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsBylineOnHorizontal(
            [In] PointF[] pts,            
            out RectangleF basePts,
            bool isSubpixel
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsBylineOnSurface(
            [In] PointF[] pts,
            out RectangleF basePts,
            bool isSubpixel
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsByPointsOnHorizontal(
            [In] PointF[] pts,
            out RectangleF basePts,
            bool isSubpixel
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsByPointsOnSurface(
            [In] PointF[] pts,
            out RectangleF basePts,
            bool isSubpixel
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletGetBasepointsBySemiautoOnSurface(
            MainShapeType mainShapeType,
            int nBaselines,
            [In] PointF[] baselines,
            out RectangleF basePts,
            bool isSubpixel
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletAutoFittingOnHorizontal(
            DropletFitMode dropletFitMode,
            [In] RectangleF pts,
            out FittingInfo outInfo
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletAutoFittingOnSurface(
            MainShapeType mainShape,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            [In] RectangleF pts,
            out FittingInfo outInfo
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletManualFitting(
            MainShapeType mainShape,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            int nDroplets,
            [In] PointF[] droplets,
            int nBaselines,
            [In] PointF[] baselines,            
            bool hasPin,
            out FittingInfo outInfo,
            bool isSubpixel
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletSemiautoFitting(
            MainShapeType mainShape,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            [In] RectangleF contactRects,  // 2 elements
            int nBaselines,
            [In] PointF[] baselines,
            out FittingInfo outInfo
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern bool dropletPointsFitting(
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            [In] RectangleF pts,
            out FittingInfo outInfo
        );

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void dropletFreeFittingInfo(ref FittingInfo info);

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        private static extern IntPtr dropletGetVersion();

        /// <summary>
        /// 初始化
        /// </summary>
        /// <param name="appPath">应用程序路径</param>
        /// <returns>true:成功，false:失败	</returns>
        public static bool Init(string appPath)
        {
            return dropletInitLib(appPath);
        }

        /// <summary>
        /// 释放
        /// </summary>
        public static void Release()
        {
            dropletReleaseLib();
        }

        /// <summary>
        /// 设置图片
        /// </summary>
        /// <param name="imagePath">图片路径</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool LoadImage(string imagePath)
        {
            return dropletSetImage(imagePath);
        }

#if OLD_API
        /// <summary>
        /// 自动拟合, 接触角计算
        /// </summary>
        /// <param name="dropletFitMode">液滴拟合模式(CV椭圆拟合)</param>
        /// <param name="baselineFitMode">基线拟合模式(CV椭圆拟合)</param>
        /// <param name="mainShape">主形状(未知)</param>
        /// <param name="isSubpixel">是否亚像素拟合(false)</param>
        /// <param name="rotated">是否旋转(false)</param>
        /// <param name="horMode">水平模式(未知)</param>
        /// <param name="hasPin">是否有针(false)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool AutoFit(
            DropletFitMode dropletFitMode = DropletFitMode.eDropletEllipse,
            BaseLineFitMode baselineFitMode = BaseLineFitMode.eBaseLineEllipse,
            MainShapeType mainShape = MainShapeType.eShapeUnknown,
            bool isSubpixel = false,
            bool rotated = false,
            HorizonMode horMode = HorizonMode.eHmUnknow,
            bool hasPin = false
        ) {
            return dropletAutoFit(
                dropletFitMode,
                baselineFitMode,
                mainShape, 
                isSubpixel,
                rotated,
                horMode,
                hasPin
            );
        }

        /// <summary>
        /// 直线找基线位置, 接触角计算
        /// </summary>
        /// <param name="points">两个点的坐标为直线</param>
        /// <param name="dropletFitMode">液滴拟合模式(CV椭圆拟合)</param>
        /// <param name="baselineFitMode">基线拟合模式(CV椭圆拟合)</param>
        /// <param name="mainShape">主形状(未知)</param>
        /// <param name="isSubpixel">是否亚像素拟合(false)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool FitByBaseLine(
            PointF[] points,
            DropletFitMode dropletFitMode = DropletFitMode.eDropletEllipse,
            BaseLineFitMode baselineFitMode = BaseLineFitMode.eBaseLineEllipse,
            MainShapeType mainShape = MainShapeType.eShapeUnknown,
            bool isSubpixel = false
        ) {
            return dropletFitByBaseLine(
                points, 
                dropletFitMode,
                baselineFitMode, 
                mainShape, 
                isSubpixel
            );
        }

        /// <summary>
        /// 手动输入基线点, 接触角计算
        /// </summary>
        /// <param name="points">两个基线点的坐标</param>
        /// <param name="dropletFitMode">液滴拟合模式(CV椭圆拟合)</param>
        /// <param name="baselineFitMode">基线拟合模式(CV椭圆拟合)</param>
        /// <param name="mainShape">主形状(未知)</param>
        /// <param name="isSubpixel">是否亚像素拟合(false)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool FitByBasePoint(
            PointF[] points,
            DropletFitMode dropletFitMode = DropletFitMode.eDropletEllipse,
            BaseLineFitMode baselineFitMode = BaseLineFitMode.eBaseLineEllipse,
            MainShapeType mainShape = MainShapeType.eShapeUnknown,
            bool isSubpixel = false
        ) {
            return dropletFitByBasePoint(
                points,
                dropletFitMode,
                baselineFitMode, 
                mainShape,
                isSubpixel
            );
        }

        /// <summary>
        /// 手动输入液滴和基线点, 接触角计算	
        /// </summary>
        /// <param name="nDroplets">液滴点个数</param>
        /// <param name="droplets">液滴点坐标</param>
        /// <param name="nBaselines">基线点个数</param>
        /// <param name="baselines">基线点坐标</param>
        /// <param name="dropletFitMode">液滴拟合模式(CV椭圆拟合)</param>
        /// <param name="baselineFitMode">基线拟合模式(CV椭圆拟合)</param>
        /// <param name="mainShape">主形状(未知)</param>
        /// <param name="isSubpixel">是否亚像素拟合(false)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool ManualFit(
            int nDroplets,
            PointF[] droplets,
            int nBaselines,
            PointF[] baselines,
            DropletFitMode dropletFitMode = DropletFitMode.eDropletEllipse,
            BaseLineFitMode baselineFitMode = BaseLineFitMode.eBaseLineEllipse,
            MainShapeType mainShape = MainShapeType.eShapeUnknown,
            bool isSubpixel = false
        ) {
            return dropletManualFit(
                nDroplets,
                droplets,
                nBaselines,
                baselines,
                dropletFitMode,
                baselineFitMode,
                mainShape,
                isSubpixel
            );
        }

        /// <summary>
        /// 半自动拟合(液滴自动，基线手动), 接触角计算
        /// </summary>
        /// <param name="baselineCount">基线点个数</param>
        /// <param name="baselines">基线点坐标</param>
        /// <param name="dropletFitMode">液滴拟合模式(CV椭圆拟合)</param>
        /// <param name="baselineFitMode">基线拟合模式(CV椭圆拟合)</param>
        /// <param name="mainShape">主形状(未知)</param>
        /// <param name="isSubpixel">是否亚像素拟合(false)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool SemiAutoFit(
            int baselineCount,
            PointF[] baselines,
            DropletFitMode dropletFitMode = DropletFitMode.eDropletEllipse,
            BaseLineFitMode baselineFitMode = BaseLineFitMode.eBaseLineEllipse,
            MainShapeType mainShape = MainShapeType.eShapeUnknown,
            bool isSubpixel = false
        ) {

            return dropletSemiAutoFit(
                baselineCount,
                baselines, 
                dropletFitMode, 
                baselineFitMode,
                mainShape,
                isSubpixel
            );
        }

        /// <summary>
        /// 获取基线形状
        /// </summary>
        /// <returns>基线形状</returns>
        public static BaseLineShapeKind GetShape()
        {
            return dropletGetShape();
        }
#endif//OLD_API
        /// <summary>
        /// 获取接触角和详细信息（两个角点）
        /// </summary>
        /// <param name="angles">输出的两个接触角（单位：弧度），double[2]</param>
        /// <param name="angleDetails">输出的两个角点信息（PointAndAngle[2]），可选</param>
        /// <returns>有效输出；false: 结果无效</returns>
        public static bool GetResult(
            out double[] angles,
            out PointAndAngle[] angleDetails
        ) {
            angles = new double[2];
            angleDetails = new PointAndAngle[2];
            return dropletGetResult(angles, angleDetails);
        }

        /// <summary>
        /// 获取接触角和详细信息
        /// </summary>
        /// <param name="shape">形状</param>
        /// <param name="hasPin">是否有针</param>
        /// <param name="isEmpty">是否为空</param>
        /// <param name="horMode">水平模式</param>
        /// <param name="basePts">基线点坐标</param>
        /// <param name="boxs">液滴基线拟合框(_outBasePts[2])</param>
        /// <returns>true: 有效输出；false: 结果无效</returns>
        public static bool GetMidResult(
            out MainShapeType shape,
            out bool hasPin,
            out bool isEmpty,
            out HorizonMode horMode,
            out RectangleF basePts,
            out EllipseFitBox[] boxs
        ) {
            shape = MainShapeType.eShapeUnknown;
            hasPin = false;
            isEmpty = false;
            horMode = HorizonMode.eHmUnknow;
            basePts = new RectangleF();
            boxs = new EllipseFitBox[2];
            return dropletMidResult(ref shape, ref hasPin, ref isEmpty, ref horMode, ref basePts, boxs);
        }

        /// <summary>
        /// 获取液滴和基线轮廓点个数
        /// </summary>
        /// <param name="dropletCount">液滴点个数</param>
        /// <param name="baselineCount">基线点个数</param>
        /// <returns>true: 有效输出；false: 结果无效</returns>
        public static bool GetContourCounts(
            out int dropletCount, 
            out int baselineCount
        ) {
            return dropletGetContourCounts(
                out dropletCount,
                out baselineCount
            );
        }

        /// <summary>
        /// 获取液滴和基线轮廓点坐标
        /// </summary>
        /// <param name="dropletCount">液滴点个数</param>
        /// <param name="baselineCount">基线点个数</param>
        /// <param name="dropletPts">液滴点坐标</param>
        /// <param name="baselinePts">基线点坐标</param>
        /// <returns>true: 有效输出；false: 结果无效</returns>
        public static bool GetContours(
            int dropletCount, 
            int baselineCount, 
            out PointF[] dropletPts, 
            out PointF[] baselinePts
        ) {
            dropletPts = new PointF[dropletCount];
            baselinePts = new PointF[baselineCount];
            return dropletGetContours(
                dropletCount, 
                baselineCount, 
                dropletPts, 
                baselinePts
            );
        }


        /// <summary>
        /// 获取错误信息
        /// </summary>
        /// <param name="code">错误码</param>
        /// <returns> 错误信息</returns>
        public static string GetResultToString(ErrorCode code)
        {
            //return dropletGetResultString(code);
            IntPtr ptr = dropletGetResultString(code);
            if (ptr == IntPtr.Zero)
            {
                return string.Empty;
            }
            string resultString = Marshal.PtrToStringUni(ptr);
            Marshal.FreeBSTR(ptr); // Safe cleanup
            return resultString;
        }

        /// <summary>
        /// 获取最后错误
        /// </summary>
        /// <returns></returns>       
        public static ErrorCode GetLastError()
        {
            return dropletGetLastError();
        }

        /// <summary>
        /// 获取结果JSON格式
        /// </summary>
        /// <returns>结果JSON格式</returns>
        public static string GetResultByJsonFormatString()
        {
            IntPtr ptr = dropletGetResultByJsonFormat();
            if (ptr == IntPtr.Zero)
            {
                return string.Empty;
            }
            string jsonString = Marshal.PtrToStringUni(ptr);
            Marshal.FreeBSTR(ptr); // Safe cleanup
            return jsonString;
        }
                

        /// <summary>
        /// 基线形状
        /// </summary>
        /// <param name="shape">基线形状</param>
        /// <returns>基线形状</returns>
        public static string BaseLineShapeKindToString(BaseLineShapeKind shape)
        {
            switch (shape)
            {
                case BaseLineShapeKind.eBS_Unknown:
                    return "未知形状";
                case BaseLineShapeKind.eBS_BlankConvex:
                    return "空的凸面";
                case BaseLineShapeKind.eBS_BlankConcave:
                    return "空的凹面";
                case BaseLineShapeKind.eBS_BlankHorizontal:
                    return "空的水平面";
                case BaseLineShapeKind.eBS_ConvexWithPins:
                    return "插针凸面";
                case BaseLineShapeKind.eBS_ConcaveWithPins:
                    return "插针凹面";
                case BaseLineShapeKind.eBS_HorizontalWithPins:
                    return "插针水平面";
                case BaseLineShapeKind.eBS_ConvexWithoutPins:
                    return "没针凸面";
                case BaseLineShapeKind.eBS_ConcaveWithoutPins:
                    return "没针凹面";
                case BaseLineShapeKind.eBS_HorizontalWithoutPins:
                    return "没针水平面";
                default:
                    return "未知形状";
            }
        }

        ////////////////////////////////////////////////////////////////////


        /// <summary>
        ///  获取基线形状, 水平模式，是否有针 
        /// </summary>
        /// <param name="shape">基线形状</param>
        /// <param name="horizon">水平模式</param>
        /// <param name="hasPin">是否有针</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetShapeEx(
            out BaseLineShapeKind shape,
            out HorizonMode horizon,
            out bool hasPin
        ) {
            return dropletGetShapeEx(out shape, out horizon, out hasPin);
        }

        /// <summary>
        /// 获取基线位置(水平面，自动)
        /// </summary>
        /// <param name="hasPin">是否有针</param>
        /// <param name="rotated">是否旋转</param>
        /// <param name="horMode">水平模式</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="keyPts">输出关键点位置(左、右、液滴上中点：3个点)</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <param name="isBoundSupplement">是否边界补充(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsByAutoOnHorizontal(
            [In, Out] ref bool hasPin,
            [In, Out] ref bool rotated,
            [In, Out] ref HorizonMode horMode,            
            [Out] out RectangleF basePts,
            [Out] PointF[] keyPts,
            [In] bool isSubpixel = true,
            [In] bool isBoundSupplement = true
        ) {
            return dropletGetBasepointsByAutoOnHorizontal(
                ref hasPin,
                ref rotated,
                ref horMode,                
                out basePts,
                keyPts,
                isSubpixel,
                isBoundSupplement
            );
        }
        
        /// <summary>
        ///  获取基线位置(凸凹面，自动)
        /// </summary>
        /// <param name="hasPin">是否有针</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <param name="isBoundSupplement">是否边界补充(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsByAutoOnSurface(
            bool hasPin,            
            out RectangleF basePts,
            bool isSubpixel = true,
            bool isBoundSupplement = true
        ) {
            return dropletGetBasepointsByAutoOnSurface(
                hasPin,                 
                out basePts,
                isSubpixel,
                isBoundSupplement
            );
        }

        /// <summary>
        /// 获取基线位置(水平面，找基线)
        /// </summary>
        /// <param name="pts">两个点的坐标为直线</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsBylineOnHorizontal(
            in PointF[] pts,            
            out RectangleF basePts,
            bool isSubpixel = true
        ) {
            return dropletGetBasepointsBylineOnHorizontal(
                pts, 
                out basePts,
                isSubpixel
            );
        }

        /// <summary>
        /// 获取基线位置(凸凹面，找基线)
        /// </summary>
        /// <param name="pts">两个点的坐标为直线</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsBylineOnSurface(
            in PointF[] pts,            
            out RectangleF basePts,
            bool isSubpixel = true
        ) {
            return dropletGetBasepointsBylineOnSurface(
                pts,
                out basePts,
                isSubpixel
            );
        }

        /// <summary>
        /// 获取基线位置(水平面，找基点)
        /// </summary>
        /// <param name="pts">两个点的坐标为直线</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsByPointsOnHorizontal(
            in PointF[] pts,
            out RectangleF basePts,
            bool isSubpixel = true
        ) {
            return dropletGetBasepointsByPointsOnHorizontal(
                pts, 
                out basePts,
                isSubpixel
            );
        }

        /// <summary>
        /// 获取基线位置(凸凹面，找基点)
        /// </summary>
        /// <param name="pts">两个点的坐标为直线</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsByPointsOnSurface(
            in PointF[] pts,            
            out RectangleF basePts,
            bool isSubpixel = true
        ) {
            return dropletGetBasepointsByPointsOnSurface(
                pts, 
                out basePts,
                isSubpixel
            );
        }

        /// <summary>
        /// 获取基线位置(半自动)
        /// </summary>
        /// <param name="mainShapeType">主形状</param>
        /// <param name="nBaselines">基线点个数</param>
        /// <param name="baselines">基线点坐标</param>        
        /// <param name="basePts">输出基线位置</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool GetBasepointsBySemiautoOnSurface(
            MainShapeType mainShapeType,
            int nBaselines,
            in PointF[] baselines,            
            out RectangleF basePts,
            bool isSubpixel = true
        ) {
            return dropletGetBasepointsBySemiautoOnSurface(
                mainShapeType,
                nBaselines,
                baselines,
                out basePts,
                isSubpixel
            );
        }

        /// <summary>
        /// 自动拟合(水平面), 接触角计算(输出详细信息)
        /// </summary>
        /// <param name="dropletFitMode">液滴拟合模式</param>
        /// <param name="pts">两个基线点的坐标</param>
        /// <param name="outInfo">输出拟合信息</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool AutoFittingOnHorizontal(
            DropletFitMode dropletFitMode,
            in RectangleF pts,
            out FittingInfoCS outInfo
        ) {
            bool res = dropletAutoFittingOnHorizontal(
                dropletFitMode, 
                pts,
                out var outInfoNative
            );

            // Convert to managed
            outInfo = FittingInfoCS.FromNative(outInfoNative);

            return res;
        }

        /// <summary>
        /// 自动拟合(凸凹面), 接触角计算(输出详细信息)
        /// </summary>
        /// <param name="mainShape">主形状</param>
        /// <param name="dropletFitMode">液滴拟合模式</param>
        /// <param name="baselineFitMode">基线拟合模式</param>
        /// <param name="pts">两个基线点的坐标</param>
        /// <param name="outInfo">输出拟合信息</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool AutoFittingOnSurface(
            MainShapeType mainShape,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            [In] RectangleF pts,
            out FittingInfoCS outInfo
        ) {
            bool res = dropletAutoFittingOnSurface(
                mainShape,
                dropletFitMode,
                baselineFitMode,
                pts,
                out var outInfoNative
            );

            // Convert to managed
            outInfo = FittingInfoCS.FromNative(outInfoNative);

            return res;
        }

        /// <summary>
        /// 手动输入液滴和基线点, 接触角计算(输出详细信息)
        /// </summary>
        /// <param name="mainShape">主形状</param>
        /// <param name="dropletFitMode">液滴拟合模式</param>
        /// <param name="baselineFitMode">基线拟合模式</param>
        /// <param name="nDroplets">液滴点个数</param>
        /// <param name="droplets">液滴点坐标</param>
        /// <param name="nBaselines">基线点个数</param>
        /// <param name="baselines">基线点坐标</param>        
        /// <param name="hasPin">是否有针</param>
        /// <param name="outInfo">输出拟合信息</param>
        /// <param name="isSubpixel">是否亚像素拟合(true, 不再支持)</param>
        /// <returns>true:成功，false:失败</returns>
        public static bool ManualFitting(
            MainShapeType mainShape,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            int nDroplets,
            in PointF[] droplets,
            int nBaselines,
            in PointF[] baselines,            
            bool hasPin,
            out FittingInfoCS outInfo,
            bool isSubpixel = true
        ) {
            bool res = dropletManualFitting(
                mainShape,
                dropletFitMode,
                baselineFitMode,
                nDroplets,
                droplets,
                nBaselines,
                baselines,
                hasPin,
                out var outInfoNative,
                isSubpixel);
            outInfo = FittingInfoCS.FromNative(outInfoNative);
            return res;
        }

        /// <summary>
        /// 半自动拟合(液滴自动，基线手动), 接触角计算(输出详细信息)
        /// </summary>
        /// <param name="mainShape"></param>
        /// <param name="dropletFitMode"></param>
        /// <param name="baselineFitMode"></param>
        /// <param name="contactRects"></param>
        /// <param name="baselineCount"></param>
        /// <param name="baselines"></param>
        /// <param name="outInfo"></param>
        /// <returns></returns>
        public static bool SemiautoFitting(
            MainShapeType mainShape,
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            in RectangleF contactRects,  // 2 elements
            int baselineCount,
            in PointF[] baselines,
            out FittingInfoCS outInfo
        ) {
            bool res = DropletLib.dropletSemiautoFitting(
                mainShape,
                dropletFitMode,
                baselineFitMode,
                contactRects,
                baselineCount,
                baselines,
                out var outInfoNative);
            outInfo = FittingInfoCS.FromNative(outInfoNative);
            return res;
        }

        /// <summary>
        /// 液滴点拟合, 接触角计算(输出详细信息)
        /// </summary>
        /// <param name="dropletFitMode"></param>
        /// <param name="baselineFitMode"></param>
        /// <param name="basePoints"></param>
        /// <param name="fittingInfoCS"></param>
        /// <returns></returns>
        public static bool PointsFitting(
            DropletFitMode dropletFitMode,
            BaseLineFitMode baselineFitMode,
            in RectangleF basePoints,
            out FittingInfoCS fittingInfoCS
        ) {
            bool res = dropletPointsFitting(dropletFitMode, baselineFitMode, basePoints, out var outInfoNative);
            fittingInfoCS = FittingInfoCS.FromNative(outInfoNative);
            return res;
        }

        public static string GetVersion()
        {
            IntPtr ptr = dropletGetVersion();
            if (ptr == IntPtr.Zero)
            {
                return string.Empty;
            }
            string versionString = Marshal.PtrToStringUni(ptr);
            Marshal.FreeBSTR(ptr); // Safe cleanup
            return versionString;
        }

        // JSON field names
        public const string JSNAME_INPUT = "input";
        public const string JSNAME_OUTPUT = "result";

        // input
        public const string JSNAME_IMAGEPATH = "image_path";                // image path (in)
        public const string JSNAME_FITMODE = "fit_mode";                    // fitting mode(auto, bypoint, byline, semi-auto, manual) (in)
        public const string JSNAME_MAINSHAPE = "main_shape";            // main shape(convex, concave, horizontal) (in)

        public const string JSNAME_USESUBPIXEL = "use_subpixel";            // use subpixel (in)
        public const string JSNAME_ROTATED = "rotated";             // rotated (in/out)
        public const string JSNAME_HASPIN = "has_pin";                  // has pin (in/out)
        public const string JSNAME_HORIZONMODE = "horizon_mode";            // horizontal mode (in/out)
        public const string JSNAME_BASEPTS = "base_points";             // 2 base points (in/out)
        public const string JSNAME_MANUALBSLINEPTS = "manual_baseline_points";  // manual baseline points (in)
        public const string JSNAME_MANUALDROPLETPTS = "manual_droplet_points";      // manual droplet points (in)

        public const string JSNAME_DROPLETFITMODE = "droplet_fit_mode";         // droplet fitting mode (in)
        public const string JSNAME_BASELINEFITMODE = "baseline_fit_mode";       // baseline fitting mdoe (in)

        // output
        public const string JSNAME_RESCODE = "code";

        public const string JSNAME_KEYPTS = "key_points";
        public const string JSNAME_BSLINESHAPE = "baseline_shape";

        public const string JSNAME_DROPBOX = "droplet_box";
        public const string JSNAME_DROPLEFTBOX = "left_drop_box";
        public const string JSNAME_DROPRIGHTBOX = "right_drop_box";
        public const string JSNAME_BASELINEBOX = "baseline_box";
        public const string JSNAME_DROPCONTOUR = "droplet_contour";
        public const string JSNAME_BASELINECONTOUR = "baseline_contour";
        public const string JSNAME_LEFTPOLYLINE = "left_polyline";
        public const string JSNAME_RIGHTPOLYLINE = "right_polyline";

        public const string JSNAME_RESULTLEFT = "left_angle";
        public const string JSNAME_RESULTRIGHT = "right_angle";
        public const string JSNAME_RESPOINTS = "points";
        public const string JSNAME_RESANGLES = "angles";
        public const string JSNAME_RESVEC1 = "vec1";
        public const string JSNAME_RESVEC2 = "vec2";
    }
}
//.EOF
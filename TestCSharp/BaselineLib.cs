using System.Runtime.InteropServices;

namespace JiaoShuiLibCSharp
{
    /// <summary>
    /// Represents the return codes for model initialization and image processing operations. // 返回值
    /// </summary>
    public enum RetCode : int
    {
        /// <summary>
        /// Model loading failed (file not found or invalid model file). // 模型加载失败（找不到模型文件，或者不是有效的模型文件；JiaoShui.cfg，JiaoShui.weights）
        /// </summary>
        RET_MODEL = -2,

        /// <summary>
        /// unknown error. // 未知错误
        /// </summary>
        RET_UNKNOWN = -1,

        /// <summary>
        /// Operation succeeded. // 成功
        /// </summary>
        RET_OK = 0,

        /// <summary>
        /// A global exception occurred. // 全局异常处理
        /// </summary>
        RET_EXCEPTION = 1,

        /// <summary>
        /// Failed to open the image. // 打不开图片
        /// </summary>
        RET_OPEN = 2,

        /// <summary>
        /// No effective droplets detected. // 检测不到有效液滴
        /// </summary>
        RET_NOGLUE = 3,

        /// <summary>
        /// Fitting analysis failed. // 拟合分析失败
        /// </summary>
        RET_NOTFOUND = 4
    }


    public class BaselineLib
    {
#if DEBUG
        const string dllPath = "baselined.dll";
#else
        const string dllPath = "baseline.dll";
#endif
        /// <summary>
        /// Initializes the analysis (baseline) DLL. Must be called before using any other functionality.
        /// 通常在程序启动时调用，用于初始化分析库。 // baseline DLL 初始化
        /// </summary>
        /// <param name="appPath">
        /// The full path to the application directory or model files.
        /// 应用程序的路径或模型文件所在路径。
        /// </param>
        /// <returns>
        /// Returns true if initialization succeeded; otherwise, false.
        /// 如果初始化成功返回 true，否则返回 false。
        /// </returns>
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern bool initializeBaselineLibrary(string appPath);


        /// <summary>
        /// Releases and cleans up resources used by the baseline analysis DLL. 
        /// This should be called before application exit to ensure proper cleanup.
        /// baseline DLL 释放函数，应在程序退出前调用以释放资源。
        /// </summary>
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern void releaseBaselineLibrary();


        /// <summary>
        /// Performs image recognition and feature extraction using the baseline analysis DLL. // 图像识别与特征提取
        /// </summary>
        /// <param name="imageName">
        /// The full path to the image to be processed. // 图像路径
        /// </param>
        /// <param name="mode">
        /// The recognition mode. Typically defined by an enum or preset value. // 模式
        /// </param>
        /// <param name="has_pin">
        /// Whether pins are present (1 for yes, 0 for no). // 是否有针（1表示有针，0表示无针）
        /// </param>
        /// <param name="inclined">
        /// Whether the object is inclined (true/false). // 是否倾斜
        /// </param>
        /// <param name="boundary_supplement">
        /// Whether to perform boundary supplementation (true/false). // 是否边界补充
        /// </param>
        /// <param name="dimensions_c">
        /// An output array to receive calculated dimensions or results. // 输出的尺寸信息（数组）
        /// </param>
        /// <returns>
        /// A return code indicating success or specific failure. // 返回识别状态码
        /// </returns>
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
        public static extern RetCode DetectDropletOnHorizontalSurface(
            string imageName,
            int mode,
            int has_pin,
            [MarshalAs(UnmanagedType.Bool)] bool inclined,
            [MarshalAs(UnmanagedType.Bool)] bool boundary_supplement,
            [Out] int[] dimensions_c
        );


        public static string ReultToString(RetCode retCode)
        {
            switch (retCode)
            {
                case RetCode.RET_MODEL:
                    return "模型加载失败";
                case RetCode.RET_OK:
                    return "成功";
                case RetCode.RET_EXCEPTION:
                    return "全局异常处理";
                case RetCode.RET_OPEN:
                    return "打不开图片";
                case RetCode.RET_NOGLUE:
                    return "检测不到有效液滴";
                case RetCode.RET_NOTFOUND:
                    return "拟合分析失败";
                default:
                    return "未知错误";
            }
        }
    }
}

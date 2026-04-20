#ifndef __JIAOSHUI_CV_COMMON_H__
#define __JIAOSHUI_CV_COMMON_H__

#include <tchar.h>
#include <opencv2/opencv.hpp>
#include <string>
#include "../inc/common.h"

#if DEBUG_IMG
#	define DEBUG_IMAGEPATH "D:\\TestImage\\"
#else
#	define DEBUG_IMAGEPATH ""
#endif

void SetCurrentDirToExecutablePath(const TCHAR* appPath);
/*
* TCHAR to std::string
* @param tstr: TCHAR string
* @return std::string
*/
std::string TCHARToStdString(const TCHAR* tstr);

/*
* TCHAR to cv::String
*/
cv::String TCHARToCvString(const TCHAR* tstr);
/*
* load image from unicode path
* @param path: unicode path
*/
cv::Mat loadImageFromUnicodePath(const TCHAR* path);

/*
* save image to unicode path
* @param path: unicode path
* @param img: image to save
*/
void saveImageToUnicodePath(const TCHAR* path, const cv::Mat& img);

/*
* save debug image
* @param name: image name
* @param img: image to save
*/
void saveDebugImage(const char* name, cv::Mat& img, bool isRoot = false);
void saveFnDbgImg(const char* name, cv::Mat& img, const char* fn, int ln, bool isRoot = false);
#define SaveDebugImg(name, img) saveFnDbgImg(name, img, __FUNCTION__, __LINE__, false)

/*
* load image from utf8 path
* @param wpath: utf8 path
* @param flags: cv::imread flags
* @return cv::Mat
*/
cv::Mat imread_utf8(const std::wstring& wpath, int flags = cv::IMREAD_COLOR);

/*
* Convert wchar_t (UTF-16) to UTF-8 std::string
*/
std::string wchar_to_utf8(const wchar_t* wstr);

/*
* Convert UTF-8 std::string to wchar_t (UTF-16)
*/
std::wstring utf8_to_wchar(const std::string& str);
#endif//__JIAOSHUI_CV_COMMON_H__
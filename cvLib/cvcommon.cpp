#include "cvcommon.h"
#include <windows.h>
#include <locale>
#include <codecvt>
#include "common.h"
#include <fstream>

#include <windows.h>
#include <vector>
#include <opencv2/core.hpp>
#include <shlwapi.h>  // For PathRemoveFileSpec
#pragma comment(lib, "Shlwapi.lib")

#ifdef _DEBUG
#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>
#	define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

std::string TCHARToStdString(const TCHAR* tstr)
{
#ifdef UNICODE
	if (tstr == nullptr) return std::string();

	// Calculate required buffer size (UTF-8 encoding)
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, tstr, -1, NULL, 0, NULL, NULL);
	if (utf8_size <= 0)
		return std::string();

	// Allocate buffer correctly (includes null terminator)
	std::vector<char> utf8_str(utf8_size);

	// Convert wchar_t (TCHAR in UNICODE) to UTF-8
	WideCharToMultiByte(CP_UTF8, 0, tstr, -1, utf8_str.data(), utf8_size, NULL, NULL);

	// Return std::string without the trailing null
	return std::string(utf8_str.data());
#else
	// ANSI mode: simple assignment
	return (tstr != nullptr) ? std::string(tstr) : std::string();
#endif
}


void SetCurrentDirToExecutablePath(const TCHAR* appPath)
{
	TCHAR exePath[MAX_PATH];
	_tcscpy_s(exePath, MAX_PATH, appPath);
	PathRemoveFileSpec(exePath);
	SetCurrentDirectory(exePath);
}


std::string wchar_to_utf8(const wchar_t* wstr)
{
	if (!wstr) return {};

	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	std::string strTo(size_needed - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, NULL, NULL);
	return strTo;
}


std::wstring utf8_to_wchar(const std::string& str)
{
	if (str.empty()) return {};

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	std::wstring wstrTo(size_needed - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstrTo[0], size_needed);
	return wstrTo;
}

cv::String TCHARToCvString(const TCHAR* tstr)
{
#ifdef UNICODE
	// TCHAR is wchar_t (Unicode)
	if (tstr == nullptr)
		return cv::String();

	// Determine required buffer size for UTF-8
	int utf8Length = WideCharToMultiByte(CP_UTF8, 0, tstr, -1, nullptr, 0, nullptr, nullptr);
	if (utf8Length <= 0)
		return cv::String();

	std::vector<char> utf8Buffer(utf8Length);

	// Convert to UTF-8
	WideCharToMultiByte(CP_UTF8, 0, tstr, -1, utf8Buffer.data(), utf8Length, nullptr, nullptr);

	// Return as OpenCV string (cv::String is std::string)
	return cv::String(utf8Buffer.data());
#else
	// TCHAR is char (ANSI), direct copy
	return (tstr != nullptr) ? cv::String(tstr) : cv::String();
#endif
}


cv::Mat loadImageFromUnicodePath(const TCHAR* path)
{
#ifdef UNICODE
	std::wstring wpath(path);
#else
	// If ANSI mode, convert to wide-string first
	int size_needed = MultiByteToWideChar(CP_ACP, 0, path, -1, NULL, 0);
	std::wstring wpath(size_needed, 0);
	MultiByteToWideChar(CP_ACP, 0, path, -1, &wpath[0], size_needed);
#endif

	std::ifstream file(wpath, std::ios::binary);
	if (!file.is_open())
		return cv::Mat();

	std::vector<uchar> buffer(std::istreambuf_iterator<char>(file), {});
	return cv::imdecode(buffer, cv::IMREAD_COLOR);
}


void saveImageToUnicodePath(const TCHAR* path, const cv::Mat& img)
{
	if (!path || img.empty()) 
		return;

	// Extract extension (e.g., ".jpg")
	const TCHAR* extPtr = PathFindExtension(path);
	if (!extPtr || extPtr[0] == '\0') return;

	// Convert TCHAR extension to UTF-8 for OpenCV
#ifdef UNICODE
	int len = WideCharToMultiByte(CP_UTF8, 0, extPtr, -1, NULL, 0, NULL, NULL);
	std::string extUtf8(len - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, extPtr, -1, (char*)extUtf8.data(), len, NULL, NULL);
#else
	std::string extUtf8(extPtr);
#endif

	// Convert to lowercase for consistency
	std::transform(extUtf8.begin(), extUtf8.end(), extUtf8.begin(), ::tolower);

	// Encode image to buffer
	std::vector<uchar> buffer;
	if (!cv::imencode(extUtf8, img, buffer))
		return;

	// Save with Unicode-safe file handle
	FILE* f = NULL;
	_tfopen_s(&f, path, _T("wb"));
	if (f) {
		fwrite(buffer.data(), 1, buffer.size(), f);
		fclose(f);
	}
}

void saveDebugImage(const char* name, cv::Mat& img, bool isRoot)
{
	if (!name || img.empty())
		return;

	// Convert char* name → wchar_t (TCHAR) if in UNICODE build
#ifdef UNICODE
	wchar_t wName[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wName, MAX_PATH);
#else
	char wName[MAX_PATH];
	strcpy_s(wName, name);
#endif

	// Build full path (DEBUG_IMAGEPATH + name)
	TCHAR fullPath[MAX_PATH * 2];
	_tcscpy_s(fullPath, _T(DEBUG_IMAGEPATH));
	_tcscat_s(fullPath, wName);

	if(isRoot)
		_tcscpy_s(fullPath, wName);
	// Save image using Unicode-safe function
	saveImageToUnicodePath(fullPath, img);
}

void saveFnDbgImg(
	const char* name, 
	cv::Mat& img, 
	const char* fn, 
	int ln, 
	bool isRoot
) {
	static int count = 0;
	char fullname[256];
	count++;
	sprintf_s(fullname, "%d-%s(%s-%d).png", count, name, fn, ln);
	char* str = fullname;
	while (*str) { // Replace invalid characters '::' with '.'
		if (*str == ':') *str = '.';
		str++;
	}
	saveDebugImage(fullname, img, isRoot);
}

cv::Mat imread_utf8(const std::wstring& wpath, int flags/* = cv::IMREAD_COLOR*/)
{
	// Convert wide path to UTF-8
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	std::string utf8_path = conv.to_bytes(wpath);

	// Set the locale to UTF-8 (modern Windows)
	std::locale::global(std::locale(".UTF8"));

	return cv::imread(utf8_path, flags);
}


// Demo.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// 主符号
#include <gdiplus.h>

// CDemoApp:
// 有关此类的实现，请参阅 Demo.cpp
//

class CDemoApp : public CWinApp
{
public:
	CDemoApp();

// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance(); // to shut down GDI+
// 实现
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	DECLARE_MESSAGE_MAP()
};

extern CDemoApp theApp;

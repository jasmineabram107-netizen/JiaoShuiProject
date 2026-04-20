
// DemoDlg.h: 头文件
//

#pragma once
#include <vector>
#include "../inc/common.h"
#include "PicWndDemo.h"
#include "DoubleEdit.h"
#include "TestRunner.h"  // include the class
#include "AutorecRunner.h"
#include <memory>

// CDemoDlg 对话框
class CDemoDlg : public CDialogEx
{
// 构造
public:
	CDemoDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CDemoDlg();
	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEMO_DIALOG };
#endif
private:
	void DoRecog(bool isEmpty = false);
#if _DEV_OLDSTYLE_API
	void DoRecog2(bool isEmpty = false);
#endif//_DEV_OLDSTYLE_API
	// filter
	void showCurrentImage();
	void NextImage();
	void PrevImage();
	void writeResult();
	void getTxtFile(CString &imageName, CString &txtName);
	void InitControls();
private:
	// base line
	RectangleF m_baseRect;

	// control params
	int m_result_code;

	DropletFitMode	m_upFlag;
	int				m_upFlagInt;

	BaseLineFitMode m_dnFlag;
	int				m_dnFlagInt;

	BaseLineShapeKind m_detailedShape;	
	
	MainShapeType	m_mainShape;		
	int				m_mainShapeInt;

	FitMode			m_fit_mode;
	int				m_fit_modeInt;
	PointF			m_keyPts[3];
			
	CString dstFolder, txt_folder;
	CString imageName;
	CString preTitle;	

	std::vector<CString> images_all;
	int images_num;
	int cur_image_index;
	int max_load_images_num;
	bool isProcessing;
	bool m_bHasPin;
	bool imageLoaded;
	BOOL m_bInit;
	BOOL m_bInternalUpdate;

	CPicEditWnd			m_wndPic;
	CProgressCtrl m_progressCtrl;
	CButton m_rdBasePtMode;
	CDoubleEdit m_edtLeftX;
	CDoubleEdit m_edtLeftY;
	CDoubleEdit m_edtRightX;
	CDoubleEdit m_edtRightY;
	CButton m_chkPin;
	CButton m_rdDropletFit;
	CButton m_rdBaselineFit;
	CButton m_rdMainShape;
	CButton m_rdFitMode;
	CEdit m_edtResultText;
	CEdit m_edtLeftAngle;
	CEdit m_edtRightAngle;
	CButton m_chkSubPixel;
	CButton m_chkTest;
	CButton m_chkManualAngleInput;
	//CMenu m_menu;
	CButton m_btnTest;
	CButton m_btnRecorgnize;
	CButton m_chkInclined;
	int m_iModeOnHor;
	CEdit m_edtPath;

	CButton m_chkManualMode;
	CEdit m_edtTestAngle;	
	CListCtrl m_lstFiles;
	CToolTipCtrl m_toolTip;

	// for threads
	std::unique_ptr<CTestRunner>	m_runnerForTest;
	BOOL							m_isRunningTestThread;	
	BOOL							m_isRunningAutoThread;
	std::unique_ptr<CAutorecRunner>	m_runnerForAutorec;	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void UpdateProcControlStates();
	void UpdateManualModeState(BOOL bEnable = TRUE);
	void drawFitResult(const FittingInfo* outInfo, const PointF& left, const PointF& right);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	//afx_msg void OnMenuOpenFolder();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedRadioManualBaseLine();
	afx_msg void OnBnClickedRadioAutoBaseLine();	
	afx_msg void OnBnClickedRadioLowerTumian();
	afx_msg void OnBnClickedRadioLowerAomian();
	afx_msg void OnBnClickedRadioLowerLine();
	afx_msg void OnBnClickedRadioRegressionEllipse();
	afx_msg void OnBnClickedRadioRegressionOpencv();
	afx_msg void OnBnClickedRadioRegressionCircle();
	afx_msg void OnBnClickedRadioRegressionCircleDn();
	afx_msg void OnBnClickedRadioRegressionEllipseDn();
	afx_msg void OnBnClickedRadioRegressionOpencvDn();
	afx_msg void OnBnClickedRadioFitAuto();
	afx_msg void OnBnClickedRadioFitManual();
	afx_msg LRESULT OnFinishedEdit(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReadyProc(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTestThreadProc(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAutoThreadProc(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedRadioRegressionOpencvDirect();
	afx_msg void OnBnClickedRadioRegressionOpencvDn2();
	afx_msg void OnBnClickedRadioRegressionDoubleEllipse();
	afx_msg void OnBnClickedRadioRegressionDoublePolynomial();
	afx_msg void OnBnClickedCheckSubpixel();
	afx_msg void OnBnClickedBtnZoomOut();	
	afx_msg void OnBnClickedRadioHighWidth();
	afx_msg void OnBnClickedRadioRegressionPolynomial();
	afx_msg void OnBnClickedRadioFindBasepoints();

	afx_msg void OnBnClickedButtonRecognize();
	afx_msg void OnBnClickedCheckPin();
	afx_msg void OnBnClickedRadioRegressionCircle2();
	afx_msg void OnChkTestClick();
	afx_msg void OnBnClickedButtonTest();	
	afx_msg void OnChkManualInput();
	afx_msg void OnDestroy();	
	
	afx_msg void OnBnClickedBtnBrowser();
	afx_msg void OnBnClickedRdModeLens();
	afx_msg void OnBnClickedCheckIncline();
	
	afx_msg void OnLbnDblclkListImageNames();
	
	afx_msg void OnBnClickedChkSetManual();
	afx_msg void OnBnClickedRdModeHolu();
	afx_msg void OnBnClickedRdModeInside();
	afx_msg void OnBnClickedRdModeMushroom();

	afx_msg void OnBnClickedRadioFindSemiauto();
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg void OnNMDblclkListFiles(NMHDR* pNMHDR, LRESULT* pResult);
	CStatic m_lblFiles;
	afx_msg void OnBnClickedChkIgnoreOld();
	afx_msg void OnBnClickedBtnAutorecog();
	CButton m_btnAutoRecog;
	CButton m_chkIgnoreOld;
	afx_msg void OnBnClickedChkResAngle();
	afx_msg void OnBnClickedChkResFitting();
	afx_msg void OnBnClickedChkResBasept();
	CButton m_chkResAngle;
	CButton m_chkResFitting;
	CButton m_chkResBasePt;
	afx_msg void OnBnClickedChkBoundary();
	CButton m_chkBoundary;
};

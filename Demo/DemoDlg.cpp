
// DemoDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Demo.h"
#include "DemoDlg.h"
#include "afxdialogex.h"
#include "JiaoShuiEllipseFit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static void SetNormalRect(CRect& rect, int left, int top, int width, int height)
{
	// set it
	rect.left = left;
	rect.top = top;
	rect.right = left + width;
	rect.bottom = top + height;

	// normalize it
	int nTemp;
	if (rect.left > rect.right) {
		nTemp = rect.left;
		rect.left = rect.right;
		rect.right = nTemp;
	}
	if (rect.top > rect.bottom) {
		nTemp = rect.top;
		rect.top = rect.bottom;
		rect.bottom = nTemp;
	}
}

// CDemoDlg 对话框

CDemoDlg::CDemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DEMO_DIALOG, pParent)
	, m_result_code(0)
	, m_upFlagInt(eDropletEllipseDirect)
	, m_upFlag(eDropletEllipseDirect)
	, m_dnFlagInt(eBaseLineCircle)
	, m_dnFlag(eBaseLineCircle)
	, m_mainShape(eShapeConvex)
	, m_mainShapeInt(eShapeConvex)
	, m_fit_mode(eFitAuto)
	, m_fit_modeInt(eFitAuto)
	, m_bHasPin(false)
	, images_num(0)
	, cur_image_index(-1)
	, max_load_images_num(10000)
	, imageLoaded(false)
	, isProcessing(false)
	, m_iModeOnHor(0)
	, m_bInit(FALSE)
	, m_isRunningTestThread(FALSE)
	, m_bInternalUpdate(FALSE)
	, m_detailedShape(eBS_Unknown)
	, m_isRunningAutoThread(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_baseRect = { 243, 299, 443, 301 };
}

CDemoDlg::~CDemoDlg()
{	
}

void CDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LEFT_X, m_baseRect.left);
	DDX_Text(pDX, IDC_EDIT_LEFT_Y, m_baseRect.top);
	DDX_Text(pDX, IDC_EDIT_RIGHT_X, m_baseRect.right);
	DDX_Text(pDX, IDC_EDIT_RIGHT_Y, m_baseRect.bottom);
	DDX_Control(pDX, IDC_EDIT_LEFT_X, m_edtLeftX);
	DDX_Control(pDX, IDC_EDIT_LEFT_Y, m_edtLeftY);
	DDX_Control(pDX, IDC_EDIT_RIGHT_X, m_edtRightX);
	DDX_Control(pDX, IDC_EDIT_RIGHT_Y, m_edtRightY);

	DDX_Control(pDX, IDC_PROGRESS_TEST, m_progressCtrl);
	DDX_Control(pDX, IDC_RADIO_MANUAL_BASE_LINE, m_rdBasePtMode);
	DDX_Control(pDX, IDC_CHECK_PIN, m_chkPin);
	DDX_Control(pDX, IDC_RADIO_REGRESSION_CIRCLE, m_rdDropletFit);
	DDX_Control(pDX, IDC_RADIO_REGRESSION_CIRCLE_DN, m_rdBaselineFit);
	DDX_Control(pDX, IDC_RADIO_LOWER_TUMIAN, m_rdMainShape);
	DDX_Control(pDX, IDC_RADIO_FIT_AUTO, m_rdFitMode);
	DDX_Control(pDX, IDC_EDIT_RESULT, m_edtResultText);
	DDX_Control(pDX, IDC_EDIT_ANGLE_LEFT, m_edtLeftAngle);
	DDX_Control(pDX, IDC_EDIT_ANGLE_RIGHT, m_edtRightAngle);
	DDX_Control(pDX, IDC_CHECK_SUBPIXEL, m_chkSubPixel);
	DDX_Control(pDX, IDC_CHECK_TEST, m_chkTest);
	DDX_Control(pDX, IDC_CHECK_MANUAL_INPUT, m_chkManualAngleInput);
	DDX_Control(pDX, IDC_BUTTON_TEST, m_btnTest);
	DDX_Control(pDX, IDC_BUTTON_RECOGNIZE, m_btnRecorgnize);
	DDX_Control(pDX, IDC_CHECK_INCLINE, m_chkInclined);
	DDX_Radio(pDX, IDC_RD_MODE_LENS, m_iModeOnHor);
	DDX_Control(pDX, IDC_EDIT1, m_edtPath);
	DDX_Radio(pDX, IDC_RADIO_FIT_AUTO, m_fit_modeInt);
	DDX_Radio(pDX, IDC_RADIO_LOWER_TUMIAN, m_mainShapeInt);
	DDX_Radio(pDX, IDC_RADIO_REGRESSION_CIRCLE, m_upFlagInt);
	DDX_Radio(pDX, IDC_RADIO_REGRESSION_CIRCLE_DN, m_dnFlagInt);
	DDX_Control(pDX, IDC_CHK_SET_MANUAL, m_chkManualMode);
	DDX_Control(pDX, IDC_EDT_ANGLE_INPUT, m_edtTestAngle);
	DDX_Control(pDX, IDC_LIST_FILES, m_lstFiles);
	DDX_Control(pDX, IDC_LBL_FILES, m_lblFiles);
	DDX_Control(pDX, IDC_BTN_AUTORECOG, m_btnAutoRecog);
	DDX_Control(pDX, IDC_CHK_IGNORE_OLD, m_chkIgnoreOld);
	DDX_Control(pDX, IDC_CHK_RES_ANGLE, m_chkResAngle);
	DDX_Control(pDX, IDC_CHK_RES_FITTING, m_chkResFitting);
	DDX_Control(pDX, IDC_CHK_RES_BASEPT, m_chkResBasePt);
	DDX_Control(pDX, IDC_CHK_BOUNDARY, m_chkBoundary);
}

BEGIN_MESSAGE_MAP(CDemoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_COMMAND(ID_MENU_OPEN_FOLDER, &CDemoDlg::OnMenuOpenFolder)
	ON_BN_CLICKED(IDC_RADIO_MANUAL_BASE_LINE, &CDemoDlg::OnBnClickedRadioManualBaseLine)
	ON_BN_CLICKED(IDC_RADIO_AUTO_BASE_LINE, &CDemoDlg::OnBnClickedRadioAutoBaseLine)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_RADIO_LOWER_TUMIAN, &CDemoDlg::OnBnClickedRadioLowerTumian)
	ON_BN_CLICKED(IDC_RADIO_LOWER_AOMIAN, &CDemoDlg::OnBnClickedRadioLowerAomian)
	ON_BN_CLICKED(IDC_RADIO_LOWER_LINE, &CDemoDlg::OnBnClickedRadioLowerLine)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_ELLIPSE, &CDemoDlg::OnBnClickedRadioRegressionEllipse)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_OPENCV, &CDemoDlg::OnBnClickedRadioRegressionOpencv)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_CIRCLE, &CDemoDlg::OnBnClickedRadioRegressionCircle)

	ON_BN_CLICKED(IDC_RADIO_REGRESSION_CIRCLE_DN, &CDemoDlg::OnBnClickedRadioRegressionCircleDn)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_ELLIPSE_DN, &CDemoDlg::OnBnClickedRadioRegressionEllipseDn)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_OPENCV_DN, &CDemoDlg::OnBnClickedRadioRegressionOpencvDn)
	ON_BN_CLICKED(IDC_RADIO_FIT_AUTO, &CDemoDlg::OnBnClickedRadioFitAuto)
	ON_BN_CLICKED(IDC_RADIO_FIT_MANUAL, &CDemoDlg::OnBnClickedRadioFitManual)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_OPENCV_DIRECT, &CDemoDlg::OnBnClickedRadioRegressionOpencvDirect)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_OPENCV_DN2, &CDemoDlg::OnBnClickedRadioRegressionOpencvDn2)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_DOUBLE_ELLIPSE, &CDemoDlg::OnBnClickedRadioRegressionDoubleEllipse)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_DOUBLE_POLYNOMIAL, &CDemoDlg::OnBnClickedRadioRegressionDoublePolynomial)
	ON_BN_CLICKED(IDC_CHECK_SUBPIXEL, &CDemoDlg::OnBnClickedCheckSubpixel)
	ON_BN_CLICKED(IDC_BTN_ZOOM_OUT, &CDemoDlg::OnBnClickedBtnZoomOut)
	ON_BN_CLICKED(IDC_RADIO_HIGH_WIDTH, &CDemoDlg::OnBnClickedRadioHighWidth)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_POLYNOMIAL, &CDemoDlg::OnBnClickedRadioRegressionPolynomial)
	ON_BN_CLICKED(IDC_RADIO_FIND_BASEPOINTS, &CDemoDlg::OnBnClickedRadioFindBasepoints)
	ON_BN_CLICKED(IDC_BUTTON_RECOGNIZE, &CDemoDlg::OnBnClickedButtonRecognize)
	ON_BN_CLICKED(IDC_CHECK_PIN, &CDemoDlg::OnBnClickedCheckPin)
	ON_BN_CLICKED(IDC_RADIO_REGRESSION_CIRCLE2, &CDemoDlg::OnBnClickedRadioRegressionCircle2)
	ON_BN_CLICKED(IDC_CHECK_TEST, &CDemoDlg::OnChkTestClick)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CDemoDlg::OnBnClickedButtonTest)	
	ON_BN_CLICKED(IDC_CHECK_MANUAL_INPUT, &CDemoDlg::OnChkManualInput)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_BROWSER, &CDemoDlg::OnBnClickedBtnBrowser)

	ON_BN_CLICKED(IDC_CHECK_INCLINE, &CDemoDlg::OnBnClickedCheckIncline)
	ON_BN_CLICKED(IDC_CHK_SET_MANUAL, &CDemoDlg::OnBnClickedChkSetManual)

	ON_BN_CLICKED(IDC_RD_MODE_LENS, &CDemoDlg::OnBnClickedRdModeLens)	
	ON_BN_CLICKED(IDC_RD_MODE_HOLU, &CDemoDlg::OnBnClickedRdModeHolu)
	ON_BN_CLICKED(IDC_RD_MODE_INSIDE, &CDemoDlg::OnBnClickedRdModeInside)
	ON_BN_CLICKED(IDC_RD_MODE_MUSHROOM, &CDemoDlg::OnBnClickedRdModeMushroom)

	ON_REGISTERED_MESSAGE(WM_FINISHED_EDIT_MSG, &CDemoDlg::OnFinishedEdit)
	ON_REGISTERED_MESSAGE(WM_READY_RECORG_MSG, &CDemoDlg::OnReadyProc)
	ON_REGISTERED_MESSAGE(UM_DROPLET_AUTOTESTTHREAD_MSG, &CDemoDlg::OnTestThreadProc)
	ON_REGISTERED_MESSAGE(UM_DROPLET_AUTORECOGTHREAD_MSG, &CDemoDlg::OnAutoThreadProc)

	ON_BN_CLICKED(IDC_RADIO_FIND_SEMIAUTO, &CDemoDlg::OnBnClickedRadioFindSemiauto)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILES, &CDemoDlg::OnNMDblclkListFiles)
	ON_BN_CLICKED(IDC_CHK_IGNORE_OLD, &CDemoDlg::OnBnClickedChkIgnoreOld)
	ON_BN_CLICKED(IDC_BTN_AUTORECOG, &CDemoDlg::OnBnClickedBtnAutorecog)
	ON_BN_CLICKED(IDC_CHK_RES_ANGLE, &CDemoDlg::OnBnClickedChkResAngle)
	ON_BN_CLICKED(IDC_CHK_RES_FITTING, &CDemoDlg::OnBnClickedChkResFitting)
	ON_BN_CLICKED(IDC_CHK_RES_BASEPT, &CDemoDlg::OnBnClickedChkResBasept)
	ON_BN_CLICKED(IDC_CHK_BOUNDARY, &CDemoDlg::OnBnClickedChkBoundary)
END_MESSAGE_MAP()


// CDemoDlg 消息处理程序
CString GetAppPath()
{
	TCHAR szFullPath[MAX_PATH];
	::GetModuleFileName(NULL, szFullPath, MAX_PATH);

	CString strAppPath(szFullPath);
	int nPos = strAppPath.ReverseFind(_T('\\'));

	if (nPos != -1)
		strAppPath = strAppPath.Left(nPos + 1);

	return strAppPath; // includes trailing '\'
}

BOOL CDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
		
	InitControls();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CDemoDlg::InitControls()
{
	m_fit_mode = eFitAuto;
	m_fit_modeInt = eFitAuto;
	m_chkSubPixel.SetCheck(BST_UNCHECKED); // 选中子像素
	isProcessing = false;

	// 测试		
	m_btnTest.EnableWindow(FALSE);
	m_progressCtrl.ShowWindow(SW_HIDE);
	m_chkManualAngleInput.EnableWindow(FALSE);
	m_edtTestAngle.EnableWindow(FALSE);

	m_progressCtrl.SetRange(0, 100);
	m_progressCtrl.SetPos(0);
	m_edtTestAngle.SetWindowText(_T("0.0"));

	UpdateProcControlStates();

	CString sPath = GetAppPath();
	// cv::String cvPath = CStringToCvString(sPath);
	bool res = dropletInitLib(sPath);
	if (!res)
		MessageBox(_T("CEPH DLL初始化失败！"));

	CRect rcWnd;
	GetDlgItem(IDC_PIC_IMAGE)->GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);
	m_wndPic.CreateWnd(this, rcWnd, IDC_PIC_IMAGE, 0);

	m_toolTip.Create(this);
	m_toolTip.SetTipBkColor(RGB(0, 255, 255));
	m_toolTip.AddTool(GetDlgItem(IDC_RADIO_FIT_AUTO), _T(
		"角度会自动计算，无需特别设置。如果基线形状为水平面，可以开启“手动设置”，手动调整倾斜角、插针位置和处理方式。"
	));
	m_toolTip.AddTool(GetDlgItem(IDC_RADIO_MANUAL_BASE_LINE), _T(
		"可以通过输入或鼠标左键点击来指定两个基点。基点绘制完成后，可用鼠标左键选中并拖动进行移动。"
	));
	m_toolTip.AddTool(GetDlgItem(IDC_RADIO_FIND_BASEPOINTS), _T(
		"按住鼠标左键并拖动，可绘制一条经过基点的线段。"
	));
	m_toolTip.AddTool(GetDlgItem(IDC_RADIO_FIND_SEMIAUTO), _T(
		"在基线上使用鼠标左键绘制点。如果基线是水平的-2个点；曲面-3个点用于圆形拟合，或5个点用于椭圆拟合。点绘制完成后，可用鼠标左键选中并拖动移动。"
	));
	m_toolTip.AddTool(GetDlgItem(IDC_RADIO_FIT_MANUAL), _T(
		"在基线和液滴上手动输入点。基线上的点请使用鼠标左键，液滴上的点请使用鼠标右键。（水平面：2 个点；圆形拟合：3 个点；椭圆拟合：5 个点。）"
	));
	m_toolTip.Activate(TRUE);

	m_lstFiles.InsertColumn(0, _T("图片"), LVCFMT_LEFT, 300);
	m_lstFiles.InsertColumn(1, _T("结果"), LVCFMT_LEFT, 70);
	m_lstFiles.InsertColumn(2, _T("左角"), LVCFMT_LEFT, 50);
	m_lstFiles.InsertColumn(3, _T("右角"), LVCFMT_LEFT, 50);
	m_lstFiles.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_runnerForTest = std::make_unique<CTestRunner>(this);
	m_runnerForAutorec = std::make_unique<CAutorecRunner>(this);

	m_chkResAngle.SetCheck(BST_CHECKED);
	m_chkResFitting.SetCheck(BST_CHECKED);
	m_chkResBasePt.SetCheck(BST_CHECKED);
	m_chkSubPixel.SetCheck(BST_CHECKED);
	m_chkBoundary.SetCheck(BST_CHECKED);
}

void CDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
	m_bInit = TRUE;
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//void CDemoDlg::OnMenuOpenFolder()
//{	
//}

void CDemoDlg::showCurrentImage()
{
	if (cur_image_index < 0)
		return;

	imageName = m_lstFiles.GetItemText(cur_image_index, 0);
	SetWindowText(preTitle = _T("悬滴（描边）[") + imageName + _T("]"));
	
	CString sCurState;
	sCurState.Format(_T("图片：%d/%d"), cur_image_index + 1, images_num);
	m_lblFiles.SetWindowText(sCurState);

	m_edtResultText.SetWindowText(_T(""));	
	imageName = images_all[cur_image_index];
	m_wndPic.SetImage(imageName);
	// get baseline shape
	//std::string filename = CT2A(imageName.GetString());
	m_detailedShape = eBS_Unknown;

	auto procRes = dropletSetImage(imageName);	

	if(!procRes) {
		AfxMessageBox(_T("加载图片失败！"));
		return;
	}
	imageLoaded = true;
	BaseLineShapeKind detailedShape = eBS_Unknown;
	HorizonMode horMode = eHmUnknow;
	bool hasPin = false;
	bool res = dropletGetShapeEx(&detailedShape, &horMode, &hasPin); // eBS_Fail
	if (res) {
		m_iModeOnHor = horMode;
		m_bHasPin = hasPin;
		m_detailedShape = detailedShape;
	}	

	bool isEmpty = (m_detailedShape < eBS_ConvexWithPins);
	m_chkPin.SetCheck(m_bHasPin ? BST_CHECKED : BST_UNCHECKED);	

	m_mainShape = (MainShapeType)(m_detailedShape % 3);
	m_mainShapeInt = m_mainShape;

	UpdateData(FALSE);
	UpdateProcControlStates();

	// recog
	if(m_fit_mode == eFitAuto)
		DoRecog(isEmpty);
}

void CDemoDlg::NextImage()
{
	writeResult();
	if (cur_image_index == images_num - 1)  {
		if (MessageBox(_T("当前图片都处理好了，继续加载下一批图片吗？"), _T("基线识别"), MB_YESNO) == IDYES) 
			OnBnClickedBtnBrowser();
		return;
	}
	int oldSel = cur_image_index;
	cur_image_index++;
	m_lstFiles.SetItemState(oldSel, 0, LVIS_SELECTED); // unselect current
	m_lstFiles.SetItemState(cur_image_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_lstFiles.EnsureVisible(cur_image_index, FALSE);
	showCurrentImage();
}

void CDemoDlg::PrevImage()
{
	if (cur_image_index < 1)
		return;
	int oldSel = cur_image_index;
	cur_image_index--;
	m_lstFiles.SetItemState(oldSel, 0, LVIS_SELECTED); // unselect current
	m_lstFiles.SetItemState(cur_image_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	m_lstFiles.EnsureVisible(cur_image_index, FALSE);
	showCurrentImage();
}

void CDemoDlg::writeResult()
{
	// Check if a valid image is selected
	if (cur_image_index < 0)
		return;

	// Generate result file path
	CString txtPath;
	getTxtFile(images_all[cur_image_index], txtPath);

	// Open the result file safely (use Unicode-safe method)
	FILE* fp = nullptr;
	_tfopen_s(&fp, txtPath, _T("wt"));
	if (!fp) {
		// Handle file open error (log or display as needed)
		AfxMessageBox(_T("无法创建结果文件。"), MB_ICONERROR);
		return;
	}

	// Write rectangle coordinates and result code to file
	fprintf(fp, "%d %d\n", static_cast<int>(m_baseRect.left), static_cast<int>(m_baseRect.top));
	fprintf(fp, "%d %d\n", static_cast<int>(m_baseRect.right), static_cast<int>(m_baseRect.bottom));
	fprintf(fp, "%d\n", m_result_code);

	// Close file safely
	fclose(fp);
}


void CDemoDlg::getTxtFile(CString &imageName, CString &txtName)
{
	int path_index = imageName.ReverseFind('\\');
	txtName = txt_folder + imageName.Mid(path_index + 1) + _T(".txt");
}


BOOL CDemoDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_toolTip.m_hWnd != NULL)
		m_toolTip.RelayEvent(pMsg);

	if (pMsg->message == WM_KEYDOWN)  {
		if (pMsg->wParam == VK_RETURN)
			return FALSE;
		if (pMsg->wParam == VK_ESCAPE)
			return FALSE;
		if (pMsg->wParam == VK_PRIOR) {
			PrevImage();
			return TRUE;
		}
		else if (pMsg->wParam == VK_NEXT) {
			NextImage();
			return TRUE;
		}
		else if (pMsg->wParam == '1' || pMsg->wParam == VK_NUMPAD1)  {
			//((CButton*)GetDlgItem(IDC_RADIO_RESULT_SUCCESS))->SetCheck(TRUE);
			//((CButton*)GetDlgItem(IDC_RADIO_RESULT_FAILED))->SetCheck(FALSE);
			m_result_code = 0;
		} 
		else if (pMsg->wParam == '2' || pMsg->wParam == VK_NUMPAD2)  {
			//((CButton*)GetDlgItem(IDC_RADIO_RESULT_SUCCESS))->SetCheck(FALSE);
			//((CButton*)GetDlgItem(IDC_RADIO_RESULT_FAILED))->SetCheck(TRUE);
			m_result_code = 1;
		}
	}/*
	if (pMsg->message == WM_MOUSEMOVE) {
		CPoint point(pMsg->pt);
		ScreenToClient(&point);
		OnMouseMove(pMsg->wParam, point);
		return TRUE;
	}*/
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDemoDlg::OnBnClickedRadioAutoBaseLine()
{
	UpdateData(TRUE);
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::DoRecog(bool isEmpty)
{
	double output_quantities[2] = { 0.0, 0.0 };
	bool recRes = false;
	// Start timing
	clock_t t1 = clock();
	clock_t duration = t1;
	mainShapeType shape = m_mainShape;
	CString durationText;
	bool isSubPixel = false;
	CString resString;
	RectangleF basePt;
	FittingInfo outInfo;
	PointF keyPts[3] = { {0,0}, {0,0}, {0,0} };

	m_wndPic.ClearDrawItems();
	if (isEmpty) {
		resString = _T("无有效液滴");
		m_edtResultText.SetWindowText(_T("无有效液滴"));
		goto end_proc;
	}
	if (!imageLoaded || isProcessing) {
		resString = _T("请先加载图片");
		m_edtResultText.SetWindowText(_T("请先加载图片"));
		goto end_proc;
	}
	isSubPixel = (m_chkSubPixel.GetCheck() == BST_CHECKED);
	// Set processing indicator
	SetWindowText(preTitle + _T("- 处理中。。。"));
	isProcessing = true;

	bool isBoundary = (m_chkBoundary.GetCheck() == BST_CHECKED);
	if (m_fit_mode == eFitAuto) {
		if (m_mainShape == eShapeHorizontal)  {
			bool rotated = false;
			bool hasPin = (m_chkPin.GetCheck() == BST_CHECKED);			
			HorizonMode hmMode = eHmUnknow;

			if (m_chkManualMode.GetCheck() == BST_CHECKED) {
				rotated = (m_chkInclined.GetCheck() == BST_CHECKED);				
				hmMode = (HorizonMode)m_iModeOnHor;
			}
			
			recRes = dropletGetBasepointsByAutoOnHorizontal(hasPin, rotated, hmMode, &basePt, keyPts, isSubPixel, isBoundary);

			if (!recRes) {
				// todo: output log
				goto end_proc;
			}
			recRes = dropletAutoFittingOnHorizontal(
				m_upFlag,
				&basePt,
				&outInfo
			);
			if (!recRes) {
				// todo: output log
				goto end_proc;
			}
		}
		else {
			bool hasPin = (m_chkPin.GetCheck() == BST_CHECKED);
			m_chkInclined.SetCheck(BST_UNCHECKED);
			recRes = dropletGetBasepointsByAutoOnSurface(hasPin, &basePt, isSubPixel, isBoundary);
			if (!recRes)
				goto end_proc;

			recRes = dropletAutoFittingOnSurface(
				m_mainShape,
				m_upFlag,
				m_dnFlag,
				&basePt,
				&outInfo
			);
			if (!recRes) {
				// todo: output log
				goto end_proc;
			}
		}
	}
	else if(m_fit_mode == eFitByBasePoint || m_fit_mode == eFitByLine) { // eFitByLine or eFitByBasePoint or eFitBySemiauto
		int nBasePts = 0, nDropletPts = 0;
		PointF* pBasePts = NULL;
		PointF* pDropletPts = NULL;
		if (m_fit_mode == eFitByBasePoint) {
			PointF pts[2] = {
				{ m_baseRect.left, m_baseRect.top },
				{ m_baseRect.right, m_baseRect.bottom }
			};
			if (m_mainShape == eShapeHorizontal) {
				recRes = dropletGetBasepointsByPointsOnHorizontal(pts, &basePt, isSubPixel);
			}
			else {
				recRes = dropletGetBasepointsByPointsOnSurface(pts, &basePt, isSubPixel);
			}
		}
		else if (m_fit_mode == eFitByLine) {
			PointF pts[2];
			if (!m_wndPic.GetLineSegment(&pts[0].x, &pts[0].y, &pts[1].x, &pts[1].y)) {
				recRes = false;
				resString = _T("请先绘制基线");
				// todo: output log
				goto end_proc;
			}
			if (m_mainShape == eShapeHorizontal) {
				recRes = dropletGetBasepointsBylineOnHorizontal(pts, &basePt, isSubPixel);
			}
			else {
				recRes = dropletGetBasepointsBylineOnSurface(pts, &basePt, isSubPixel);
			}
		}

		if (recRes) {			
			recRes = dropletPointsFitting(
				m_upFlag,
				m_dnFlag,
				&basePt,
				&outInfo);
		}
	}
	else { // eFitManual or eFitSemiAuto
		int nBasePts = 0, nDropletPts = 0;
		PointF* pBasePts = NULL;
		PointF* pDropletPts = NULL;
		BOOL ready = true;

		if (m_upFlag == eDropletWidthHeight) {
			if (m_fit_mode == eFitSemiAuto) {
				ready = false;
			}
			else
			{
				CRect rc;
				if (m_wndPic.GetTrackerRect(&rc)) {
					nDropletPts = 2;
					pDropletPts = new PointF[2];
					pDropletPts[0].x = (double)rc.left;
					pDropletPts[0].y = (double)rc.top;
					pDropletPts[1].x = (double)rc.right;
					pDropletPts[1].y = (double)rc.bottom;
				}
				else {
					ready = false;
				}
			}
		}
		else if (m_fit_mode == eFitManual) {
			if (!m_wndPic.GetDropletPoints(&nDropletPts, &pDropletPts))
				ready = false;
			if (!m_wndPic.GetBaseLinePoints(&nBasePts, &pBasePts))
				ready = false;
		}
		else if (m_fit_mode == eFitSemiAuto) {
			if (!m_wndPic.GetBaseLinePoints(&nBasePts, &pBasePts))
				ready = false;
		}
		else {
			ready = false;
		}

		if (ready) {
			if (m_fit_mode == eFitSemiAuto) {
				if (m_mainShape == eShapeHorizontal && nBasePts == 2) {
					recRes = dropletGetBasepointsBylineOnHorizontal(pBasePts, &basePt, isSubPixel);
				}
				else {
					recRes = dropletGetBasepointsBySemiautoOnSurface(shape, nBasePts, pBasePts, &basePt, isSubPixel);
				}
				if (recRes) {
					recRes = dropletSemiautoFitting(
						shape,
						m_upFlag,
						m_dnFlag,
						&basePt,
						nBasePts,
						pBasePts,
						&outInfo
					);
				}
			}
			else {
				recRes = dropletManualFitting(
					shape,
					m_upFlag,
					m_dnFlag,
					nDropletPts,
					pDropletPts,
					nBasePts,
					pBasePts,
					false,
					&outInfo,
					isSubPixel
				);
			}
		}
		if (!recRes) {
			// todo: output log
			goto end_proc;
		}
	}
end_proc:
	// Processing finished: record elapsed time
	duration = clock() - t1;
	durationText.Format(_T(" - %d 毫秒"), duration);
	SetWindowText(preTitle + durationText);

	// Update UI results
	GetDlgItem(IDC_EDIT_ANGLE_LEFT)->SetWindowText(_T("-"));
	GetDlgItem(IDC_EDIT_ANGLE_RIGHT)->SetWindowText(_T("-"));
	// drawing part
	bool showRes[eLayerCount] = {
		m_chkResAngle.GetCheck() == BST_CHECKED,
		m_chkResFitting.GetCheck() == BST_CHECKED,
		m_chkResBasePt.GetCheck() == BST_CHECKED
	};
	if (outInfo.fgType == eFgPolyline) {
		if (outInfo.rightPolylineSize > 0 && 
			outInfo.rightPolyline) { // right polyline in case of polynomial fitting
			std::vector<PointF> pts;
			for (int i = 0; i < outInfo.rightPolylineSize; i++) {
				pts.push_back(PointF(outInfo.rightPolyline[i].x, outInfo.rightPolyline[i].y));
			}
			m_wndPic.AddPolyline(pts, eLayerFit, COLOR_RIGHT_FIT, showRes[eLayerFit]);
		}
		if (outInfo.leftPolylineSize > 0 &&
			outInfo.leftPolyline) { // left polyline in case of polynomial fitting
			std::vector<PointF> pts;
			for (int i = 0; i < outInfo.leftPolylineSize; i++) {
				pts.push_back(PointF(outInfo.leftPolyline[i].x, outInfo.leftPolyline[i].y));
			}
			m_wndPic.AddPolyline(pts, eLayerFit, COLOR_LEFT_FIT, showRes[eLayerFit]);
		}
		if(m_mainShape != eShapeHorizontal) {			
			m_wndPic.AddEllipse(outInfo.boxes[2], eLayerFit, COLOR_BASELINE, showRes[eLayerFit]);
		}
	}
	else if (outInfo.fgType == eFgRectangle) {
		if (outInfo.boxes[0].a > 0 && outInfo.boxes[0].b > 0) {
			m_wndPic.AddRectangle(outInfo.boxes[0], eLayerFit, COLOR_LEFT_FIT, showRes[eLayerFit]);
		}
	}
	else if (outInfo.fgType == eFgOneEllipse || outInfo.fgType == eFgTwoEllipses) {
		COLORREF ellipseColor[3] = {
			COLOR_BASELINE, COLOR_LEFT_FIT, COLOR_RIGHT_FIT
		};
		for (int i = 0; i < 3; i++) {
			if (outInfo.boxes[i].a > 0 && outInfo.boxes[i].b > 0) {
				m_wndPic.AddEllipse(outInfo.boxes[i], eLayerFit, ellipseColor[i], showRes[eLayerFit]);
			}
		}
	}	
	if (outInfo.pointAngle[0].angle >= 0.0) {
		m_wndPic.AddAngleLine(outInfo.pointAngle[0], showRes[eLayerAngle]);
	}
	if (outInfo.pointAngle[1].angle >= 0.0) {
		m_wndPic.AddAngleLine(outInfo.pointAngle[1], showRes[eLayerAngle]);
	}
	if (basePt.left != 0.0 && basePt.top != 0.0) { // base points
		m_wndPic.AddPoint(PointF(basePt.left, basePt.top), showRes[eLayerBasePt]);
		m_wndPic.AddPoint(PointF(basePt.right, basePt.bottom), showRes[eLayerBasePt]);
	}

	//for (int i = 0; i < 3; i++) {
		if(keyPts[2].x > 0.0 && keyPts[2].y > 0.0)
			m_wndPic.AddPoint(keyPts[2], showRes[eLayerBasePt]);
	//}

	if (recRes) {
		resString = _T("成功");
		m_edtResultText.SetWindowText(resString);
		m_result_code = 0;
		CString angleText;
		if (dropletGetResult(output_quantities, NULL)) {
			angleText.Format(_T("%.3f"), output_quantities[0]);
			m_edtLeftAngle.SetWindowText(angleText);
			m_lstFiles.SetItemText(cur_image_index, 2, angleText);

			angleText.Format(_T("%.3f"), output_quantities[1]);
			m_edtRightAngle.SetWindowText(angleText);
			m_lstFiles.SetItemText(cur_image_index, 3, angleText);
		}
		OnBnClickedBtnZoomOut();
		m_lstFiles.SetItemText(cur_image_index, 1, _T("成功"));
	}
	else {
		if (resString.Trim().IsEmpty()) {
			auto errCode = dropletGetLastError();
			auto str = dropletGetResultString(errCode);
			resString = str;
			SysFreeString(str);
		}
		m_edtResultText.SetWindowText(resString);
		m_result_code = 1;
		m_lstFiles.SetItemText(cur_image_index, 1, _T("  失败"));
		m_lstFiles.SetItemText(cur_image_index, 2, _T(""));
		m_lstFiles.SetItemText(cur_image_index, 3, _T(""));
	}
	m_wndPic.ShowResult(recRes, resString);

	BSTR json = dropletGetResultByJsonFormat();
	if (json) {
		CString jsonStr = CString(json);
		SysFreeString(json);
		OutputDebugString(jsonStr);
	}
	// Done processing
	isProcessing = false;

	dropletFreeFittingInfo(&outInfo);
}

#if _DEV_OLDSTYLE_API
void CDemoDlg::DoRecog2(bool isEmpty)
{	
	double output_quantities[2] = { 0.0, 0.0 };
	bool recRes = false;
	// Start timing
	clock_t t1 = clock();
	clock_t duration = t1;
	mainShapeType shape = m_mainShape;
	CString durationText;
	bool isSubPixel = false;
	CString resString;

	if (isEmpty) {
		resString = _T("无有效液滴");
		m_edtResultText.SetWindowText(_T("无有效液滴"));
		goto end_proc;
	}
	if (!imageLoaded || isProcessing) {
		resString = _T("请先加载图片");
		m_edtResultText.SetWindowText(_T("请先加载图片"));
		goto end_proc;
	}

	isSubPixel = (m_chkSubPixel.GetCheck() == BST_CHECKED);
	// Set processing indicator
	SetWindowText(preTitle + _T("- 处理中。。。"));
	isProcessing = true;
	
	// Mode-based processing	
	if (m_fit_mode == eFitAuto) {
		if (m_mainShape == eShapeHorizontal && 
			m_chkManualMode.GetCheck() == BST_CHECKED) {
			bool rotated = (m_chkInclined.GetCheck() == BST_CHECKED);
			bool haspin = (m_chkPin.GetCheck() == BST_CHECKED);
			recRes = dropletAutoFit(m_upFlag, m_dnFlag, shape, isSubPixel, rotated, (HorizonMode)m_iModeOnHor, haspin);
		} 
		else {
			bool haspin = (m_chkPin.GetCheck() == BST_CHECKED);
			m_chkInclined.SetCheck(BST_UNCHECKED);
			recRes = dropletAutoFit(m_upFlag, m_dnFlag, shape, isSubPixel, false, eHmUnknow, haspin);
		}

		if (recRes) {
			MainShapeType shape;
			bool hasPin = false;
			bool isEmpty = false;
			RectangleF rect;
			HorizonMode mode = (HorizonMode)m_iModeOnHor;
			dropletMidResult(&shape, &hasPin, &isEmpty, &mode, &rect, NULL);
			BOOL bMustChange = FALSE;
			if (m_iModeOnHor != mode) {
				if (!m_bInternalUpdate) {
					m_bInternalUpdate = TRUE;
					m_iModeOnHor = mode;
					// Directly set the radio without causing command message
					for(int i = 0; i < 4; i++) {
						if (i == mode) continue;
						CButton* pButton = (CButton*)GetDlgItem(IDC_RD_MODE_LENS + i);
						if (pButton) {
							pButton->SetCheck(BST_UNCHECKED);
						}
					}
					CButton* pButton = (CButton*)GetDlgItem(IDC_RD_MODE_LENS + mode);
					if (pButton) {
						pButton->SetCheck(BST_CHECKED);
					}
					m_bInternalUpdate = FALSE;
				}
				//m_iModeOnHor = mode;
				//m_bInternalUpdate = TRUE;
				//CheckRadioButton(IDC_RD_MODE_LENS, IDC_RD_MODE_MUSHROOM, IDC_RD_MODE_LENS + mode); // update radios
				//m_bInternalUpdate = FALSE;
			}
			if (m_mainShape != shape) {
				m_mainShape = shape;
				m_mainShapeInt = shape;
				CheckRadioButton(IDC_RADIO_LOWER_TUMIAN, IDC_RADIO_LOWER_LINE, IDC_RADIO_LOWER_TUMIAN + shape); // update radios
			}
			if (m_bHasPin != hasPin) {
				m_bHasPin = hasPin;
				m_chkPin.SetCheck(hasPin ? BST_CHECKED : BST_UNCHECKED);
			}			
		}
	}
	else if (m_fit_mode == eFitByBasePoint) {
		RectangleF rect = m_baseRect;
		PointF pts[2] = {
			{ (double)rect.left, (double)rect.top },
			{ (double)rect.right, (double)rect.bottom }
		};
		recRes = dropletFitByBasePoint(pts, m_upFlag, m_dnFlag, shape, isSubPixel);
	}
	else if (m_fit_mode == eFitByLine) {
		PointF pts[2];
		memset(pts, 0, sizeof(pts));
		if(m_wndPic.GetLineSegment(&pts[0].x, &pts[0].y, &pts[1].x, &pts[1].y))
			recRes = dropletFitByBaseLine(pts, m_upFlag, m_dnFlag, shape, isSubPixel);
	}
	else { // semi-auto or manual
		int nBasePts = 0, nDropletPts = 0;
		PointF* pBasePts = NULL;
		PointF* pDropletPts = NULL;
		BOOL ready = true;

		if (m_upFlag == eDropletWidthHeight) {
			if (m_fit_mode == eFitSemiAuto) {
				ready = false;
			}
			else
			{
				CRect rc;
				if (m_wndPic.GetTrackerRect(&rc)) {
					nDropletPts = 2;
					pDropletPts = new PointF[2];
					pDropletPts[0].x = (double)rc.left;
					pDropletPts[0].y = (double)rc.top;
					pDropletPts[1].x = (double)rc.right;
					pDropletPts[1].y = (double)rc.bottom;
				}
				else {
					ready = false;
				}
			}
		}
		else if(m_fit_mode == eFitManual) {
			if (!m_wndPic.GetDropletPoints(&nDropletPts, &pDropletPts))
				ready = false;
			if(!m_wndPic.GetBaseLinePoints(&nBasePts, &pBasePts))
				ready = false;
		}
		else if (m_fit_mode == eFitSemiAuto) {
			if (!m_wndPic.GetBaseLinePoints(&nBasePts, &pBasePts))
				ready = false;
		}
		else {
			ready = false;
		}
		if (ready) {
			if (m_fit_mode == eFitSemiAuto) {
				// Semi-auto mode
				recRes = dropletSemiAutoFit(nBasePts, pBasePts, m_upFlag, m_dnFlag, shape, isSubPixel);
			}
			else {
				// Manual mode
				recRes = dropletManualFit(nDropletPts, pDropletPts, nBasePts, pBasePts, m_upFlag, m_dnFlag, shape, isSubPixel);
			}
		}
		else
		{
			resString = _T("无效操作");
		}
		if (pBasePts != NULL) {
			delete[] pBasePts;
		}
		if (pDropletPts != NULL) {
			delete[] pDropletPts;
		}
	}
	
	// Processing finished: record elapsed time
	duration = clock() - t1;	
	durationText.Format(_T(" - %d 毫秒"), duration);
	SetWindowText(preTitle + durationText);

	// Update UI results
	GetDlgItem(IDC_EDIT_ANGLE_LEFT)->SetWindowText(_T("-"));
	GetDlgItem(IDC_EDIT_ANGLE_RIGHT)->SetWindowText(_T("-"));
end_proc:	
	if (recRes) {
		resString = _T("成功");
		m_edtResultText.SetWindowText(resString);
		m_result_code = 0;
		CString angleText;
		if (dropletGetResult(output_quantities, NULL)) {
			angleText.Format(_T("%.3f"), output_quantities[0]);
			m_edtLeftAngle.SetWindowText(angleText);
			m_lstFiles.SetItemText(cur_image_index, 2, angleText);

			angleText.Format(_T("%.3f"), output_quantities[1]);
			m_edtRightAngle.SetWindowText(angleText);
			m_lstFiles.SetItemText(cur_image_index, 3, angleText);
		}
		OnBnClickedBtnZoomOut();
		m_lstFiles.SetItemText(cur_image_index, 1, _T("成功"));
	}
	else {
		if (resString.Trim().IsEmpty()) {
			auto errCode = dropletGetLastError();
			auto str = dropletGetResultString(errCode);
			resString = str;
			SysFreeString(str);
		}
		m_edtResultText.SetWindowText(resString);
		m_result_code = 1;
		m_lstFiles.SetItemText(cur_image_index, 1, _T("  失败"));
		m_lstFiles.SetItemText(cur_image_index, 2, _T(""));
		m_lstFiles.SetItemText(cur_image_index, 3, _T(""));
	}	
	m_wndPic.ShowResult(recRes, resString);

	//((CButton*)GetDlgItem(IDC_RADIO_RESULT_SUCCESS))->SetCheck(recRes);
	//((CButton*)GetDlgItem(IDC_RADIO_RESULT_FAILED))->SetCheck(!recRes);	
	BSTR json = dropletGetResultByJsonFormat();
	if (json) {
		CString jsonStr = CString(json);
		SysFreeString(json);
		OutputDebugString(jsonStr);
	}
	// Done processing
	isProcessing = false;
}
#endif//_DEV_OLDSTYLE_API

void CDemoDlg::OnBnClickedBtnZoomOut()
{
	if (!imageLoaded) 
		return;
}

void CDemoDlg::OnBnClickedButtonRecognize()
{	
	DoRecog();
}

void CDemoDlg::OnChkTestClick()
{
	BOOL isTest = (m_chkTest.GetCheck() == BST_CHECKED);	
	
	m_btnTest.EnableWindow(isTest);	
	m_progressCtrl.ShowWindow(isTest ? SW_SHOW : SW_HIDE);
	m_chkManualAngleInput.EnableWindow(isTest);
	m_edtTestAngle.EnableWindow(isTest);
}

void WriteCStringToFile(const CString& str, const CString& filePath)
{
	// Convert CString to std::string
	CT2CA pszConvertedAnsiString(str);
	std::string stdStr(pszConvertedAnsiString);
	
	FILE* fp = _tfopen(filePath, _T("wt"));	
	fprintf(fp, stdStr.c_str());
	fclose(fp);	
}

CString GetLastFolderName(const CString& path)
{
	CString lastFolderName = path;
	int lastSlashPos = path.ReverseFind(_T('\\'));

	if (lastSlashPos != -1)
		lastFolderName = path.Mid(lastSlashPos + 1);
	int length = lastFolderName.GetLength();
	CString numStr;
	for (int i = 0; i < length; ++i) {
		if (isdigit(lastFolderName[i]))
			numStr.AppendChar(lastFolderName[i]);
	}

	return numStr;
}

int EstimateTotalFiles(const CString& rootFolder, bool bIncludeSubfolders)
{
	int count = 0;
	CFileFind finder;

	CString searchPath = rootFolder + _T("\\*.*");
	BOOL bWorking = finder.FindFile(searchPath);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;

		if (finder.IsDirectory() && bIncludeSubfolders)
			count += EstimateTotalFiles(finder.GetFilePath(), bIncludeSubfolders);
		else {
			CString ext = finder.GetFileName().Right(4).MakeLower();
			if (ext == _T(".bmp") || ext == _T(".png"))
				count++;
		}
	}
	return count;
}

void CDemoDlg::OnBnClickedButtonTest()
{
	if (m_isRunningAutoThread)
		return;
	if (m_isRunningTestThread) {
		m_runnerForTest->CancelTest();
	}
	else {
		int totalFileEstimate = EstimateTotalFiles(dstFolder, true);
		if (totalFileEstimate == 0) {
			MessageBox(_T("没有找到任何图片！"));
			return;
		}

		UpdateData(TRUE);

		CString sFolderAngle;
		m_edtTestAngle.GetWindowText(sFolderAngle);
		double m_dFolderAngle = _ttof(sFolderAngle);

		bool bInput = (m_chkManualAngleInput.GetCheck() == BST_CHECKED);
		bool isSubPixel = (m_chkSubPixel.GetCheck() == BST_CHECKED);
		bool isBoundary = (m_chkBoundary.GetCheck() == BST_CHECKED);
		// Start thread with current UI settings
		if (m_runnerForTest) {			
			AutoTestInParam inParam;
			inParam.dropletFitMode = m_upFlag;
			inParam.baselineFitMode = m_dnFlag;
			inParam.mainShape = eShapeUnknown;
			inParam.isSubpixel = isSubPixel;
			inParam.isBoundary = isBoundary;
			inParam.rotated = (m_chkInclined.GetCheck() == BST_CHECKED);
			inParam.horMode = eHmUnknow;
			inParam.hasPin = (m_chkPin.GetCheck() == BST_CHECKED);

			m_runnerForTest->StartTest(bInput, m_dFolderAngle, inParam, dstFolder);
			m_isRunningTestThread = TRUE;
			m_progressCtrl.ShowWindow(SW_SHOW);
			m_progressCtrl.SetRange(0, totalFileEstimate);
			m_btnTest.SetWindowText(_T("停止"));
		}
	}	
}

LRESULT CDemoDlg::OnAutoThreadProc(WPARAM wParam, LPARAM lParam)
{
	int msgID = (int)wParam;
	AutoTestOutParam* outParam = (AutoTestOutParam*)lParam;
	if (outParam == NULL) {
		CString result;
		if (m_runnerForAutorec)
			result = m_runnerForAutorec->GetResultString();
		
		m_isRunningAutoThread = FALSE;
		m_btnAutoRecog.SetWindowText(_T("自动识别"));
		m_progressCtrl.ShowWindow(SW_HIDE);

		if (msgID < 0)
			AfxMessageBox(_T("用户取消"));
		else {
			AfxMessageBox(_T("自动识别成功"));
		}
	}
	else {
		SetWindowText(_T("...[" + outParam->filePath + "]"));
		m_wndPic.SetImage(outParam->filePath, false);
		if (outParam->result) {
			CString angleText;
			angleText.Format(_T("%.3f"), outParam->angles[0]);
			m_edtLeftAngle.SetWindowText(angleText);
			m_lstFiles.SetItemText(msgID, 2, angleText);

			angleText.Format(_T("%.3f"), outParam->angles[1]);
			m_edtRightAngle.SetWindowText(angleText);
			m_lstFiles.SetItemText(msgID, 3, angleText);

			m_result_code = 0;
			m_lstFiles.SetItemText(msgID, 1, _T("成功"));
			m_chkPin.SetCheck(outParam->hasPin ? BST_CHECKED : BST_UNCHECKED);
			UpdateProcControlStates();
			UpdateData(FALSE);			
		}
		else {
			CString errMsg = _T("识别失败");
			m_edtResultText.SetWindowText(errMsg);
			m_lstFiles.SetItemText(msgID, 1, _T("  失败"));
			m_lstFiles.SetItemText(msgID, 2, _T(""));
			m_lstFiles.SetItemText(msgID, 3, _T(""));
			m_result_code = 1;
		}
		int n = m_lstFiles.GetItemCount();
		for (int i = 0; i < n; i++) {
			m_lstFiles.SetItemState(i, 0, LVIS_SELECTED | LVIS_FOCUSED); // unselect all items
		}
		// Select and focus
		m_lstFiles.SetItemState(msgID, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		m_lstFiles.EnsureVisible(msgID, FALSE);
		drawFitResult(
			outParam->pFittingInfo,
			PointF(outParam->basePoints.left, outParam->basePoints.top),
			PointF(outParam->basePoints.right, outParam->basePoints.bottom)
		);
		m_wndPic.ShowResult(outParam->result, outParam->resultString);
		Sleep(0);
		//Sleep(10);
		m_progressCtrl.SetPos(msgID);
		if (msgID >= 0 && msgID < images_num) {
			CString sCurState;
			sCurState.Format(_T("图片：%d/%d"), msgID + 1, images_num);
			m_lblFiles.SetWindowText(sCurState);
			cur_image_index = msgID;
		}
		if (outParam->pFittingInfo) {
			dropletFreeFittingInfo(outParam->pFittingInfo);
			delete outParam->pFittingInfo;
			outParam->pFittingInfo = NULL;
		}
	}
	
	return 0L;
}

void CDemoDlg::drawFitResult(const FittingInfo* fitInfo, const PointF& left, const PointF& right)
{
	bool visible = true;
	visible = (m_chkResFitting.GetCheck() == BST_CHECKED);
	if (fitInfo) {		
		if (fitInfo->fgType == eFgPolyline) {
			if (fitInfo->rightPolylineSize > 0 &&
				fitInfo->rightPolyline) { // right polyline in case of polynomial fitting
				std::vector<PointF> pts;
				for (int i = 0; i < fitInfo->rightPolylineSize; i++) {
					pts.push_back(PointF(fitInfo->rightPolyline[i].x, fitInfo->rightPolyline[i].y));
				}
				m_wndPic.AddPolyline(pts, eLayerFit, COLOR_RIGHT_FIT, visible);
			}
			if (fitInfo->leftPolylineSize > 0 &&
				fitInfo->leftPolyline) { // left polyline in case of polynomial fitting
				std::vector<PointF> pts;
				for (int i = 0; i < fitInfo->leftPolylineSize; i++) {
					pts.push_back(PointF(fitInfo->leftPolyline[i].x, fitInfo->leftPolyline[i].y));
				}
				m_wndPic.AddPolyline(pts, eLayerFit, COLOR_LEFT_FIT, visible);
			}
			if (m_mainShape != eShapeHorizontal) {
				m_wndPic.AddEllipse(fitInfo->boxes[2], eLayerFit, COLOR_BASELINE, visible);
			}
		}
		else if (fitInfo->fgType == eFgRectangle) {
			if (fitInfo->boxes[0].a > 0 && fitInfo->boxes[0].b > 0) {
				m_wndPic.AddRectangle(fitInfo->boxes[0], eLayerFit, COLOR_LEFT_FIT, visible);
			}
		}
		else if (fitInfo->fgType == eFgOneEllipse || fitInfo->fgType == eFgTwoEllipses) {
			COLORREF ellipseColor[3] = { COLOR_BASELINE, COLOR_LEFT_FIT, COLOR_RIGHT_FIT };
			for (int i = 0; i < 3; i++) {
				if (fitInfo->boxes[i].a > 0 && fitInfo->boxes[i].b > 0) {
					m_wndPic.AddEllipse(fitInfo->boxes[i], eLayerFit, ellipseColor[i], visible);
				}
			}
		}
		visible = (m_chkResAngle.GetCheck() == BST_CHECKED);
		if (fitInfo->pointAngle[0].angle >= 0.0) {
			m_wndPic.AddAngleLine(fitInfo->pointAngle[0], visible);
		}
		if (fitInfo->pointAngle[1].angle >= 0.0) {
			m_wndPic.AddAngleLine(fitInfo->pointAngle[1], visible);
		}
	}

	visible = (m_chkResBasePt.GetCheck() == BST_CHECKED);

	m_wndPic.AddPoint(left, visible);
	m_wndPic.AddPoint(right, visible);
}

LRESULT CDemoDlg::OnTestThreadProc(WPARAM wParam, LPARAM lParam)
{
	int msgID = (int)wParam;
	AutoTestOutParam* outParam = (AutoTestOutParam*)lParam;
	if (outParam == NULL) {
		CString result;
		if (m_runnerForTest)
			result = m_runnerForTest->GetResultString();
		if (msgID < 0)
			AfxMessageBox(_T("Cancelled by user"));
		else {
			AfxMessageBox(result);
			WriteCStringToFile(result, _T("output.csv"));
		}
		m_isRunningTestThread = FALSE;
		m_btnTest.SetWindowText(_T("测试"));
		m_progressCtrl.ShowWindow(SW_HIDE);
	}
	else {
		SetWindowText(_T("...[" + outParam->filePath + "]"));
		m_wndPic.SetImage(outParam->filePath, false);
		if (outParam->result) {
			CString angleText;
			angleText.Format(_T("%.3f"), outParam->angles[0]);
			m_edtLeftAngle.SetWindowText(angleText);
			angleText.Format(_T("%.3f"), outParam->angles[1]);
			m_edtRightAngle.SetWindowText(angleText);

			m_chkPin.SetCheck(outParam->hasPin ? BST_CHECKED : BST_UNCHECKED);
			UpdateProcControlStates();
			UpdateData(FALSE);			
		}
		else {
			CString errMsg = _T("识别失败");
			m_edtResultText.SetWindowText(errMsg);
		}
		drawFitResult(
			outParam->pFittingInfo, 
			PointF(outParam->basePoints.left, outParam->basePoints.top),
			PointF(outParam->basePoints.right, outParam->basePoints.bottom)
		);
		m_wndPic.ShowResult(outParam->result, outParam->resultString);
		Sleep(0);
		m_progressCtrl.SetPos(msgID);

		FittingInfo* fitInfo = outParam->pFittingInfo;
		if (fitInfo != NULL) {
			dropletFreeFittingInfo(fitInfo);
			delete fitInfo;
			fitInfo = NULL;
		}
		delete outParam;
	}
	
	return 0L;
}


void CDemoDlg::OnChkManualInput()
{
	bool bInput = (m_chkManualAngleInput.GetCheck() == BST_CHECKED);
	m_edtTestAngle.EnableWindow(bInput);
}


void CDemoDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_runnerForTest) {
		m_runnerForTest->CancelTest();
	}
	if (m_runnerForAutorec) {
		m_runnerForAutorec->CancelTest();
	}

	dropletReleaseLib();
}


void CDemoDlg::OnBnClickedBtnBrowser()
{
	bool isTest = (m_chkTest.GetCheck() == BST_CHECKED);
	CFolderPickerDialog dlg;
	if (!dlg.DoModal() == IDOK)
		return;

	if (isTest) {
		dstFolder = dlg.GetFolderPath();
	}
	else {
		int nMaxExtent = 0;
		CClientDC dc(this);
		m_lstFiles.DeleteAllItems();		
		images_all.clear();
		dstFolder = dlg.GetFolderPath();
		txt_folder = dstFolder + _T("(txt)");
		dstFolder += _T("\\");
		txt_folder += _T("\\");
		if (!PathIsDirectory(txt_folder))
			CreateDirectory(txt_folder, NULL);
		CFileFind filefind;
		BOOL bworking = filefind.FindFile(dstFolder + _T("*.*"));
		int count = 0;
		while (bworking && images_all.size() < max_load_images_num) {
			bworking = filefind.FindNextFile();
			if (!filefind.IsDots() && !filefind.IsDirectory()) {
				CString pureName = filefind.GetFileName();
				CString titleName = filefind.GetFileTitle();
				CString extName = pureName.Mid(titleName.GetLength());
				if (extName != _T(".bmp") && extName != _T(".png")) 
					continue;
				CString filename = dstFolder + pureName;
				CString txtName;
				getTxtFile(filename, txtName);
				if (PathFileExists(txtName)) 
					continue;
				m_lstFiles.InsertItem(count, pureName); // Add the file path to the list control		
				count++;
				CSize cs = dc.GetTextExtent(pureName);
				if (cs.cx > nMaxExtent)
					nMaxExtent = cs.cx;
				images_all.push_back(filename);
			}
		}

		imageLoaded = false;

		SetWindowText(_T("识别"));
		images_num = (int)images_all.size();
		if (images_num == 0)
			MessageBox(_T("所有图片都看过了！"));
		cur_image_index = -1;
		Invalidate();
	}
	m_edtPath.SetWindowText(dstFolder);
	m_lstFiles.SetColumnWidth(0, LVSCW_AUTOSIZE);

	UpdateProcControlStates();
}

void CDemoDlg::OnLbnDblclkListImageNames()
{
	/*int index = m_lstImages.GetCurSel();
	if (index < 0 || index >= images_all.size())
		return;
	cur_image_index = index;
	showCurrentImage();*/
}

void CDemoDlg::OnBnClickedChkSetManual()
{
	UpdateManualModeState();
}

void CDemoDlg::UpdateManualModeState(BOOL bEnable)
{
	m_chkManualMode.EnableWindow(bEnable);
	BOOL checkManual = bEnable ? (m_chkManualMode.GetCheck() == BST_CHECKED) : FALSE;
	m_chkInclined.EnableWindow(checkManual);
	// m_chkPin.EnableWindow(checkManual);

	GetDlgItem(IDC_RD_MODE_HOLU)->EnableWindow(checkManual);
	GetDlgItem(IDC_RD_MODE_LENS)->EnableWindow(checkManual);
	GetDlgItem(IDC_RD_MODE_INSIDE)->EnableWindow(checkManual);
	GetDlgItem(IDC_RD_MODE_MUSHROOM)->EnableWindow(checkManual);
}

void CDemoDlg::OnBnClickedRadioFitAuto()
{
	UpdateData(TRUE);
	m_fit_mode = (FitMode)m_fit_modeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioManualBaseLine()
{
	UpdateData(TRUE);
	m_fit_mode = (FitMode)m_fit_modeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioFindBasepoints()
{
	UpdateData(TRUE);
	m_fit_mode = (FitMode)m_fit_modeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioFindSemiauto()
{
	UpdateData(TRUE);
	m_fit_mode = (FitMode)m_fit_modeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioFitManual()
{
	UpdateData(TRUE);
	m_fit_mode = (FitMode)m_fit_modeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedCheckIncline()
{
	UpdateData(TRUE);
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedCheckPin()
{
	m_bHasPin = (m_chkPin.GetCheck() == BST_CHECKED);
	UpdateProcControlStates();
	DoRecog();
}


void CDemoDlg::OnBnClickedRdModeLens()
{
	if (m_bInternalUpdate) 
		return;  // prevent loop
	UpdateData(TRUE);
	DoRecog();
}

void CDemoDlg::OnBnClickedRdModeHolu()
{
	if (m_bInternalUpdate) 
		return;  // prevent loop
	UpdateData(TRUE);
	DoRecog();
}


void CDemoDlg::OnBnClickedRdModeInside()
{
	if (m_bInternalUpdate)
		return;  // prevent loop
	UpdateData(TRUE);
	DoRecog();
}

void CDemoDlg::OnBnClickedRdModeMushroom()
{
	if (m_bInternalUpdate)
		return;  // prevent loop
	UpdateData(TRUE);
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionCircle()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}


void CDemoDlg::OnBnClickedRadioRegressionEllipse()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionOpencv()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionOpencvDirect()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionDoubleEllipse()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionDoublePolynomial()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioHighWidth()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionPolynomial()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionCircle2()
{
	UpdateData(TRUE);
	m_upFlag = (DropletFitMode)m_upFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionCircleDn()
{
	UpdateData(TRUE);
	m_dnFlag = (BaseLineFitMode)m_dnFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionEllipseDn()
{
	UpdateData(TRUE);
	m_dnFlag = (BaseLineFitMode)m_dnFlagInt;
	UpdateProcControlStates();
	DoRecog();
}


void CDemoDlg::OnBnClickedRadioRegressionOpencvDn()
{
	UpdateData(TRUE);
	m_dnFlag = (BaseLineFitMode)m_dnFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioRegressionOpencvDn2()
{
	UpdateData(TRUE);
	m_dnFlag = (BaseLineFitMode)m_dnFlagInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedCheckSubpixel()
{
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioLowerTumian()
{
	UpdateData(TRUE);
	m_mainShape = (MainShapeType)m_mainShapeInt;
	UpdateProcControlStates();	
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioLowerAomian()
{
	UpdateData(TRUE);
	m_mainShape = (MainShapeType)m_mainShapeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::OnBnClickedRadioLowerLine()
{
	UpdateData(TRUE);
	m_mainShape = (MainShapeType)m_mainShapeInt;
	UpdateProcControlStates();
	DoRecog();
}

void CDemoDlg::UpdateProcControlStates()
{
	BOOL isHorzonMode = FALSE;
	switch (m_mainShape)
	{
	case eShapeConvex:
	case eShapeConcave:
		isHorzonMode = FALSE;
		break;
	case eShapeHorizontal:
		isHorzonMode = TRUE;
		break;
	default:
		break;
	}

	// Enable/Disable baseline fit mode controls based on the selected mode
	GetDlgItem(IDC_RADIO_REGRESSION_CIRCLE_DN)->EnableWindow(!isHorzonMode);
	GetDlgItem(IDC_RADIO_REGRESSION_ELLIPSE_DN)->EnableWindow(!isHorzonMode);
	GetDlgItem(IDC_RADIO_REGRESSION_OPENCV_DN)->EnableWindow(!isHorzonMode);
	GetDlgItem(IDC_RADIO_REGRESSION_OPENCV_DN2)->EnableWindow(!isHorzonMode);

	// Enable/Disable droplet fit mode controls based on the selected mode
	UINT dropletFitModeIDs[9] = {
		IDC_RADIO_REGRESSION_CIRCLE,
		IDC_RADIO_REGRESSION_ELLIPSE,
		IDC_RADIO_REGRESSION_OPENCV,
		IDC_RADIO_REGRESSION_OPENCV_DIRECT,
		IDC_RADIO_REGRESSION_DOUBLE_ELLIPSE,
		IDC_RADIO_REGRESSION_DOUBLE_POLYNOMIAL,
		IDC_RADIO_HIGH_WIDTH,
		IDC_RADIO_REGRESSION_POLYNOMIAL,
		IDC_RADIO_REGRESSION_CIRCLE2
	};
	BOOL stateDropletFitMode[9] = { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE };
	stateDropletFitMode[eDropletCircle] = !m_bHasPin;
	//stateDropletFitMode[eDropletDoublePolynomial] = (isHorzonMode && !m_bHasPin);
	stateDropletFitMode[eDropletWidthHeight] = (isHorzonMode && !m_bHasPin && m_fit_mode != eFitSemiAuto);
	
	for (int i = 0; i < 9; ++i) {
		GetDlgItem(dropletFitModeIDs[i])->EnableWindow(stateDropletFitMode[i]);
	}
	if(m_upFlagInt >= 0 && m_upFlagInt < 9 && !stateDropletFitMode[m_upFlagInt]) {
		((CButton*)GetDlgItem(dropletFitModeIDs[m_upFlagInt]))->SetCheck(BST_UNCHECKED);
		m_upFlagInt = 1;
		m_upFlag = (DropletFitMode)m_upFlagInt;
		((CButton*)GetDlgItem(IDC_RADIO_REGRESSION_ELLIPSE))->SetCheck(BST_CHECKED);
	}

	BOOL bManualInput = (m_fit_mode == eFitByBasePoint);

	m_edtLeftX.SetReadOnly(!bManualInput);
	m_edtLeftY.SetReadOnly(!bManualInput);
	m_edtRightX.SetReadOnly(!bManualInput);
	m_edtRightY.SetReadOnly(!bManualInput);

	BOOL bManualEnable = (m_mainShape == eShapeHorizontal && m_fit_mode == eFitAuto);
	UpdateManualModeState(bManualEnable);

	m_wndPic.SetDrawingMode(m_fit_mode, !isHorzonMode, m_bHasPin, m_upFlag, m_dnFlag);

	// m_chkIgnoreOld.EnableWindow(m_fit_mode == eFitAuto);
	m_btnAutoRecog.EnableWindow(m_fit_mode == eFitAuto && !images_all.empty());
}

LRESULT CDemoDlg::OnReadyProc(WPARAM wParam, LPARAM lParam)
{
	UINT id = (UINT)wParam;
	bool update = true;
	if (m_fit_mode == eFitByBasePoint) {	
		double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
		if (m_wndPic.GetBasePoint(&x0, &y0, &x1, &y1)) {
			m_baseRect = { x0, y0, x1, y1 };
			UpdateData(FALSE);
		}
		else
			update = false;
	}
	if(update)
		DoRecog();

	return 0L;
}

LRESULT CDemoDlg::OnFinishedEdit(WPARAM wParam, LPARAM lParam)
{
	UINT kind = (UINT)wParam;
	CWnd* ctrlWnd = (CWnd*)lParam;

	CWnd* pNext = ctrlWnd->GetNextWindow();
	if (pNext && kind == 1) {
		pNext->SetFocus();
		if (pNext->IsKindOf(RUNTIME_CLASS(CEdit))) {
			((CEdit*)pNext)->SetSel(0, -1); // Select all text if it's an edit control
		}
	}
	auto oldRect = m_baseRect;
	UpdateData(TRUE);
	if (oldRect != m_baseRect) {
		m_wndPic.SetBasePoint(m_baseRect.left, m_baseRect.top, m_baseRect.right, m_baseRect.bottom);
		DoRecog();
	}

	return 0L;
}

void CDemoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_bInit) {
		CRect rc;
		GetDlgItem(IDC_PIC_IMAGE)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		m_wndPic.MoveWindow(rc);
	}
}


void CDemoDlg::OnNMDblclkListFiles(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	int idx = pNMItemActivate->iItem;
	if (idx < 0 || idx >= images_all.size())
		return;
	cur_image_index = idx;
	showCurrentImage();

	*pResult = 0;
}


void CDemoDlg::OnBnClickedChkIgnoreOld()
{
	// TODO: Add your control notification handler code here
}

void CDemoDlg::OnBnClickedBtnAutorecog()
{
	if (m_isRunningTestThread)
		return;

	if (m_isRunningAutoThread) {
		m_runnerForAutorec->CancelTest();
	}
	else {		
		int totalFileEstimate = (int)images_all.size();
		if (totalFileEstimate == 0 || m_fit_mode != eFitAuto) {
			return;
		}

		UpdateData(TRUE);
		bool isSubPixel = (m_chkSubPixel.GetCheck() == BST_CHECKED);
		bool isBoundary = (m_chkBoundary.GetCheck() == BST_CHECKED);

		// Start thread with current UI settings
		if (m_runnerForAutorec) {
			AutoTestInParam inParam;
			inParam.dropletFitMode = m_upFlag;
			inParam.baselineFitMode = m_dnFlag;
			inParam.mainShape = eShapeUnknown;
			inParam.isSubpixel = isSubPixel;
			inParam.isBoundary = isBoundary;
			inParam.rotated = (m_chkInclined.GetCheck() == BST_CHECKED);
			inParam.horMode = eHmUnknow;
			inParam.hasPin = (m_chkPin.GetCheck() == BST_CHECKED);

			m_runnerForAutorec->StartTest(inParam, images_all, cur_image_index);
			m_isRunningAutoThread = TRUE;
			m_progressCtrl.ShowWindow(SW_SHOW);
			m_progressCtrl.SetRange(0, totalFileEstimate);
			m_btnAutoRecog.SetWindowText(_T("停止"));
		}
	}
}


void CDemoDlg::OnBnClickedChkResAngle()
{
	m_wndPic.UpdateLayerVisibility(eLayerAngle, m_chkResAngle.GetCheck() == BST_CHECKED);
}


void CDemoDlg::OnBnClickedChkResFitting()
{
	m_wndPic.UpdateLayerVisibility(eLayerFit, m_chkResFitting.GetCheck() == BST_CHECKED);
}


void CDemoDlg::OnBnClickedChkResBasept()
{
	m_wndPic.UpdateLayerVisibility(eLayerBasePt, m_chkResBasePt.GetCheck() == BST_CHECKED);
}


void CDemoDlg::OnBnClickedChkBoundary()
{	
}
//.EOF
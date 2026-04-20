#include "pch.h"
#include "TestRunner.h"
#include "JiaoShuiEllipseFit.h"


CTestRunner::CTestRunner(CWnd* dlg)
    : m_dlg(dlg)
    , m_cancelled(false)
    , m_running(false) 
    , m_iPos(0)
{
}

CTestRunner::~CTestRunner()
{ 
    CancelTest();
    if (m_thread.joinable()) 
        m_thread.join(); 
}

void CTestRunner::StartTest(
    bool isManualAngle, 
    double manualAngle, 
    const AutoTestInParam& inParam, 
    const CString& dstFolder
) {
    CancelTest();
    if (m_thread.joinable())
        m_thread.join();
    m_cancelled = false;
    m_running = true;
    m_sDstFolder = dstFolder;
    m_inParam = inParam;
    m_thread = std::thread(&CTestRunner::RunTest, this, isManualAngle, manualAngle);
}

void CTestRunner::CancelTest()
{
    m_cancelled = true;
}

void CTestRunner::RunTest(bool isManualAngle, double manualAngle)
{
    CString dstFolder = m_sDstFolder;
    CFileFind filefind;
    std::vector<CString> folders;
    m_sResult.Empty();
    m_iPos = 0;

    BOOL bworking = filefind.FindFile(dstFolder + _T("\\*.*"));
    while (bworking) {
        bworking = filefind.FindNextFile();
        if (filefind.IsDots())
            continue;
        if (filefind.IsDirectory()) 
            folders.push_back(filefind.GetFilePath());
    }

    int totalFiles = 0, totalErrors = 0;
    double totalErrorValue = 0;

    if (isManualAngle)
        ProcessFolder(dstFolder, manualAngle, totalFiles, totalErrors, totalErrorValue);

    for (const auto& folder : folders) {
        CString folderName = folder.Mid(folder.ReverseFind(_T('\\')) + 1);
        double angle = _ttof(folderName);
        ProcessFolder(folder, angle, totalFiles, totalErrors, totalErrorValue);
    }

    if (m_dlg) {
        WPARAM id = (WPARAM)(m_cancelled ? -1 : 0);
        m_dlg->PostMessage(UM_DROPLET_AUTOTESTTHREAD_MSG, id, NULL);
    }
    
    m_running = false;
}

int CTestRunner::ProcOneImage(const CString& filepath, AutoTestOutParam* outRes)
{
    outRes->result = false;
    auto res = dropletSetImage(filepath);
    if (!res)
        return -1;
    FittingInfo* fitInfo = new FittingInfo;
    BaseLineShapeKind detailedShape;
    HorizonMode horMode;
    bool hasPin = false;
    res = dropletGetShapeEx(&detailedShape, &horMode, &hasPin); // eBS_Fail
    if (!res)
        return -1;
    bool isEmpty = (detailedShape < eBS_ConvexWithPins);
    RectangleF basePt;
    bool isHorizontal = ((detailedShape % 3) == 2);
    if (isHorizontal) {
        res = dropletGetBasepointsByAutoOnHorizontal(
            hasPin,
            m_inParam.rotated,
            horMode,            
            &basePt,
            NULL,
            m_inParam.isSubpixel,
            m_inParam.isBoundary
        );
}
    else {
        res = dropletGetBasepointsByAutoOnSurface(
            hasPin,
            &basePt,
            m_inParam.isSubpixel,
            m_inParam.isBoundary
        );
    }
    if (!res) {
        goto end_proc;
    }

    if (isHorizontal) {
        res = dropletAutoFittingOnHorizontal(
            m_inParam.dropletFitMode,
            &basePt,
            fitInfo
        );
    }
    else {
        res = dropletAutoFittingOnSurface(
            m_inParam.mainShape,
            m_inParam.dropletFitMode,
            m_inParam.baselineFitMode,
            &basePt,
            fitInfo
        );
    }    
    outRes->result = res;
end_proc:    
    outRes->basePoints = basePt;
    outRes->pFittingInfo = fitInfo;

    if (!res) {
        auto err = dropletGetLastError();
        auto errtxt = dropletGetResultString(err);
        outRes->resultString = _T("Error: ") + CString(errtxt); 
        SysFreeString(errtxt);
        return -1;
    }
    outRes->resultString = _T("成功");
    dropletGetResult(outRes->angles);
    RectangleF rect;    
    dropletMidResult(&outRes->mainShape, &outRes->hasPin, &outRes->isEmpty, &outRes->horMode, &rect, NULL);

    return 0;
}

void CTestRunner::ProcessFolder(
    const CString& folder, 
    double angle, 
    int& totalFiles, 
    int& totalErrors, 
    double& totalErrorValue
) {
    CFileFind finder;
    BOOL bworking = finder.FindFile(folder + _T("\\*.*"));
    int errorCount = 0, correct2 = 0, correct1 = 0, fileCount = 0;

    CString line;
    line.Format(_T("%s : angle %.2f, "), folder.GetString(), angle);

    while (bworking && !m_cancelled) {
        bworking = finder.FindNextFile();
        if (finder.IsDots() || finder.IsDirectory())
            continue;
        CString ext = finder.GetFileName().Right(4).MakeLower();
        if (ext != _T(".bmp") && ext != _T(".png")) 
            continue;

        CString filepath = finder.GetFilePath();
        AutoTestOutParam* outParam = new AutoTestOutParam;
        outParam->filePath = filepath;
        int res = ProcOneImage(filepath, outParam);
        m_iPos++; fileCount++;
        double angles[2] = { 0 };
        memcpy(angles, outParam->angles, sizeof(double) * 2);
        m_dlg->PostMessage(UM_DROPLET_AUTOTESTTHREAD_MSG, 
			(WPARAM)m_iPos,
			(LPARAM)outParam); // Send message to update UI

        if (res == 0) {
            if (abs(angles[0] - angle) <= 0.1 && abs(angles[1] - angle) <= 0.1)
                correct2++;
            else if (abs(angles[0] - angle) <= 0.1 || abs(angles[1] - angle) <= 0.1)
                correct1++;

            totalErrorValue += abs(angles[0] - angle) + abs(angles[1] - angle);
        }
        else 
            errorCount++;
    }

    CString str;
    str.Format(_T("Count: %d, 2-0.1: %d, 0.1: %d, Error Count: %d"), fileCount, correct2, correct1, errorCount);
    line += str;
    str.Format(_T(", Accuracy: %.2f%%, Error Rate: %.2f%%, Avg Error: %.2f"),
        100.0 * (correct2 + correct1) / fileCount,
        100.0 * errorCount / fileCount,
        fileCount > errorCount ? totalErrorValue / (fileCount - errorCount) : 0.0);
    line += str + _T("\n");

    m_sResult += line;
    totalFiles += fileCount;
    totalErrors += errorCount;
}

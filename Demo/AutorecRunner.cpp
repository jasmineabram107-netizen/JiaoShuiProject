#include "pch.h"
#include "AutorecRunner.h"
#include "JiaoShuiEllipseFit.h"

CAutorecRunner::CAutorecRunner(CWnd* dlg)
    : m_dlg(dlg)
    , m_cancelled(false)
    , m_running(false)
    , m_iStart(0)
{
}

CAutorecRunner::~CAutorecRunner()
{
    CancelTest();
    if (m_thread.joinable())
        m_thread.join();
}

void 
CAutorecRunner::StartTest(
    const AutoTestInParam& inParam,
    const std::vector<CString>& files, 
    int idxStart
) {
    CancelTest();
    if (m_thread.joinable())
        m_thread.join();
    m_cancelled = false;
    m_running = true;
    m_Files = files;
    m_inParam = inParam;
    if (idxStart < 0 || idxStart >= m_Files.size())
        m_iStart = 0; // Ensure valid start index
    else
        m_iStart = idxStart;

    m_thread = std::thread(&CAutorecRunner::RunTest, this);
}

void CAutorecRunner::CancelTest()
{
    m_cancelled = true;
}

void CAutorecRunner::RunTest()
{ 
    int n = (int)m_Files.size();
    for (int i = m_iStart; i < n; i++)
    {
        if (m_cancelled)
            break;
        const auto& filePath = m_Files[i];
        AutoTestOutParam* outParam = new AutoTestOutParam;        
        outParam->filePath = filePath;
        int res = ProcOneImage(filePath, outParam);
        Sleep(0); // Sleep to allow UI to update and avoid blocking the main thread        
        if (m_dlg) {
            m_dlg->PostMessage(UM_DROPLET_AUTORECOGTHREAD_MSG,
                (WPARAM)i,
                (LPARAM)outParam); // Send message to update UI
        }
        Sleep(100);
    }
    if (m_dlg) {
        WPARAM id = (WPARAM)(m_cancelled ? -1 : 0);
        m_dlg->PostMessage(UM_DROPLET_AUTORECOGTHREAD_MSG, id, NULL);
    }
}

int CAutorecRunner::ProcOneImage(const CString& filepath, AutoTestOutParam* outRes)
{
    outRes->result = false;
    FittingInfo* fitInfo = new FittingInfo;
    auto res = dropletSetImage(filepath);
    if (!res) {
        auto err = dropletGetLastError();
        auto errtxt = dropletGetResultString(err);
        outRes->resultString = _T("Error: ") + CString(errtxt);
        SysFreeString(errtxt);        
        return -1;
    }
    BaseLineShapeKind detailedShape;
    HorizonMode horMode;
    bool hasPin = false;
    res = dropletGetShapeEx(&detailedShape, &horMode, &hasPin); // eBS_Fail
    bool isEmpty = (detailedShape < eBS_ConvexWithPins);

    if (isEmpty) {
        outRes->resultString = _T("图像中没有液滴");
        return -1;
    }
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
        outRes->resultString = CString(errtxt); 
        SysFreeString(errtxt);
        return -1;
    }
    outRes->resultString = _T("成功");
    dropletGetResult(outRes->angles);
    RectangleF rect;
    dropletMidResult(&outRes->mainShape, &outRes->hasPin, &outRes->isEmpty, &outRes->horMode, &rect, NULL);
    return 0;
}
//.EOF
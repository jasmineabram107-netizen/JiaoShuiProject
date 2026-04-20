#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include "common.h"
#include "common_thread.h"

const UINT UM_DROPLET_AUTOTESTTHREAD_MSG = ::RegisterWindowMessage(_T("UM_DROPLET_AUTOTESTTHREAD_MSG"));

class CTestRunner
{
public:
    CTestRunner(CWnd* dlg);
    virtual ~CTestRunner();

    void StartTest(bool isManualAngle, double manualAngle, const AutoTestInParam& inParam, const CString& dstFolder);
    void CancelTest();
    bool IsRunning() const { return m_running; }
    CString GetResultString() const { return m_sResult; }
private:
    void RunTest(bool isManualAngle, double manualAngle);
    void ProcessFolder(const CString& folder, double folderAngle, int& totalFiles, int& totalErrors, double& totalErrorValue);
    int ProcOneImage(const CString& filepath, AutoTestOutParam* outRes);
    CWnd* m_dlg;
    std::thread m_thread;
    std::atomic<bool> m_cancelled;
    std::atomic<bool> m_running;
    CString m_sDstFolder;
    AutoTestInParam m_inParam;
    int m_iPos;
    CString m_sResult;
};


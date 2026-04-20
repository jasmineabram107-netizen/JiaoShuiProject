#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include "common.h"
#include "common_thread.h"

const UINT UM_DROPLET_AUTORECOGTHREAD_MSG = ::RegisterWindowMessage(_T("UM_DROPLET_AUTORECOGTHREAD_MSG"));

class CAutorecRunner
{
public:
	CAutorecRunner(CWnd* dlg);
	virtual ~CAutorecRunner();

	void StartTest(const AutoTestInParam& inParam, const std::vector<CString>& files, int idxStart);
	void CancelTest();
	bool IsRunning() const { return m_running; }
	CString GetResultString() const { return m_sResult; }
private:
	void RunTest();
	int ProcOneImage(const CString& filepath, AutoTestOutParam* outRes);
private:
	CWnd* m_dlg;
	std::thread m_thread;
	std::atomic<bool> m_cancelled;
	std::atomic<bool> m_running;
	std::vector<CString> m_Files;
	AutoTestInParam m_inParam;
	CString m_sResult;
	int m_iStart;
};


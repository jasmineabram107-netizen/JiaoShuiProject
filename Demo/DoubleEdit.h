#pragma once
#include <afxwin.h>

const UINT WM_FINISHED_EDIT_MSG = ::RegisterWindowMessage(_T("WM_FINISHED_EDIT_MSG"));

class CDoubleEdit : public CEdit
{
    DECLARE_DYNAMIC(CDoubleEdit)

public:
    CDoubleEdit();
    virtual ~CDoubleEdit();

protected:
    DECLARE_MESSAGE_MAP()

    virtual BOOL PreTranslateMessage(MSG* pMsg) override;
    //virtual HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    //afx_msg void OnEnChange(); // listen for text change

private:
    void AutoCorrect();
    bool IsValidDouble(const CString& text) const;
    //void UpdateTextColor();

private:
    bool m_bValid;
    //CBrush m_brushNormal;
    //CBrush m_brushError;
};
#include "pch.h"
#include "DoubleEdit.h"

IMPLEMENT_DYNAMIC(CDoubleEdit, CEdit)

CDoubleEdit::CDoubleEdit()
    : m_bValid(true)
{
    /*
    m_brushNormal.CreateSolidBrush(RGB(255, 255, 255)); // white
    m_brushError.CreateSolidBrush(RGB(255, 230, 230));  // light red background
    */
}

CDoubleEdit::~CDoubleEdit()
{
}

BEGIN_MESSAGE_MAP(CDoubleEdit, CEdit)
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    ON_WM_CHAR()
    //ON_CONTROL_REFLECT(EN_CHANGE, &CDoubleEdit::OnEnChange)
END_MESSAGE_MAP()

BOOL CDoubleEdit::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        if (GetParent()) {
            GetParent()->PostMessage(WM_FINISHED_EDIT_MSG, 1, (LPARAM)this); // Notify parent
        }
        return TRUE;
    }
    return CEdit::PreTranslateMessage(pMsg);
}

void CDoubleEdit::OnKillFocus(CWnd* pNewWnd)
{
    CEdit::OnKillFocus(pNewWnd);
    AutoCorrect();
}

void CDoubleEdit::OnSetFocus(CWnd* pOldWnd)
{
    CEdit::OnSetFocus(pOldWnd);
    SetSel(0, -1);
}

void CDoubleEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if ((nChar >= '0' && nChar <= '9') || nChar == '.' /*|| nChar == '-'*/ || nChar == VK_BACK || nChar == VK_DELETE)
    {
        CEdit::OnChar(nChar, nRepCnt, nFlags);
    }
    else
    {
        ::MessageBeep(MB_ICONWARNING);
    }
}



bool CDoubleEdit::IsValidDouble(const CString& text) const
{
    double value;
    return _stscanf_s(text, _T("%lf"), &value) == 1;
}
/*
void CDoubleEdit::OnEnChange()
{
    UpdateTextColor();
}

void CDoubleEdit::UpdateTextColor()
{
    CString text;
    GetWindowText(text);
    text.Trim();
    bool newValid = IsValidDouble(text);

    if (newValid != m_bValid)
    {
        m_bValid = newValid;
        Invalidate(); // repaint
    }
}

HBRUSH CDoubleEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
    if (!m_bValid)
    {
        pDC->SetBkColor(RGB(255, 230, 230)); // Light red
        pDC->SetTextColor(RGB(255, 0, 0));    // Red text
        return (HBRUSH)m_brushError.GetSafeHandle();
    }
    else
    {
        pDC->SetBkColor(RGB(255, 255, 255)); // White
        pDC->SetTextColor(RGB(0, 0, 0));      // Black text
        return (HBRUSH)m_brushNormal.GetSafeHandle();
    }
}
*/
void CDoubleEdit::AutoCorrect()
{
    CString text;
    GetWindowText(text);
    text.Trim();

    // Validate as double
    double value = _tstof(text);
    if (value < 0.0 || text.IsEmpty())
    {
        value = 0.0;
    }

    // Update text
    CString newText;
    newText.Format(_T("%.2f"), value);  // Keep 6 decimal precision
    newText.TrimRight(_T('0'));         // Remove trailing 0
    newText.TrimRight(_T('.'));          // Remove trailing dot if needed
    SetWindowText(newText);

    // Send event
    CWnd* pParent = GetParent();
    if (pParent)
    {
        pParent->PostMessage(WM_FINISHED_EDIT_MSG, 0, (LPARAM)this);
    }
}

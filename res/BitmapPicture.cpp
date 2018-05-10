// BitmapPicture.cpp : implementation file
//
// Copyright (c) 1997 Chris Maunder (Chris.Maunder@cbr.clw.csiro.au)
// Written 1 December, 1997

#include "stdafx.h"
#include "BitmapPicture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// if UPDATE_ENTIRE_CLIENT_AREA is not defined then only the update region of
// the bitmap is updated. Otherwise, on each update, the whole client area of
// the bitmap is drawn. UPDATE_ENTIRE_CLIENT_AREA is slower, but distortion 
// of the picture may occur if it is not defined.

#define UPDATE_ENTIRE_CLIENT_AREA

/////////////////////////////////////////////////////////////////////////////
// CBitmapPicture

CBitmapPicture::CBitmapPicture()
{
    m_hBitmap = NULL;

    m_nResourceID = -1;
    m_strResourceName.Empty();
}

CBitmapPicture::~CBitmapPicture()
{
    if (m_hBitmap) ::DeleteObject(m_hBitmap);
}

BEGIN_MESSAGE_MAP(CBitmapPicture, CStatic)
    //{{AFX_MSG_MAP(CBitmapPicture)
    ON_WM_ERASEBKGND()
    ON_WM_DRAWITEM_REFLECT()
    ON_WM_SYSCOLORCHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBitmapPicture message handlers

BOOL CBitmapPicture::SetBitmap(HBITMAP hBitmap)
{
    ::DeleteObject(m_hBitmap);
    m_hBitmap = hBitmap;
    return ::GetObject(m_hBitmap, sizeof(BITMAP), &m_bmInfo);
}

BOOL CBitmapPicture::SetBitmap(UINT nIDResource)
{
    m_nResourceID = nIDResource;
    m_strResourceName.Empty();

    HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
                                        MAKEINTRESOURCE(nIDResource),
                                        IMAGE_BITMAP, 
                                        0,0, 
                                        LR_LOADMAP3DCOLORS);
    if (!hBmp) return FALSE;
    return CBitmapPicture::SetBitmap(hBmp);
}

BOOL CBitmapPicture::SetBitmap(LPCTSTR lpszResourceName)
{
    m_nResourceID = -1;
    m_strResourceName = lpszResourceName;

    HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), 
                                        lpszResourceName,
                                        IMAGE_BITMAP, 
                                        0,0, 
                                        LR_LOADMAP3DCOLORS);
    if (!hBmp) return FALSE;
    return CBitmapPicture::SetBitmap(hBmp);
}

// Suggested by Pål K. Used to reload the bitmap on system colour changes.
BOOL CBitmapPicture::ReloadBitmap()
{
    if (m_nResourceID > 0) 
        return SetBitmap(m_nResourceID);
    else if (!m_strResourceName.IsEmpty())
        return SetBitmap(m_strResourceName);
    else    // if SetBitmap(HBITMAP hBitmap) was used directly then we can't reload.
        return FALSE;
}

void CBitmapPicture::PreSubclassWindow() 
{
    CStatic::PreSubclassWindow();
    ModifyStyle(0, SS_OWNERDRAW);
}

BOOL CBitmapPicture::OnEraseBkgnd(CDC* pDC) 
{
    CRect rect;
    GetClientRect(rect);

    // If no bitmap selected, simply erase the background as per normal and return
    if (!m_hBitmap)
    {
        CBrush backBrush(::GetSysColor(COLOR_3DFACE)); // (this is meant for dialogs)
        CBrush* pOldBrush = pDC->SelectObject(&backBrush);

        pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
        pDC->SelectObject(pOldBrush);

        return TRUE;
    }

    // We have a bitmap - draw it.

    // Create compatible memory DC using the controls DC
    CDC dcMem;
    VERIFY( dcMem.CreateCompatibleDC(pDC));
    
    // Select bitmap into memory DC.
    HBITMAP* pBmpOld = (HBITMAP*) ::SelectObject(dcMem.m_hDC, m_hBitmap);

    // StretchBlt bitmap onto static's client area
#ifdef UPDATE_ENTIRE_CLIENT_AREA
    pDC->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
                     &dcMem, 0, 0, m_bmInfo.bmWidth-1, m_bmInfo.bmHeight-1,
                     SRCCOPY);
#else
    CRect TargetRect;                // Region on screen to be updated
    pDC->GetClipBox(&TargetRect);
    TargetRect.IntersectRect(TargetRect, rect);

    CRect SrcRect;                    // Region from bitmap to be painted
    SrcRect.left    = MulDiv(TargetRect.left,   m_bmInfo.bmWidth,  rect.Width());
    SrcRect.top     = MulDiv(TargetRect.top,    m_bmInfo.bmHeight, rect.Height());
    SrcRect.right   = MulDiv(TargetRect.right,  m_bmInfo.bmWidth,  rect.Width());
    SrcRect.bottom  = MulDiv(TargetRect.bottom, m_bmInfo.bmHeight, rect.Height());    

    pDC->StretchBlt(TargetRect.left, TargetRect.top, TargetRect.Width(), TargetRect.Height(), 
                    &dcMem, 
                    SrcRect.left, SrcRect.top, SrcRect.Width(), SrcRect.Height(),
                    SRCCOPY);
#endif

    ::SelectObject(dcMem.m_hDC, pBmpOld);

    return TRUE;
}

void CBitmapPicture::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    ASSERT(lpDrawItemStruct != NULL);
    
    CString str;
    GetWindowText(str);
    if (!str.GetLength()) return;

    CDC*  pDC     = CDC::FromHandle(lpDrawItemStruct->hDC);
    CRect rect    = lpDrawItemStruct->rcItem;
    DWORD dwStyle = GetStyle();
    int   nFormat = DT_NOPREFIX | DT_NOCLIP | DT_WORDBREAK | DT_SINGLELINE;

    if (dwStyle & SS_CENTERIMAGE) nFormat |= DT_VCENTER;
    if (dwStyle & SS_CENTER)      nFormat |= DT_CENTER;
    else if (dwStyle & SS_RIGHT)  nFormat |= DT_RIGHT;
    else                          nFormat |= DT_LEFT;

    int nOldMode = pDC->SetBkMode(TRANSPARENT);
    pDC->DrawText(str, rect, nFormat);
    pDC->SetBkMode(nOldMode);
}

// Suggested by Pål K. Tønder.
void CBitmapPicture::OnSysColorChange() 
{
    CStatic::OnSysColorChange();
    ReloadBitmap(); 
}
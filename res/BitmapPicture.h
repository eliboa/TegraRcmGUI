#if !defined(AFX_BITMAPPICTURE_H__A4BE2021_689E_11D1_ABBA_00A0243D1382__INCLUDED_)
#define AFX_BITMAPPICTURE_H__A4BE2021_689E_11D1_ABBA_00A0243D1382__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// BitmapPicture.h : header file
//
// Copyright (c) Chris Maunder (Chris.Maunder@cbr.clw.csiro.au)
// Written 1 December, 1997


/////////////////////////////////////////////////////////////////////////////
// CBitmapPicture window

class CBitmapPicture : public CStatic
{
// Construction
public:
    CBitmapPicture();

// Operations
public:
    BOOL SetBitmap(HBITMAP hBitmap);            // Not recommended
    BOOL SetBitmap(UINT nIDResource);
    BOOL SetBitmap(LPCTSTR lpszResourceName);
    BOOL ReloadBitmap();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBitmapPicture)
    protected:
    virtual void PreSubclassWindow();
    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CBitmapPicture();

// Attributes
protected:
    HBITMAP  m_hBitmap;
    BITMAP   m_bmInfo;

private:
    int     m_nResourceID;
    CString m_strResourceName;

// Generated message map functions
protected:
    //{{AFX_MSG(CBitmapPicture)
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnSysColorChange();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPPICTURE_H__A4BE2021_689E_11D1_ABBA_00A0243D1382__INCLUDED_)


#pragma once

#include "FlexHelperImpl.hpp"

typedef struct _MARGINS
{
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
} MARGINS, *PMARGINS;

typedef HRESULT(WINAPI *ApiDwmIsCompositionEnabled)(BOOL* pfEnabled);
typedef HRESULT(WINAPI *ApiDwmExtendFrameIntoClientArea)(HWND, PMARGINS);

static ApiDwmIsCompositionEnabled PtrDwmIsCompositionEnabled = reinterpret_cast<ApiDwmIsCompositionEnabled>(QLibrary::resolve("dwmapi.dll", "DwmIsCompositionEnabled"));
static ApiDwmExtendFrameIntoClientArea PtrDwmExtendFrameIntoClientArea = reinterpret_cast<ApiDwmExtendFrameIntoClientArea>(QLibrary::resolve("dwmapi.dll", "DwmExtendFrameIntoClientArea"));

#define FLEX_GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define FLEX_GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

class FlexHelperImplWin : public FlexHelperImpl
{
public:
    FlexHelperImplWin()
    {
        _dwmEnabled = (_dwmAllowed && PtrDwmIsCompositionEnabled && PtrDwmIsCompositionEnabled(&_dwmEnabled) == S_OK && _dwmEnabled);
        _borders[0] = 0;
        _borders[1] = 0;
        _borders[2] = 0;
        _borders[3] = 0;
    }

public:
    void notifyFrame(HWND hwnd);
    void redrawFrame(HWND hwnd);
    void updateFrame(HWND hwnd);
    void updateStyle(HWND hwnd);

public:
    BOOL modifyStyle(HWND hwnd, DWORD rsStyle, DWORD asStyle, UINT nFlags);

public:
    static LRESULT WINAPI keyEvent(int nCode, WPARAM wParam, LPARAM lParam);

public:
    BOOL _dwmAllowed = TRUE;
    BOOL _dwmEnabled = TRUE;

    BOOL _lock = 0;
    BOOL _skip = 1;
    BOOL _calc = 0;
    BOOL _curr = 1;
    BOOL _ctrl = 0;

    static HHOOK _hook;

    int _test = 0;
    int _wndW = 500;
    int _wndH = 500;
    int _borders[4];
};

HHOOK FlexHelperImplWin::_hook;

Q_DECLARE_METATYPE(QMargins)

void FlexHelperImplWin::notifyFrame(HWND hwnd)
{
    QGuiApplication::platformNativeInterface()->setWindowProperty(QWidget::find(reinterpret_cast<WId>(hwnd))->windowHandle()->handle(), QByteArrayLiteral("WindowsCustomMargins"), QVariant::fromValue(QMargins(-8, -31, -8, -8)));
}

void FlexHelperImplWin::redrawFrame(HWND hwnd)
{
    if (_dwmEnabled)
    {
        return;
    }

    RECT rc;
    RECT cc;
    GetWindowRect(hwnd, &rc);
    GetClientRect(hwnd, &cc);
    POINT pt = { cc.left, cc.top };
    ClientToScreen(hwnd, &pt);
    int h = rc.bottom - rc.top;
    int w = rc.right - rc.left;
    RECT sc = { 0, 0, w, h };
    HDC hDc = GetWindowDC(hwnd);
    HDC cDc = CreateCompatibleDC(hDc);
    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = rc.right - rc.left;
    bi.biHeight = rc.bottom - rc.top;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    HBITMAP hBitmap = CreateDIBSection(hDc, (BITMAPINFO*)&bi, 0, NULL, NULL, 0);
    HGDIOBJ hPixmap = SelectObject(cDc, hBitmap);
    HBRUSH hBr = CreateSolidBrush(_curr ? RGB(0, 125, 250) : RGB(125, 125, 125));
    HBRUSH hBb = CreateSolidBrush(RGB(125, 125, 125));
    HBRUSH hBa = CreateSolidBrush(RGB(252, 252, 252));
    HBRUSH hBc = CreateSolidBrush(RGB(232, 17, 35));
    int bl = pt.x - rc.left;
    int bt = pt.y - rc.top;
    int br = rc.right - pt.x - cc.right + cc.left;
    int bb = rc.bottom - pt.y - cc.bottom + cc.top;
    FillRect(cDc, &sc, hBr);
    FillRect(cDc, &RECT{ sc.right - 36 * 1, 2, sc.right - 36 * 0 - 2, 32 }, _test == HTCLOSE ? hBc : hBb);
    FillRect(cDc, &RECT{ sc.right - 36 * 2, 2, sc.right - 36 * 1 - 2, 32 }, _test == HTMAXBUTTON ? hBa : hBb);
    FillRect(cDc, &RECT{ sc.right - 36 * 3, 2, sc.right - 36 * 2 - 2, 32 }, _test == HTMINBUTTON ? hBa : hBb);
    BitBlt(hDc, 0, 0, bl, h, cDc, 0, 0, SRCCOPY);
    BitBlt(hDc, 0, 0, w, bt, cDc, 0, 0, SRCCOPY);
    BitBlt(hDc, w - br, 0, br, h, cDc, 0, 0, SRCCOPY);
    BitBlt(hDc, 0, h - bb, w, bb, cDc, 0, 0, SRCCOPY);
    SelectObject(cDc, hPixmap);
    DeleteObject(hBa);
    DeleteObject(hBb);
    DeleteObject(hBc);
    DeleteObject(hBr);
    DeleteObject(hBitmap);
    DeleteDC(cDc);
    ReleaseDC(hwnd, hDc);
}

void FlexHelperImplWin::updateFrame(HWND hwnd)
{
    if (_calc)
    {
        return;
    }

    _calc = TRUE;
    RECT rc;
    GetWindowRect(hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    if (_wndW != w || _wndH != h)
    {
        if (_dwmEnabled)
        {
            if (PtrDwmExtendFrameIntoClientArea(hwnd, &MARGINS{ 1, 1, 1, 1 }) == S_OK)
            {
            }
        }
        else
        {
            DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
            DWORD exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            if (!(dwStyle & WS_MAXIMIZE) || !(exStyle & WS_EX_MDICHILD))
            {
                HRGN hRgn = 0;
                if (dwStyle & WS_MAXIMIZE)
                {
                    int frameBorder = 2;
                    int frameRegion = frameBorder - (exStyle & WS_EX_CLIENTEDGE ? 2 : 0);
                    hRgn = CreateRectRgn(frameRegion, frameRegion, w - frameRegion, h - frameRegion);
                    _borders[0] = frameBorder;
                    _borders[1] = frameBorder;
                    _borders[2] = frameBorder;
                    _borders[3] = frameBorder;
                }
                else
                {
                    int frameBorder = 1;
                    hRgn = CreateRectRgn(0, 0, w, h);
                    _borders[0] = frameBorder;
                    _borders[1] = frameBorder;
                    _borders[2] = frameBorder;
                    _borders[3] = frameBorder;
                }
                SetWindowRgn(hwnd, hRgn, TRUE);
            }
        }
    }

    _wndW = w;
    _wndH = h;
    _calc = FALSE;
}

void FlexHelperImplWin::updateStyle(HWND hwnd)
{
    if (_dwmEnabled)
    {
        DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        DWORD rsStyle = WS_THICKFRAME | WS_DLGFRAME | WS_VSCROLL | WS_HSCROLL;
        if (dwStyle & rsStyle)
        {
            RECT rc;
            _lock = TRUE;
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~rsStyle);
            GetWindowRect(hwnd, &rc);
            SendMessage(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rc);
            if (_windowFlags & Qt::WindowMinimizeButtonHint)
            {
                dwStyle |= WS_MINIMIZEBOX;
            }
            if (_windowFlags & Qt::WindowMaximizeButtonHint)
            {
                dwStyle |= WS_MAXIMIZEBOX;
            }
            if (_windowFlags & Qt::WindowSystemMenuHint)
            {
                dwStyle |= WS_SYSMENU;
            }
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle | WS_CAPTION);
            _lock = FALSE;
            notifyFrame(hwnd);
        }
    }
    else
    {
        DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        DWORD rsStyle = WS_DLGFRAME | WS_VSCROLL | WS_HSCROLL;
        if (dwStyle & rsStyle)
        {
            RECT rc;
            _lock = TRUE;
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~rsStyle);
            GetWindowRect(hwnd, &rc);
            SendMessage(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rc);
            SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle);
            _lock = FALSE;
            redrawFrame(hwnd);
        }
    }
}

BOOL FlexHelperImplWin::modifyStyle(HWND hwnd, DWORD rsStyle, DWORD asStyle, UINT nFlags)
{
    DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
    DWORD nsStyle = (dwStyle & ~rsStyle) | asStyle;

    if (dwStyle == nsStyle)
    {
        return FALSE;
    }

    SetWindowLongPtr(hwnd, GWL_STYLE, nsStyle);

    if (nFlags != 0)
    {
        SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | nFlags);
    }

    return TRUE;
}

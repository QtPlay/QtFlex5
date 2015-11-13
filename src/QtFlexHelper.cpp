#include "QtFlexHelper.h"
#include "QtDockGuider.h"
#include <QtCore/QLibrary>
#include <QtCore/QEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QPainter>
#include <QtGui/QWindow>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QApplication>

#include <private/qwidgetresizehandler_p.h>

#include "FlexButton.h"
#include "FlexButtons.h"
#include "FlexExtents.h"

#include "private/FlexHelperImpl.hpp"


class FlexHandler : public QWidgetResizeHandler
{
public:
    FlexHandler(QWidget* parent, QWidget* widget) : QWidgetResizeHandler(parent, widget)
    {
        setMovingEnabled(false);
    }

public:
    bool eventFilter(QObject* obj, QEvent* evt)
    {
        if (obj->property("Site").isValid())
        {
            return false;
        }
        else
        {
            return QWidgetResizeHandler::eventFilter(obj, evt);
        }
    }
};

#ifdef Q_OS_WIN
#include "private/FlexHelperImplWin.hpp"
#endif

#ifdef Q_OS_LINUX
#include "private/FlexHelperImplXCB.hpp"
#endif

#ifdef Q_OS_MAC
#include "private/FlexHelperImplOSX.hpp"
#endif

FlexHelper::FlexHelper(QWidget* parent)
	: QObject(parent)
{
#ifdef Q_OS_WIN
	impl.reset(new FlexHelperImplWin);
    auto d = static_cast<FlexHelperImplWin*>(impl.data());
    d->_buttons = new FlexButtons(parent, parent);
    d->_extents = new FlexExtents(parent, parent);
    connect(d->_extents->_dockPullButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_extents->_autoHideButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_buttons->_clsButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_buttons->_maxButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_buttons->_minButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
#endif
#ifdef Q_OS_MAC
	impl.reset(new FlexHelperImplMac);
    auto d = static_cast<FlexHelperImplMac*>(impl.data());
    d->_handler = new FlexHandler(parent, parent);
    d->_buttons = new FlexButtons(parent, parent);
    d->_extents = new FlexExtents(parent, parent);
    connect(d->_extents->_dockPullButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_extents->_autoHideButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_buttons->_clsButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_buttons->_maxButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
    connect(d->_buttons->_minButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
#endif
#ifdef Q_OS_LINUX
	if (QGuiApplication::platformName() == "xcb")
	{
		impl.reset(new FlexHelperImplXCB);

		auto d = static_cast<FlexHelperImplXCB*>(impl.data());

//		d->_handler = new FlexHandler(parent, parent);
		d->_buttons = new FlexButtons(parent, parent);
		d->_extents = new FlexExtents(parent, parent);
		connect(d->_extents->_dockPullButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
		connect(d->_extents->_autoHideButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
		connect(d->_buttons->_clsButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
		connect(d->_buttons->_maxButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
		connect(d->_buttons->_minButton, SIGNAL(clicked()), SLOT(on_button_clicked()));
		parent->installEventFilter(this);
	}
	else if (QGuiApplication::platformName() == "wayland")
	{

	}
#endif
    parent->installEventFilter(this);
}

FlexHelper::~FlexHelper()
{
    impl->_buttons->setParent(nullptr);
    impl->_extents->setParent(nullptr);
    impl->_buttons->deleteLater();
    impl->_extents->deleteLater();
}

QWidget* FlexHelper::buttons() const
{
    return impl->_buttons;
}

QWidget* FlexHelper::extents() const
{
    return impl->_extents;
}

QToolButton* FlexHelper::button(Flex::Button button) const
{
    switch (button)
    {
    case Flex::DockPull:
        return impl->_extents->_dockPullButton;
    case Flex::AutoHide:
        return impl->_extents->_autoHideButton;
    case Flex::DockShow:
        return impl->_extents->_autoHideButton;
    case Flex::Minimize:
        return impl->_buttons->minButton();
    case Flex::Maximize:
        return impl->_buttons->maxButton();
    case Flex::Restore:
        return impl->_buttons->maxButton();
    case Flex::Close:
        return impl->_buttons->clsButton();
    default:
        Q_ASSERT(false);
    }
    return nullptr;
}

void FlexHelper::change(Flex::Button src, Flex::Button dst)
{
    static_cast<FlexButton*>(button(src))->setButton(dst);
}

void FlexHelper::setWindowInfo(int titleBarHeight, Qt::WindowFlags windowFlags)
{
    impl->_titleBarHeight = titleBarHeight;
    impl->_windowFlags = windowFlags;
}

bool FlexHelper::eventFilter(QObject* obj, QEvent* evt)
{
#ifdef Q_OS_WIN
    auto hwnd = qobject_cast<QWidget*>(obj)->internalWinId();
#endif
#ifdef Q_OS_WIN
    auto d = static_cast<FlexHelperImplWin*>(impl.data());
#endif
#ifdef Q_OS_MAC
    auto d = static_cast<FlexHelperImplMac*>(impl.data());
#endif

    if (evt->type() == QEvent::Resize)
    {
        auto tmp = static_cast<QResizeEvent*>(evt);

        int w = tmp->size().width();

        QSize buttonsSize = impl->_buttons->sizeHint();
        QSize extentsSize = impl->_extents->sizeHint();

        auto aw = 0;
        auto bw = buttonsSize.width();
        auto ew = extentsSize.width();
        auto by = (impl->_titleBarHeight - buttonsSize.height()) / 2 + 1;
        auto ey = (impl->_titleBarHeight - extentsSize.height()) / 2 + 1;

        impl->_buttons->minButton()->setOver(false);
        impl->_buttons->maxButton()->setOver(false);
        impl->_buttons->clsButton()->setOver(false);
        impl->_buttons->setGeometry(QRect(QPoint(w - bw - aw - 5, by), buttonsSize));
        impl->_extents->dockPullButton()->setOver(false);
        impl->_extents->autoHideButton()->setOver(false);
        impl->_extents->setGeometry(QRect(QPoint(w - bw - ew - 6, ey), extentsSize));

        return false;
    }

    if (obj->property("Site").isValid())
    {
        return false;
    }

    if (evt->type() == QEvent::WinIdChange)
    {
#ifdef Q_OS_WIN
        d->updateStyle(reinterpret_cast<HWND>(hwnd));
#endif
    }
    else if (evt->type() == QEvent::Move)
    {
        if (impl->_moving)
        {
            QMetaObject::invokeMethod(obj, "moving", Q_ARG(QObject*, obj));
        }
    }
#ifdef Q_OS_WIN
    else
    {
        return d->eventFilter(obj, evt);
    }
#endif

    return false;
}

#ifdef Q_OS_WIN
LRESULT WINAPI FlexHelperImplWin::keyEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return CallNextHookEx(FlexHelperImplWin::_hook, nCode, wParam, lParam);
    }

    if (wParam == VK_CONTROL && DockGuider::instance())
    {
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
        {
            DockGuider::instance()->hide();
        }
        else
        {
            DockGuider::instance()->show();
        }
    }

    return CallNextHookEx(FlexHelperImplWin::_hook, nCode, wParam, lParam);
}
#endif

bool FlexHelper::nativeEvent(const QByteArray&, void* event, long* result)
{
#ifdef Q_OS_WIN
    auto d = static_cast<FlexHelperImplWin*>(impl.data());
    auto msg = reinterpret_cast<MSG*>(event);
    auto hwnd = msg->hwnd;
    auto message = msg->message;
    auto lParam = msg->lParam;
    auto wParam = msg->wParam;

    if (!d->_dwmEnabled && d->_lock)
    {
        switch (message)
        {
        case WM_STYLECHANGING:
        case WM_STYLECHANGED:
        case WM_WINDOWPOSCHANGED:
        case WM_NCPAINT:
            *result = 0;
            return true;
        case WM_WINDOWPOSCHANGING:
            *result = 0;
            reinterpret_cast<WINDOWPOS*>(lParam)->flags &= ~SWP_FRAMECHANGED;
            return true;
        }
    }

    QObject* object = parent();

    switch (message)
    {
    case WM_ENTERSIZEMOVE:
    {
        if (d->_moving)
        {
            d->_moving = true;
            QMetaObject::invokeMethod(object, "enterMove", Q_ARG(QObject*, object));
            if (FlexHelperImplWin::_hook == nullptr)
            {
                FlexHelperImplWin::_hook = SetWindowsHookEx(WH_KEYBOARD, FlexHelperImplWin::keyEvent, NULL, GetCurrentThreadId());
            }
        }
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        break;
    }
    case WM_EXITSIZEMOVE:
    {
        if (d->_moving)
        {
            d->_moving = false;
            if (FlexHelperImplWin::_hook != nullptr)
            {
                UnhookWindowsHookEx(FlexHelperImplWin::_hook);
                FlexHelperImplWin::_hook = nullptr;
            }
            QMetaObject::invokeMethod(object, "leaveMove", Q_ARG(QObject*, object));
        }
        QApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        break;
    }
    case WM_DWMCOMPOSITIONCHANGED:
    {
        BOOL dwmEnabled = TRUE;
        dwmEnabled = (d->_dwmAllowed && PtrDwmIsCompositionEnabled && PtrDwmIsCompositionEnabled(&dwmEnabled) == S_OK && dwmEnabled);
        if (d->_dwmEnabled != dwmEnabled)
        {
            d->_dwmEnabled = dwmEnabled;
            SetWindowRgn(hwnd, 0, TRUE);
            if (!d->_dwmEnabled)
            {
                d->updateStyle(hwnd);
            }
            SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }
        break;
    }
    case WM_WINDOWPOSCHANGING:
    {
        DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
        WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lParam);
        if (((wp->flags & SWP_NOSIZE) == 0) && (wp->cx != d->_wndW || wp->cy != d->_wndH) && ((dwStyle & WS_CHILD) == 0))
        {
            d->_skip = TRUE;
        }
        break;
    }
    case WM_WINDOWPOSCHANGED:
    {
        WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(lParam);
        if ((wp->flags & SWP_FRAMECHANGED) && !d->_calc)
        {
            d->updateFrame(hwnd);
        }
        break;
    }
    case WM_SIZE:
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        if (w != d->_wndW || h != d->_wndH)
        {
            d->updateFrame(hwnd);
            d->redrawFrame(hwnd);
        }
        if (wParam == SIZE_MINIMIZED)
        {
            if (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_MAXIMIZE)
            {
                d->modifyStyle(hwnd, WS_MAXIMIZE, 0, 0);
            }
        }
        else if (wParam == SIZE_RESTORED)
        {
            d->_buttons->maxButton()->setButton(Flex::Maximize);
            d->_buttons->maxButton()->setToolTip(QWidget::tr("Maximize"));
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            d->_buttons->maxButton()->setToolTip(QWidget::tr("Restore"));
            d->_buttons->maxButton()->setButton(Flex::Restore);
        }
        break;
    }
    case WM_STYLECHANGED:
    {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        d->updateFrame(hwnd);
        d->redrawFrame(hwnd);
        SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        RedrawWindow(hwnd, &rc, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
        *result = 0;
        return true;
    }
    case WM_NCRBUTTONUP:
    {
        SendMessage(hwnd, 0x0313, (WPARAM)hwnd, lParam);
        break;
    }
    case WM_SYSCOMMAND:
    {
        if ((msg->wParam & 0xFFF0) == SC_MOVE && !IsIconic(hwnd) && !(GetKeyState(VK_CONTROL) & 0x8000))
        {
            d->_moving = true;
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        *result = DefWindowProc(hwnd, message, wParam, lParam);
        MINMAXINFO* lpmmi = reinterpret_cast<MINMAXINFO*>(lParam);
        int yMin = 40;
        int xMin = 3 * yMin;
        xMin += GetSystemMetrics(SM_CYSIZE) + 2 * GetSystemMetrics(SM_CXEDGE);
        //lpmmi->ptMaxPosition.x = -4;
        //lpmmi->ptMaxPosition.y = -4;
        lpmmi->ptMinTrackSize.x = std::max(lpmmi->ptMinTrackSize.x, (LONG)xMin);
        lpmmi->ptMinTrackSize.y = std::max(lpmmi->ptMinTrackSize.y, (LONG)yMin);
        //lpmmi->ptMaxTrackSize.x -= 4;
        //lpmmi->ptMaxTrackSize.y -= 4;
        return true;
    }
    }

    if (d->_dwmEnabled)
    {
        switch (message)
        {
        case WM_NCCALCSIZE:
        {
            if (!d->_lock && wParam)
            {
                auto lpncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

                auto l = lpncsp->rgrc[0].left;
                auto r = lpncsp->rgrc[0].right;
                auto t = lpncsp->rgrc[0].top;
                auto b = lpncsp->rgrc[0].bottom;

                *result = DefWindowProc(hwnd, message, wParam, lParam);

                DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);

                if ((dwStyle & (WS_CHILD | WS_MINIMIZE)) != (WS_CHILD | WS_MINIMIZE))
                {
                    lpncsp->rgrc[0].left   = l;
                    lpncsp->rgrc[0].right  = r;
                    lpncsp->rgrc[0].top    = t;
                    lpncsp->rgrc[0].bottom = b;
                }

                return true;
            }
            break;
        }
        case WM_NCHITTEST:
        {
            POINT pt;
            pt.x = FLEX_GET_X_LPARAM(lParam);
            pt.y = FLEX_GET_Y_LPARAM(lParam);

            QWidget* window = QWidget::find(reinterpret_cast<WId>(hwnd));

            QPoint pos = window->mapFromGlobal(QPoint(pt.x, pt.y));

            if (d->_buttons->geometry().contains(pos) || d->_extents->geometry().contains(pos))
            {
                *result = HTCLIENT; return true;
            }

            RECT rw;
            GetWindowRect(hwnd, &rw);

            if (pt.y >= rw.top + 4 && pt.y < rw.top + d->_titleBarHeight && pt.x >= rw.left + 4 && pt.x < rw.right - 4)
            {
                *result = HTCAPTION; return true;
            }

            int row = 1;
            int col = 1;

            if (pt.y >= rw.top && pt.y < rw.top + 4)
            {
                row = 0;
            }
            else if (pt.y < rw.bottom && pt.y >= rw.bottom - 4)
            {
                row = 2;
            }

            if (pt.x >= rw.left && pt.x < rw.left + 4)
            {
                col = 0;
            }
            else if (pt.x < rw.right && pt.x >= rw.right - 4)
            {
                col = 2;
            }

            LRESULT hitTests[3][3] =
            {
                { HTTOPLEFT, HTTOP, HTTOPRIGHT },
                { HTLEFT, HTCLIENT, HTRIGHT },
                { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
            };

            *result = hitTests[row][col];

            return true;
        }
        }
    }
    else
    {
        switch (message)
        {
        case WM_NCCALCSIZE:
        {
            if (!d->_lock && wParam)
            {
                auto lpncsp = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
                lpncsp->rgrc[0].left += d->_borders[0];
                lpncsp->rgrc[0].top += d->_borders[1];
                lpncsp->rgrc[0].right -= d->_borders[2];
                lpncsp->rgrc[0].bottom -= d->_borders[3];
                DWORD dwStyle = ::GetWindowLongPtr(hwnd, GWL_STYLE);
                if (((dwStyle & (WS_MAXIMIZE | WS_CHILD)) == WS_MAXIMIZE))
                {
                    APPBARDATA abd; abd.cbSize = sizeof(APPBARDATA);
                    if ((SHAppBarMessage(ABM_GETSTATE, &abd) & ABS_AUTOHIDE) != 0)
                    {
                        lpncsp->rgrc[0].bottom -= 1;
                    }
                }
                *result = 0;
                return true;
            }
            break;
        }
        case WM_NCHITTEST:
        {
            POINT pt;
            pt.x = FLEX_GET_X_LPARAM(lParam);
            pt.y = FLEX_GET_Y_LPARAM(lParam);

            QWidget* window = QWidget::find(reinterpret_cast<WId>(hwnd));

            QPoint pos = window->mapFromGlobal(QPoint(pt.x, pt.y));

            if (d->_buttons->geometry().contains(pos) || d->_extents->geometry().contains(pos))
            {
                *result = HTCLIENT; return true;
            }

            RECT rw;
            GetWindowRect(hwnd, &rw);

            if (pt.y >= rw.top + 4 && pt.y < rw.top + d->_titleBarHeight && pt.x >= rw.left + 4 && pt.x < rw.right - 4)
            {
                *result = HTCAPTION; return true;
            }

            int row = 1;
            int col = 1;

            if (pt.y >= rw.top && pt.y < rw.top + 4)
            {
                row = 0;
            }
            else if (pt.y < rw.bottom && pt.y >= rw.bottom - 4)
            {
                row = 2;
            }

            if (pt.x >= rw.left && pt.x < rw.left + 4)
            {
                col = 0;
            }
            else if (pt.x < rw.right && pt.x >= rw.right - 4)
            {
                col = 2;
            }

            LRESULT hitTests[3][3] =
            {
                { HTTOPLEFT, HTTOP, HTTOPRIGHT },
                { HTLEFT, HTCLIENT, HTRIGHT },
                { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
            };

            *result = hitTests[row][col];

            return true;
        }
        case WM_NCPAINT:
        {
            *result = 0;
            if (d->_skip)
            {
                d->_skip = FALSE;
                return true;
            }
            DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
            DWORD exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            if (!(dwStyle & WS_MAXIMIZE) || !(exStyle & WS_EX_MDICHILD))
            {
                d->redrawFrame(hwnd);
                d->_skip = TRUE;
            }
            return true;
        }
        case WM_NCACTIVATE:
        {
            DWORD dwStyle = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (dwStyle & WS_VISIBLE)
            {
                DWORD rsStyle = WS_DLGFRAME | WS_VSCROLL | WS_HSCROLL;
                if (dwStyle & rsStyle)
                {
                    d->_lock = TRUE;
                    SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle & ~rsStyle);
                    RECT rc;
                    GetWindowRect(hwnd, &rc);
                    SendMessage(hwnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rc);
                    SetWindowLongPtr(hwnd, GWL_STYLE, dwStyle);
                    d->_lock = FALSE;
                    d->redrawFrame(hwnd);
                }
                d->_lock = TRUE;
                if (dwStyle & WS_SIZEBOX)
                {
                    d->modifyStyle(hwnd, WS_SIZEBOX, 0, 0);
                }
                *result = DefWindowProc(hwnd, message, wParam, lParam);
                if (dwStyle & WS_SIZEBOX)
                {
                    d->modifyStyle(hwnd, 0, WS_SIZEBOX, 0);
                }
                d->_lock = FALSE;
                return true;
            }
            break;
        }
        case WM_ACTIVATEAPP:
        {
            if (d->_curr != wParam)
            {
                d->_curr = wParam;
                d->redrawFrame(hwnd);
            }
            break;
        }
        }
    }

    return false;
#else
    return false;
#endif
}

void FlexHelper::on_button_clicked()
{
    bool accepted = false;

    auto object = static_cast<FlexButton*>(sender());

    emit clicked(object->button(), &accepted);

    if (!accepted)
    {
        auto window = object->window();
        switch (object->button())
        {
        case Flex::Minimize:
            if (window->isTopLevel())
            {
#ifdef Q_OS_WIN
                ::SendMessage(reinterpret_cast<HWND>(window->internalWinId()), WM_SYSCOMMAND, SC_MINIMIZE, 0);
#else
                window->showMinimized();
#endif
            }
            break;
        case Flex::Maximize:
            if (window->isTopLevel())
            {
#ifdef Q_OS_WIN
                SendMessage(reinterpret_cast<HWND>(window->internalWinId()), WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#else
                window->showMaximized();
#endif
            }
            break;
        case Flex::Restore:
            if (window->isTopLevel())
            {
#ifdef Q_OS_WIN
                SendMessage(reinterpret_cast<HWND>(window->internalWinId()), WM_SYSCOMMAND, SC_RESTORE, 0);
#else
                window->showNormal();
#endif
            }
            break;
        case Flex::Close:
            if (window->isTopLevel())
            {
#ifdef Q_OS_WIN
                SendMessage(reinterpret_cast<HWND>(window->internalWinId()), WM_SYSCOMMAND, SC_CLOSE, 0);
#else
                window->close();
#endif
            }
            break;
        default:
            break;
        }
    }
}


#pragma once

#include "FlexHelperImpl.hpp"

class FlexHelperImplMac : public FlexHelperImpl
{
public:
    FlexHelperImplMac()
    {
    }
public:
    void hittest(QWidget* widget, const QPoint& pos);
public:
    bool eventFilter(QObject*, QEvent*);
public:
    QWidgetResizeHandler* _handler = nullptr;
    int _hit = -1;
    QPoint _old;
};

void FlexHelperImplMac::hittest(QWidget* widget, const QPoint& pos)
{
    QRect rect = widget->rect();

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();

    if (pos.y() >= y + 4 && pos.y() < y + _titleBarHeight && pos.x() >= x + 4 && pos.x() < x + w - 4)
    {
        _hit = 0;
    }
    else
    {
        _hit = -1;
    }
}

bool FlexHelperImplMac::eventFilter(QObject* obj, QEvent* evt)
{
    auto widget = qobject_cast<QWidget*>(obj);

    switch (evt->type())
    {
    case QEvent::WindowStateChange:
    {
        auto state = widget->windowState();

        if (state & Qt::WindowMaximized)
        {
            _buttons->maxButton()->setButton(Flex::Restore);
            _buttons->maxButton()->setToolTip(QWidget::tr("Restore"));
        }
        else
        {
            _buttons->maxButton()->setToolTip(QWidget::tr("Maximize"));
            _buttons->maxButton()->setButton(Flex::Maximize);
        }

        break;
    }
    case QEvent::MouseButtonPress:
    {
        if (widget->isMaximized())
        {
            break;
        }

        QMouseEvent* event = static_cast<QMouseEvent*>(evt);

        if (event->button() == Qt::LeftButton)
        {
            hittest(widget, event->pos());

            if (_hit >= 0)
            {
                widget->grabKeyboard();

                if (_hit == 0)
                {
                    _moving = true; QMetaObject::invokeMethod(widget, "enterMove", Q_ARG(QObject*, widget));
                }

                _old = event->globalPos();
            }
        }

        break;
    }
    case QEvent::MouseButtonRelease:
    {
        if (widget->isMaximized())
        {
            break;
        }

        QMouseEvent* event = static_cast<QMouseEvent*>(evt);

        if (event->button() == Qt::LeftButton)
        {
            widget->releaseKeyboard();

            if (_moving)
            {
                QMetaObject::invokeMethod(widget, "leaveMove", Q_ARG(QObject*, widget));
            }

            _moving = false;
        }

        break;
    }
    case QEvent::MouseMove:
    {
        if (widget->isMaximized())
        {
            break;
        }

        QMouseEvent* event = static_cast<QMouseEvent*>(evt);

        if (_moving)
        {
            if (widget->testAttribute(Qt::WA_WState_ConfigPending))
            {
                break;
            }

            QPoint off = event->globalPos() - _old;

            if (_hit == 0)
            {
                widget->move(widget->pos() + off);
            }

            _old = event->globalPos();
        }
        else
        {
            hittest(widget, event->pos());
        }

        break;
    }
    case QEvent::MouseButtonDblClick:
    {
        if (widget->isMinimized())
        {
            break;
        }

        QMouseEvent* event = static_cast<QMouseEvent*>(evt);

        if (event->button() == Qt::LeftButton)
        {
            if (_hit == 0)
            {
                widget->showMinimized(); _hit = -1;
            }
        }

        break;
    }
    case QEvent::KeyPress:
    {
        QKeyEvent* event = static_cast<QKeyEvent*>(evt);
        if (_moving && (event->key() == Qt::Key_Control || event->key() == Qt::Key_Meta))
        {
            if (DockGuider::instance())
            {
                DockGuider::instance()->hide();
            }
        }
        break;
    }
    case QEvent::KeyRelease:
    {
        QKeyEvent* event = static_cast<QKeyEvent*>(evt);
        if (_moving && (event->key() == Qt::Key_Control || event->key() == Qt::Key_Meta))
        {
            if (DockGuider::instance())
            {
                DockGuider::instance()->show();
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }

    return false;
}


#pragma once

#include <QToolButton>

#include "QtFlexManager.h"
#include "QtDockGuider.h"

class FlexButton : public QToolButton
{
public:
    FlexButton(QWidget* parent, Flex::Button button) : QToolButton(parent), _button(button)
    {
        setFixedSize(16, 16); setFocusPolicy(Qt::NoFocus);
    }
    ~FlexButton()
    {
    }
    Flex::Button button() const
    {
        return _button;
    }
    void setButton(Flex::Button button)
    {
        _button = button; update();
    }
public:
    void setOver(bool over)
    {
        _over = over;
    }
    void setDown(bool down)
    {
        _down = down;
    }
protected:
    void mousePressEvent(QMouseEvent* evt)
    {
        auto save = _down;
        QToolButton::mousePressEvent(evt);
        _down = isDown();
        if (_down != save)
        {
            update();
        }
    }
    void mouseMoveEvent(QMouseEvent* evt)
    {
        auto save = _down;
        QToolButton::mouseMoveEvent(evt);
        _down = isDown();
        if (_down != save)
        {
            update();
        }
    }
    void mouseReleaseEvent(QMouseEvent* evt)
    {
        auto save = _down;
        QToolButton::mouseReleaseEvent(evt);
        _down = isDown();
        if (_down != save)
        {
            update();
        }
    }
    void paintEvent(QPaintEvent*);
    void enterEvent(QEvent* evt)
    {
        _over = true;
        if (isEnabled()) update();
        QToolButton::enterEvent(evt);
    }
    void leaveEvent(QEvent* evt)
    {
        _over = false;
        if (isEnabled()) update();
        QToolButton::leaveEvent(evt);
    }
private:
    Flex::Button _button;
    bool _over = false;
    bool _down = false;
};

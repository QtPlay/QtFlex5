
#pragma once

#include <QWidget>

class FlexExtents : public QWidget
{
public:
    FlexExtents(QWidget* parent, QWidget* docket) : QWidget(parent), _docket(docket)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        _layout = new QHBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(1);
        _dockPullButton = new FlexButton(this, Flex::DockPull);
        _autoHideButton = new FlexButton(this, Flex::AutoHide);
        _dockPullButton->setObjectName("_flex_dockPull");
        _autoHideButton->setObjectName("_flex_autoHide");
        _layout->addWidget(_dockPullButton);
        _layout->addWidget(_autoHideButton);
    }
    FlexButton* dockPullButton() const { return _dockPullButton; }
    FlexButton* autoHideButton() const { return _autoHideButton; }

public:
    QWidget* _docket;
    QHBoxLayout* _layout;
    FlexButton* _dockPullButton;
    FlexButton* _autoHideButton;
};

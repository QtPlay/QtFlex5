
#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVariant>
#include <QStylePainter>

class FlexButtons : public QWidget
{
public:
    FlexButtons(QWidget* parent, QWidget* docket) : QWidget(parent), _widget(docket)
    {
        setFocusPolicy(Qt::NoFocus);
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        _layout = new QHBoxLayout(this);
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(1);
        _clsButton = new FlexButton(this, Flex::Close);
        _maxButton = new FlexButton(this, Flex::Maximize);
        _minButton = new FlexButton(this, Flex::Minimize);
        _clsButton->setObjectName("_flex_close");
        _maxButton->setObjectName("_flex_maximize");
        _minButton->setObjectName("_flex_minimize");
        _clsButton->setToolTip(QWidget::tr("Close"));
        _maxButton->setToolTip(QWidget::tr("Maximize"));
        _minButton->setToolTip(QWidget::tr("Minimize"));
        _layout->addWidget(_minButton);
        _layout->addWidget(_maxButton);
        _layout->addWidget(_clsButton);
    }
    FlexButton* minButton() const { return _minButton; }
    FlexButton* maxButton() const { return _maxButton; }
    FlexButton* clsButton() const { return _clsButton; }

public:
    QWidget* _widget;
    QHBoxLayout* _layout;
    FlexButton* _clsButton;
    FlexButton* _minButton;
    FlexButton* _maxButton;
};


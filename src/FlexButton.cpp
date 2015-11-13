
#include "FlexButton.h"
#include "FlexButtons.h"
#include "QtFlexManager.h"

void FlexButton::paintEvent(QPaintEvent*)
{
    auto widget = static_cast<FlexButtons*>(parentWidget())->_widget;

    bool active = widget->property("active").toBool();

    QRect geom = rect();

    QStylePainter painter(this);

    QIcon icon = FlexManager::instance()->icon(_button);

    painter.setPen(QColor("#E5C365"));
    painter.setBrush(QColor(_down ? "#FFE8A6" : "#FFFCF4"));

    if (_over)
    {
        painter.drawRect(geom.adjusted(0, 0, -1, -1));
    }

    QIcon::Mode mode = QIcon::Normal;

    Flex::ViewMode viewMode = (Flex::ViewMode)widget->property("viewMode").value<int>();

    if (viewMode == Flex::ToolView || viewMode == Flex::ToolPagesView)
    {
        mode = active ? QIcon::Active : QIcon::Normal;
    }

    icon.paint(&painter, geom, Qt::AlignCenter, mode, _over ? QIcon::On : QIcon::Off);
}


#pragma once

#include <Qt>

class FlexHelperImpl
{
public:
    FlexHelperImpl()
    {
    }

public:
    bool _moving = false;

    Qt::WindowFlags _windowFlags;

    FlexButtons* _buttons;
    FlexExtents* _extents;

    int _titleBarHeight = 27;
};

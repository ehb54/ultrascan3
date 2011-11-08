#include <qstyle.h>
#include "../../include/qwt/scrollbar.h"

ScrollBar::ScrollBar(QWidget * parent, const char *name):
    QScrollBar(parent, name)
{
    init();
}

ScrollBar::ScrollBar(Qt::Orientation o, 
        QWidget *parent, const char *name):
    QScrollBar(o, parent, name)
{
    init();
}

ScrollBar::ScrollBar(double minBase, double maxBase, 
        Qt::Orientation o, QWidget *parent, const char *name):
    QScrollBar(o, parent, name)
{
    init();
    setBase(minBase, maxBase);
    moveSlider(minBase, maxBase);
}

void ScrollBar::init()
{
    d_inverted = orientation() == Qt::Vertical;
    d_baseTicks = 1000000;
    d_minBase = 0.0;
    d_maxBase = 1.0;
    moveSlider(d_minBase, d_maxBase);

    connect(this, SIGNAL(sliderMoved(int)), SLOT(catchSliderMoved(int)));
    connect(this, SIGNAL(valueChanged(int)), SLOT(catchValueChanged(int)));
}

void ScrollBar::setInverted(bool inverted)
{
    if ( d_inverted != inverted )
    {
        d_inverted = inverted;
        moveSlider(minSliderValue(), maxSliderValue());
    }
}

bool ScrollBar::isInverted() const
{
    return d_inverted;
}

void ScrollBar::setBase(double min, double max)
{
    if ( min != d_minBase || max != d_maxBase )
    {
        d_minBase = min;
        d_maxBase = max;

        moveSlider(minSliderValue(), maxSliderValue());
    }
}

void ScrollBar::moveSlider(double min, double max)
{
    const int sliderTicks = qRound((max - min) / 
        (d_maxBase - d_minBase) * d_baseTicks);

    // setRange initiates a valueChanged of the scrollbars
    // in some situations. So we block
    // and unblock the signals.

    blockSignals(TRUE);

    setRange(sliderTicks / 2, d_baseTicks - sliderTicks / 2);
    int steps = sliderTicks / 200;
    if ( steps <= 0 )
        steps = 1;

    // setPageStep, setLineStep ???

    setSteps(steps, sliderTicks);

    int tick = mapToTick(min + (max - min) / 2);
    if ( isInverted() )
        tick = d_baseTicks - tick;

    directSetValue(tick);
    blockSignals(FALSE);

    rangeChange();
}

double ScrollBar::minBaseValue() const
{
    return d_minBase;
}

double ScrollBar::maxBaseValue() const
{
    return d_maxBase;
}

void ScrollBar::sliderRange(int value, double &min, double &max) const
{
    if ( isInverted() )
        value = d_baseTicks - value;

    const int visibleTicks = pageStep();

    min = mapFromTick(value - visibleTicks / 2);
    max = mapFromTick(value + visibleTicks / 2);
}

double ScrollBar::minSliderValue() const
{
    double min, dummy;
    sliderRange(value(), min, dummy);

    return min;
}

double ScrollBar::maxSliderValue() const
{
    double max, dummy;
    sliderRange(value(), dummy, max);

    return max;
}

int ScrollBar::mapToTick(double v) const
{   
    return (int) ( ( v - d_minBase) / (d_maxBase - d_minBase ) * d_baseTicks );
}

double ScrollBar::mapFromTick(int tick) const
{   
    return d_minBase + ( d_maxBase - d_minBase ) * tick / d_baseTicks;
}

void ScrollBar::catchValueChanged(int value)
{
    double min, max;
    sliderRange(value, min, max);
    emit valueChanged(orientation(), min, max);
}

void ScrollBar::catchSliderMoved(int value)
{
    double min, max;
    sliderRange(value, min, max);
    emit sliderMoved(orientation(), min, max);
}

int ScrollBar::extent() const
{
    int dim;
#if QT_VERSION >= 300
    dim = style().pixelMetric(QStyle::PM_ScrollBarExtent, this);
#else
    const QSize sz = style().scrollBarExtent();
    dim = (orientation() == Qt::Horizontal) ? sz.height() : sz.width();
#endif
    return dim;
}

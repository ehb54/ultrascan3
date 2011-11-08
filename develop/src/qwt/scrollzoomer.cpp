#include <qevent.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include "../../include/qwt/scrollbar.h"
#include "../../include/qwt/scrollzoomer.h"

class ScrollData
{
public:
    ScrollData():
        scrollBar(NULL),
        position(ScrollZoomer::OppositeToScale),
        mode(QScrollView::Auto)
    {
    }

    ~ScrollData()
    {
        delete scrollBar;
    }

    ScrollBar *scrollBar;
    ScrollZoomer::ScrollBarPosition position;
    QScrollView::ScrollBarMode mode;
};

ScrollZoomer::ScrollZoomer(QwtPlotCanvas *canvas, const char *name):
    QwtPlotZoomer(canvas, name),
    d_cornerWidget(NULL),
    d_hScrollData(NULL),
    d_vScrollData(NULL)
{
    if ( !canvas )
        return;

    d_hScrollData = new ScrollData;
    d_vScrollData = new ScrollData;
}

ScrollZoomer::~ScrollZoomer()
{
    delete d_cornerWidget;
    delete d_vScrollData;
    delete d_hScrollData;
}

void ScrollZoomer::rescale()
{
    QwtPlotZoomer::rescale();
    updateScrollBars();
}

ScrollBar *ScrollZoomer::scrollBar(Qt::Orientation o)
{
    ScrollBar *&sb = (o == Qt::Vertical) 
        ? d_vScrollData->scrollBar : d_hScrollData->scrollBar;

    if ( sb == NULL )
    {
        sb = new ScrollBar(o, canvas());
        sb->hide();
        connect(sb,
            SIGNAL(valueChanged(Qt::Orientation, double, double)),
            SLOT(scrollBarMoved(Qt::Orientation, double, double)));
    }
    return sb;
}

ScrollBar *ScrollZoomer::horizontalScrollBar() const
{
    return d_hScrollData->scrollBar;
}

ScrollBar *ScrollZoomer::verticalScrollBar() const
{
    return d_vScrollData->scrollBar;
}
    
void ScrollZoomer::setHScrollBarMode(QScrollView::ScrollBarMode mode)
{
    if ( hScrollBarMode() != mode )
    {
        d_hScrollData->mode = mode;
        updateScrollBars();
    }
}

void ScrollZoomer::setVScrollBarMode(QScrollView::ScrollBarMode mode)
{
    if ( vScrollBarMode() != mode )
    {
        d_vScrollData->mode = mode;
        updateScrollBars();
    }
}

QScrollView::ScrollBarMode ScrollZoomer::hScrollBarMode() const
{
    return d_hScrollData->mode;
}

QScrollView::ScrollBarMode ScrollZoomer::vScrollBarMode () const
{
    return d_vScrollData->mode;
}

void ScrollZoomer::setHScrollBarPosition(ScrollBarPosition pos)
{
    if ( d_hScrollData->position != pos )
    {
        d_hScrollData->position = pos;
        updateScrollBars();
    }
}

void ScrollZoomer::setVScrollBarPosition(ScrollBarPosition pos)
{
    if ( d_vScrollData->position != pos )
    {
        d_vScrollData->position = pos;
        updateScrollBars();
    }
}

ScrollZoomer::ScrollBarPosition ScrollZoomer::hScrollBarPosition() const
{
    return d_hScrollData->position;
}

ScrollZoomer::ScrollBarPosition ScrollZoomer::vScrollBarPosition() const
{
    return d_vScrollData->position;
}

void ScrollZoomer::setCornerWidget(QWidget *w)
{
    if ( w != d_cornerWidget )
    {
        if ( canvas() )
        {
            delete d_cornerWidget;
            d_cornerWidget = w;
            if ( d_cornerWidget->parent() != canvas() )
                d_cornerWidget->reparent(canvas(), QPoint(0, 0));

            updateScrollBars();
        }
    }
}

QWidget *ScrollZoomer::cornerWidget() const
{
    return d_cornerWidget;
}

bool ScrollZoomer::eventFilter(QObject *o, QEvent *e)
{
    if (  o == canvas() )
    {
        switch(e->type())
        {
            case QEvent::Resize:
            {
                const int fw = ((QwtPlotCanvas *)canvas())->frameWidth();

                QRect rect;
                rect.setSize(((QResizeEvent *)e)->size());
                rect.setRect(rect.x() + fw, rect.y() + fw,
                    rect.width() - 2 * fw, rect.height() - 2 * fw);

                layoutScrollBars(rect);
                break;
            }
            case QEvent::ChildRemoved:
            {
                const QObject *child = ((QChildEvent *)e)->child();
                if ( child == d_cornerWidget )
                    d_cornerWidget = NULL;
                else if ( child == d_hScrollData->scrollBar )
                    d_hScrollData->scrollBar = NULL;
                else if ( child == d_vScrollData->scrollBar )
                    d_vScrollData->scrollBar = NULL;
                break;
            }
            default:
                break;
        }
    }
    return QwtPlotZoomer::eventFilter(o, e);
}

bool ScrollZoomer::needScrollBar(Qt::Orientation o) const
{
    QScrollView::ScrollBarMode mode;
    double zoomMin, zoomMax, baseMin, baseMax;

    if ( o == Qt::Horizontal )
    {
        mode = d_hScrollData->mode;
        baseMin = zoomBase().x1();
        baseMax = zoomBase().x2();
        zoomMin = zoomRect().x1();
        zoomMax = zoomRect().x2();
    }
    else
    {
        mode = d_vScrollData->mode;
        baseMin = zoomBase().y1();
        baseMax = zoomBase().y2();
        zoomMin = zoomRect().y1();
        zoomMax = zoomRect().y2();
    }

    bool needed = FALSE;
    switch(mode)
    {
        case QScrollView::AlwaysOn:
            needed = TRUE;
            break;
        case QScrollView::AlwaysOff:    
            needed = FALSE;
            break;
        case QScrollView::Auto:
        default:
        {
            if ( baseMin < zoomMin || baseMax > zoomMax )
                needed = TRUE;
            break;
        }
    }
    return needed;
}

void ScrollZoomer::updateScrollBars()
{
    if ( !canvas() )
        return;

    const int xAxis = QwtPlotZoomer::xAxis();
    const int yAxis = QwtPlotZoomer::yAxis();

    int xScrollBarAxis = xAxis;
    if ( hScrollBarPosition() == OppositeToScale )
        xScrollBarAxis = oppositeAxis(xScrollBarAxis);

    int yScrollBarAxis = yAxis;
    if ( vScrollBarPosition() == OppositeToScale ) 
        yScrollBarAxis = oppositeAxis(yScrollBarAxis); 


    QwtPlotLayout *layout = plot()->plotLayout();

    bool showHScrollBar = needScrollBar(Qt::Horizontal);
    if ( showHScrollBar )
    {
        ScrollBar *sb = scrollBar(Qt::Horizontal);

        sb->setPalette(plot()->palette());
        sb->setInverted(plot()->axisOptions(xAxis) & QwtAutoScale::Inverted);
        sb->setBase(zoomBase().x1(), zoomBase().x2());
        sb->moveSlider(zoomRect().x1(), zoomRect().x2());

        if ( !sb->isVisibleTo(canvas()) )
        {
            sb->show();
            layout->setCanvasMargin(layout->canvasMargin(xScrollBarAxis) 
                + sb->extent(), xScrollBarAxis);
        }
    }
    else
    {
        if ( horizontalScrollBar() )
        {
            horizontalScrollBar()->hide();
            layout->setCanvasMargin(layout->canvasMargin(xScrollBarAxis) 
                - horizontalScrollBar()->extent(), xScrollBarAxis);
        }
    }

    bool showVScrollBar = needScrollBar(Qt::Vertical);
    if ( showVScrollBar )
    {
        ScrollBar *sb = scrollBar(Qt::Vertical);

        sb->setPalette(plot()->palette());
        sb->setInverted(!(plot()->axisOptions(yAxis) & QwtAutoScale::Inverted));
        sb->setBase(zoomBase().y1(), zoomBase().y2());
        sb->moveSlider(zoomRect().y1(), zoomRect().y2());

        if ( !sb->isVisibleTo(canvas()) )
        {
            sb->show();
            layout->setCanvasMargin(layout->canvasMargin(yScrollBarAxis) 
                + sb->extent(), yScrollBarAxis);
        }
    }
    else
    {
        if ( verticalScrollBar() )
        {
            verticalScrollBar()->hide();
            layout->setCanvasMargin(layout->canvasMargin(yScrollBarAxis) 
                - verticalScrollBar()->extent(), yScrollBarAxis);
        }
    }

    if ( showHScrollBar && showVScrollBar )
    {
        if ( d_cornerWidget == NULL )
        {
            d_cornerWidget = new QWidget(canvas());
            d_cornerWidget->setPalette(plot()->palette());
        }
        d_cornerWidget->show();
    }
    else
    {
        if ( d_cornerWidget )
            d_cornerWidget->hide();
    }

    layoutScrollBars(((QwtPlotCanvas *)canvas())->contentsRect());
}

void ScrollZoomer::layoutScrollBars(const QRect &rect)
{
    int hPos = xAxis();
    if ( hScrollBarPosition() == OppositeToScale )
        hPos = oppositeAxis(hPos);

    int vPos = yAxis();
    if ( vScrollBarPosition() == OppositeToScale )
        vPos = oppositeAxis(vPos);

    ScrollBar *hScrollBar = horizontalScrollBar();
    ScrollBar *vScrollBar = verticalScrollBar();

    const int hdim = hScrollBar ? hScrollBar->extent() : 0;
    const int vdim = vScrollBar ? vScrollBar->extent() : 0;
    
    if ( hScrollBar && hScrollBar->isVisible() )
    {
        int x = rect.x();
        int y = (hPos == QwtPlot::xTop) 
            ? rect.top() : rect.bottom() - hdim + 1;
        int w = rect.width();

        if ( vScrollBar && vScrollBar->isVisible() )
        {
            if ( vPos == QwtPlot::yLeft )
                x += vdim;
            w -= vdim + 1;
        }

        hScrollBar->setGeometry(x, y, w, hdim);
    }
    if ( vScrollBar && vScrollBar->isVisible() )
    {
        int pos = yAxis();
        if ( vScrollBarPosition() == OppositeToScale )
            pos = oppositeAxis(pos);

        int x = (vPos == QwtPlot::yLeft)
            ? rect.left() : rect.right() - vdim;
        int y = rect.y();

        int h = rect.height();

        if ( hScrollBar && hScrollBar->isVisible() )
        {
            if ( hPos == QwtPlot::xTop )
                y += hdim;
                
            h -= hdim;
        }

        vScrollBar->setGeometry(x, y, vdim, h);
    }
    if ( hScrollBar && hScrollBar->isVisible() &&
        vScrollBar && vScrollBar->isVisible() )
    {
        if ( d_cornerWidget )
        {
            QRect cornerRect(
                vScrollBar->pos().x(), hScrollBar->pos().y(),
                vdim, hdim);
            d_cornerWidget->setGeometry(cornerRect);
        }
    }
}

void ScrollZoomer::scrollBarMoved(Qt::Orientation o, double min, double)
{
    if ( o == Qt::Horizontal )
        move(min, zoomRect().y1());
    else
        move(zoomRect().x1(), min);

    emit zoomed(zoomRect());
}

int ScrollZoomer::oppositeAxis(int axis) const
{
    switch(axis)
    {
        case QwtPlot::xBottom:
            return QwtPlot::xTop;
        case QwtPlot::xTop:
            return QwtPlot::xBottom;
        case QwtPlot::yLeft:
            return QwtPlot::yRight;
        case QwtPlot::yRight:
            return QwtPlot::yLeft;
        default:
            break;
    }

    return axis;
}

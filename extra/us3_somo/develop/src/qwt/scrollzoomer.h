#ifndef _SCROLLZOOMER_H
#define _SCROLLZOOMER_H

#include <qglobal.h>
#if QT_VERSION < 0x040000
#include <q3scrollview.h>
#endif
#include <qwt_plot_zoomer.h>
//Added by qt3to4:
#include <QEvent>

class ScrollData;
class ScrollBar;

class ScrollZoomer: public QwtPlotZoomer
{
    Q_OBJECT
public:
    enum ScrollBarPosition
    {
        AttachedToScale,
        OppositeToScale
    };

    ScrollZoomer(QwtPlotCanvas *);
    virtual ~ScrollZoomer();

    ScrollBar *horizontalScrollBar() const;
    ScrollBar *verticalScrollBar() const;

#if QT_VERSION < 0x040000
    void setHScrollBarMode(Q3ScrollView::ScrollBarMode);
    void setVScrollBarMode(Q3ScrollView::ScrollBarMode);

    Q3ScrollView::ScrollBarMode vScrollBarMode () const;
    Q3ScrollView::ScrollBarMode hScrollBarMode () const;
#else
    void setHScrollBarMode(Qt::ScrollBarPolicy);
    void setVScrollBarMode(Qt::ScrollBarPolicy);

    Qt::ScrollBarPolicy vScrollBarMode () const;
    Qt::ScrollBarPolicy hScrollBarMode () const;
#endif

    void setHScrollBarPosition(ScrollBarPosition);
    void setVScrollBarPosition(ScrollBarPosition);

    ScrollBarPosition hScrollBarPosition() const;
    ScrollBarPosition vScrollBarPosition() const;

    QWidget* cornerWidget() const;
    virtual void setCornerWidget(QWidget *); 
    
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void rescale();

protected:
    virtual ScrollBar *scrollBar(Qt::Orientation);
    virtual void updateScrollBars();
    virtual void layoutScrollBars(const QRect &);

private slots:
    void scrollBarMoved(Qt::Orientation o, double min, double max);

private:
    bool needScrollBar(Qt::Orientation) const;
    int oppositeAxis(int) const;

    QWidget *d_cornerWidget;

    ScrollData *d_hScrollData;
    ScrollData *d_vScrollData;

    bool d_inZoom;
    bool d_alignCanvasToScales;
};
            
#endif

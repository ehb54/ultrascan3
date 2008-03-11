#ifndef US_IMGVIEWER_H
#define US_IMGVIEWER_H

#include <qwidget.h>
#include <qimage.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qclipboard.h>

#include "us_extern.h"

class QLabel;
class QMenuBar;
class QPopupMenu;

class US_EXTERN US_ImageViewer : public QWidget
{
    Q_OBJECT
public:
    US_ImageViewer( QWidget *parent=0, const char *name=0, int wFlags=0 );
    ~US_ImageViewer();
    bool        loadImage( const QString& );
protected:
    void        paintEvent( QPaintEvent * );
    void        resizeEvent( QResizeEvent * );
    void        mousePressEvent( QMouseEvent * );
    void        mouseReleaseEvent( QMouseEvent * );
    void        mouseMoveEvent( QMouseEvent * );

private:
    void        scale();
    int         conversion_flags;
    bool        smooth() const;
    bool        useColorContext() const;
    int         alloc_context;
    bool        convertEvent( QMouseEvent* e, int& x, int& y );
    QString     filename;
    QImage      image;                  // the loaded image
    QPixmap     pm;                     // the converted pixmap
    QPixmap     pmScaled;               // the scaled pixmap

    QMenuBar   *menubar;
    QPopupMenu  *file;
    QPopupMenu   *saveimage;
    QPopupMenu   *savepixmap;
    QPopupMenu  *edit;
    QPopupMenu  *options;

    QWidget    *helpmsg;
    QLabel     *status;
    int         si, sp, ac, co, mo, fd, bd, // Menu item ids
                td, ta, ba, fa, au, ad, dd,
                ss, cc, t1, t8, t32;
    void        updateStatus();
    void        setMenuItemFlags();
    bool        reconvertImage();
    int         pickx, picky;
    int         clickx, clicky;
    bool        may_be_other;
    static US_ImageViewer* other;
    void        setImage(const QImage& newimage);

public slots:
	void openFile(QString);


private slots:
    void        to1Bit();
    void        to8Bit();
    void        to32Bit();
    void        toBitDepth(int);

    void        copy();
    void        paste();

    void        hFlip();
    void        vFlip();
    void        rot180();

    void        newWindow();
    void        openFile();
    void        saveImage(int);
    void        savePixmap(int);
    void        giveHelp();
    void        doOption(int);
    void        copyFrom(US_ImageViewer*);
};


#endif 


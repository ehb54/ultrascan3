//Added by qt3to4:
#include <QResizeEvent>
#ifndef US_EDITOR_H
#define US_EDITOR_H

#include <QtWidgets>
#include "us_extern.h"

class US_EXTERN TextEdit : public QFrame
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, const char *name = 0 );
    TextEdit(int id, QWidget *parent = 0, const char *name = 0);
    void load( const QString &f, QString title = "", bool ourfmt = false /*, TextFormat fmt = RichText */ );
    void load_text( QString text, QString title = "" );
};

#endif


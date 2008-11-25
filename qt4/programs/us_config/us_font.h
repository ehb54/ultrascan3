//! \file us_font.h
#ifndef US_FONT_H
#define US_FONT_H

#include <QtGui>

#include "us_widgets.h"
#include "us_global.h"
#include "us_help.h"

class US_Font : public US_Widgets
{
  Q_OBJECT
  
  public:
    US_Font( QWidget* = 0,  Qt::WindowFlags f = 0 );
    ~US_Font();

  private:

    QLineEdit*   le_family;
    QLineEdit*   le_pointSize;
                
    QLabel*      small;
    QLabel*      regular;
    QLabel*      regularBold;
    QLabel*      large;
    QLabel*      largeBold;
    QLabel*      title;

    QPushButton* pb_save;
    QPushButton* pb_help;
    QPushButton* pb_cancel;

    US_Global    g;

  private slots:
  
    void save      ( void );
    void help      ( void );
    void selectFont( void );
};

#endif

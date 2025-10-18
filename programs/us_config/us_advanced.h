//! \file us_advanced.h
#ifndef US_ADVANCED_H
#define US_ADVANCED_H

#include <QtCore>

#include "us_widgets.h"

/*! \brief A class to allow the user to set, modify or delete
    advanced parameters
*/
class US_Advanced : public US_Widgets
{
  Q_OBJECT
  
  public:
    
    //!  Construct the window to manage advanced parameters.
    US_Advanced( QWidget* w = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
    
  private:

    QPushButton* pb_reset; 
    QPushButton* pb_save;   
    QPushButton* pb_help;
    QPushButton* pb_close;

    QSpinBox*    sb_debug;
    QSpinBox*    sb_advanced;
    QSpinBox*    sb_threads;
    QSpinBox*    sb_noisdiag;

    QTextEdit*   te_dbgtext;

  private slots:
    void save           ( void );
    void help           ( void );
    void reset          ( void );
};
#endif

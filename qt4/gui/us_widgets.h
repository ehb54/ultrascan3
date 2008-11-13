#ifndef US_WIDGETS_H
#define US_WIDGETS_H

#include <QtGui>

class US_Widgets : public QFrame
{
  public: 
    US_Widgets ( QWidget* p = 0, Qt::WindowFlags flags = 0 ) 
      : QFrame( p, flags) {};
    
    ~US_Widgets( void ) {};

    QLabel*      us_label     ( const QString&, int = 0,  int = QFont::Normal );
    QLabel*      us_textlabel ( const QString&, int = -1, int = QFont::Normal );
    QLabel*      us_banner    ( const QString&, int = 0,  int = QFont::Bold );
    QPushButton* us_pushbutton( const QString&, bool = true ); 
    QTextEdit*   us_textedit  ( void );
    QLineEdit*   us_lineedit  ( const QString& = 0 );
    QCheckBox*   us_checkbox  ( const QString&, Qt::CheckState = Qt::Unchecked );
};
#endif


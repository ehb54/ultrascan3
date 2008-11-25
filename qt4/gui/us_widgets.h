//! \file us_widgets.h
#ifndef US_WIDGETS_H
#define US_WIDGETS_H

#include <QtGui>

class US_Widgets : public QFrame
{
  public: 
    US_Widgets ( QWidget* p = 0, Qt::WindowFlags flags = 0 ) 
      : QFrame( p, flags) {};
    
    ~US_Widgets( void ) {};

    //! \param labelString - contents of label
    //! \param fontAdjust  - adjustment to default point size 
    //! \param weight      - QFont::{Light,Normal,DemiBold,Bold,Black}
    //! * Alignment is set to Qt::AlignVCenter | Qt::AlignLeft
    //! * Margin is set to 5
    //! * Palette is set to US_GuiSettings::labelColor()
    QLabel*      us_label     ( const QString&, int = 0,  int = QFont::Normal );

    //! Same as us_label except
    //! * different default font size (-1 point)
    //! * color set to US_GuiSettings::editColor()
    QLabel*      us_textlabel ( const QString&, int = -1, int = QFont::Normal );

    //! Same as us_label except  
    //! * different default font weight (Bold )
    //! * color set to US_GuiSettings::frameColor()
    //! * alignment set to Qt::AlignCenter
    //! * style set to QFrame::WinPanel | QFrame::Raised
    //! * MidLineWidth set to 2 for frame border
    QLabel*      us_banner    ( const QString&, int = 0,  int = QFont::Bold );

    //! \param labelString - text in pushbutton
    //! \param enabled
    //! \param fontAdjust -  adjustment to default point size
    //! * Standard Font family and size
    //! * color set to US_GuiSettings::pushbColor()
    //! * button is enabled
    //! * AutoDevault is set false
    QPushButton* us_pushbutton( const QString&, bool = true, int = 0 ); 
    
    //! * Font size is set to default - 1
    //! * Color is set to US_GuiSettings::normalColor()
    //! * Style is WinPanel | Sunken
    //! * AcceptRichText is true
    //! * ReadOnly is true
    QTextEdit*   us_textedit  ( void );

    //! \param test - starting text in line edit box
    //! \param fontAdjust -  adjustment to default point size
    //! * Color is set to US_GuiSettings::editColor()
    //! * ReadOnly is false
    QLineEdit*   us_lineedit  ( const QString& = 0, int = -1 );
    
    //! \param text - Text to set with checkbox
    //! \param state - Qt::{Unchecked,PartiallyChecked,Checked}
    //! * Font weight is bold
    //! * Color is set to US_GuiSettings::normalColor()
    QCheckBox*   us_checkbox  ( const QString&, Qt::CheckState = Qt::Unchecked );

    //! \param text - Text to set with radiobutton
    //! \param state - Qt::{Unchecked,PartiallyChecked,Checked}
    //! * Color is set to US_GuiSettings::editColor()
    QRadioButton* us_radiobutton( const QString&, Qt::CheckState = Qt::Unchecked );
};
#endif


//! \file us_widgets.cpp
#include "us_widgets.h"
#include "us_gui_settings.h"

// label
QLabel* US_Widgets::us_label( const QString& labelString, int fontAdjust, 
                              int weight )
{
  QLabel* newLabel = new QLabel( labelString, this );

  newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
  newLabel->setAlignment ( Qt::AlignVCenter | Qt::AlignLeft );
  newLabel->setMargin    ( 5 );
  newLabel->setAutoFillBackground( true );

  newLabel->setFont(
      QFont( US_GuiSettings::fontFamily(), 
             US_GuiSettings::fontSize() + fontAdjust, 
             weight ) );

  newLabel->setPalette( US_GuiSettings::labelColor() );

  return newLabel;
}

// textlabel ( defaults to smaller font and changes text colors )
QLabel* US_Widgets::us_textlabel( const QString& labelString, int fontAdjust, 
                                  int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setPalette( US_GuiSettings::editColor() );

  return newLabel;
}

// banner ( defaults to Bold and changes text colors )
QLabel* US_Widgets::us_banner( const QString& labelString, int fontAdjust, 
                            int weight )
{
  QLabel* newLabel = us_label( labelString, fontAdjust, weight );

  newLabel->setAlignment ( Qt::AlignCenter );
  newLabel->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
  newLabel->setMidLineWidth( 2 );

  // Set label colors
  newLabel->setPalette( US_GuiSettings::frameColor() );

  return newLabel;
}

// pushbutton
QPushButton* US_Widgets::us_pushbutton( const QString& labelString, bool enabled,
                                        int fontAdjust )
{
  QPushButton* button =  new QPushButton( tr( labelString.toAscii() ), this );

  button->setFont( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize() + fontAdjust ) );

  button->setPalette( US_GuiSettings::pushbColor() );

  button->setAutoDefault( false );
  button->setEnabled( enabled );

  return button;
}

// textedit
QTextEdit* US_Widgets::us_textedit( void )
{
  QTextEdit* te = new QTextEdit( this );

  te->setFont          ( QFont( US_GuiSettings::fontFamily(), 
                                US_GuiSettings::fontSize() - 1 ) );
  
  te->setPalette       ( US_GuiSettings::normalColor() );
  te->setFrameStyle    ( WinPanel | Sunken );
  te->setAcceptRichText( true );
  te->setReadOnly      ( true );
  te->show();

  return te;
}

// lineedit
QLineEdit* US_Widgets::us_lineedit( const QString& text, int fontAdjust )
{
  QLineEdit* le = new QLineEdit( this );


  le->setFont    ( QFont( US_GuiSettings::fontFamily(), 
                          US_GuiSettings::fontSize() + fontAdjust ) );
  
  le->insert     ( text );
  le->setPalette ( US_GuiSettings::editColor() );
  le->setReadOnly( false );
  le->show();

  return le;
}

// checkbox
QCheckBox* US_Widgets::us_checkbox( const QString& text, Qt::CheckState state )
{
  QCheckBox* cb = new QCheckBox( text.toAscii(), this );
  
  cb->setFont( QFont ( US_GuiSettings::fontFamily(), 
                       US_GuiSettings::fontSize(),
                       QFont::Bold ) );
  
  cb->setPalette( US_GuiSettings::normalColor() );

  cb->setCheckState( state );
  return cb;
}

// radiobutton
QRadioButton* US_Widgets::us_radiobutton( const QString& text, Qt::CheckState state )
{
  QRadioButton* rb = new QRadioButton( text.toAscii(), this );
  
  rb->setFont( QFont ( US_GuiSettings::fontFamily(), 
                       US_GuiSettings::fontSize(),
                       QFont::Bold ) );
  
  rb->setPalette( US_GuiSettings::editColor() );

  rb->setChecked( state );
  return rb;
}


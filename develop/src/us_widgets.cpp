#include "../include/us_widgets.h"

US_Widgets::US_Widgets( QWidget* p, const char* name ): QFrame( p, name )
{
	USglobal = new US_Config();

	setPalette( QPalette( USglobal->global_colors.cg_frame,
	                      USglobal->global_colors.cg_frame,
	                      USglobal->global_colors.cg_frame ) );
}

// label
QLabel* US_Widgets::label( const QString& labelString, int fontAdjust, 
                           int weight )
{
	QLabel* newLabel = new QLabel( tr( labelString ), this );

	newLabel->setFrameStyle( StyledPanel | Raised );
	newLabel->setAlignment ( AlignCenter );
	
	newLabel->setFont(
			QFont( USglobal->config_list.fontFamily, 
				     USglobal->config_list.fontSize + fontAdjust, 
						 weight ) );
	
	QPalette defaultpalette( USglobal->global_colors.cg_label,  // active
			                     USglobal->global_colors.cg_label,  // inactive
							             USglobal->global_colors.cg_label ); // disabled

	newLabel->setPalette( defaultpalette );

	return newLabel;
}

// textlabel ( defaults to smaller font and changes text colors )
QLabel* US_Widgets::textlabel( const QString& labelString, int fontAdjust, 
                               int weight )
{
	QLabel* newLabel = label( labelString, fontAdjust, weight );

	QPalette defaultpalette( USglobal->global_colors.cg_edit,
			                     USglobal->global_colors.cg_edit,
							             USglobal->global_colors.cg_edit );

	newLabel->setPalette( defaultpalette );

	return newLabel;
}

// banner ( defaults to Bold and changes text colors )
QLabel* US_Widgets::banner( const QString& labelString, int fontAdjust, 
                               int weight )
{
	QLabel* newLabel = label( labelString, fontAdjust, weight );

	// Set label colors
	QPalette defaultpalette( USglobal->global_colors.cg_frame,
			                     USglobal->global_colors.cg_frame,
							             USglobal->global_colors.cg_frame );

	newLabel->setPalette( defaultpalette );

	return newLabel;
}

// pushbutton
QPushButton* US_Widgets::pushbutton( const QString& labelString, bool enabled )
{
	QPushButton* button =  new QPushButton( tr( labelString ), this );

	button->setFont(
			QFont( USglobal->config_list.fontFamily, 
				     USglobal->config_list.fontSize ) );
  
	button->setPalette( 
			QPalette( USglobal->global_colors.cg_pushb, 
				        USglobal->global_colors.cg_pushb_disabled, 
								USglobal->global_colors.cg_pushb_active ) );

	button->setAutoDefault( false );
	button->setEnabled( enabled );

	return button;
}

// textedit
QTextEdit* US_Widgets::textedit( void )
{
  QTextEdit* te = new QTextEdit( this );
	
	te->setFont(
			QFont( USglobal->config_list.fontFamily,
		         USglobal->config_list.fontSize - 1 ) );
	
	te->setPalette( 
			QPalette( USglobal->global_colors.cg_normal,
							  USglobal->global_colors.cg_normal,
								USglobal->global_colors.cg_normal ) );
	
	te->setFrameStyle( WinPanel | Sunken );
	te->setTextFormat( RichText );
	te->setReadOnly  ( true );
	te->show();

	return te;
}

// checkbox
QCheckBox* US_Widgets::checkbox( const QString& text, bool flag )
{
  QCheckBox* cb = new QCheckBox( tr( text ), this );
	
	cb->setFont(
			QFont( USglobal->config_list.fontFamily,
		         USglobal->config_list.fontSize,
						 QFont::Bold ) );
	
	cb->setPalette( 
			QPalette( USglobal->global_colors.cg_normal,
							  USglobal->global_colors.cg_normal,
								USglobal->global_colors.cg_normal ) );
	
	cb->setChecked( flag );
	return cb;
}

// popup menu

QPopupMenu* US_Widgets::popupmenu ( int fontAdjust )
{
	QPopupMenu* pu = new QPopupMenu;
	
	pu->setFont(
			QFont( USglobal->config_list.fontFamily, 
				     USglobal->config_list.fontSize + fontAdjust ) );

	return pu;
}

// Menu bar
QMenuBar* US_Widgets::menubar( int fontAdjust, int fontWeight )
{
	QMenuBar* mb = new QMenuBar( this );

	mb->setFont( 
			QFont( USglobal->config_list.fontFamily, 
				     USglobal->config_list.fontSize + fontAdjust, 
						 fontWeight ) );

	return mb;
}




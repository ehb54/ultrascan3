#include "../include/us_db_widgets.h"
#include <qframe.h>

US_DB_Widgets::US_DB_Widgets( QWidget* p, const char* name ): US_DB( p, name )
{
	setPalette( QPalette( USglobal->global_colors.cg_frame,
	                      USglobal->global_colors.cg_frame,
	                      USglobal->global_colors.cg_frame ) );
}

// label
QLabel* US_DB_Widgets::label( const QString& labelString, int fontAdjust, 
                              int weight )
{
	QLabel* newLabel = new QLabel( tr( labelString ), this );

	newLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
	newLabel->setAlignment ( QFrame::AlignCenter );
	
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
QLabel* US_DB_Widgets::textlabel( const QString& labelString, int fontAdjust, 
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
QLabel* US_DB_Widgets::banner( const QString& labelString, int fontAdjust, 
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
QPushButton* US_DB_Widgets::pushbutton( const QString& labelString, bool enabled )
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
QTextEdit* US_DB_Widgets::textedit( void )
{
  QTextEdit* te = new QTextEdit( this );
	
	te->setFont(
			QFont( USglobal->config_list.fontFamily,
		         USglobal->config_list.fontSize - 1 ) );
	
	te->setPalette( 
			QPalette( USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal ) );
	
	te->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
	te->setTextFormat( Qt::RichText );
	te->setReadOnly  ( true );
	te->show();

	return te;
}

// checkbox
QCheckBox* US_DB_Widgets::checkbox( const QString& text, bool flag )
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

// listbox
QListBox* US_DB_Widgets::listbox( const QString& name, bool select )
{
  QListBox* lb = new QListBox( this, tr( name ) );
	
	lb->setPalette( 
	    QPalette( USglobal->global_colors.cg_normal,
	              USglobal->global_colors.cg_normal,
	              USglobal->global_colors.cg_normal ) );

	lb->setSelected( 0, select );
	
	return lb;
}

// progress dialog
QProgressDialog* US_DB_Widgets::progressdialog( const QString& label, 
                                                const QString& name,
                                                int   steps )
{
  QProgressDialog* p = new QProgressDialog( tr( label ), 0, steps, this, 
	                                          name );
	
	p->setPalette( 
			QPalette( USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal ) );

	p->setFont( QFont( USglobal->config_list.fontFamily, 
	                   USglobal->config_list.fontSize - 1,
	                   QFont::Bold ) );

	p->setProgress( 0 ); 
	
	return p;
}

// line edit
QLineEdit* US_DB_Widgets::lineedit( const QString& name )
{
	QLineEdit* le = new QLineEdit( this, name );

	le->setPalette( 
			QPalette( USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal ) );

	return le;
}

// combo box
QComboBox* US_DB_Widgets::combobox( const QString& name, int rw )
{
	QComboBox* cb = new QComboBox( rw, this, name );

	cb->setPalette( 
			QPalette( USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal,
			          USglobal->global_colors.cg_normal ) );

	cb->setFont( QFont( USglobal->config_list.fontFamily, 
	                    USglobal->config_list.fontSize - 1 ) );

	return cb;
}

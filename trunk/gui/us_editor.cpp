#include "us_editor.h"
#include "us_gui_settings.h"

US_Editor::US_Editor( int menu, bool readonly, QWidget* parent, 
      Qt::WindowFlags flags ) : QMainWindow( parent, flags )                                                             
{
   filename = "";
   
   // Add menu types as necessary
   switch ( menu )
   {
      default:
         QAction* saveAsAction = new QAction( tr( "&SaveAs" ), this );;
         saveAsAction->setShortcut( tr( "Ctrl+S" ) );
         connect( saveAsAction, SIGNAL( triggered() ), this, SLOT( saveAs() ) );

         QAction* clearAction = new QAction( tr( "Clear" ), this );;
         connect( clearAction, SIGNAL( triggered() ), this, SLOT( clear() ) );

         QAction* printAction = new QAction( tr( "&Print" ), this );;
         printAction->setShortcut( tr( "Ctrl+P" ) );
         connect( printAction, SIGNAL( triggered() ), this, SLOT( print() ) );

         QAction* fontAction = new QAction( tr( "&Font" ), this );;
         fontAction->setShortcut( tr( "Ctrl+F" ) );
         connect( fontAction, SIGNAL( triggered() ), this, SLOT( update_font() ) );

         QMenu* fileMenu = menuBar()->addMenu( tr( "&File" ) );
         fileMenu->addAction( saveAsAction );
         fileMenu->addAction( printAction );
         fileMenu->addAction( fontAction );
         fileMenu->addAction( clearAction );
         fileMenu->setFont  ( QFont( US_GuiSettings::fontFamily(),
                                     US_GuiSettings::fontSize() - 1 ) );
   }

   menuBar()->setPalette( US_GuiSettings::normalColor() );
   menuBar()->setFont   ( QFont( US_GuiSettings::fontFamily(),
                          US_GuiSettings::fontSize() ) );

   currentFont = QFont( "Courier", 11, QFont::Bold );
   
   e = new QTextEdit( this );
   e->setFont          ( currentFont );
   e->setPalette( US_GuiSettings::editColor() );
   e->setAcceptRichText( false );
   e->setWordWrapMode  ( QTextOption::WrapAtWordBoundaryOrAnywhere );
   e->setReadOnly      ( readonly );
   setCentralWidget( e );
}

void US_Editor::saveAs(  )
{
   QString fn;

   fn = QFileDialog::getSaveFileName( this );
   
   if ( ! fn.isEmpty(  ) )
   {
      QString text = e->toPlainText();
      
      QFile f( fn );

      if ( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         QTextStream t( &f );

         t << text;
         f.close(  );
      }
      else
         QMessageBox::information( this,
               tr( "Error" ),
               tr( "Could not open\n\n" ) + fn + tr( "\n\n for writing." ) );

      filename = fn;
   }
}

void US_Editor::update_font(  )
{
   bool  ok;
   QFont newFont = QFontDialog::getFont( &ok, currentFont, this );
   
   if ( ok )
   {
      currentFont = newFont;
      e->setFont( currentFont );
   }
}

void US_Editor::print( void )
{
   QPrinter      printer;
   QPrintDialog* dialog = new QPrintDialog( &printer, this );

   dialog->setWindowTitle( tr( "Print Document" ) );
   
   if ( dialog->exec() != QDialog::Accepted ) return;

   QStringList text = e->toPlainText().split( "\n" );
   QPainter p;

   p.begin( &printer );      // paint on printer
   p.setFont( e->font() );
      
   int yPos = 0;             // y position for each line
   QFontMetrics fm = p.fontMetrics();
   
   const int MARGIN = 10;

   for ( int i = 0; i < text.size(); i++ )
   {
      if ( MARGIN + yPos > printer.height(  ) - MARGIN )
      {
         printer.newPage();   // no more room on this page
         yPos = 0;            // back to top of page
      }
      
      p.drawText( MARGIN, MARGIN + yPos,
                  printer.width(), fm.lineSpacing(),
                  Qt::TextExpandTabs, text[ i ]  );
      
      yPos = yPos + fm.lineSpacing();
    }

    p.end();                  // send job to printer
}
   


#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"

US_Editor::US_Editor( int menu, bool readonly, const QString& extention, 
      QWidget* parent,  Qt::WindowFlags flags ) : QMainWindow( parent, flags )                                                             
{
   file_extention = extention;
   filename = "";
   
   QMenu* fileMenu = menuBar()->addMenu( tr( "&File" ) );
   fileMenu->setFont  ( QFont( US_GuiSettings::fontFamily(),
                               US_GuiSettings::fontSize() - 1 ) );

   QAction* loadAction;
   QAction* saveAction;

   // Add menu types as necessary
   switch ( menu )
   {
      case LOAD:
         loadAction = new QAction( tr( "&Load" ), this );;
         loadAction->setShortcut( tr( "Ctrl+L" ) );
         connect( loadAction, SIGNAL( triggered() ), this, SLOT( load() ) );

         saveAction = new QAction( tr( "&Save" ), this );;
         saveAction->setShortcut( tr( "Ctrl+S" ) );
         connect( saveAction, SIGNAL( triggered() ), this, SLOT( save() ) );

         fileMenu->addAction( loadAction );
         fileMenu->addAction( saveAction );
         // Fall through

      default:
         QAction* saveAsAction = new QAction( tr( "Save&As" ), this );;
         saveAsAction->setShortcut( tr( "Ctrl+A" ) );
         connect( saveAsAction, SIGNAL( triggered() ), this, SLOT( saveAs() ) );

         QAction* clearAction = new QAction( tr( "Clear" ), this );;
         connect( clearAction, SIGNAL( triggered() ), this, SLOT( clear() ) );

         QAction* printAction = new QAction( tr( "&Print" ), this );;
         printAction->setShortcut( tr( "Ctrl+P" ) );
         connect( printAction, SIGNAL( triggered() ), this, SLOT( print() ) );

         QAction* fontAction = new QAction( tr( "&Font" ), this );;
         fontAction->setShortcut( tr( "Ctrl+F" ) );
         connect( fontAction, SIGNAL( triggered() ), this, SLOT( update_font() ) );

         fileMenu->addAction( saveAsAction );
         fileMenu->addAction( printAction );
         fileMenu->addAction( fontAction );
         fileMenu->addAction( clearAction );
   }

   menuBar()->setPalette( US_GuiSettings::normalColor() );
   menuBar()->setFont   ( QFont( US_GuiSettings::fontFamily(),
                          US_GuiSettings::fontSize() ) );

   currentFont = QFont( "Courier", US_GuiSettings::fontSize() - 1, 
         QFont::Bold );
   
   e = new QTextEdit( this );
   e->setFont          ( currentFont );
   e->setPalette       ( US_GuiSettings::editColor() );
   e->setAcceptRichText( true );
   e->setWordWrapMode  ( QTextOption::WrapAtWordBoundaryOrAnywhere );
   e->setReadOnly      ( readonly );
   setCentralWidget    ( e );
}

void US_Editor::load( void )
{
   QString fn;
   QString text;

   fn = QFileDialog::getOpenFileName( this, 
         tr( "Open File" ), 
         US_Settings::dataDir(), 
         file_extention );

  if ( fn == "" ) return;

  filename = fn;

  QFile f( filename );

  if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
     QTextStream t( &f );

     text = t.readAll();
     f.close(  );

     e->setPlainText( text );
     emit US_EditorLoadComplete();
  }
  else
     QMessageBox::information( this,
           tr( "Error" ),
           tr( "Could not open\n\n" ) + fn + tr( "\n\n for reading." ) );
}

void US_Editor::save(  )
{
   if ( filename == "" ) 
      saveAs();
   else
      saveFile();
}

void US_Editor::saveAs(  )
{
   QString fn;

   fn = QFileDialog::getSaveFileName( this );
   
   if ( ! fn.isEmpty(  ) )
   {
      filename = fn;
      saveFile();
   }
}

void US_Editor::saveFile( void )
{
      QString text = e->toPlainText();
      
      QFile f( filename );

      if ( f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         QTextStream t( &f );

         t << text;
         f.close(  );
      }
      else
         QMessageBox::information( this,
            tr( "Error" ),
            tr( "Could not open\n\n" ) + filename + tr( "\n\n for writing." ) );
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
   


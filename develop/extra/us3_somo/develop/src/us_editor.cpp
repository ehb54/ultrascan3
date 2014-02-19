#include "../include/us_editor.h"
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3ValueList>
// #include <QSqlQuery>
#include <QPixmap>
#include <Q3Frame>
#include <q3mimefactory.h>
#include <QResizeEvent>
#include <Q3PopupMenu>
#include <Q3ActionGroup>

US_Editor::US_Editor( int flag, QWidget * parent, const char *name ) : Q3Frame( parent, name )
{
   if ( flag == 0 )
   {
      m = new QMenuBar( this, "menu" );
      Q3PopupMenu *file = new Q3PopupMenu(  );

      Q_CHECK_PTR( file );
      m->insertItem( tr( "&File" ), file );
#ifndef NO_EDITOR_PRINT
      file->insertItem( tr( "Print" ), this, SLOT( print(  ) ), Qt::ALT + Qt::Key_P );
      file->insertSeparator(  );
#endif
      file->insertItem( tr( "Close" ), this, SLOT( closeDoc(  ) ),
                        Qt::ALT + Qt::Key_W );
   }
   else
   {
      m = new QMenuBar( this, "menu" );
      Q3PopupMenu *file = new Q3PopupMenu(  );

      Q_CHECK_PTR( file );
      m->insertItem( tr( "&File" ), file );
      file->insertItem( tr( "New" ), this, SLOT( newDoc(  ) ), Qt::ALT + Qt::Key_N );
      file->insertItem( tr( "Open" ), this, SLOT( load(  ) ), Qt::ALT + Qt::Key_O );
      file->insertItem( tr( "Font" ), this, SLOT( update_font(  ) ),
                        Qt::ALT + Qt::Key_F );
      file->insertItem( tr( "Save" ), this, SLOT( save(  ) ), Qt::ALT + Qt::Key_S );
      file->insertItem( tr( "Save As" ), this, SLOT( saveAs(  ) ),
                        Qt::ALT + Qt::Key_A );
      file->insertSeparator(  );
#ifndef NO_EDITOR_PRINT
      file->insertItem( tr( "Print" ), this, SLOT( print(  ) ), Qt::ALT + Qt::Key_P );
#endif
      file->insertSeparator(  );
      file->insertItem( tr( "Close" ), this, SLOT( closeDoc(  ) ),
                        Qt::ALT + Qt::Key_W );
      file->insertItem( tr( "Quit" ), qApp, SLOT( quit(  ) ), Qt::ALT + Qt::Key_Q );
   }

   e = new Q3TextEdit( this, "editor" );
   // e->setMaxLineLength(1000);
   e->setFocus(  );
   ft.setFamily( "Courier" );
   ft.setPointSize( 11 );
   ft.setBold( true );
   e->setFont( ft );
   e->setTextFormat( Qt::PlainText );
   e->setWordWrap( Q3TextEdit::WidgetWidth );
   if ( flag == 0 )
   {
      e->setReadOnly( TRUE );
   }
   else
   {
      e->setReadOnly( FALSE );
   }
   fileName = "";
}

US_Editor::~US_Editor(  )
{
}

void US_Editor::newDoc(  )
{
   e->clear(  );
   setCaption( "Unnamed Document" );
}

void US_Editor::load(  )
{
   QString fn = Q3FileDialog::getOpenFileName(  );

   fileName = fn;
   if ( !fn.isEmpty(  ) )
      load( fn );
}

void US_Editor::load( const QString & fName )
{
   QFile f( fName );

   e->clear(  );
   if ( f.open( QIODevice::ReadOnly ) )
   {
      QString s;
      Q3TextStream ts( &f );

      s = ts.read(  );
      e->append( s );
      f.close(  );
      e->repaint(  );
      e->setCursorPosition( 0, 0 );
   }
   else
   {
      QMessageBox::message( "Warning:", "Cannot find file: " + fileName );
   }
   setCaption( fName );
}

void US_Editor::saveAs(  )
{
   QString fn;

   fn = Q3FileDialog::getSaveFileName( QString::null, QString::null, this );
   if ( !fn.isEmpty(  ) )
   {
      QString text = e->text(  );
      QFile f( fn );

      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         return;
      }
      Q3TextStream t( &f );

      t << text;
      f.close(  );
      e->setModified( false );
      setCaption( fn );
      fileName = fn;
   }
}

void US_Editor::save(  )
{
   QString fn;

   if ( fileName.isEmpty(  ) )
   {
      saveAs(  );
   }
   else
   {
      QString text = e->text(  );
      QFile f( fileName );

      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         return;
      }
      Q3TextStream t( &f );

      t << text;
      f.close(  );
      e->setModified( false );
      setCaption( fileName );
   }
}

void US_Editor::update_font(  )
{
   bool ok;
   QFont newFont;

   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   e->setFont( ft );
}


void US_Editor::print(  )
{
#ifndef NO_EDITOR_PRINT
   const int MARGIN = 10;

   printer.setPageSize( QPrinter::Letter );

   if ( printer.setup( this ) )
   {                            // opens printer dialog
      QPainter p;

      p.begin( &printer );      // paint on printer
      p.setFont( e->font(  ) );
      int yPos = 0;             // y position for each line
      QFontMetrics fm = p.fontMetrics(  );
      Q3PaintDeviceMetrics metrics( &printer );  // need width/height

      // of printer surface
      for ( int i = 0; i < e->lines(  ); i++ )
      {
         if ( MARGIN + yPos > metrics.height(  ) - MARGIN )
         {
            printer.newPage(  ); // no more room on this page
            yPos = 0;           // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     metrics.width(  ), fm.lineSpacing(  ),
                     ExpandTabs | DontClip, e->text( i ) );
         yPos = yPos + fm.lineSpacing(  );
      }
      p.end(  );                // send job to printer
   }
#endif
}

void US_Editor::closeDoc(  )
{
   close(  );                   // will call closeEvent()
}

void US_Editor::resizeEvent( QResizeEvent * )
{
   if ( e && m )
      e->setGeometry( 0, m->height(  ), width(  ), height(  ) - m->height(  ) );
}

void US_Editor::append( const QString & str )
{
   e->append( str );
}

/****************************************************************************
** $Id: us_editor.cpp,v 1.23 2005/11/15 01:58:44 demeler Exp $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

TextEdit::TextEdit( QWidget * parent, const char *name ):Q3MainWindow( parent,
                                                                      name )
{
   tableID = 0;
   setupFileActions(  );
   setupEditActions(  );
   setupTextActions(  );

   tabWidget = new QTabWidget( this );
   connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ),
            this, SLOT( editorChanged( QWidget * ) ) );
   setCentralWidget( tabWidget );
}
TextEdit::TextEdit( int id, QWidget * parent,
                    const char *name ):Q3MainWindow( parent, name )
{
   tableID = id;
   setupFileActions(  );
   setupEditActions(  );
   setupTextActions(  );

   tabWidget = new QTabWidget( this );
   connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ),
            this, SLOT( editorChanged( QWidget * ) ) );
   setCentralWidget( tabWidget );
}

void TextEdit::setupFileActions(  )
{
   Q3ToolBar *tb = new Q3ToolBar( this );

   tb->setLabel( "File Actions" );
   Q3PopupMenu *menu = new Q3PopupMenu( this );

   menuBar(  )->insertItem( tr( "&File" ), menu );

   QAction *a;

   a = new QAction( qPixmapFromMimeSource( "filenew.xpm" ), tr( "&New..." ),
                    Qt::CTRL + Qt::Key_N, this, "fileNew" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( fileNew(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   a = new QAction( qPixmapFromMimeSource( "fileopen.xpm" ), tr( "&Open..." ),
                    Qt::CTRL + Qt::Key_O, this, "fileOpen" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( fileOpen(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   menu->insertSeparator(  );
   a = new QAction( qPixmapFromMimeSource( "filesave.xpm" ), tr( "&Save..." ),
                    Qt::CTRL + Qt::Key_S, this, "fileSave" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( fileSave(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   a = new QAction( tr( "Save &As..." ), 0, this, "fileSaveAs" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( fileSaveAs(  ) ) );
   a->addTo( menu );
   if ( tableID )
   {
      a = new QAction( tr( "Save &DB..." ), 0, this, "fileSaveDB" );
      connect( a, SIGNAL( activated(  ) ), this, SLOT( fileSaveDB(  ) ) );
      a->addTo( menu );
   }
   menu->insertSeparator(  );
   a = new QAction( qPixmapFromMimeSource( "fileprint.xpm" ),
                    tr( "&Print..." ), Qt::CTRL + Qt::Key_P, this, "filePrint" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( filePrint(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   a = new QAction( tr( "&Close" ), 0, this, "fileClose" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( fileClose(  ) ) );
   a->addTo( menu );
   a = new QAction( tr( "E&xit" ), 0, this, "fileExit" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( fileExit(  ) ) );
   a->addTo( menu );
}

void TextEdit::setupEditActions(  )
{
   Q3ToolBar *tb = new Q3ToolBar( this );

   tb->setLabel( "Edit Actions" );
   Q3PopupMenu *menu = new Q3PopupMenu( this );

   menuBar(  )->insertItem( tr( "&Edit" ), menu );

   QAction *a;

   a = new QAction( qPixmapFromMimeSource( "editundo.xpm" ), tr( "&Undo" ),
                    Qt::CTRL + Qt::Key_Z, this, "editUndo" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( editUndo(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   a = new QAction( qPixmapFromMimeSource( "editredo.xpm" ), tr( "&Redo" ),
                    Qt::CTRL + Qt::Key_Y, this, "editRedo" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( editRedo(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   menu->insertSeparator(  );
   a = new QAction( qPixmapFromMimeSource( "editcopy.xpm" ), tr( "&Copy" ),
                    Qt::CTRL + Qt::Key_C, this, "editCopy" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( editCopy(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   a = new QAction( qPixmapFromMimeSource( "editcut.xpm" ), tr( "Cu&t" ),
                    Qt::CTRL + Qt::Key_X, this, "editCut" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( editCut(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
   a = new QAction( qPixmapFromMimeSource( "editpaste.xpm" ), tr( "&Paste" ),
                    Qt::CTRL + Qt::Key_V, this, "editPaste" );
   connect( a, SIGNAL( activated(  ) ), this, SLOT( editPaste(  ) ) );
   a->addTo( tb );
   a->addTo( menu );
}

void TextEdit::setupTextActions(  )
{
   Q3ToolBar *tb = new Q3ToolBar( this );

   tb->setLabel( "Format Actions" );
   Q3PopupMenu *menu = new Q3PopupMenu( this );

   menuBar(  )->insertItem( tr( "F&ormat" ), menu );

   comboFont = new Q3ComboBox( TRUE, tb );
   QFontDatabase db;

   comboFont->insertStringList( db.families(  ) );
   connect( comboFont, SIGNAL( activated( const QString & ) ),
            this, SLOT( textFamily( const QString & ) ) );
   comboFont->lineEdit(  )->setText( QApplication::font(  ).family(  ) );

   comboSize = new Q3ComboBox( TRUE, tb );
   Q3ValueList < int >sizes = db.standardSizes(  );
   Q3ValueList < int >::Iterator it = sizes.begin(  );

   for ( ; it != sizes.end(  ); ++it )
      comboSize->insertItem( QString::number( *it ) );
   connect( comboSize, SIGNAL( activated( const QString & ) ),
            this, SLOT( textSize( const QString & ) ) );
   comboSize->lineEdit(  )->
      setText( QString::number( QApplication::font(  ).pointSize(  ) ) );

   actionTextBold =
      new QAction( qPixmapFromMimeSource( "textbold.xpm" ), tr( "&Bold" ),
                   Qt::CTRL + Qt::Key_B, this, "textBold" );
   connect( actionTextBold, SIGNAL( activated(  ) ), this,
            SLOT( textBold(  ) ) );
   actionTextBold->addTo( tb );
   actionTextBold->addTo( menu );
   actionTextBold->setToggleAction( TRUE );
   actionTextItalic =
      new QAction( qPixmapFromMimeSource( "textitalic.xpm" ), tr( "&Italic" ),
                   Qt::CTRL + Qt::Key_I, this, "textItalic" );
   connect( actionTextItalic, SIGNAL( activated(  ) ), this,
            SLOT( textItalic(  ) ) );
   actionTextItalic->addTo( tb );
   actionTextItalic->addTo( menu );
   actionTextItalic->setToggleAction( TRUE );
   actionTextUnderline =
      new QAction( qPixmapFromMimeSource( "textunder.xpm" ),
                   tr( "&Underline" ), Qt::CTRL + Qt::Key_U, this, "textUnderline" );
   connect( actionTextUnderline, SIGNAL( activated(  ) ), this,
            SLOT( textUnderline(  ) ) );
   actionTextUnderline->addTo( tb );
   actionTextUnderline->addTo( menu );
   actionTextUnderline->setToggleAction( TRUE );
   menu->insertSeparator(  );

   Q3ActionGroup *grp = new Q3ActionGroup( this );

   connect( grp, SIGNAL( selected( QAction * ) ), this,
            SLOT( textAlign( QAction * ) ) );

   actionAlignLeft =
      new QAction( qPixmapFromMimeSource( "textleft.xpm" ), tr( "&Left" ),
                   Qt::CTRL + Qt::Key_L, grp, "textLeft" );
   actionAlignLeft->setToggleAction( TRUE );
   actionAlignCenter =
      new QAction( qPixmapFromMimeSource( "textcenter.xpm" ), tr( "C&enter" ),
                   Qt::CTRL + Qt::Key_E, grp, "textCenter" );
   actionAlignCenter->setToggleAction( TRUE );
   actionAlignRight =
      new QAction( qPixmapFromMimeSource( "textright.xpm" ), tr( "&Right" ),
                   Qt::CTRL + Qt::Key_R, grp, "textRight" );
   actionAlignRight->setToggleAction( TRUE );
   actionAlignJustify =
      new QAction( qPixmapFromMimeSource( "textjustify.xpm" ),
                   tr( "&Justify" ), Qt::CTRL + Qt::Key_J, grp, "textjustify" );
   actionAlignJustify->setToggleAction( TRUE );

   grp->addTo( tb );
   grp->addTo( menu );

   menu->insertSeparator(  );

   QPixmap pix( 16, 16 );

   pix.fill( Qt::black );
   actionTextColor =
      new QAction( pix, tr( "&Color..." ), 0, this, "textColor" );
   connect( actionTextColor, SIGNAL( activated(  ) ), this,
            SLOT( textColor(  ) ) );
   actionTextColor->addTo( tb );
   actionTextColor->addTo( menu );
}

void TextEdit::load( const QString & f, QString title, bool ourfmt, Qt::TextFormat fmt )
{
   this->title = title;
   this->fmt = fmt;

   if ( !QFile::exists( f ) )
   {
      QMessageBox::message( "Warning:", "Cannot find file: " + f );
      return;
   }
   Q3TextEdit *edit = new Q3TextEdit( tabWidget );

   // edit->setTextFormat( RichText );
   edit->setWordWrap( Q3TextEdit::WidgetWidth );
   doConnections( edit );
   tabWidget->addTab( edit, QFileInfo( f ).fileName(  ) );
   QFile file( f );

   if ( !file.open( QIODevice::ReadOnly ) )
      return;
   Q3TextStream ts( &file );
   QString txt = ts.read(  );

   if ( ourfmt ) 
   {
      edit->setTextFormat( fmt );
   } else {
      if ( !Q3StyleSheet::mightBeRichText( txt ) )
      {
         txt =
            Q3StyleSheet::convertFromPlainText( txt,
                                               Q3StyleSheetItem::WhiteSpacePre );
      }
   }
   edit->setText( txt );
   tabWidget->showPage( edit );
   edit->viewport(  )->setFocus(  );
   filenames.replace( edit, f );
}

void TextEdit::load_text( QString text, QString title )
{
   Q3TextEdit *edit = new Q3TextEdit( tabWidget );
   if ( !title.isEmpty() )
   {
      this->title = title;
   }
      
   // edit->setTextFormat( RichText );
   edit->setWordWrap( Q3TextEdit::WidgetWidth );
   tabWidget->addTab( edit, "Notes:" );
   doConnections( edit );
   edit->setText( text );
   tabWidget->showPage( edit );
   edit->viewport(  )->setFocus(  );
   filenames.replace( edit, text );
}

Q3TextEdit *TextEdit::currentEditor(  ) const
{
   if ( tabWidget->currentPage(  ) &&
        tabWidget->currentPage(  )->inherits( "QTextEdit" ) )
   {
      return ( Q3TextEdit * ) tabWidget->currentPage(  );
   }
   return 0;
}

void TextEdit::doConnections( Q3TextEdit * e )
{
   connect( e, SIGNAL( currentFontChanged( const QFont & ) ),
            this, SLOT( fontChanged( const QFont & ) ) );
   connect( e, SIGNAL( currentColorChanged( const QColor & ) ),
            this, SLOT( colorChanged( const QColor & ) ) );
   connect( e, SIGNAL( currentAlignmentChanged( int ) ),
            this, SLOT( alignmentChanged( int ) ) );
}

void TextEdit::fileNew(  )
{
   Q3TextEdit *edit = new Q3TextEdit( tabWidget );

   //   edit->setTextFormat( RichText );
   edit->setTextFormat( fmt );
   doConnections( edit );
   tabWidget->addTab( edit, tr( "noname" ) );
   tabWidget->showPage( edit );
   edit->viewport(  )->setFocus(  );
}

void TextEdit::fileOpen(  )
{
   QString fn =
      Q3FileDialog::getOpenFileName( QString::null, tr( "All Files (*)" ),
                                    this );
   if ( !fn.isEmpty(  ) )
      load( fn );
}

void TextEdit::fileSave(  )
{
   if ( !currentEditor(  ) )
      return;
   QString fn;

   if ( filenames.find( currentEditor(  ) ) == filenames.end(  ) )
   {
      fileSaveAs(  );
   }
   else
   {
      QFile file( *filenames.find( currentEditor(  ) ) );

      if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
         return;
      Q3TextStream ts( &file );

      ts << currentEditor(  )->text(  );
   }
}

void TextEdit::fileSaveAs(  )
{
   if ( !currentEditor(  ) )
      return;
   QString fn =
      Q3FileDialog::getSaveFileName( QString::null, tr( "All Files (*)" ),
                                    this );
   if ( !fn.isEmpty(  ) )
   {
      filenames.replace( currentEditor(  ), fn );
      fileSave(  );
      tabWidget->setTabLabel( currentEditor(  ), QFileInfo( fn ).fileName(  ) );
   }
}

void TextEdit::fileSaveDB(  )
{
#ifndef NO_DB
   QString str, notes;

   if ( !currentEditor(  ) )
      return;
   notes = currentEditor(  )->text(  );
   notes = notes.replace( "\'", "\\\'" );
   currentDB = new US_DB(  );
   currentDB->db_connect(  );
//    QSqlQuery query;

   str =
      "UPDATE tblRequest SET Notes = '" + notes + "' WHERE RequestID =" +
      QString::number( tableID );
   bool flag = query.exec( str );

   if ( !flag )
   {
      QMessageBox::message( tr( "Attention:" ),
                            tr
                            ( "Sorry, saving the Notes to the database failed.\n" ) );
      return;
   }
   else
   {
      QMessageBox::message( tr( "Yes!" ),
                            tr
                            ( "The Notes were successfully saved to the Database.\n" ) );
      return;
   }
#endif
}


void TextEdit::filePrint(  )
{
   if ( !currentEditor(  ) )
      return;
#ifndef QT_NO_PRINTER
   QPrinter printer( QPrinter::HighResolution );

   printer.setFullPage( TRUE );
   if ( printer.setup( this ) )
   {
      QPainter p( &printer );

      // Check that there is a valid device to print to.
      if ( !p.device(  ) )
         return;
      Q3PaintDeviceMetrics metrics( p.device(  ) );
      int dpiy = metrics.logicalDpiY(  );
      int margin = ( int ) ( ( 2 / 2.54 ) * dpiy );   // 2 cm margins
      QRect body( margin, margin, metrics.width(  ) - 2 * margin,
                  metrics.height(  ) - 2 * margin );
      QFont font( currentEditor(  )->QWidget::font(  ) );

      font.setPointSize( 10 );  // we define 10pt to be a nice base size for
                                // printing

      Q3SimpleRichText richText( currentEditor(  )->text(  ), font,
                                currentEditor(  )->context(  ),
                                currentEditor(  )->styleSheet(  ),
                                currentEditor(  )->mimeSourceFactory(  ),
                                body.height(  ) );
      richText.setWidth( &p, body.width(  ) );
      QRect view( body );
      int page = 1;

      do
      {
         richText.draw( &p, body.left(  ), body.top(  ), view, colorGroup(  ) );
         view.moveBy( 0, body.height(  ) );
         p.translate( 0, -body.height(  ) );
         p.setFont( font );
         p.drawText( view.right(  ) -
                     p.fontMetrics(  ).width( QString::number( page ) ),
                     view.bottom(  ) + p.fontMetrics(  ).ascent(  ) + 5,
                     QString::number( page ) );
         if ( view.top(  ) >= richText.height(  ) )
            break;
         printer.newPage(  );
         page++;
      }
      while ( TRUE );
   }
#endif
}

void TextEdit::fileClose(  )
{
   delete currentEditor(  );

   if ( currentEditor(  ) )
      currentEditor(  )->viewport(  )->setFocus(  );
}

void TextEdit::fileExit(  )
{
   qApp->quit(  );
}

void TextEdit::editUndo(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->undo(  );
}

void TextEdit::editRedo(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->redo(  );
}

void TextEdit::editCut(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->cut(  );
}

void TextEdit::editCopy(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->copy(  );
}

void TextEdit::editPaste(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->paste(  );
}

void TextEdit::textBold(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->setBold( actionTextBold->isOn(  ) );
}

void TextEdit::textUnderline(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->setUnderline( actionTextUnderline->isOn(  ) );
}

void TextEdit::textItalic(  )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->setItalic( actionTextItalic->isOn(  ) );
}

void TextEdit::textFamily( const QString & f )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->setFamily( f );
   currentEditor(  )->viewport(  )->setFocus(  );
}

void TextEdit::textSize( const QString & p )
{
   if ( !currentEditor(  ) )
      return;
   currentEditor(  )->setPointSize( p.toInt(  ) );
   currentEditor(  )->viewport(  )->setFocus(  );
}

void TextEdit::textStyle( int i )
{
   if ( !currentEditor(  ) )
      return;
   if ( i == 0 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayBlock,
                                       Q3StyleSheetItem::ListDisc );
   else if ( i == 1 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayListItem,
                                       Q3StyleSheetItem::ListDisc );
   else if ( i == 2 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayListItem,
                                       Q3StyleSheetItem::ListCircle );
   else if ( i == 3 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayListItem,
                                       Q3StyleSheetItem::ListSquare );
   else if ( i == 4 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayListItem,
                                       Q3StyleSheetItem::ListDecimal );
   else if ( i == 5 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayListItem,
                                       Q3StyleSheetItem::ListLowerAlpha );
   else if ( i == 6 )
      currentEditor(  )->setParagType( Q3StyleSheetItem::DisplayListItem,
                                       Q3StyleSheetItem::ListUpperAlpha );
   currentEditor(  )->viewport(  )->setFocus(  );
}

void TextEdit::textColor(  )
{
   if ( !currentEditor(  ) )
      return;
   QColor col = QColorDialog::getColor( currentEditor(  )->color(  ), this );

   if ( !col.isValid(  ) )
      return;
   currentEditor(  )->setColor( col );
   QPixmap pix( 16, 16 );

   pix.fill( Qt::black );
   actionTextColor->setIconSet( pix );
}

void TextEdit::textAlign( QAction * a )
{
   if ( !currentEditor(  ) )
      return;
   if ( a == actionAlignLeft )
      currentEditor(  )->setAlignment( Qt::AlignLeft );
   else if ( a == actionAlignCenter )
      currentEditor(  )->setAlignment( Qt::AlignHCenter );
   else if ( a == actionAlignRight )
      currentEditor(  )->setAlignment( Qt::AlignRight );
   else if ( a == actionAlignJustify )
      currentEditor(  )->setAlignment( Qt::AlignJustify );
}

void TextEdit::fontChanged( const QFont & f )
{
   comboFont->lineEdit(  )->setText( f.family(  ) );
   comboSize->lineEdit(  )->setText( QString::number( f.pointSize(  ) ) );
   actionTextBold->setOn( f.bold(  ) );
   actionTextItalic->setOn( f.italic(  ) );
   actionTextUnderline->setOn( f.underline(  ) );
}

void TextEdit::colorChanged( const QColor & c )
{
   QPixmap pix( 16, 16 );

   pix.fill( c );
   actionTextColor->setIconSet( pix );
}

void TextEdit::alignmentChanged( int a )
{
   if ( ( a == Qt::AlignLeft ) || ( a & Qt::AlignLeft ) )
      actionAlignLeft->setOn( TRUE );
   else if ( ( a & Qt::AlignHCenter ) )
      actionAlignCenter->setOn( TRUE );
   else if ( ( a & Qt::AlignRight ) )
      actionAlignRight->setOn( TRUE );
   else if ( ( a & Qt::AlignJustify ) )
      actionAlignJustify->setOn( TRUE );
}

void TextEdit::editorChanged( QWidget * )
{
   if ( !currentEditor(  ) )
      return;
   fontChanged( currentEditor(  )->font(  ) );
   colorChanged( currentEditor(  )->color(  ) );
   alignmentChanged( currentEditor(  )->alignment(  ) );
}

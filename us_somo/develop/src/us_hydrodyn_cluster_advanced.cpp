#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_advanced.h"
//Added by qt3to4:
#include <QTextStream>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
 //#include <Q3PopupMenu>
#include <QVBoxLayout>

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Advanced::US_Hydrodyn_Cluster_Advanced(
                                               csv &csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *
                                               ) : QDialog( p )
{
   this->csv1 = csv1;
   this->original_csv1 = &csv1;
   if ( !csv1.data.size() )
   {
      reset_csv();
   }

   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: Cluster Advanced Options"));

   QDir::setCurrent( USglobal->config_list.root_dir + SLASH +  "somo" + SLASH + "cluster" );

   setupGUI();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 30;
   unsigned int csv_width = t_csv->columnWidth(0) + 45;
   for ( int i = 0; i < t_csv->rowCount(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->columnCount(); i++ )
   {
      csv_width += t_csv->columnWidth(i);
   }
   if ( csv_height > 800 )
   {
      csv_height = 800;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 400 + csv_height );
}

US_Hydrodyn_Cluster_Advanced::~US_Hydrodyn_Cluster_Advanced()
{
}

void US_Hydrodyn_Cluster_Advanced::setupGUI()
{
   int minHeight1 = 30;
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   int minHeight3 = 30;
#endif

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTableWidget(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);
   t_csv->setSelectionMode( QTableWidget::NoSelection );

   interval_starting_row = 0;
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      if ( csv1.data[ i ].size() < 3 || csv1.data[ i ][ 2 ].isEmpty() )
      {
         interval_starting_row = i + 1;
      }
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setCellWidget( i, j, new QCheckBox() );
            ((QCheckBox *)(t_csv->cellWidget( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setItem( i, j, new QTableWidgetItem( csv1.data[i][j] ) );
         }
      }
      if ( csv1.data[ i ].size() < 3 ||
           ( csv1.data[ i ][ 1 ].isEmpty() &&
             csv1.data[ i ][ 2 ].isEmpty() ) )
      {
         // #warning crash here, tried i,j but that also crashed, needs more debugging
         { for ( int i = 0; i < t_csv->columnCount(); ++i ) { t_csv->item( i, i )->setFlags( t_csv->item( i, i )->flags() ^ Qt::ItemIsEditable ); } };
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->setHorizontalHeaderItem(i, new QTableWidgetItem( csv1.header[i]));
   }
   t_csv->setSortingEnabled(false);
    t_csv->verticalHeader()->setSectionsMovable(false);
    t_csv->horizontalHeader()->setSectionsMovable(false);
   //  t_csv->setReadOnly(false);

   t_csv->setColumnWidth( 0, 350 );
   { for ( int i = 0; i < t_csv->rowCount(); ++i ) { t_csv->item( i,  0 )->setFlags( t_csv->item( i,  0 )->flags() ^ Qt::ItemIsEditable ); } };

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   // t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if defined(Q_OS_MAC)
   m = new QMenuBar( this );
   m->setObjectName( "menu" );
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );
   m = new QMenuBar( frame );    m->setObjectName( "menu" );
# endif
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );

   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
#endif


   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight( 50 );
   
   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton( us_tr("Close"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget( frame );
#endif
   vbl_editor_group->addWidget( editor );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_ok );
   hbl_bottom->addSpacing( 4 );


   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( t_csv );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Cluster_Advanced::ok()
{
   *original_csv1 = current_csv();
   close();
}

void US_Hydrodyn_Cluster_Advanced::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Advanced::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_cluster_advanced_options.html");
}

void US_Hydrodyn_Cluster_Advanced::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster_Advanced::table_value( int row, int col )
{
   if ( (unsigned int) row < interval_starting_row && col > 1 )
   {
      t_csv->setItem( row, col, new QTableWidgetItem( "" ) );
      return;
   }

   if ( col == 4 || col == 2 || col == 3 )
   {
      recompute_interval_from_points();
   }

   if ( col == 5 )
   {
      recompute_points_from_interval();
   }
}

void US_Hydrodyn_Cluster_Advanced::clear_display()
{
   editor->clear( );
   editor->append("\n\n");
}

void US_Hydrodyn_Cluster_Advanced::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn_Cluster_Advanced::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString() , QString() );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

csv US_Hydrodyn_Cluster_Advanced::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            tmp_csv.data[i][j] = ((QCheckBox *)(t_csv->cellWidget( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->item( i, j )->text();
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Cluster_Advanced::recompute_interval_from_points()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      QString toset =
                      t_csv->item(i, 4)->text().toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( ( t_csv->item(i, 3)->text().toDouble() -
                              t_csv->item(i, 2)->text().toDouble() )
                             / ( t_csv->item(i, 4)->text().toDouble() - 1e0 ) ) 
         ;
      
      t_csv->setItem( i, 5, new QTableWidgetItem( toset ) );
   }
}

void US_Hydrodyn_Cluster_Advanced::recompute_points_from_interval()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->rowCount(); i++ )
   {
      QString toset =
                      t_csv->item(i, 5)->text().toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( 1 + (unsigned int)(( t_csv->item(i, 3)->text().toDouble() -
                                                 t_csv->item(i, 2)->text().toDouble() )
                                               / t_csv->item(i, 5)->text().toDouble() + 0.5) )
         ;
      t_csv->setItem( i, 4, new QTableWidgetItem( toset ) );
   }
}

void US_Hydrodyn_Cluster_Advanced::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
}

void US_Hydrodyn_Cluster_Advanced::reset_csv()
{
   csv1.name = "Advanced cluster options";

   csv1.header.clear( );
   csv1.header_map.clear( );
   csv1.data.clear( );
   csv1.num_data.clear( );
   csv1.prepended_names.clear( );

   csv1.header.push_back("Parameter");
   csv1.header.push_back("Active");
   csv1.header.push_back("Low value");
   csv1.header.push_back("High value");
   csv1.header.push_back("Points");
   csv1.header.push_back("Interval");

   vector < QString > tmp_data;

   tmp_data.push_back( us_tr( "--- Multiple I(q) methods are selectable ---" ) );
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) Full Debye");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) Hybrid");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) Hybrid2");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) Hybrid3");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) Fast");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) FoXS");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("I(q) Crysol");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back( us_tr( "--- Parameter sweeps ---" ) );
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Buffer electron density");
   tmp_data.push_back("N");
   tmp_data.push_back("0.1");
   tmp_data.push_back("0.9");
   tmp_data.push_back("11");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Scaling excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back(".95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("11");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("WAT excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back("15");
   tmp_data.push_back("30");
   tmp_data.push_back("11");
   
   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Crysol: average atomic radius");
   tmp_data.push_back("N");
   tmp_data.push_back("1.5");
   tmp_data.push_back("1.7");
   tmp_data.push_back("11");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Crysol: Excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back("10000");
   tmp_data.push_back("20000");
   tmp_data.push_back("11");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear( );
   tmp_data.push_back("Crysol: contrast of hydration shell");
   tmp_data.push_back("N");
   tmp_data.push_back("0.01");
   tmp_data.push_back("0.05");
   tmp_data.push_back("11");
   
   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         tmp_num_data.push_back(csv1.data[i][j].toDouble());
      }
      csv1.num_data.push_back(tmp_num_data);
   }
}

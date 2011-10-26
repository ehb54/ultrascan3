#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_advanced.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Advanced::US_Hydrodyn_Cluster_Advanced(
                                               csv &csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name
                                               ) : QDialog(p, name)
{
   this->csv1 = csv1;
   this->original_csv1 = &csv1;
   if ( !csv1.data.size() )
   {
      reset_csv();
   }

   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: Cluster Advanced Options"));

   setupGUI();

   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 30;
   unsigned int csv_width = t_csv->columnWidth(0) + 45;
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->numCols(); i++ )
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
   int minHeight3 = 30;

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTable(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);
   t_csv->setSelectionMode( QTable::NoSelection );

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
            t_csv->setItem( i, j, new QCheckTableItem( t_csv, "" ) );
            ((QCheckTableItem *)(t_csv->item( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setText( i, j, csv1.data[i][j] );
         }
      }
      if ( csv1.data[ i ].size() < 3 ||
           ( csv1.data[ i ][ 1 ].isEmpty() &&
             csv1.data[ i ][ 2 ].isEmpty() ) )
      {
         t_csv->setRowReadOnly( i, true );
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv1.header[i]);
   }
   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(false);
   t_csv->setColumnMovingEnabled(false);
   t_csv->setReadOnly(false);

   t_csv->setColumnWidth( 0, 350 );
   t_csv->setColumnReadOnly( 0, true );

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight( 50 );
   
   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton( tr("Close"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget( frame );
   vbl_editor_group->addWidget( editor );

   QHBoxLayout *hbl_bottom = new QHBoxLayout(0);
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_ok );
   hbl_bottom->addSpacing( 4 );


   QVBoxLayout *background = new QVBoxLayout(this);
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
   online_help->show_help("manual/somo_cluster_advanced.html");
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
      t_csv->setText( row, col, "" );
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
   editor->clear();
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
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
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
            tmp_csv.data[i][j] = ((QCheckTableItem *)(t_csv->item( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->text( i, j );
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Cluster_Advanced::recompute_interval_from_points()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, 5, 
                     ( 
                      t_csv->text(i, 4).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( ( t_csv->text(i, 3).toDouble() -
                              t_csv->text(i, 2).toDouble() )
                             / ( t_csv->text(i, 4).toDouble() - 1e0 ) ) 
                      )
                     );
   }
}

void US_Hydrodyn_Cluster_Advanced::recompute_points_from_interval()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, 4, 
                     ( 
                      t_csv->text(i, 5).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( 1 + (unsigned int)(( t_csv->text(i, 3).toDouble() -
                                                 t_csv->text(i, 2).toDouble() )
                                               / t_csv->text(i, 5).toDouble() + 0.5) ) 
                      )
                     );
   }
}

void US_Hydrodyn_Cluster_Advanced::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
}

void US_Hydrodyn_Cluster_Advanced::reset_csv()
{
   csv1.name = "Advanced cluster options";

   csv1.header_map.clear();
   csv1.data.clear();
   csv1.num_data.clear();
   csv1.prepended_names.clear();

   csv1.header.push_back("Parameter");
   csv1.header.push_back("Active");
   csv1.header.push_back("Low value");
   csv1.header.push_back("High value");
   csv1.header.push_back("Points");
   csv1.header.push_back("Interval");

   vector < QString > tmp_data;

   tmp_data.push_back( tr( "--- Multiple I(q) methods are selectable ---" ) );
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Full Debye");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Hybrid");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Hybrid2");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Hybrid3");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Fast");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) FoXS");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Crysol");
   tmp_data.push_back("N");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back( tr( "--- Parameter sweeps ---" ) );
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");
   tmp_data.push_back("");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("Buffer electron density");
   tmp_data.push_back("N");
   tmp_data.push_back("0.1");
   tmp_data.push_back("0.9");
   tmp_data.push_back("11");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("Scaling excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back(".95");
   tmp_data.push_back("1.05");
   tmp_data.push_back("11");

   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("SWH excluded volume");
   tmp_data.push_back("N");
   tmp_data.push_back("15");
   tmp_data.push_back("30");
   tmp_data.push_back("11");
   
   csv1.prepended_names.push_back(tmp_data[0]);
   csv1.data.push_back(tmp_data);

   tmp_data.clear();
   tmp_data.push_back("I(q) Crysol: contrast of hydration shell");
   tmp_data.push_back("N");
   tmp_data.push_back("0.0");
   tmp_data.push_back("0.5");
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

#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_hydrodyn_cluster_results.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster_Results::US_Hydrodyn_Cluster_Results(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   setCaption(tr("US-SOMO: Cluster Results"));
   USglobal = new US_Config();

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   completed_dir = pkg_dir + SLASH + "completed";
   tmp_dir = pkg_dir + SLASH + "tmp";
   results_dir = pkg_dir + SLASH + "results";

   QDir::setCurrent( completed_dir );

   files.clear();

   // traverse directory and build up files

   if ( !update_files( false ) )
   {
      QMessageBox::information( this, 
                                tr("US-SOMO: Cluster Results"),
                                tr("No results found"),
                                0 );
   }

   setupGUI();
   editor_msg("blue", "THIS WINDOW IS UNDER DEVELOPMENT." );

   {
      QDir dir1( tmp_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( tmp_dir ) );
         dir1.mkdir( tmp_dir );
      }
   }

   {
      QDir dir1( results_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( results_dir ) );
         dir1.mkdir( results_dir );
      }
   }

   update_files();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 700, 600 );
}

US_Hydrodyn_Cluster_Results::~US_Hydrodyn_Cluster_Results()
{
}

void US_Hydrodyn_Cluster_Results::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "Manage results from cluster jobs" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_files = new QLabel(tr("Available results:"), this);
   lbl_files->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_files->setMinimumHeight(minHeight1);
   lbl_files->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lv_files = new QListView(this);
   lv_files->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lv_files->setMinimumHeight(minHeight1 * 3);
   lv_files->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lv_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lv_files->setEnabled(true);
   lv_files->setSelectionMode( QListView::Multi );

   lv_files->addColumn( tr( "Name" ) );
   lv_files->addColumn( tr( "Date created" ) );
   lv_files->addColumn( tr( "Size" ) );
   connect( lv_files, SIGNAL( selectionChanged() ), SLOT( update_enables() ) );

   pb_select_all = new QPushButton(tr("Select all"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_purge = new QPushButton(tr("Purge results"), this);
   pb_purge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_purge->setMinimumHeight(minHeight1);
   pb_purge->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_purge, SIGNAL(clicked()), SLOT(purge()));

   pb_load_results = new QPushButton(tr("Extract results"), this);
   pb_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_results->setMinimumHeight(minHeight1);
   pb_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_load_results, SIGNAL( clicked() ), SLOT( load_results() ) );
   
   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight(100);

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout

   QHBoxLayout *hbl_buttons1 = new QHBoxLayout( 0 );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_select_all);
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_purge );
   hbl_buttons1->addSpacing( 4 );
   hbl_buttons1->addWidget ( pb_load_results );
   hbl_buttons1->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing( 4 );
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addWidget ( lbl_files );
   background->addSpacing( 4 );
   background->addWidget ( lv_files );
   background->addSpacing( 4 );
   background->addLayout ( hbl_buttons1 );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4 );
}

void US_Hydrodyn_Cluster_Results::cancel()
{
   close();
}

void US_Hydrodyn_Cluster_Results::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster_results.html");
}

void US_Hydrodyn_Cluster_Results::closeEvent(QCloseEvent *e)
{
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}


void US_Hydrodyn_Cluster_Results::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Cluster_Results::update_font()
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

void US_Hydrodyn_Cluster_Results::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null, this );
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

void US_Hydrodyn_Cluster_Results::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
}
   
void US_Hydrodyn_Cluster_Results::update_enables()
{
   if ( !disable_updates )
   {
      bool any_selected = false;
      QListViewItem *lvi = lv_files->firstChild();
      if ( lvi )
      {
         do {
            if ( lvi->isSelected() )
            {
               any_selected = true;
            }

         } while ( ( lvi = lvi->nextSibling() ) );
      }
      pb_purge->setEnabled( any_selected );
      pb_load_results->setEnabled( any_selected );
   } else {
      pb_purge->setEnabled( false );
   }
}

void US_Hydrodyn_Cluster_Results::purge()
{
   bool any_selected = false;
   QListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            any_selected = true;
         }
         
      } while ( ( lvi = lvi->nextSibling() ) );
   }

   if ( any_selected )
   {
      if ( QMessageBox::question(
                                 this,
                                 tr( "US-SOMO: Cluster Results" ),
                                 tr( "Do you want purge these job results?" ),
                                 tr( "&Yes" ),
                                 tr( "&No" ),
                                 QString::null, 0, 1 | QMessageBox::Default ) )
      {
         return;
      }

      // purge
      QDir::setCurrent( completed_dir );

      lv_files->setEnabled( false );

      QListViewItem *lvi = lv_files->firstChild();
      if ( lvi )
      {
         do {
            // purge
            if ( lvi->isSelected() )
            {
               if ( !QFile::remove( lvi->text( 0 ) ) )
               {
                  editor_msg( "red", QString( tr( "Error: can not remove %1" ) ).arg( lvi->text( 0 ) ) );
               } else {
                  editor_msg( "black", QString( tr( "Removed %1" ) ).arg( lvi->text( 0 ) ) );
               }
            }
         } while ( ( lvi = lvi->nextSibling() ) );
      }

      editor_msg( "black", tr( "purge complete" ) );
      if ( !update_files() )
      {
         QMessageBox::information( this, 
                                   tr("US-SOMO: Cluster Results"),
                                   tr("No further job results found"),
                                   0 );
         close();
         return;
      }

      update_enables();

      lv_files->setEnabled( true );
   }   
}

void US_Hydrodyn_Cluster_Results::select_all()
{
   bool any_not_selected = false;
   QListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( !lvi->isSelected() )
         {
            any_not_selected = true;
         }
      } while ( ( lvi = lvi->nextSibling() ) );
   }

   disable_updates = true;
   lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         lv_files->setSelected( lvi, any_not_selected );
      } while ( ( lvi = lvi->nextSibling() ) );
   }
   disable_updates = false;
   update_enables();
}

bool US_Hydrodyn_Cluster_Results::clean_dir( QString dir ) 
{
   errormsg = "";
   if ( !QDir::setCurrent( dir ) )
   {
      errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( dir );
      return false;
   }

   // clean up tmp_dir first
   QDir qd;
   QStringList tmp_files = qd.entryList( "*" );
   for ( unsigned int i = 0; i < tmp_files.size() ; i++ )
   {
      if ( tmp_files[ i ] != "." &&
           tmp_files[ i ] != ".." )
      {
         if ( QFileInfo( tmp_files[ i ] ).isDir() )
         {
            if ( !clean_dir( tmp_files[ i ] ) )
            {
               return false;
            }
            if ( !QDir::setCurrent( dir ) )
            {
               errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( dir );
               return false;
            }
            QDir qd;
            if ( !qd.rmdir( tmp_files[ i ] ) )
            {
               errormsg = QString( tr( "Error: can not remove directory %1 in %2" ) ).arg( tmp_files[ i ] ).arg( tmp_dir );
               return false;
            }
         } else {
            if ( !QFile::remove( tmp_files[ i ] ) ) 
            {
               errormsg = QString( tr( "Error: can not remove file %1 in %2" ) ).arg( tmp_files[ i ] ).arg( tmp_dir );
               return false;
            }
         }
      }
   }

   return true;
}
   
void US_Hydrodyn_Cluster_Results::load_results()
{
   bool any_selected = false;
   QListViewItem *lvi = lv_files->firstChild();
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            any_selected = true;
         }
         
      } while ( ( lvi = lvi->nextSibling() ) );
   }

   if ( !any_selected )
   {
      return;
   }

   lvi = lv_files->firstChild();
   lv_files->setEnabled( false );
   if ( lvi )
   {
      do {
         if ( lvi->isSelected() )
         {
            if ( !load_one_result( lvi->text( 0 ) ) )
            {
               editor_msg( "red", errormsg );
               lv_files->setEnabled( true );
               return;
            }
         }
      } while ( ( lvi = lvi->nextSibling() ) );
   }

   purge();

   lv_files->setEnabled( true );
}

bool US_Hydrodyn_Cluster_Results::load_one_result( QString file )
{
   QString org_file = file;

   errormsg = "";
   if ( !clean_dir( tmp_dir ) )
   {
      return false;
   }

   // copy file to tmp

   if ( !QDir::setCurrent( completed_dir ) )
   {
      errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( completed_dir );
      return false;
   }

   if ( !QFile::exists( file ) )
   {
      errormsg = QString( tr( "Error: file %1 does not exist in %2" ) ).arg( file ).arg( completed_dir );
      return false;
   }
      
   US_File_Util ufu;
   if ( !ufu.copy( file, tmp_dir + SLASH ) )
   {
      errormsg = ufu.errormsg;
      return false;
   }

   if ( !QDir::setCurrent( tmp_dir ) )
   {
      errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( tmp_dir );
      return false;
   }

   // ungzip if needed
   if ( file.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
   {
      QString dest = file;
      dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
      QDir qd;
      qd.remove( dest );
      if ( !qd.rename( file, dest ) )
      {
         errormsg = QString( tr( "Error: renaming %1 to %2 " ) ).arg( file ).arg( dest );
         return false;
      }

      file = dest;

      US_Gzip usg;
      int result = usg.gunzip( file );
      if ( GZIP_OK != result )
      {
         errormsg = QString( tr( "Error: %1 problem ungzipping (%2)" ) ).arg( file ).arg( usg.explain( result ) );
         return false;
      }
      file = usg.last_written_name;
   }

   // file should be .tar now

   if ( !file.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
   {
      errormsg = QString( tr( "Error: file %1 is not .tar" ) ).arg( file );
      return false;
   }

   // untar

   US_Tar ust;

   QStringList tar_list;

   int result = ust.list( file, tar_list, true );
   if ( TAR_OK != result )
   {
      errormsg = QString( tr( "Error: %1 problem list tar archive (%2)" ) ) .arg( file ).arg( ust.explain( result ) );
      return false;
   }

   QStringList mkdirs;
   for ( unsigned int i = 0; i < tar_list.size(); i++ )
   {
      if ( QFileInfo( tar_list[ i ] ).dirPath().length() )
      {
         QString dirPath = QFileInfo( tar_list[ i ] ).dirPath();
         if ( dirPath != "." )
         {
            QDir qd;
            qd.mkdir( tmp_dir + SLASH + dirPath );
         }
      }
   }

   result = ust.extract( file );
   if ( TAR_OK != result )
   {
      errormsg = QString( tr( "Error: %1 problem extracting tar archive (%2)" ) ).arg( file ).arg( ust.explain( result ) );
      return false;
   }

   // remove tar file

   if ( !QFile::remove( file ) ) 
   {
      errormsg = QString( tr( "Error: can not remove file %1 in %2" ) ).arg( file ).arg( tmp_dir );
      return false;
   }
   
   // ok, now we just have the files and they should be in tar_list

   if ( !tar_list.size() )
   {
      errormsg = QString( tr( "Error: job results are empty for %1 " ) ).arg( org_file );
      return false;
   }

   QStringList final_results;
   QStringList further_extraction;

   for ( unsigned int i = 0; i < tar_list.size(); i++ )
   {
      if ( tar_list[ i ].contains( QRegExp( "\\.(tgz|TGZ|tar|TAR)$" ) ) )
      {
         // further extraction needed
         further_extraction << tar_list[ i ];
      } else {
         final_results << tar_list[ i ];
      }
   }

   if ( further_extraction.size() )
   {
      for ( unsigned int i = 0; i < further_extraction.size(); i++ )
      {
         QString file = QFileInfo( further_extraction[ i ] ).fileName();
         QString subDirPath = QFileInfo( further_extraction[ i ] ).dirPath();
         QString dirPath = tmp_dir + SLASH + subDirPath;
         if ( !QDir::setCurrent( dirPath ) )
         {
            errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( dirPath );
            return false;
         }

         // ungzip if needed
         if ( file.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
         {
            QString dest = file;
            dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
            QDir qd;
            qd.remove( dest );
            if ( !qd.rename( file, dest ) )
            {
               errormsg = QString( tr( "Error: renaming %1 to %2 " ) ).arg( file ).arg( dest );
               return false;
            }
            
            file = dest;

            US_Gzip usg;
            int result = usg.gunzip( file );
            if ( GZIP_OK != result )
            {
               errormsg = QString( tr( "Error: %1 problem ungzipping (%2)" ) ).arg( file ).arg( usg.explain( result ) );
               return false;
            }
            file = usg.last_written_name;
            // this is a problem with gunzip, as far as I can tell
            // it is giving the file a wrong name
            if ( file.contains( QRegExp( "\\.(tgz|TGZ)" ) ) )
            {
               QString dest = file;
               dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar" );
               QDir qd;
               qd.remove( dest );
               if ( !qd.rename( file, dest ) )
               {
                  errormsg = QString( tr( "Error: renaming %1 to %2 " ) ).arg( file ).arg( dest );
                  return false;
               }
               
               file = dest;
            }
         }

         // file should be .tar now
         
         if ( !file.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
         {
            errormsg = QString( tr( "Error: file %1 is not .tar" ) ).arg( file );
            return false;
         }

         // untar

         US_Tar ust;

         QStringList tar_list;
         
         int result = ust.list( file, tar_list, true );
         if ( TAR_OK != result )
         {
            errormsg = QString( tr( "Error: %1 problem list tar archive (%2)" ) ) .arg( file ).arg( ust.explain( result ) );
            return false;
         }
         
         QStringList mkdirs;
         for ( unsigned int i = 0; i < tar_list.size(); i++ )
         {
            if ( QFileInfo( tar_list[ i ] ).dirPath().length() &&
                 QFileInfo( tar_list[ i ] ).dirPath() != "." )
            {
               errormsg = QString( tr( "Error: Subdirectories now allowed in results (<%1> %2 %3 %4 %5)" ) )
                  .arg( QFileInfo( tar_list[ i ] ).dirPath() )
                  .arg( QFileInfo( tar_list[ i ] ).dirPath().length() )
                  .arg( org_file ).arg( file ).arg( tar_list[ i ] );
               return false;
            }
            final_results << subDirPath + SLASH + tar_list[ i ];
         }
         
         result = ust.extract( file );
         if ( TAR_OK != result )
         {
            errormsg = QString( tr( "Error: %1 problem extracting tar archive (%2)" ) ).arg( file ).arg( ust.explain( result ) );
            return false;
         }
         
         // remove tar file
         
         if ( !QFile::remove( file ) ) 
         {
            errormsg = QString( tr( "Error: can not remove file %1 in %2" ) ).arg( file ).arg( tmp_dir );
            return false;
         }
      }
   }

   if ( !QDir::setCurrent( tmp_dir ) )
   {
      errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( tmp_dir );
      return false;
   }

   cout << "Final results:\n" << final_results.join( "\n" ) << endl;
   if ( !merge_csvs( final_results ) )
   {
      return false;
   }
   cout << "Final results after merge_csv:\n" << final_results.join( "\n" ) << endl;

   if ( !move_to_results( org_file, final_results ) )
   {
      return false;
   }
   return true;
}

bool US_Hydrodyn_Cluster_Results::merge_csvs( QStringList &final_results )
{
   QStringList csvs = final_results.grep( QRegExp( "\\.(csv|CSV)$" ) );
   if ( !csvs.size() )
   {
      // no csvs
      return true;
   }

   map < QString, vector < QString > > dup_csvs;

   for ( unsigned int i = 0; i < csvs.size(); i++ )
   {
      dup_csvs[ QFileInfo( csvs[ i ] ).fileName() ].push_back( csvs[ i ] );
   }

   map < QString, bool > csvs_merged;

   for ( map < QString, vector < QString > >::iterator it = dup_csvs.begin();
         it != dup_csvs.end();
         it++ )
   {
      if ( it->second.size() > 1 )
      {
         if ( !merge_this_csv( it->first, it->second ) )
         {
            return false;
         }
         csvs_merged[ it->first ] = true;
      }
   }

   // redo the final_results

   QStringList new_results;

   for ( unsigned int i = 0; i < final_results.size(); i++ )
   {
      if ( !csvs_merged.count( QFileInfo( final_results[ i ] ).fileName() ) )
      {
         new_results << final_results[ i ];
      }
   }

   for ( map < QString, bool >::iterator it = csvs_merged.begin();
         it != csvs_merged.end();
         it++ )
   {
      new_results << it->first;
   }

   final_results = new_results;

   return true;
}

bool US_Hydrodyn_Cluster_Results::merge_this_csv( QString dest, vector < QString > csvs )
{
   // cat all the csvs into dest
   errormsg = "";

   if ( !QDir::setCurrent( tmp_dir ) )
   {
      errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( tmp_dir );
      return false;
   }

   // simply concatenate for now, maybe later we will get more advanced:
   // i.e. checking header compability and removing redundant headers

   QFile fo( dest );
   if ( !fo.open( IO_WriteOnly ) )
   {
      errormsg = QString( tr( "Error: can not create file %1 for merged csvs" ) ).arg( dest );
      return false;
   }
      
   QTextStream tso( &fo );

   for ( unsigned int i = 0; i < csvs.size(); i++ )
   {
      if ( !QFile::exists( csvs[ i ] ) )
      {
         errormsg = QString( tr( "Error: csv file %1 does not exist for merging" ) ).arg( csvs[ i ] );
         fo.close();
         return false;
      }
         
      QFile fi( csvs[ i ] );
      if ( !fi.open( IO_ReadOnly ) )
      {
         errormsg = QString( tr( "Error: can not open file %1 for merging" ) ).arg( csvs[ i ] );
         fo.close();
         return false;
      }
      
      QTextStream tsi( &fi );

      while( !tsi.atEnd() )
      {
         QString qs = tsi.readLine();
         tso << qs << endl;
      }

      fi.close();
   }

   fo.close();
   return true;
}

bool US_Hydrodyn_Cluster_Results::move_to_results( QString jobname, QStringList final_results )
{
   QString output_dir = results_dir + SLASH + QString( "%1" )
      .arg( jobname ).replace( QRegExp( "_(out|OUT).*$" ), "" );
   QDir qd( output_dir );
   if ( qd.exists() )
   {
      switch ( QMessageBox::question(
                                     this,
                                     tr( "US-SOMO: Cluster Results" ),
                                     QString(
                                             tr( "Results directory %1 already exists\n"
                                                 "What to you want to do?" ) ).arg( output_dir ),
                                     tr( "&Rename to a unique results directory" ),
                                     tr( "&Overwrite the results into the existing directory" ),
                                     tr( "&Clear the existing directory first" ),
                                     0 ) )
      {
      case 0 : 
         {
            QString new_dir;
            unsigned int ext = 1;
            QDir qd;
            do 
            {
               new_dir = QString( "%1-%2" ).arg( output_dir ).arg( ext );
               ext++;
            } while( qd.exists( new_dir ) );
            output_dir = new_dir;
         }
         break;

      case 1 : 
         break;
      
      case 2 :
         clean_dir( output_dir );
      }
   }

   if ( !qd.exists( output_dir ) )
   {
      if ( !qd.mkdir( output_dir ) )
      {
         errormsg = QString( tr( "Error: can not make directory %1" ) ).arg( output_dir );
         return false;
      }
   }

   US_File_Util usu;

   for ( unsigned int i = 0; i < final_results.size(); i++ )
   {
      QString from_dir = tmp_dir;
      if ( QFileInfo( final_results[ i ] ).dirPath().length() )
      {
         QString dirPath = QFileInfo( final_results[ i ] ).dirPath();
         if ( dirPath != "." )
         {
            from_dir += SLASH + dirPath;
         }
      }
      if ( !QDir::setCurrent( from_dir ) )
      {
         errormsg = QString( tr( "Error: can not change to directory %1" ) ).arg( from_dir );
         return false;
      }
      if ( !usu.move( QFileInfo( final_results[ i ] ).fileName(), output_dir ) )
      {
         errormsg = usu.errormsg;
         return false;
      }
   }      
   editor_msg( "dark blue", QString( tr( "Results for job %1 are now in %2" ) ).arg( jobname ).arg( output_dir ) );
   return true;
}


unsigned int US_Hydrodyn_Cluster_Results::update_files( bool set_lv_files )
{
   files.clear();

   // traverse directory and build up files
   QDir::setCurrent( completed_dir );
   QDir qd;
   QStringList tgz_files = qd.entryList( "*_out.tgz" );
   QStringList tar_files = qd.entryList( "*_out.tar" );
   files = QStringList::split( "\n", 
                               tgz_files.join("\n") + 
                               ( tgz_files.size() ? "\n" : "" ) +
                               tar_files.join("\n") );
   
   if ( set_lv_files )
   {
      lv_files->clear();
      for ( unsigned int i = 0; i < files.size(); i++ )
      {
         new QListViewItem( lv_files, 
                            files[ i ], 
                            QString( " %1 " ).arg( QFileInfo( files[ i ] ).created().toString() ),
                            QString( " %1 bytes " ).arg( QFileInfo( files[ i ] ).size() )
                            );
      }
   }

   return files.size();
}

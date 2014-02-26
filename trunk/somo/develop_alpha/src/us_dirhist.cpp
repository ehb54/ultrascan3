#include "../include/us_dirhist.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QLabel>

US_Dirhist::US_Dirhist(
                       QStringList                 & history,
                       map < QString, QDateTime >  & last_access,
                       map < QString, QString >    & last_filetype,
                       QString                     & selected,
                       bool                        & is_ok,
                       QWidget                     * p,
                       const char                  * name
                       ) : QDialog(p, name)
{
   this->history       = & history;
   this->last_access   = & last_access;
   this->last_filetype = & last_filetype;
   this->selected      = & selected;
   this->is_ok         = & is_ok;

   order_ascending   = false;

   USglobal = new US_Config();

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   setCaption( tr("US-SOMO: Previously used directories") );

   setupGUI();
   // global_Xpos += 30;
   // global_Ypos += 30;
   // setGeometry(global_Xpos, global_Ypos, 0, 0);
   t_hist->setMinimumWidth( 1.1 *  
                            ( t_hist->columnWidth( 0 ) +
                              t_hist->columnWidth( 1 ) +
                              t_hist->columnWidth( 3 ) ) );
}

US_Dirhist::~US_Dirhist()
{
}

void US_Dirhist::setupGUI()
{
   int minHeight1 = 30;
   //   int minHeight2 = 50;

   lbl_info = new QLabel( tr( "Previously used directories" ), this );
   lbl_info->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   t_hist = new Q3Table( history->size(), 5, this);
   t_hist->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   t_hist->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_hist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   t_hist->setEnabled(true);
   t_hist->setSelectionMode( Q3Table::SingleRow );

   t_hist->setShowGrid( false );
   t_hist->setReadOnly( true );

   t_hist->horizontalHeader()->setLabel( 0, tr( "Directory" ) );
   t_hist->horizontalHeader()->setLabel( 1, tr( "Last access" ) );
   t_hist->horizontalHeader()->setLabel( 3, tr( "Last type loaded" ) );

   {
      unsigned int unknowns = 0;
      for ( int i = 0; i < (int) history->size(); ++i )
      {
         t_hist->setText( i, 0, (*history)[ i ] );
         t_hist->setText( i, 1, 
                          last_access->count( (*history)[ i ] ) ?
                          (*last_access)[ (*history)[ i ] ].toString( Qt::LocalDate ) : "" );
         t_hist->setText( i, 3, 
                          ( last_filetype->count( (*history)[ i ] ) && !(*last_filetype)[ (*history)[ i ] ].isEmpty() ) ?
                          ( "." + (*last_filetype)[ (*history)[ i ] ] ) : "" );
         {
            QString qs = 
               QString( "%1" )
               .arg( last_access->count( (*history)[ i ] ) ?
                     (unsigned int)(*last_access)[ (*history)[ i ] ].toTime_t() :
                     unknowns++ );
            while ( qs.length() < 20 )
            {
               qs = "0" + qs;
            }
            t_hist->setText( i, 2, qs );

            // now type sorted by last access
            QString type =
               last_filetype->count( (*history)[ i ] ) ?
               (*last_filetype)[ (*history)[ i ] ] : "";

            while ( type.length() < 20 )
            {
               type += " ";
            }
            t_hist->setText( i, 4, type + qs );
         }
      }
   }
   t_hist->setSelectionMode( Q3Table::MultiRow );
   t_hist->setColumnWidth( 0, 400 );
   t_hist->setColumnWidth( 1, 180 );
   t_hist->setColumnWidth( 3, 120 );
   t_hist->hideColumn( 2 );
   t_hist->hideColumn( 4 );
   t_hist->horizontalHeader()->setClickEnabled( true );
   t_hist->setColumnMovingEnabled( false );

   t_hist->sortColumn( 2, false, true );
   t_hist->clearSelection();

   bool any_selected = false;
   for ( int i = 0; i < t_hist->numRows(); ++i )
   {
      if ( t_hist->text( i, 0 ) == *selected )
      {
         any_selected = true;
         t_hist->selectRow( i );
         t_hist->setCurrentCell( i, 0 );
         break;
      }
   }
   if ( !any_selected &&
        t_hist->numRows() )
   {
      t_hist->selectRow( 0 );
      t_hist->setCurrentCell( 0, 0 );
   }

   connect( t_hist->horizontalHeader(), SIGNAL( clicked(int) ), SLOT( t_sort_column(int) ) );
   connect( t_hist, SIGNAL( doubleClicked( int, int, int, const QPoint & ) ), SLOT( t_doubleClicked( int, int, int, const QPoint & ) ) );
   connect( t_hist, SIGNAL( selectionChanged() ), SLOT( t_selectionChanged() ) );

   pb_del = new QPushButton(tr("Delete directory"), this);
   pb_del->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_del->setMinimumHeight(minHeight1);
   pb_del->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_del, SIGNAL(clicked()), SLOT(del()));

   pb_ok = new QPushButton(tr("OK"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   int j = 0;

   Q3GridLayout * background = new Q3GridLayout( this );

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;

   background->addMultiCellWidget(t_hist  , j, j, 0, 1);
   j++;

   background->addWidget( pb_del , j, 0 );
   background->addWidget( pb_ok  , j, 1 );

   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );

   t_selectionChanged();
}

void US_Dirhist::cancel()
{
   *is_ok = false;
   close();
}

void US_Dirhist::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_dirhist.html");
}

void US_Dirhist::closeEvent(QCloseEvent *e)
{
   // global_Xpos -= 30;
   // global_Ypos -= 30;
   e->accept();
}

void US_Dirhist::t_sort_column( int col )
{
   // keep selection & currrent
   set < QString > selected;

   QString current = t_hist->text( t_hist->currentRow(), 0 );
   for ( int i = 0; i < t_hist->numRows(); ++i )
   {
      if ( t_hist->isRowSelected( i ) )
      {
         selected.insert( t_hist->text( i, 0 ) );
      }
   }
   
   t_hist->sortColumn( col ? col + 1 : col, order_ascending, true );

   disconnect( t_hist, SIGNAL( selectionChanged() ), 0, 0 );
   t_hist->clearSelection();
   for ( int i = 0; i < t_hist->numRows(); ++i )
   {
      if ( selected.count( t_hist->text( i, 0 ) ) )
      {
         t_hist->selectRow( i );
      }
      if ( current == t_hist->text( i, 0 ) )
      {
         t_hist->setCurrentCell( i, 0 );
      }
   }
   connect( t_hist, SIGNAL( selectionChanged() ), SLOT( t_selectionChanged() ) );
   t_selectionChanged();

   order_ascending = !order_ascending;
}

void US_Dirhist::t_doubleClicked( int, int col, int, const QPoint &  )
{
   *is_ok = true;
   *selected = t_hist->text( col, 0 );
   //   (*last_access)[ *selected ] = QDateTime::currentDateTime();
   ok();
   history->clear();
   for ( int i = 0; i < t_hist->numRows(); ++i )
   {
      history->push_back( t_hist->text( i, 0 ) );
   }
   close();
}

void US_Dirhist::ok()
{
   *is_ok = true;
   history->clear();
   for ( int i = 0; i < t_hist->numRows(); ++i )
   {
      history->push_back( t_hist->text( i, 0 ) );
      if ( t_hist->isRowSelected( i ) )
      {
         *selected = t_hist->text( i, 0 );
         // (*last_access)[ *selected ] = QDateTime::currentDateTime();
      }
   }
   close();
}

void US_Dirhist::del()
{
   disconnect( t_hist, SIGNAL( selectionChanged() ), 0, 0 );
   for ( int i = t_hist->numRows() - 1; i >= 0; --i )
   {
      if ( t_hist->isRowSelected( i ) )
      {
         t_hist->removeRow( i );
      }
   }
   t_hist->clearSelection();
   if ( t_hist->numRows() )
   {
      t_hist->selectRow( 0 );
      t_hist->setCurrentCell( 0, 0 );
   }      
   connect( t_hist, SIGNAL( selectionChanged() ), SLOT( t_selectionChanged() ) );
   t_selectionChanged();
}

void US_Dirhist::t_selectionChanged()
{
   // do all the table stuff
   // qDebug( QString( "num selections %1" ).arg( t_hist->numSelections() ) );
   // qDebug( QString( "current sel    %1" ).arg( t_hist->currentSelection() ) );
   pb_ok ->setEnabled( t_hist->numSelections() == 1 );
   pb_del->setEnabled( t_hist->numSelections() );
}

#include "../include/us_dirhist.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

US_Dirhist::US_Dirhist(
                       QStringList                 & history,
                       map < QString, QDateTime >  & last_access,
                       map < QString, QString >    & last_filetype,
                       QString                     & selected,
                       bool                        & is_ok,
                       QWidget                     * p,
                       const char                  * 
                       ) : QDialog( p )
{
   this->history       = & history;
   this->last_access   = & last_access;
   this->last_filetype = & last_filetype;
   this->selected      = & selected;
   this->is_ok         = & is_ok;

   order_ascending   = false;

   USglobal = new US_Config();

   setPalette( PALET_FRAME );

   setWindowTitle( us_tr("US-SOMO: Previously used directories") );

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

   lbl_info = new QLabel( us_tr( "Previously used directories" ), this );
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   t_hist = new QTableWidget( history->size(), 5, this);
   t_hist->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   t_hist->setPalette( PALET_EDIT );
   t_hist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   t_hist->setEnabled(true);
   // t_hist->setSelectionMode( QTable::SingleRow );

   t_hist->setShowGrid( false );

   t_hist->setHorizontalHeaderItem( 0, new QTableWidgetItem( us_tr( "Directory" ) ));
   t_hist->setHorizontalHeaderItem( 1, new QTableWidgetItem( us_tr( "Last access" ) ));
   t_hist->setHorizontalHeaderItem( 3, new QTableWidgetItem( us_tr( "Last type loaded" ) ));

   {
      unsigned int unknowns = 0;
      for ( int i = 0; i < (int) history->size(); ++i )
      {
         t_hist->setItem( i, 0, new QTableWidgetItem( (*history)[ i ] ) );
         t_hist->item( i, 0 )->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );
         {
            QString toset =
               last_access->count( (*history)[ i ] )
               ? (*last_access)[ (*history)[ i ] ].toString( QLocale::system().dateFormat( QLocale::ShortFormat ) )
               : "";
            t_hist->setItem( i, 1, new QTableWidgetItem( toset ) );
         }
         {
            QString toset =
               ( last_filetype->count( (*history)[ i ] ) && !(*last_filetype)[ (*history)[ i ] ].isEmpty() )
               ? ( "." + (*last_filetype)[ (*history)[ i ] ] )
               : ""
               ;
            t_hist->setItem( i, 3, new QTableWidgetItem( toset ) );
         }
         {
            QString qs = 
               QString( "%1" )
               .arg( last_access->count( (*history)[ i ] ) ?
                     // (unsigned int)(*last_access)[ (*history)[ i ] ].toTime_t() :
                     (unsigned int)(*last_access)[ (*history)[ i ] ].toSecsSinceEpoch() :
                     unknowns++ );
            while ( qs.length() < 20 )
            {
               qs = "0" + qs;
            }
            t_hist->setItem( i, 2, new QTableWidgetItem( qs ) );

            // now type sorted by last access
            QString type =
               last_filetype->count( (*history)[ i ] ) ?
               (*last_filetype)[ (*history)[ i ] ] : "";

            while ( type.length() < 20 )
            {
               type += " ";
            }
            t_hist->setItem( i, 4, new QTableWidgetItem( type + qs ) );
         }
         t_hist->setRowHeight( i, minHeight1 * 1.4 );
      }
   }
   t_hist->setSelectionMode( QAbstractItemView::MultiSelection );t_hist->setSelectionBehavior( QAbstractItemView::SelectRows );
   t_hist->setColumnWidth( 0, 400 );
   t_hist->setColumnWidth( 1, 180 );
   t_hist->setColumnWidth( 3, 120 );
   t_hist->hideColumn( 2 );
   t_hist->hideColumn( 4 );
    t_hist->horizontalHeader()->setSectionsClickable( true );
    t_hist->horizontalHeader()->setSectionsMovable( false );

   t_hist->sortByColumn( 2,  false ? Qt::AscendingOrder : Qt::DescendingOrder );
   t_hist->clearSelection();

   bool any_selected = false;
   for ( int i = 0; i < t_hist->rowCount(); ++i )
   {
      if ( t_hist->item( i, 0 )->text() == *selected )
      {
         any_selected = true;
         t_hist->selectRow( i );
         t_hist->setCurrentCell( i, 0 );
         break;
      }
   }
   if ( !any_selected &&
        t_hist->rowCount() )
   {
      t_hist->selectRow( 0 );
      t_hist->setCurrentCell( 0, 0 );
   }

  { for ( int i = 0; i < t_hist->rowCount(); ++i ) { for ( int j = 0; j < t_hist->columnCount(); ++j ) { t_hist->item( i, j )->setFlags( t_hist->item( i, j )->flags() ^ Qt::ItemIsEditable ); } } };

#if QT_VERSION < 0x040000   
   connect( t_hist->horizontalHeader(), SIGNAL( clicked(int) ), SLOT( t_sort_column(int) ) );
   connect( t_hist, SIGNAL( doubleClicked( int, int, int, const QPoint & ) ), SLOT( t_doubleClicked( int, int, int, const QPoint & ) ) );
#else
   connect( t_hist->horizontalHeader(), SIGNAL( sectionClicked(int) ), SLOT( t_sort_column(int) ) );
   connect( t_hist, SIGNAL( cellDoubleClicked( int, int) ), SLOT( t_doubleClicked( int, int ) ) );
#endif
   connect( t_hist, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );

   pb_del = new QPushButton(us_tr("Delete directory from history"), this);
   pb_del->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_del->setMinimumHeight(minHeight1);
   pb_del->setPalette( PALET_PUSHB );
   connect(pb_del, SIGNAL(clicked()), SLOT(del()));

   pb_ok = new QPushButton(us_tr("OK"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   int j = 0;

   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   background->addWidget( t_hist   , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   background->addWidget( pb_del , j, 0 );
   background->addWidget( pb_ok  , j, 1 );

   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );

   update_enables();
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
   online_help->show_help("manual/somo/somo_dirhist.html");
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

   QString current = t_hist->item( t_hist->currentRow(), 0 )->text();
   for ( int i = 0; i < t_hist->rowCount(); ++i )
   {
      if ( t_hist->item( i , 0 )->isSelected() )
      {
         selected.insert( t_hist->item( i, 0 )->text() );
      }
   }
   
   t_hist->sortByColumn( col ? col + 1 : col,  order_ascending ? Qt::AscendingOrder : Qt::DescendingOrder );

   disconnect( t_hist, SIGNAL( itemSelectionChanged() ), 0, 0 );
   t_hist->clearSelection();
   for ( int i = 0; i < t_hist->rowCount(); ++i )
   {
      if ( selected.count( t_hist->item( i, 0 )->text() ) )
      {
         t_hist->selectRow( i );
      }
      if ( current == t_hist->item( i, 0 )->text() )
      {
         t_hist->setCurrentCell( i, 0 );
      }
   }
   connect( t_hist, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );
   update_enables();

   order_ascending = !order_ascending;
}

void US_Dirhist::t_doubleClicked( int row, int )
{
   *is_ok = true;
   *selected = t_hist->item( row, 0 )->text();
   //   (*last_access)[ *selected ] = QDateTime::currentDateTime();
   // ok();
   history->clear( );
   for ( int i = 0; i < t_hist->rowCount(); ++i )
   {
      history->push_back( t_hist->item( i, 0 )->text() );
   }
   close();
}

void US_Dirhist::t_doubleClicked( int row, int col, int, const QPoint &  )
{
   return t_doubleClicked( row, col );
}

void US_Dirhist::ok()
{
   *is_ok = true;
   history->clear( );
   for ( int i = 0; i < t_hist->rowCount(); ++i )
   {
      history->push_back( t_hist->item( i, 0 )->text() );
      if ( t_hist->item( i , 0 )->isSelected() )
      {
         *selected = t_hist->item( i, 0 )->text();
         // (*last_access)[ *selected ] = QDateTime::currentDateTime();
      }
   }
   close();
}

void US_Dirhist::del()
{
   disconnect( t_hist, SIGNAL( itemSelectionChanged() ), 0, 0 );
   for ( int i = t_hist->rowCount() - 1; i >= 0; --i )
   {
      if ( t_hist->item( i , 0 )->isSelected() )
      {
         t_hist->removeRow( i );
      }
   }
   t_hist->clearSelection();
   if ( t_hist->rowCount() )
   {
      t_hist->selectRow( 0 );
      t_hist->setCurrentCell( 0, 0 );
   }      
   connect( t_hist, SIGNAL( itemSelectionChanged() ), SLOT( update_enables() ) );
   update_enables();
}

#if QT_VERSION >= 0x040000   
# include <QList>
# include <QTableWidgetSelectionRange>
#endif

void US_Dirhist::update_enables()
{
   int num_selections = 0;
   for ( int i = 0; i < t_hist->rowCount(); ++i ) {
      if ( t_hist->item( i , 0 )->isSelected() ) {
         num_selections++;
      }
   }
   pb_ok ->setEnabled( num_selections == 1 );
   pb_del->setEnabled( num_selections );
}

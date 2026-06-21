#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
#include "../include/us_hydrodyn.h"
#include <QRegularExpression>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QColor>
#include <QHeaderView>

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc(
                                                                          QStringList names,
                                                                          map < QString, double > prepop_conc,
                                                                          map < QString, double > *out_name_to_conc,
                                                                          bool *out_ok,
                                                                          void *us_hydrodyn,
                                                                          QWidget *p,
                                                                          const char *
                                                                          ) : QDialog( p )
{
   this->names            = names;
   this->prepop_conc      = prepop_conc;
   this->out_name_to_conc = out_name_to_conc;
   this->out_ok           = out_ok;
   this->us_hydrodyn      = us_hydrodyn;

   *out_ok = false;

   disable_updates = true;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS: Assign concentrations for extrapolation to zero concentration" ) );
   setModal( true );

   setupGUI();

   disable_updates = false;
   validate_all_rows();

   // curve names can get quite long (e.g. derived HPLC/KIN frame names), so size
   // the window width to the longest name rather than using a fixed width
   int max_name_len = 0;
   for ( int i = 0; i < names.size(); i++ )
   {
      QString display_name = names[ i ];
      display_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
      if ( display_name.length() > max_name_len )
      {
         max_name_len = display_name.length();
      }
   }
   int width = qBound( 600, 250 + max_name_len * 8, 1400 );

   setGeometry( 200, 150, width, 100 + 30 * ( names.size() + 4 ) );
}

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc()
{
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(
                         us_tr( "Enter a non-negative concentration for each curve to be used in the\n"
                                "linear extrapolation to zero concentration" ),
                         this
                         );
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1 * 2);
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   t_conc = new QTableWidget( names.size(), 2, this );
   t_conc->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   t_conc->setPalette( PALET_EDIT );
   AUTFBACK( t_conc );
   t_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   t_conc->setHorizontalHeaderLabels( QStringList() << us_tr( "Curve Name" ) << us_tr( "Concentration" ) );
   t_conc->setSortingEnabled( false );
   t_conc->verticalHeader()->hide();
   t_conc->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
   t_conc->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::ResizeToContents );

   populate_table();

   connect( t_conc, SIGNAL( itemChanged( QTableWidgetItem * ) ), SLOT( cell_changed( QTableWidgetItem * ) ) );

   lbl_status = new QLabel( "", this );
   lbl_status->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_status->setMinimumHeight(minHeight1);
   lbl_status->setPalette( PALET_LABEL );
   AUTFBACK( lbl_status );
   lbl_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_ok = new QPushButton(us_tr("Extrapolate"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_cancel );
   hbl_bottom->addWidget( pb_ok );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_info );
   background->addWidget( t_conc );
   background->addWidget( lbl_status );
   background->addLayout( hbl_bottom );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::populate_table()
{
   disable_updates = true;
   for ( int i = 0; i < names.size(); i++ )
   {
      QString display_name = names[ i ];
      display_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );

      QTableWidgetItem *name_item = new QTableWidgetItem( display_name );
      name_item->setFlags( name_item->flags() & ~Qt::ItemIsEditable );
      t_conc->setItem( i, 0, name_item );

      QString conc_text = "";
      if ( prepop_conc.count( names[ i ] ) )
      {
         conc_text = QString( "%1" ).arg( prepop_conc[ names[ i ] ] );
      }
      t_conc->setItem( i, 1, new QTableWidgetItem( conc_text ) );
   }
   t_conc->resizeColumnsToContents();
   disable_updates = false;
}

bool US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::validate_all_rows()
{
   unsigned int valid_count = 0;

   for ( int i = 0; i < t_conc->rowCount(); i++ )
   {
      QTableWidgetItem *item = t_conc->item( i, 1 );
      bool ok = false;
      double val = item->text().toDouble( &ok );
      bool row_valid = ok && val >= 0e0;

      item->setBackground( row_valid ? QColor( 255, 255, 255 ) : QColor( 255, 200, 200 ) );

      if ( row_valid )
      {
         valid_count++;
      }
   }

   bool all_valid = (int) valid_count == t_conc->rowCount();

   lbl_status->setText(
                       all_valid ?
                       us_tr( "All rows valid" ) :
                       QString( us_tr( "%1 of %2 rows valid" ) ).arg( valid_count ).arg( t_conc->rowCount() )
                       );

   pb_ok->setEnabled( all_valid );

   return all_valid;
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::cell_changed( QTableWidgetItem * )
{
   if ( disable_updates )
   {
      return;
   }
   validate_all_rows();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::ok()
{
   if ( !validate_all_rows() )
   {
      return;
   }

   out_name_to_conc->clear();
   for ( int i = 0; i < names.size(); i++ )
   {
      ( *out_name_to_conc )[ names[ i ] ] = t_conc->item( i, 1 )->text().toDouble();
   }
   *out_ok = true;
   close();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::cancel()
{
   *out_ok = false;
   close();
}

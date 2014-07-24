//! \file us_constraints_edit.cpp

#include "us_constraints_edit.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_properties.h"
#include "us_investigator.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_associations_gui.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

// Constructor of dialog for editing discreteGA constraints
US_ConstraintsEdit::US_ConstraintsEdit( US_Model& current_model )
   : US_WidgetsDialog( 0, 0 ), model( current_model )
{
   const QString notapl = tr( "n/a" );
   setWindowTitle   ( "Discrete Model GA Constraints Editor" );
   setPalette       ( US_GuiSettings::frameColor() );
   setWindowModality( Qt::WindowModal );
   
   dbg_level = US_Settings::us_debug();
   oldRow    = -2;
   inUpdate  = false;
   chgStoi   = false;
   crow      = -1;
   arow      = -1;
   normal    = US_GuiSettings::editColor();

   // Very light gray
   gray      = normal;
   gray.setColor( QPalette::Base, QColor( 0xd0, 0xd0, 0xd0 ) );
   
   // Initialize the check icon
   check = QIcon( US_Settings::appBaseDir() + "/etc/check.png" );

   // Grid
   QGridLayout* main = new QGridLayout( this );
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );

   // Components
   QLabel*      lb_comps  = us_banner( tr( "Components" ) );
   lw_comps               = new US_ListWidget;

   // Components column headers
   QLabel*      lb_attr   = us_label( tr( "Attribute" ) );
   QLabel*      lb_avalue = us_label( tr( "Value" ) );
   QLabel*      lb_alow   = us_label( tr( "Low" ) );
   QLabel*      lb_ahigh  = us_label( tr( "High" ) );
   QLabel*      lb_float  = us_label( tr( "Float?" ) );
   QLabel*      lb_locsc  = us_label( tr( "LogSc?" ) );
//   QLabel*      lb_desc   = us_label( tr( "Analyte Description:" ) );

   // Attribute Fix? checkboxes and labels
   QGridLayout* lo_vbar   = us_checkbox( 
      tr( "Vbar at 20 " ) + DEGC + " (ml/g):", ck_sel_vbar, true );
   QGridLayout* lo_mw     = us_checkbox( 
      tr( "Molecular Wt. (mw)" ),              ck_sel_mw,   true );
   QGridLayout* lo_ff0    = us_checkbox( 
      tr( "Frictional Ratio (f/f0)" ),         ck_sel_ff0,  true );
   QGridLayout* lo_s      = us_checkbox(
      tr( "Sedimentation Coeff. (s)" ),        ck_sel_s    );
   QGridLayout* lo_D      = us_checkbox(
      tr( "Diffusion Coeff. (D)" ),            ck_sel_D    );
   QGridLayout* lo_f      = us_checkbox(
      tr( "Frictional Coeff. (f)" ),           ck_sel_f    );
   QGridLayout* lo_conc   = us_checkbox(
      tr( "Partial Concentration" ),           ck_sel_conc, true );
   ck_sel_vbar->setEnabled( false );
   ck_sel_mw  ->setEnabled( true  );
   ck_sel_ff0 ->setEnabled( true  );
   ck_sel_s   ->setEnabled( true  );
   ck_sel_D   ->setEnabled( true  );
   ck_sel_f   ->setEnabled( true  );
   ck_sel_conc->setEnabled( false );
DbgLv(1) << "cnG:main: hds,lbs defined";

   // Attribute value/low/high text boxes
   le_val_vbar             = us_lineedit( "0.7200" );
   le_min_vbar             = us_lineedit( notapl, true );
   le_max_vbar             = us_lineedit( notapl, true );
   le_val_mw               = us_lineedit( "10000"  );
   le_min_mw               = us_lineedit( notapl, true );
   le_max_mw               = us_lineedit( notapl, true );
   le_val_ff0              = us_lineedit( "2.0"    );
   le_min_ff0              = us_lineedit( notapl, true );
   le_max_ff0              = us_lineedit( notapl, true );
   le_val_s                = us_lineedit( "3e-13"  );
   le_min_s                = us_lineedit( notapl, true );
   le_max_s                = us_lineedit( notapl, true );
   le_val_D                = us_lineedit( "3e-7"   );
   le_min_D                = us_lineedit( notapl, true );
   le_max_D                = us_lineedit( notapl, true );
   le_val_f                = us_lineedit( "3e-6"   );
   le_min_f                = us_lineedit( notapl, true );
   le_max_f                = us_lineedit( notapl, true );
   le_val_conc             = us_lineedit( "0.5"    );
   le_min_conc             = us_lineedit( notapl, true );
   le_max_conc             = us_lineedit( notapl, true );
DbgLv(1) << "cnG:main: le_vals defined";

   // Attribute Float? checkboxes
   ck_flt_vbar            = new QCheckBox( "", this );
   ck_flt_mw              = new QCheckBox( "", this );
   ck_log_mw              = new QCheckBox( "", this );
   ck_flt_ff0             = new QCheckBox( "", this );
   ck_flt_s               = new QCheckBox( "", this );
   ck_flt_D               = new QCheckBox( "", this );
   ck_flt_f               = new QCheckBox( "", this );
   ck_flt_conc            = new QCheckBox( "", this );
   ck_flt_vbar->setChecked( true  );
   ck_flt_mw  ->setChecked( false );
   ck_log_mw  ->setChecked( true  );
   ck_flt_ff0 ->setChecked( false );
   ck_flt_s   ->setChecked( true  );
   ck_flt_D   ->setChecked( false );
   ck_flt_f   ->setChecked( false );
   ck_flt_conc->setChecked( false );
   ck_flt_vbar->setEnabled( true  );
   ck_flt_mw  ->setEnabled( false );
   ck_log_mw  ->setEnabled( false );
   ck_flt_ff0 ->setEnabled( false );
   ck_flt_s   ->setEnabled( false );
   ck_flt_D   ->setEnabled( false );
   ck_flt_f   ->setEnabled( false );
   ck_flt_conc->setEnabled( true  );
DbgLv(1) << "cnG:main: flt/log defined";

   QLabel* lb_extinction  =  us_label(
         tr( "Extinction (OD/(cm*mol)):" ) );
   QLabel* lb_wavelength  =  us_label(
         tr( "Wavelength (nm):" ) );
   le_extinction = us_lineedit( "" );
   le_wavelength = us_lineedit( QString::number( model.wavelength, 'f', 1 ) );
   le_wavelength->setMinimumWidth( 80 );
   le_wavelength->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
   us_setReadOnly( le_wavelength, true );
   //QHBoxLayout* extinction = new QHBoxLayout;

   // Advanced parameters
   QLabel* lb_sigma = us_label(
      tr( "Concentration Dependency of s (<span>&sigma;</span>):" ) );
   le_sigma = us_lineedit( "" );
   QLabel* lb_delta = us_label(
      tr( "Concentration Dependency of D (<span>&delta;</span>):" ) );
   le_delta = us_lineedit( "" );
   pb_load_c0 = us_pushbutton( tr( "Load C0 from File" ) );
   QGridLayout* co_sed_layout = us_checkbox(
      tr( "Co-sedimenting Solute" ), ck_co_sed );

   // Associations
   QLabel*      lb_assocs = us_banner( tr( "Associations (reactions)" ) );
   lw_assocs              = new US_ListWidget;

   // Associations column headers
   QLabel*      lb_attra  = us_label( tr( "Attribute" ) );
   QLabel*      lb_avala  = us_label( tr( "Value" ) );
   QLabel*      lb_alowa  = us_label( tr( "Low" ) );
   QLabel*      lb_ahigha = us_label( tr( "High" ) );
   QLabel*      lb_floata = us_label( tr( "Float?" ) );
   QLabel*      lb_logsca = us_label( tr( "LogSc?" ) );
   le_lbl_kd              = us_lineedit( tr( "K_dissociation" ), true );
   le_val_kd              = us_lineedit( "0.0001" );
   le_min_kd              = us_lineedit( notapl, true );
   le_max_kd              = us_lineedit( notapl, true );
   ck_flt_kd              = new QCheckBox( "", this );
   ck_log_kd              = new QCheckBox( "", this );
   le_lbl_koff            = us_lineedit( tr( "k_off rate" ),     true );
   le_val_koff            = us_lineedit( "0.0001" );
   le_min_koff            = us_lineedit( notapl, true );
   le_max_koff            = us_lineedit( notapl, true );
   ck_flt_koff            = new QCheckBox( "", this );
   ck_log_koff            = new QCheckBox( "", this );
   us_setReadOnly( le_lbl_kd,   true );
   us_setReadOnly( le_lbl_koff, true );
   le_lbl_kd  ->setEnabled( false );
   le_lbl_koff->setEnabled( false );
   ck_flt_kd  ->setEnabled( true  );
   ck_log_kd  ->setEnabled( true  );
   ck_flt_koff->setEnabled( true  );
   ck_log_koff->setEnabled( true  );
DbgLv(1) << "cnG:main: assocs  defined";

   QPushButton* pb_close = us_pushbutton( tr( "Cancel") );
   QPushButton* pb_accept = us_pushbutton( tr( "Accept") );
DbgLv(1) << "cnG:main: elements defined";

   int row = 0;
   main->addWidget( lb_comps,       row++,  0, 1, 12 );
   main->addWidget( lw_comps,       row,    0, 3, 12 ); row += 3;
   main->addWidget( lb_attr,        row,    0, 1,  4 );
   main->addWidget( lb_avalue,      row,    4, 1,  2 );
   main->addWidget( lb_alow,        row,    6, 1,  2 );
   main->addWidget( lb_ahigh,       row,    8, 1,  2 );
   main->addWidget( lb_float,       row,   10, 1,  1 );
   main->addWidget( lb_locsc,       row++, 11, 1,  1 );
   main->addLayout( lo_vbar,        row,    0, 1,  4 );
   main->addWidget( le_val_vbar,    row,    4, 1,  2 );
   main->addWidget( le_min_vbar,    row,    6, 1,  2 );
   main->addWidget( le_max_vbar,    row,    8, 1,  2 );
   main->addWidget( ck_flt_vbar,    row++, 10, 1,  1 );
   main->addLayout( lo_mw,          row,    0, 1,  4 );
   main->addWidget( le_val_mw,      row,    4, 1,  2 );
   main->addWidget( le_min_mw,      row,    6, 1,  2 );
   main->addWidget( le_max_mw,      row,    8, 1,  2 );
   main->addWidget( ck_flt_mw,      row,   10, 1,  1 );
   main->addWidget( ck_log_mw,      row++, 11, 1,  1 );
   main->addLayout( lo_ff0,         row,    0, 1,  4 );
   main->addWidget( le_val_ff0,     row,    4, 1,  2 );
   main->addWidget( le_min_ff0,     row,    6, 1,  2 );
   main->addWidget( le_max_ff0,     row,    8, 1,  2 );
   main->addWidget( ck_flt_ff0,     row++, 10, 1,  1 );
   main->addLayout( lo_s,           row,    0, 1,  4 );
   main->addWidget( le_val_s,       row,    4, 1,  2 );
   main->addWidget( le_min_s,       row,    6, 1,  2 );
   main->addWidget( le_max_s,       row,    8, 1,  2 );
   main->addWidget( ck_flt_s,       row++, 10, 1,  1 );
   main->addLayout( lo_D,           row,    0, 1,  4 );
   main->addWidget( le_val_D,       row,    4, 1,  2 );
   main->addWidget( le_min_D,       row,    6, 1,  2 );
   main->addWidget( le_max_D,       row,    8, 1,  2 );
   main->addWidget( ck_flt_D,       row++, 10, 1,  1 );
   main->addLayout( lo_f,           row,    0, 1,  4 );
   main->addWidget( le_val_f,       row,    4, 1,  2 );
   main->addWidget( le_min_f,       row,    6, 1,  2 );
   main->addWidget( le_max_f,       row,    8, 1,  2 );
   main->addWidget( ck_flt_f,       row++, 10, 1,  1 );
   main->addLayout( lo_conc,        row,    0, 1,  4 );
   main->addWidget( le_val_conc,    row,    4, 1,  2 );
   main->addWidget( le_min_conc,    row,    6, 1,  2 );
   main->addWidget( le_max_conc,    row,    8, 1,  2 );
   main->addWidget( ck_flt_conc,    row++, 10, 1,  1 );
   main->addWidget( lb_extinction,  row,    0, 1,  4 );
   main->addWidget( le_extinction,  row,    4, 1,  2 );
   main->addWidget( lb_wavelength,  row,    6, 1,  4 );
   main->addWidget( le_wavelength,  row++, 10, 1,  2 );
   main->addWidget( lb_sigma,       row,    0, 1,  6 );
   main->addWidget( le_sigma,       row,    6, 1,  2 );
   main->addLayout( co_sed_layout,  row++,  8, 1,  4 );
   main->addWidget( lb_delta,       row,    0, 1,  6 );
   main->addWidget( le_delta,       row,    6, 1,  2 );
   main->addWidget( pb_load_c0,     row++,  8, 1,  4 );
   main->addWidget( lb_assocs,      row++,  0, 1, 12 );
   main->addWidget( lw_assocs,      row,    0, 2, 12 ); row += 2;
   main->addWidget( lb_attra,       row,    0, 1,  4 );
   main->addWidget( lb_avala,       row,    4, 1,  2 );
   main->addWidget( lb_alowa,       row,    6, 1,  2 );
   main->addWidget( lb_ahigha,      row,    8, 1,  2 );
   main->addWidget( lb_floata,      row,   10, 1,  1 );
   main->addWidget( lb_logsca,      row++, 11, 1,  1 );
   main->addWidget( le_lbl_kd,      row,    0, 1,  4 );
   main->addWidget( le_val_kd,      row,    4, 1,  2 );
   main->addWidget( le_min_kd,      row,    6, 1,  2 );
   main->addWidget( le_max_kd,      row,    8, 1,  2 );
   main->addWidget( ck_flt_kd,      row,   10, 1,  1 );
   main->addWidget( ck_log_kd,      row++, 11, 1,  1 );
   main->addWidget( le_lbl_koff,    row,    0, 1,  4 );
   main->addWidget( le_val_koff,    row,    4, 1,  2 );
   main->addWidget( le_min_koff,    row,    6, 1,  2 );
   main->addWidget( le_max_koff,    row,    8, 1,  2 );
   main->addWidget( ck_flt_koff,    row,   10, 1,  1 );
   main->addWidget( ck_log_koff,    row++, 11, 1,  1 );
   main->addWidget( pb_close,       row,    0, 1,  6 );
   main->addWidget( pb_accept,      row++,  6, 1,  6 );

   connect( le_extinction,  SIGNAL( editingFinished() ), 
                            SLOT  ( set_molar      () ) );
   connect( pb_load_c0,     SIGNAL( clicked() ), 
                            SLOT  ( load_c0() ) );
   connect( ck_co_sed,      SIGNAL( stateChanged( int ) ), 
                            SLOT  ( co_sed      ( int ) ) );
   connect( pb_close,       SIGNAL( clicked   () ), 
                            SLOT  ( close     () ) );
   connect( pb_accept,      SIGNAL( clicked   () ), 
                            SLOT  ( acceptProp() ) );
DbgLv(1) << "cnG:main: connections made";
   const QString clets( "ABCDEFGHIJ" );

   // Populate the lists from the model
   if ( model.analysis == US_Model::DMGA_CONSTR )
   {  // Constraints model
      constraints.load_constraints( &model );
DbgLv(1) << "cnG:main:  cmodel load rtn";
      for ( int ii = 0; ii < model.components.size(); ii++ )
      {
         QString prenm  = QString( clets ).mid( ii, 1 ) + " ";
         QString flgnm  = QString( model.components[ ii ].name ).left( 4 );
         QString name   = QString( model.components[ ii ].name ).mid( 4 );
         lw_comps->addItem( prenm + name );

         if ( ! flgnm.contains( "V" ) )  ii++;
      }

      for ( int ii = 0; ii < model.associations.size(); ii += 2 )
      {
         QVector< int > rcomps  = model.associations[ ii ].rcomps;
         QVector< int > stoichs = model.associations[ ii ].stoichs;
         int     nrc    = rcomps.size();
         int     rc1    = ( nrc > 0 ) ? rcomps [ 0 ] : 0;
         int     rc2    = ( nrc > 1 ) ? rcomps [ 1 ] : 0;
         int     rc3    = ( nrc > 2 ) ? rcomps [ 2 ] : 0;
         int     st1    = ( nrc > 0 ) ? stoichs[ 0 ] : 1;
         int     st2    = ( nrc > 1 ) ? stoichs[ 1 ] : 1;
         int     st3    = ( nrc > 2 ) ? stoichs[ 2 ] : 1;
         int     ol1    = model.components[ rc1 ].oligomer;
         int     ol2    = model.components[ rc2 ].oligomer;
         int     ol3    = model.components[ rc3 ].oligomer;
         QString name;
         if ( rcomps.size() == 2 )
         {
            name           = QString::number( st1 )
                           + QString( clets ).mid( rc1, 1 )
                           + QString::number( ol1 )
                           + " => "
                           + QString::number( qAbs( st2 ) )
                           + QString( clets ).mid( rc2, 1 )
                           + QString::number( ol2 );
         }
         else if ( rcomps.size() == 3 )
         {
            name           = QString::number( st1 )
                           + QString( clets ).mid( rc1, 1 )
                           + QString::number( ol1 )
                           + " + "
                           + QString::number( st2 )
                           + QString( clets ).mid( rc2, 1 ) 
                           + QString::number( ol2 )
                           + " => "
                           + QString::number( qAbs( st3 ) )
                           + QString( clets ).mid( rc3, 1 )
                           + QString::number( ol3 );
         }
         else
         {
            name           = "(reaction " + QString::number( ii + 1 ) + ")";
         }

         lw_assocs->addItem( name );
      }

      if ( lw_assocs->count() == 0 )
         lw_assocs->addItem( "(none)" );
   } 

   else
   {  // Base model
      constraints.load_base_model( &model );
DbgLv(1) << "cnG:main:  bmodel load rtn";
      for ( int ii = 0; ii < model.components.size(); ii++ )
      {
         QString prenm  = QString( "ABCDEFGHIJ" ).mid( ii, 1 ) + " ";
         QString name   = model.components[ ii ].name;
         lw_comps->addItem( prenm + name );
      }

      for ( int ii = 0; ii < model.associations.size(); ii++ )
      {
         QVector< int > rcomps  = model.associations[ ii ].rcomps;
         QVector< int > stoichs = model.associations[ ii ].stoichs;
         int     nrc    = rcomps.size();
         int     rc1    = ( nrc > 0 ) ? rcomps [ 0 ] : 0;
         int     rc2    = ( nrc > 1 ) ? rcomps [ 1 ] : 0;
         int     rc3    = ( nrc > 2 ) ? rcomps [ 2 ] : 0;
         int     st1    = ( nrc > 0 ) ? stoichs[ 0 ] : 1;
         int     st2    = ( nrc > 1 ) ? stoichs[ 1 ] : 1;
         int     st3    = ( nrc > 2 ) ? stoichs[ 2 ] : 1;
         int     ol1    = model.components[ rc1 ].oligomer;
         int     ol2    = model.components[ rc2 ].oligomer;
         int     ol3    = model.components[ rc3 ].oligomer;
         QString name;

         if ( rcomps.size() == 2 )
         {
            name           = QString::number( st1 )
                           + QString( clets ).mid( rc1, 1 )
                           + QString::number( ol1 )
                           + "  =>  "
                           + QString::number( qAbs( st2 ) )
                           + QString( clets ).mid( rc2, 1 )
                           + QString::number( ol2 );
         }
         else if ( rcomps.size() == 3 )
         {
            name           = QString::number( st1 )
                           + QString( clets ).mid( rc1, 1 )
                           + QString::number( ol1 )
                           + " + "
                           + QString::number( st2 )
                           + QString( clets ).mid( rc2, 1 )
                           + QString::number( ol2 )
                           + "  =>  "
                           + QString::number( qAbs( st3 ) )
                           + QString( clets ).mid( rc3, 1 )
                           + QString::number( ol3 );
         }
         else
         {
            name           = "(reaction " + QString::number( ii + 1 ) + ")";
         }

         lw_assocs->addItem( name );
      }

      if ( lw_assocs->count() == 0 )
         lw_assocs->addItem( "(none)" );
   }

   connect( lw_comps,  SIGNAL( currentRowChanged ( int ) ), 
                       SLOT  ( component_select  ( int ) ) );
   connect( lw_assocs, SIGNAL( currentRowChanged ( int ) ), 
                       SLOT  ( association_select( int ) ) );

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   int fhigh = fm.lineSpacing();
   int fwide = fm.width( QChar( '6' ) );
   int chigh = fhigh * 3 + 12;
   int ahigh = fhigh * 2 + 12;
   int lwide = width() - 10;
   const int bmwide = 700;
   const int bmhigh = 540;
   const int bfwide = 8;
   const int bfhigh = 16;

   lw_comps ->setMinimumHeight( chigh );
   lw_comps ->resize( lwide, chigh );
   lw_assocs->setMinimumHeight( ahigh );
   lw_assocs->resize( lwide, ahigh );

DbgLv(1) << "cnG:main:  set comp row";
   lw_comps ->setCurrentRow( 0 );
   lw_assocs->setCurrentRow( model.associations.size() > 0 ? 0 : -1 );

   comps_connect ( true );
   assocs_connect( true );

DbgLv(1) << "cnG:main:  m size" << size() << "fwide fhigh" << fwide << fhigh;
   int mwide = ( bmwide * fwide ) / bfwide;
   int mhigh = ( bmhigh * fhigh ) / bfhigh;
   resize( mwide, mhigh );
DbgLv(1) << "cnG:main:  m size" << size();
}

void US_ConstraintsEdit::edit_vbar( void )
{
int row=0;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   if ( keep_standard() )  // Change from standard values?
   {
      le_val_vbar->setText( QString::number( sc->vbar20, 'e', 4 ) );
      return;
   }

   sc->vbar20 = le_val_vbar->text().toDouble();

   calculate();
}

void US_ConstraintsEdit::load_c0( void )
{
//   int row = lw_comps->currentRow();

//   if ( row < 0 ) return;
int row=0;

   // See if the initialization vector is already loaded.
   if ( ! pb_load_c0->icon().isNull() )
   {
      int response = QMessageBox::question( this,
         tr( "Remove C0 Data?" ),
         tr( "The C0 information is loaded.\n"
             "Remove it?" ),
         QMessageBox::Yes, QMessageBox::No );

      if ( response == QMessageBox::Yes )
      {
         US_Model::SimulationComponent* sc = &model.components[ row ];

         sc->c0.radius       .clear();
         sc->c0.concentration.clear();
         pb_load_c0->setIcon( QIcon() );
      }

      return;
   }
   
   QMessageBox::information( this,
      tr( "UltraScan Information" ),
      tr( "Please note:\n\n"
          "The initial concentration file should have\n"
          "the following format:\n\n"
          "radius_value1 concentration_value1\n"
          "radius_value2 concentration_value2\n"
          "radius_value3 concentration_value3\n"
          "etc...\n\n"
          "radius values smaller than the meniscus or\n"
          "larger than the bottom of the cell will be\n"
          "excluded from the concentration vector." ) );

   QString fn = QFileDialog::getOpenFileName(
         this, tr( "Load initial concentration" ), US_Settings::resultDir(), "*" );

   if ( ! fn.isEmpty() )
   {
      QFile f( fn );;

      if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         QTextStream ts( &f );

//         int row = lw_comps->currentRow();
int row=0;

         US_Model::SimulationComponent* sc = &model.components[ row ];

         sc->c0.radius       .clear();
         sc->c0.concentration.clear();

         // Sets concentration for this component to -1 to signal that we are
         // using a concentration vector
         double val1;
         double val2;

         while ( ! ts.atEnd() )
         {
            ts >> val1;
            ts >> val2;

            if ( val1 > 0.0 ) // Ignore radius pairs that aren't positive
            {
               sc->c0.radius        .push_back( val1 );
               sc->c0.concentration .push_back( val2 );
            }
         }

         f.close();
         pb_load_c0->setIcon( check );
      }
      else
      {
         QMessageBox::warning( this,
               tr( "UltraScan Warning" ),
               tr( "UltraScan could not open the file specified\n" ) + fn );
      }
   }
}

void US_ConstraintsEdit::set_molar( void )
{
   int row = lw_comps->currentRow();
   if ( row < 0 ) return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   double extinction = le_extinction->text().toDouble();
   double signalConc = le_val_conc  ->text().toDouble();

   if ( extinction > 0.0 )
      sc->molar_concentration = signalConc / extinction;
   else
      sc->molar_concentration = 0.0;
}

void US_ConstraintsEdit::clear_guid( void )
{
}

void US_ConstraintsEdit::save_changes( int /*row*/ )
{
}

void US_ConstraintsEdit::acceptProp( void ) 
{
   accept();
   close();
}

void US_ConstraintsEdit::checkbox( void )
{
   if ( countChecks() != 2 )
   {
      enable( le_val_mw , false, gray );
      enable( le_val_s  , false, gray );
      enable( le_val_D  , false, gray );
      enable( le_val_f  , false, gray );
      enable( le_val_ff0, false, gray );
      return;
   }

   ( ck_sel_mw  ->isChecked() ) ? enable( le_val_mw,  false, normal ) 
                                : enable( le_val_mw,  true , gray   );
   
   ( ck_sel_s   ->isChecked() ) ? enable( le_val_s,   false, normal ) 
                                : enable( le_val_s,   true , gray   );
   
   ( ck_sel_D   ->isChecked() ) ? enable( le_val_D,   false, normal ) 
                                : enable( le_val_D,   true , gray   );
   
   ( ck_sel_f   ->isChecked() ) ? enable( le_val_f,   false, normal ) 
                                : enable( le_val_f,   true , gray   );
   
   ( ck_sel_ff0 ->isChecked() ) ? enable( le_val_ff0, false, normal ) 
                                : enable( le_val_ff0, true , gray   );
}

void US_ConstraintsEdit::enable( QLineEdit* le, bool status, const QPalette& /*p*/ )
{
   us_setReadOnly( le, status );
   //le->setReadOnly( status );
   //le->setPalette ( p );
}

int US_ConstraintsEdit::countChecks( void )
{
   int checked = 0;
   if ( ck_sel_mw  ->isChecked() ) checked++;
   if ( ck_sel_s   ->isChecked() ) checked++;
   if ( ck_sel_D   ->isChecked() ) checked++;
   if ( ck_sel_f   ->isChecked() ) checked++;
   if ( ck_sel_ff0 ->isChecked() ) checked++;
   return checked;
}

void US_ConstraintsEdit::setInvalid( void )
{
}

void US_ConstraintsEdit::co_sed( int new_state )
{
   if ( inUpdate ) return;

   if ( new_state == Qt::Checked )
   {
      int row = lw_comps->currentRow();

      if ( model.coSedSolute != -1 )
      {
         int response = QMessageBox::question( this,
            tr( "Change co-sedimenting solute?" ),
            tr( "Another component is marked as the co-sedimenting solute.\n"
                "Change it to the current analyte?" ),
            QMessageBox::Yes, QMessageBox::No );

         if ( response == QMessageBox::No )
         {
             ck_co_sed->disconnect();
             ck_co_sed->setChecked( false );
             connect( ck_co_sed, SIGNAL( stateChanged( int ) ), 
                                 SLOT  ( co_sed      ( int ) ) );
             return;
         }
      }
      model.coSedSolute = row;
   }
   else
      model.coSedSolute = -1;
}

bool US_ConstraintsEdit::keep_standard( void )
{

   int response = QMessageBox::question( this,
         tr( "Changing Standard Value" ),
         tr( "You are changing a value that does not correspond\n" 
             "with a saved analyte.\n\n"
             "Continue?" ),
         QMessageBox::Yes, QMessageBox::No );

   if ( response == QMessageBox::Yes )
   {
      //le_wavelength->clear();
      analyte.extinction  .clear();
      analyte.refraction  .clear();
      analyte.fluorescence.clear();
      return false;
   }

   return true;
}

void US_ConstraintsEdit::calculate( void )
{
   int    row  = lw_comps->currentRow();

   if ( inUpdate  ||  row < 0 )
      return;

   US_Model::SimulationComponent* sc = &model.components[ row ];

   checkbox();

   // First do some sanity checking
   double vbar = le_val_vbar->text().toDouble();

   if ( row < 0  ||  vbar <= 0.0 )
      return;

   // Exactly two checkboxes must be set
   if ( countChecks() < 2 )
   {
      ck_sel_mw  ->setEnabled( true );
      ck_sel_ff0 ->setEnabled( true );
      ck_sel_f   ->setEnabled( true );
      ck_sel_s   ->setEnabled( true );
      ck_sel_D   ->setEnabled( true );
      return;
   }

   // disable all check boxes except for the two set
   ck_sel_mw  ->setEnabled( ck_sel_mw  ->isChecked() );
   ck_sel_ff0 ->setEnabled( ck_sel_ff0 ->isChecked() );
   ck_sel_f   ->setEnabled( ck_sel_f   ->isChecked() );
   ck_sel_s   ->setEnabled( ck_sel_s   ->isChecked() );
   ck_sel_D   ->setEnabled( ck_sel_D   ->isChecked() );
   
   // set values for checked boxes; clear others
   sc->mw   = ck_sel_mw  ->isChecked() ? le_val_mw ->text().toDouble() :
                         ( chgStoi ? sc->mw : 0.0 );
   sc->f_f0 = ck_sel_ff0 ->isChecked() ? le_val_ff0->text().toDouble() : 0.0;
   sc->f    = ck_sel_f   ->isChecked() ? le_val_f  ->text().toDouble() : 0.0;
   sc->s    = ck_sel_s   ->isChecked() ? le_val_s  ->text().toDouble() : 0.0;
   sc->D    = ck_sel_D   ->isChecked() ? le_val_D  ->text().toDouble() : 0.0;

   // re-calculate coefficients based on the two that are set
   US_Model::calc_coefficients( model.components[ row ] );

   // fill in text boxes with given and calculated coefficients
   le_val_mw ->setText( QString::number( sc->mw  , 'e', 3 ) );
   le_val_ff0->setText( QString::number( sc->f_f0, 'e', 3 ) );
   le_val_s  ->setText( QString::number( sc->s   , 'e', 4 ) );
   le_val_D  ->setText( QString::number( sc->D   , 'e', 4 ) );
   le_val_f  ->setText( QString::number( sc->f   , 'e', 4 ) );
}

void US_ConstraintsEdit::source_changed( bool db )
{
   emit use_db( db );  // Just pass on the signal
   qApp->processEvents();
}

void US_ConstraintsEdit::float_vbar( bool floats )
{
DbgLv(1) << "cnG:float_vbar floats" << floats;
   float_par( floats, le_val_vbar, le_min_vbar, le_max_vbar );
}

void US_ConstraintsEdit::check_mw  ( bool checked )
{
DbgLv(1) << "cnG:check_mw   checked" << checked;
   ck_flt_mw  ->setEnabled( checked );
   ck_log_mw  ->setEnabled( checked );
}

void US_ConstraintsEdit::check_ff0 ( bool checked )
{
DbgLv(1) << "cnG:check_ff0  checked" << checked;
   ck_flt_ff0 ->setEnabled( checked );
}

void US_ConstraintsEdit::check_s   ( bool checked )
{
DbgLv(1) << "cnG:check_s    checked" << checked;
   ck_flt_s   ->setEnabled( checked );
}

void US_ConstraintsEdit::check_D   ( bool checked )
{
DbgLv(1) << "cnG:check_D    checked" << checked;
   ck_flt_D   ->setEnabled( checked );
}

void US_ConstraintsEdit::check_f   ( bool checked )
{
DbgLv(1) << "cnG:check_f    checked" << checked;
   ck_flt_f   ->setEnabled( checked );
}

void US_ConstraintsEdit::float_mw  ( bool floats )
{
DbgLv(1) << "cnG:float_mw   floats" << floats;
   float_par( floats, le_val_mw, le_min_mw, le_max_mw );
}

void US_ConstraintsEdit::float_ff0 ( bool floats )
{
DbgLv(1) << "cnG:float_ff0  floats" << floats;
   float_par( floats, le_val_ff0, le_min_ff0, le_max_ff0 );
}

void US_ConstraintsEdit::float_s   ( bool floats )
{
DbgLv(1) << "cnG:float_s    floats" << floats;
   float_par( floats, le_val_s, le_min_s, le_max_s );
}

void US_ConstraintsEdit::float_D   ( bool floats )
{
DbgLv(1) << "cnG:float_D    floats" << floats;
   float_par( floats, le_val_D, le_min_D, le_max_D );
}

void US_ConstraintsEdit::float_f   ( bool floats )
{
DbgLv(1) << "cnG:float_f    floats" << floats;
   float_par( floats, le_val_f, le_min_f, le_max_f );
}

void US_ConstraintsEdit::float_conc( bool floats )
{
DbgLv(1) << "cnG:float_conc floats" << floats;
   float_par( floats, le_val_conc, le_min_conc, le_max_conc );
}

void US_ConstraintsEdit::float_kd  ( bool floats )
{
DbgLv(1) << "cnG:float_kd   floats" << floats;
   float_par( floats, le_val_kd, le_min_kd, le_max_kd );
}

void US_ConstraintsEdit::float_koff( bool floats )
{
DbgLv(1) << "cnG:float_koff floats" << floats;
   float_par( floats, le_val_koff, le_min_koff, le_max_koff );
}

void US_ConstraintsEdit::logsc_mw  ( bool logscl )
{
DbgLv(1) << "cnG:logsc_mw   logscl" << logscl;
}

void US_ConstraintsEdit::logsc_kd  ( bool logscl )
{
DbgLv(1) << "cnG:logsc_kd   logscl" << logscl;
}
void US_ConstraintsEdit::logsc_koff( bool logscl )
{
DbgLv(1) << "cnG:logsc_koff logscl" << logscl;
}

// Utility to connect/disconnect component slots
void US_ConstraintsEdit::comps_connect( bool c_on )
{
   if ( c_on )
   {  // Turn connections on
      connect( ck_sel_vbar,    SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( check_vbar( bool ) ) );
      connect( ck_flt_vbar,    SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_vbar( bool ) ) );
      connect( ck_sel_mw,      SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( check_mw  ( bool ) ) );
      connect( ck_flt_mw,      SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_mw  ( bool ) ) );
      connect( ck_log_mw,      SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( logsc_mw  ( bool ) ) );
      connect( ck_sel_ff0,     SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( check_ff0 ( bool ) ) );
      connect( ck_flt_ff0,     SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_ff0 ( bool ) ) );
      connect( ck_sel_s,       SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( check_s   ( bool ) ) );
      connect( ck_flt_s,       SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_s   ( bool ) ) );
      connect( ck_sel_D,       SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( check_D   ( bool ) ) );
      connect( ck_flt_D,       SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_D   ( bool ) ) );
      connect( ck_sel_f,       SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( check_f   ( bool ) ) );
      connect( ck_flt_f,       SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_f   ( bool ) ) );
      connect( ck_flt_conc,    SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_conc( bool ) ) );
   }
   else
   {  // Turn connections off
      ck_sel_vbar->disconnect();
      ck_flt_vbar->disconnect();
      ck_sel_mw  ->disconnect();
      ck_flt_mw  ->disconnect();
      ck_log_mw  ->disconnect();
      ck_sel_ff0 ->disconnect();
      ck_flt_ff0 ->disconnect();
      ck_sel_s   ->disconnect();
      ck_flt_s   ->disconnect();
      ck_sel_D   ->disconnect();
      ck_flt_D   ->disconnect();
      ck_sel_f   ->disconnect();
      ck_flt_f   ->disconnect();
      ck_sel_conc->disconnect();
      ck_flt_conc->disconnect();
   }
}

// Utility to connect/disconnect component slots
void US_ConstraintsEdit::assocs_connect( bool c_on )
{
   if ( c_on )
   {  // Turn connections on
      connect( ck_flt_kd,      SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_kd  ( bool ) ) );
      connect( ck_log_kd,      SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( logsc_kd  ( bool ) ) );
      connect( ck_flt_koff,    SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( float_koff( bool ) ) );
      connect( ck_log_koff,    SIGNAL( toggled   ( bool ) ), 
                               SLOT  ( logsc_koff( bool ) ) );
   }
   else
   {  // Turn connections off
      ck_flt_kd  ->disconnect();
      ck_log_kd  ->disconnect();
      ck_flt_koff->disconnect();
      ck_log_koff->disconnect();
   }
}

// Utility to count the selected main component attributes
int US_ConstraintsEdit::count_checks()
{
   int nchecks   = ( ck_sel_mw ->isChecked() ? 1 : 0 )
                 + ( ck_sel_ff0->isChecked() ? 1 : 0 )
                 + ( ck_sel_s  ->isChecked() ? 1 : 0 )
                 + ( ck_sel_D  ->isChecked() ? 1 : 0 )
                 + ( ck_sel_f  ->isChecked() ? 1 : 0 );
   return nchecks;
}

// Slot to populate component attribute values after component change
void US_ConstraintsEdit::component_select( int srow )
{
DbgLv(1) << "cnG: component_select  row" << srow << crow;
   const QString notapl = tr( "n/a" );
   QVector< US_dmGA_Constraints::Constraint > cnsv;

   if ( srow < 0 )  return;

   if ( srow != crow  &&  crow >= 0 )
   {  // Save settings from previous component screen
      save_comp_settings( crow, cnsv );

DbgLv(1) << "cnG:   update_constraints call";
      constraints.update_constraints( cnsv );
DbgLv(1) << "cnG:   update_constraints  rtn";
   }

   // Get constraints for new component
   crow     = srow;
DbgLv(1) << "cnG:   comp_constraints call";
   constraints.comp_constraints( crow, &cnsv, NULL );
DbgLv(1) << "cnG:   comp_constraints  rtn";
   //QListWidgetItem* item = lw_comps->item( crow );

   // Initialize component attribute GUI elements
   ck_sel_vbar ->setChecked( false );
   ck_sel_mw   ->setChecked( false );
   ck_sel_ff0  ->setChecked( false );
   ck_sel_s    ->setChecked( false );
   ck_sel_D    ->setChecked( false );
   ck_sel_f    ->setChecked( false );
   us_setReadOnly( le_val_vbar, true );
   us_setReadOnly( le_min_vbar, true );
   us_setReadOnly( le_max_vbar, true );
   us_setReadOnly( le_val_mw,   true );
   us_setReadOnly( le_min_mw,   true );
   us_setReadOnly( le_max_mw,   true );
   us_setReadOnly( le_val_ff0,  true );
   us_setReadOnly( le_min_ff0,  true );
   us_setReadOnly( le_max_ff0,  true );
   us_setReadOnly( le_val_s,    true );
   us_setReadOnly( le_min_s,    true );
   us_setReadOnly( le_max_s,    true );
   us_setReadOnly( le_val_D,    true );
   us_setReadOnly( le_min_D,    true );
   us_setReadOnly( le_max_D,    true );
   us_setReadOnly( le_val_f,    true );
   us_setReadOnly( le_min_f,    true );
   us_setReadOnly( le_max_f,    true );
   us_setReadOnly( le_val_conc, true );
   us_setReadOnly( le_min_conc, true );
   us_setReadOnly( le_max_conc, true );
   ck_flt_vbar->setEnabled( false );
   ck_flt_mw  ->setEnabled( false );
   ck_flt_ff0 ->setEnabled( false );
   ck_flt_s   ->setEnabled( false );
   ck_flt_D   ->setEnabled( false );
   ck_flt_f   ->setEnabled( false );
   ck_flt_conc->setEnabled( false );

   // Populate component attribute values
   for ( int ii = 0; ii < cnsv.size(); ii++ )
   {
      bool floats  = cnsv[ ii ].floats;

DbgLv(1) << "cnG:cmp_sel: ii atype" << ii << cnsv[ii].atype;
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_VBAR )
      {
         ck_sel_vbar->setChecked( true );
         check_value( cnsv[ ii ], le_val_vbar, le_min_vbar, le_max_vbar );
         ck_flt_vbar->setChecked( floats );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_MW )
      {
         ck_sel_mw  ->setChecked( true );
         check_value( cnsv[ ii ], le_val_mw, le_min_mw, le_max_mw );
         ck_flt_mw  ->setChecked( floats );
         ck_log_mw  ->setChecked( cnsv[ ii ].logscl );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_FF0  )
      {
         ck_sel_ff0 ->setChecked( true );
         check_value( cnsv[ ii ], le_val_ff0, le_min_ff0, le_max_ff0 );
         ck_flt_ff0 ->setChecked( floats );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_S    )
      {
         ck_sel_s   ->setChecked( true );
         check_value( cnsv[ ii ], le_val_s, le_min_s, le_max_s );
         ck_flt_s   ->setChecked( floats );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_D    )
      {
         ck_sel_D   ->setChecked( true );
         check_value( cnsv[ ii ], le_val_D, le_min_D, le_max_D );
         ck_flt_D ->setChecked( floats );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_F    )
      {
         ck_sel_f   ->setChecked( true );
         check_value( cnsv[ ii ], le_val_f, le_min_f, le_max_f );
         ck_flt_f   ->setChecked( floats );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_CONC )
      {
         check_value( cnsv[ ii ], le_val_conc, le_min_conc, le_max_conc );
         ck_flt_conc->setChecked( floats );
      }
      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_EXT  )
      {
         le_extinction->setText( QString::number( cnsv[ ii ].low  ) );
      }
   }
}

// Slot to populate association attribute values after association change
void US_ConstraintsEdit::association_select( int srow )
{
DbgLv(1) << "cnG: association_select  row" << srow << arow;
   QVector< US_dmGA_Constraints::Constraint > cnsv;
   const QString notapl = tr( "n/a" );

   if ( srow < 0 )  return;

   if ( srow != arow  &&  arow >= 0 )
   {  // Save settings from previous component screen
      save_assoc_settings( arow, cnsv );

      constraints.update_constraints( cnsv );
   }

   // Get constraints for new association
   arow     = srow;
   constraints.assoc_constraints( arow, &cnsv, NULL );

   // Populate association attribute values
   ck_flt_kd  ->setChecked( false );
   ck_flt_koff->setChecked( false );

   for ( int ii = 0; ii < cnsv.size(); ii++ )
   {
      bool floats = cnsv[ ii ].floats;
      bool logscl = cnsv[ ii ].logscl;

      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_KD   )
      {
         check_value( cnsv[ ii ], le_val_kd, le_min_kd, le_max_kd );
         ck_flt_kd  ->setChecked( floats );
         ck_log_kd  ->setChecked( logscl );
      }

      if ( cnsv[ ii ].atype == US_dmGA_Constraints::ATYPE_KOFF )
      {
         check_value( cnsv[ ii ], le_val_koff, le_min_koff, le_max_koff );
         ck_flt_koff->setChecked( floats );
         ck_log_koff->setChecked( logscl );
      }
   }
}

// Internal function to save current page's component settings
void US_ConstraintsEdit::save_comp_settings( int crow,
      QVector< US_dmGA_Constraints::Constraint >& cnsv )
{
DbgLv(1) << "cnG: svcs: save_comp_settings  crow" << crow;
   US_dmGA_Constraints::Constraint cnse;
   cnse.mcompx      = crow;
   bool floats      = false;
   int  kselect     = 0;

   if ( ck_sel_s   ->isChecked() )
   {
      floats           = ck_flt_s   ->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_S;
      cnse.low         = le_val_s   ->text().toDouble();
      cnse.high        = floats ? le_max_s   ->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = false;
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "S";
   }
   if ( ck_sel_ff0 ->isChecked() )
   {
      floats           = ck_flt_ff0 ->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_FF0;
      cnse.low         = le_val_ff0 ->text().toDouble();
      cnse.high        = floats ? le_max_ff0 ->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = false;
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "FF0";
   }
   if ( ck_sel_mw  ->isChecked() )
   {
      floats           = ck_flt_mw  ->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_MW;
      cnse.low         = le_val_mw  ->text().toDouble();
      cnse.high        = floats ? le_max_mw  ->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = ck_log_mw  ->isChecked();
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "MW";
   }
   if ( ck_sel_D   ->isChecked() )
   {
      floats           = ck_flt_D   ->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_D;
      cnse.low         = le_val_D   ->text().toDouble();
      cnse.high        = floats ? le_max_D   ->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = false;
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "D";
   }
   if ( ck_sel_f   ->isChecked() )
   {
      floats           = ck_flt_f   ->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_F;
      cnse.low         = le_val_f   ->text().toDouble();
      cnse.high        = floats ? le_max_f   ->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = false;
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "F";
   }
   if ( ck_sel_vbar->isChecked() )
   {
      floats           = ck_flt_vbar->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_VBAR;
      cnse.low         = le_val_vbar->text().toDouble();
      cnse.high        = floats ? le_max_vbar->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = false;
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "VBAR";
   }
   if ( ck_sel_conc->isChecked() )
   {
      floats           = ck_flt_conc->isChecked();
      cnse.atype       = US_dmGA_Constraints::ATYPE_CONC;
      cnse.low         = le_val_conc->text().toDouble();
      cnse.high        = floats ? le_max_conc->text().toDouble() : cnse.low;
      cnse.floats      = floats;
      cnse.logscl      = false;
      cnsv << cnse;
      kselect++;
DbgLv(1) << "cnG: svcs:  ks" << kselect << "CONC";
   }

   // There should be a total of 4 checked
   if ( kselect != 4 )
   {
      qDebug() << "*ERROR* dmGA_Init: component" << cnse.mcompx + 1
               << "has" << kselect << "attributes selected (SB 4)";
   }
DbgLv(1) << "cnG: svcs: cnsv size" << cnsv.size();
}

// Internal function to save current page's association settings
void US_ConstraintsEdit::save_assoc_settings( int arow,
      QVector< US_dmGA_Constraints::Constraint >& cnsv )
{
   US_dmGA_Constraints::Constraint cnse;
   bool floats      = ck_flt_kd  ->isChecked();
   cnse.mcompx      = arow;
   cnse.atype       = US_dmGA_Constraints::ATYPE_KD;
   cnse.low         = le_val_kd  ->text().toDouble();
   cnse.high        = floats ? le_max_kd  ->text().toDouble() : cnse.low;
   cnse.floats      = floats;
   cnse.logscl      = floats;
   cnsv << cnse;

   floats           = ck_flt_kd  ->isChecked();
   cnse.atype       = US_dmGA_Constraints::ATYPE_KOFF;
   cnse.low         = le_val_koff->text().toDouble();
   cnse.high        = floats ? le_max_koff->text().toDouble() : cnse.low;
   cnse.floats      = floats;
   cnse.logscl      = floats;
   cnsv << cnse;
}

void US_ConstraintsEdit::float_par( bool floats,
      QLineEdit* le_val, QLineEdit* le_min, QLineEdit* le_max )
{
   const QString notapl = tr( "n/a" );
   bool fixed  = !floats;

DbgLv(1) << "cnG:float_par floats" << floats;
   if ( floats )
   {  // Changed to float:  values for low,high
      double valu  = le_val->text().toDouble();
DbgLv(1) << "cnG:float_par valu" << valu;
      double vmin  = valu * 0.9;
      double vmax  = valu * 1.1;
      le_val->setText( notapl );
      le_min->setText( QString::number( vmin ) );
      le_max->setText( QString::number( vmax ) );
   }

   else
   {  // Changed to fixed:  value only
      double vmin  = le_min->text().toDouble();
      double vmax  = le_max->text().toDouble();
DbgLv(1) << "cnG:float_par vmin vmax" << vmin << vmax;
      double valu  = ( vmin + vmax ) * 0.5;
      le_val->setText( QString::number( valu ) );
      le_min->setText( notapl );
      le_max->setText( notapl );
   }

   le_val->setEnabled( fixed  );
   le_min->setEnabled( floats );
   le_max->setEnabled( floats );
   us_setReadOnly( le_val, floats );
   us_setReadOnly( le_min, fixed  );
   us_setReadOnly( le_max, fixed  );
}

void US_ConstraintsEdit::check_value(
      const US_dmGA_Constraints::Constraint cnse,
      QLineEdit* le_val, QLineEdit* le_min, QLineEdit* le_max  )
{
   const QString notapl = tr( "n/a" );
   bool floats  = cnse.floats;
   bool fixed   = !floats;
DbgLv(1) << "cnG:check_value: floats" << floats;

   le_val->setText( fixed  ? QString::number( cnse.low  ) : notapl );
   le_min->setText( floats ? QString::number( cnse.low  ) : notapl );
   le_max->setText( floats ? QString::number( cnse.high ) : notapl );

   us_setReadOnly( le_val, floats );
   us_setReadOnly( le_min, fixed  );
   us_setReadOnly( le_max, fixed  );
}


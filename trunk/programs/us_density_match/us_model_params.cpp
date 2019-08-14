//! \file us_model_params.cpp

#include "us_model_params.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#define DbgLv(a) if(dbg_level>=a)qDebug()

// Constructor:  remove-distributions dialog widget
US_ModelParams::US_ModelParams( QVector< DisSys >& adistros,
    QWidget* p ) : US_WidgetsDialog( p, 0 ), distros( adistros )
{
   dbg_level       = US_Settings::us_debug();
   mainLayout      = new QGridLayout( this );
   setObjectName( "US_ModelParams" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // Lay out the GUI
   setWindowTitle( tr( "Distribution Parameters" ) );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_mtitle = us_banner( tr( "Modify Model D2O Percent, Density,"
                                      " Label" ) );

   pb_help      = us_pushbutton( tr( "Help"    ) );
   pb_cancel    = us_pushbutton( tr( "Cancel"  ) );
   pb_accept    = us_pushbutton( tr( "Accept"  ) );
   pb_compute   = us_pushbutton( tr( "Compute Densities" ) );
   pb_accept ->setEnabled( false );
   pb_compute->setEnabled( false );

   // Build lists of model parameters
   nd_orig      = distros.count();
   int maxdlen  = 0;

   for ( int jj = 0; jj < nd_orig; jj++ )
   {
      QString runid  = distros[ jj ].run_name;
      QString analy  = distros[ jj ].analys_name;
      QString method = distros[ jj ].method;
      QString mlabel = distros[ jj ].label;
      QString edid   = analy.section( "_",  0, -4 );
      QString anid   = analy.section( "_", -3, -3 );
      QString iter   = analy.section( "_", -2, -1 );
      QString mdesc  = runid + "." + edid + "_" + anid + "_" + method
         + "_" + iter;

      mdescrs  << mdesc;
      d2opcts  << distros[ jj ].d2opct;
      bdensts  << distros[ jj ].bdensity;
      modelids << tr( "Model %1" ).arg( ( jj + 1 ) );
      mlabels  << mlabel;
//      labels   << QLabel( mlabel );

      maxdlen        = qMax( maxdlen, mdesc.length() );
   }

   // Fill in the bulk of GUI elements, including model table
   int row      = 0;
   mainLayout ->addWidget( lb_mtitle,  row++, 0, 1, 8 );
   row         += 9;
   QLabel* lb_hdr1     = us_banner( tr( "Model ndx" ) );
   QLabel* lb_hdr2     = us_banner( tr( "D2O Percent" ) );
   QLabel* lb_hdr3     = us_banner( tr( "Density (g/l)" ) );
   QLabel* lb_hdr4     = us_banner( tr( "Label" ) );
   QLabel* lb_hdr5     = us_banner( tr( "Description" ) );
   mainLayout->addWidget( lb_hdr1, row,   0, 1, 1 );
   mainLayout->addWidget( lb_hdr2, row,   1, 1, 1 );
   mainLayout->addWidget( lb_hdr3, row,   2, 1, 1 );
   mainLayout->addWidget( lb_hdr4, row,   3, 1, 2 );
   mainLayout->addWidget( lb_hdr5, row++, 5, 1, 3 );
   for ( int jj = 0; jj < nd_orig; jj++ )
   {
      QString modelid    = modelids[ jj ];
      double d2opc       = d2opcts[ jj ];
      double bdens       = bdensts[ jj ];
      QString sd2opc     = ( d2opc < 0.0 ) ? "" : QString::number( d2opc );
      QString sbdens     = QString::number( bdens );
      QString mlabel     = mlabels [ jj ];
      QString mdesc      = mdescrs [ jj ];
      int kk             = mdesc.indexOf( "-run" );
      mdesc              = ( kk > 0 ) ? QString( mdesc ).mid( ( kk + 1 ) )
                                      : mdesc;

      QLabel* lb_mdli    = us_label  ( modelid );
      QLineEdit* le_d2op = us_lineedit( sd2opc );
      QLineEdit* le_dens = us_lineedit( sbdens );
      QLineEdit* le_mlab = us_lineedit( mlabel );
      QLineEdit* le_mdsc = us_lineedit( mdesc  );
      us_setReadOnly( le_mdsc, true );

      QString rowx       = QString::number( jj );
      le_d2op->setObjectName( "D2OP:" + rowx );
      le_dens->setObjectName( "DENS:" + rowx );
      le_mlab->setObjectName( "MLAB:" + rowx );

      mainLayout->addWidget( lb_mdli, row,   0, 1, 1 );
      mainLayout->addWidget( le_d2op, row,   1, 1, 1 );
      mainLayout->addWidget( le_dens, row,   2, 1, 1 );
      mainLayout->addWidget( le_mlab, row,   3, 1, 2 );
      mainLayout->addWidget( le_mdsc, row++, 5, 1, 3 );

      connect( le_d2op,    SIGNAL( textChanged( const QString& ) ),
               this,       SLOT(   lnedChanged( const QString& ) ) );
      connect( le_dens,    SIGNAL( textChanged( const QString& ) ),
               this,       SLOT(   lnedChanged( const QString& ) ) );
      connect( le_mlab,    SIGNAL( textChanged( const QString& ) ),
               this,       SLOT(   lnedChanged( const QString& ) ) );
   }
DbgLv(1) << "MP:main: model rows populated";

   mainLayout ->addWidget( pb_help,    row,   0, 1, 1 );
   mainLayout ->addWidget( pb_cancel,  row,   1, 1, 2 );
   mainLayout ->addWidget( pb_accept,  row,   3, 1, 2 );
   mainLayout ->addWidget( pb_compute, row++, 6, 1, 3 );
   row         += 2;

   mainLayout->setColumnStretch( 0, 2 );
   mainLayout->setColumnStretch( 1, 2 );
   mainLayout->setColumnStretch( 2, 2 );
   mainLayout->setColumnStretch( 3, 3 );
   mainLayout->setColumnStretch( 4, 3 );
   mainLayout->setColumnStretch( 5, 4 );
   mainLayout->setColumnStretch( 6, 4 );
   mainLayout->setColumnStretch( 7, 4 );

   connect( pb_help,    SIGNAL( clicked()  ),
            this,       SLOT(   help()     ) );
   connect( pb_cancel,  SIGNAL( clicked()  ),
            this,       SLOT(   canceled() ) );
   connect( pb_accept,  SIGNAL( clicked()  ),
            this,       SLOT(   accepted() ) );
   connect( pb_compute, SIGNAL( clicked()  ),
            this,       SLOT(   compute_densities() ) );
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
//   int fhigh = fm.lineSpacing();
//   int lhigh = fhigh * 10 + 12;
   int fwide = fm.width( QChar( '6' ) );
   int lwide = fwide * ( maxdlen + 2 );
DbgLv(1) << "MP:main: fwide lwide" << fwide << lwide;

   adjustSize();
   int wwide = qMax( 500, lwide );
   int whigh = size().height();
   resize( wwide, whigh );
   qApp->processEvents();
DbgLv(1) << "MP:main: wwide whigh" << wwide << whigh;
}

// Private slot to do the actual removal of distributions and close
void US_ModelParams::accepted()
{
DbgLv(1) << "accepted";
   QList< QObject* > ochilds  = this->children();
DbgLv(1) << " acc: ochilds size" << ochilds.size();
   for ( int jj = 0; jj < ochilds.size(); jj++ )
   {
      QObject* ochild = ochilds[ jj ];
      QLineEdit* lned = (QLineEdit*) ochild;
      QString cname   = ochild->objectName();
      QString pname   = cname.section( ":", 0, 0 );
      int rowx        = cname.section( ":", 1, 1 ).toInt();
      if      ( pname == "D2OP" )
      {
         distros[ rowx ].d2opct   = lned->text().toDouble();
DbgLv(1) << " acc:  jj rowx" << jj << rowx << "pname value" << pname
 << lned->text().toDouble();
      }
      else if ( pname == "DENS" )
      {
         distros[ rowx ].bdensity = lned->text().toDouble();
DbgLv(1) << " acc:  jj rowx" << jj << rowx << "pname value" << pname
 << lned->text().toDouble();
      }
      else if ( pname == "MLAB" )
      {
         distros[ rowx ].label    = lned->text();
DbgLv(1) << " acc:  jj rowx" << jj << rowx << "pname value" << pname
 << lned->text();
      }
   }

   accept();
}

// Private slot to do the actual removal of distributions and close
void US_ModelParams::canceled()
{
DbgLv(1) << "canceled";
   reject();
}

void US_ModelParams::compute_densities()
{
   const double dnslope = 0.00108766;
   QList< QObject* > ochilds  = this->children();
   int nzd2pc     = 0;
   int npd2pc     = 0;
   bool misspc    = false;
   double d2opct  = -1.0;
   double denszpc = 0.0;
   double density = 0.0;
DbgLv(1) << " cdn: ochilds size" << ochilds.size();
   for ( int jj = 0; jj < ochilds.size(); jj++ )
   {
      QObject* ochild = ochilds[ jj ];
      QLineEdit* lned = (QLineEdit*) ochild;
      QString cname   = ochild->objectName();
      QString pname   = cname.section( ":", 0, 0 );
      if      ( pname == "D2OP" )
      {
         d2opct          = lned->text().toDouble();
         if ( d2opct == 0.0 )
         {
            nzd2pc++;
            if ( npd2pc > 0 )
               misspc         = true;
         }
         else
         {
            npd2pc++;
            if ( nzd2pc == 0 )
               misspc         = true;
         }
      }
      else if ( pname == "DENS" )
      {
         density         = lned->text().toDouble();
         if ( d2opct == 0.0 )
         {
            denszpc        = density;
         }
         else
         {
            density        = denszpc + d2opct * dnslope;
            lned->setText( QString::number( density ) );
         }
      }
   }
DbgLv(1) << " cdn: misspc" << misspc;
}

bool US_ModelParams::values_filled()
{
   QList< QObject* > ochilds  = this->children();
   int nzd2pc     = 0;
   int nad2pc     = 0;
   int nadens     = 0;
   int nalabs     = 0;
   QList< double > allden;
DbgLv(1) << " vlf: ochilds size" << ochilds.size();

   for ( int jj = 0; jj < ochilds.size(); jj++ )
   {
      QObject* ochild = ochilds[ jj ];
      QString cname   = ochild->objectName();
      if ( cname.isEmpty() )  continue;

      QLineEdit* lned = (QLineEdit*) ochild;
      QString pname   = cname.section( ":", 0, 0 );
      QString etext   = lned->text();
DbgLv(1) << " vlf:  jj" << jj << "etext" << etext << "pname" << pname;

      if      ( pname == "D2OP" )
      {  // Count D2O percent values given
         if ( !etext.isEmpty() )
         {
            int pctval      = etext.toInt();
            if ( etext == "0" )
            {  // Zero percent:  bump counts
               nzd2pc++;
               nad2pc++;
DbgLv(1) << " vlf:    pctval" << pctval << "nzd2pc nad2pc" << nzd2pc << nad2pc;
            }
            else if ( pctval != 0 )
            {  // Non-zero numeric: bump percent count
               nad2pc++;
DbgLv(1) << " vlf:    pctval" << pctval << "nad2pc" << nad2pc;
            }
         }
      }
      else if ( pname == "DENS" )
      {  // Count densities given
         if ( !etext.isEmpty() )
         {
            double density  = etext.toDouble();
            if ( density > 0.0 )
            {  // Bump if non-empty, numeric; save unique values
               nadens++;
               if ( !allden.contains( density ) )
                  allden << density;
DbgLv(1) << " vlf:    density" << density << "nadens" << nadens;
            }
         }
      }
      else if ( pname == "MLAB" )
      {  // Count labels given
         if ( !etext.isEmpty() )
            nalabs++;
DbgLv(1) << " vlf:    nalabs" << nalabs;
      }
   }
DbgLv(1) << " vlf: nzd2pc nad2pc nadens nalabs" << nzd2pc << nad2pc << nadens << nalabs
 << "allden size" << allden.size();

   // All model parameters are given if
   //  at least one zero-percent is given;
   //  percent,density,label counts equal row count;
   //  and number of unique densities equals row count.
   bool filled    = ( ( nzd2pc > 0 )  &&
                      ( nad2pc == nd_orig )  &&
                      ( nadens == nd_orig )  &&
                      ( nalabs == nd_orig )  &&
                      ( allden.size() == nadens ) );
DbgLv(1) << " vlf:   FILLED" << filled;
   pb_accept ->setEnabled( filled );  // If all filled, enable Accept button

   return filled;
}

// Return flag of whether all D2O percents have been given
bool US_ModelParams::all_percents()
{
   QList< QObject* > ochilds  = this->children();
   int nzd2pc     = 0;
   int nad2pc     = 0;
DbgLv(1) << " apc: ochilds size" << ochilds.size();

   for ( int jj = 0; jj < ochilds.size(); jj++ )
   {  // Examine any D2O percent texts
      QObject* ochild = ochilds[ jj ];
      QString cname   = ochild->objectName();
      if ( cname.isEmpty() )  continue;

      QString pname   = cname.section( ":", 0, 0 );
      QLineEdit* lned = (QLineEdit*) ochild;
      QString etext   = lned->text();

      if      ( pname == "D2OP" )
      {
         if ( !etext.isEmpty() )
         {
            int pctval      = etext.toInt();
            if ( etext == "0" )
            {  // Count 0 percent and total percent
               nzd2pc++;
               nad2pc++;
            }
            else if ( pctval != 0 )
            {  // Count total non-empty percent values given
               nad2pc++;
            }
         }
      }
   }
DbgLv(1) << " apc: nzd2pc nad2pc" << nzd2pc << nad2pc;

   // We have all needed if 0-percent exists and all percents given
   bool all_pc    = ( ( nzd2pc > 0 )  &&
                      ( nad2pc == nd_orig ) );
DbgLv(1) << " apc:   ALL-PC" << all_pc;

   // If all percents given, enable Compute Densities button
   pb_compute->setEnabled( all_pc );

   return all_pc;
}

// Slot to handle change in lineEdit value in model rows
void US_ModelParams::lnedChanged( const QString& /*text*/ )
{
DbgLv(1) << " lnedChanged IN";
   // Determine if all boxes filled or all percents given
   //  and possibly enable "Accept" or "Compute Densities"
   bool filled    = values_filled();
   bool all_pc    = all_percents();
DbgLv(1) << " lnedChanged  filled all_pc" << filled << all_pc;
   return;
}


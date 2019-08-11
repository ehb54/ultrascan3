//! \file us_model_params.cpp

#include "us_model_params.h"
#include "us_gui_settings.h"

// Constructor:  remove-distributions dialog widget
US_ModelParams::US_ModelParams( QVector< DisSys >& adistros,
    QWidget* p ) : US_WidgetsDialog( p, 0 ), distros( adistros )
{
   setObjectName( "US_ModelParams" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // Lay out the GUI
   setWindowTitle( tr( "Distribution Parameters" ) );

   mainLayout      = new QGridLayout( this );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_mtitle = us_banner( tr( "Modify Model D2O Percent, Density,"
                                      " Label" ) );

   pb_help      = us_pushbutton( tr( "Help"    ) );
   pb_cancel    = us_pushbutton( tr( "Cancel"  ) );
   pb_accept    = us_pushbutton( tr( "Accept"  ) );
   pb_compute   = us_pushbutton( tr( "Compute Densities" ) );


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
   }

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
   int fhigh = fm.lineSpacing();
   int fwide = fm.width( QChar( '6' ) );
   int lhigh = fhigh * 10 + 12;
   int lwide = fwide * ( maxdlen + 2 );

   adjustSize();
   int wwide = qMax( 500, lwide );
   int whigh = size().height();
   resize( wwide, whigh );
   qApp->processEvents();
}

// Private slot to react to a change in selections
void US_ModelParams::d2opctChanged( int sel )
{
DbgLv(1) << "d2opctChanged:  sel" << sel;
   bool can_accept = false;
   // Enable buttons according to the present state of selection/removal
   pb_accept ->setEnabled( can_accept );
}

// Private slot to react to a change in selections
void US_ModelParams::labelChanged( int sel )
{
DbgLv(1) << "labelChanged:  sel" << sel;
}


// Private slot to do the actual removal of distributions and close
void US_ModelParams::accepted()
{
DbgLv(1) << "accepted";
#if 0
   if ( nd_selected > 0 )
   {  // Accept attempt with selections and no Remove clicked
      if ( QMessageBox::Yes == QMessageBox::warning( this,
              tr( "Outstanding Selections" ),
              tr( "You have selected distributions,\n"
                  "but did not click on the Remove button.\n"
                  "Do you want to remove the selected distributions?" ),
              QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes ) )
      {  // If "Yes" to above, add current selections to the remove list
         remove();
      }
   }

   for ( int jj = nd_orig - 1; jj >= 0; jj-- )
   {  // Remove all entries in the original that are not in the current list
      QString mdesc = mdesc_orig[ jj ];

      if ( ! mdesc_list.contains( mdesc ) )
      {  // This original list item is to be removed from the passed list
         distros.removeAt( jj );
      }
   }
#endif

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


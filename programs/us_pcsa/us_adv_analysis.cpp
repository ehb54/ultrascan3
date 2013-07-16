//! \file us_adv_analysis.cpp

#include "us_pcsa.h"
#include "us_adv_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"

// constructor:  enhanced plot control widget
US_AdvAnalysis::US_AdvAnalysis( QVector< ModelRecord >* p_mrs, const int nth,
    US_SolveSim::DataSet* ds0, QWidget* p ) : US_WidgetsDialog( p, 0 )
{
   p_mrecs        = p_mrs;
   nthr           = nth;
   dset0          = ds0;
   parentw        = p;
   dbg_level      = US_Settings::us_debug();
   store_dir      = US_Settings::resultDir() + "/" + dset0->run_data.runID;
DbgLv(1) << "AA: IN";

   setObjectName( "US_AdvAnalysis" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );
   setAttribute( Qt::WA_DeleteOnClose, false );

   // lay out the GUI
   setWindowTitle( tr( "PCSA Advanced Controls" ) );

   // Define the layouts
   mainLayout      = new QHBoxLayout( this );
   finmodelLayout  = new QGridLayout( );
   mreclistLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( finmodelLayout  );
   mainLayout->addLayout( mreclistLayout  );
   mainLayout->setStretchFactor( finmodelLayout, 1 );
   mainLayout->setStretchFactor( mreclistLayout, 1 );
DbgLv(1) << "AA: define GUI elements";

   // Define banners and labels
   QLabel* lb_fitctrl   = us_banner( tr( "Fitting Controls:" ) );
   QLabel* lb_bfmstat   = us_banner( tr( "Best Final Model Status:" ) );
   QLabel* lb_mrecctrl  = us_banner( tr( "Final & Model Records Controls:" ) );
   QLabel* lb_mrecstat  = us_banner( tr( "Model Records Status:" ) );
   QLabel* lb_curvtype  = us_label ( tr( "Curve Type:" ) );
   QLabel* lb_s_lower   = us_label ( tr( "Lower Limit (s x 1e-13):" ) );
   QLabel* lb_s_upper   = us_label ( tr( "Upper Limit (s x 1e-13):" ) );
           lb_sigmpar1  = us_label ( tr( "Sigmoid Par 1:" ) );
           lb_sigmpar2  = us_label ( tr( "Sigmoid Par 2:" ) );
           lb_k_lower   = us_label ( tr( "Lower Limit (f/f0):" ) );
           lb_k_upper   = us_label ( tr( "Upper Limit (f/f0):" ) );
   QLabel* lb_crpoints  = us_label ( tr( "Curve Resolution Points:" ) );
   QLabel* lb_mciters   = us_label ( tr( "Monte Carlo Iterations:" ) );
   QLabel* lb_progress  = us_label ( tr( "Progress:" ) );
   QLabel* lb_space1    = us_banner( "" );
   QLabel* lb_space2    = us_banner( "" );

   // Define buttons
   pb_loadmrs   = us_pushbutton( tr( "Load Model Records"  ) );
   pb_storemrs  = us_pushbutton( tr( "Store Model Records" ) );
   pb_loadbfm   = us_pushbutton( tr( "Load Final Model"    ) );
   pb_storebfm  = us_pushbutton( tr( "Store Final Model"   ) );
   pb_resetbfm  = us_pushbutton( tr( "Reset Final Model"   ) );
   pb_resetmrs  = us_pushbutton( tr( "Reset Model Records" ) );
   pb_buildbfm  = us_pushbutton( tr( "Build Final Model"   ) );
   pb_mciters   = us_pushbutton( tr( "Start Monte Carlo"   ) );
   pb_help      = us_pushbutton( tr( "Help"   ) );
   pb_cancel    = us_pushbutton( tr( "Cancel" ) );
   pb_accept    = us_pushbutton( tr( "Accept" ) );

   // Define counters
   double smin  = 1.0;
   double smax  = 10.0;
   double kmin  = 1.0;
   double kmax  = 5.0;
   ct_s_lower   = us_counter( 3, -10000, 10000, smin );
   ct_s_upper   = us_counter( 3, -10000, 10000, smax );
   ct_k_lower   = us_counter( 3,      1,     8, kmin );
   ct_k_upper   = us_counter( 3,      1,   100, kmax );
   ct_sigmpar1  = us_counter( 3,  0.001,   0.5,  0.2 );
   ct_sigmpar2  = us_counter( 3,    0.0,   1.0,  0.1 );
   ct_crpoints  = us_counter( 2,     20,   501,  101 );
   ct_mciters   = us_counter( 2,      1,   100,   20 );
   ct_s_lower ->setStep(   0.1 );
   ct_s_upper ->setStep(   0.1 );
   ct_k_lower ->setStep(  0.01 );
   ct_k_upper ->setStep(  0.01 );
   ct_sigmpar1->setStep( 0.001 );
   ct_sigmpar2->setStep( 0.001 );
   ct_crpoints->setStep(     1 );
   ct_mciters ->setStep(     1 );

   // Define combo box
   cb_curvtype  = us_comboBox();
   cb_curvtype->setMaxVisibleItems( 3 );
   cb_curvtype->addItem( tr( "Straight Line"      ) );
   cb_curvtype->addItem( tr( "Increasing Sigmoid" ) );
   cb_curvtype->addItem( tr( "Decreasing Sigmoid" ) );
   cb_curvtype->setCurrentIndex( 1 );
 
   // Define status text boxes and progress bar
   te_bfmstat   = us_textedit();
   te_mrecstat  = us_textedit();
   b_progress   = us_progressBar( 0, 100, 0 );
   us_setReadOnly( te_bfmstat,  true );
   us_setReadOnly( te_mrecstat, true );

   // Lay out the left side, of BFM controls and status
DbgLv(1) << "AA: populate finmodelLayout";
   int row      = 0;
   finmodelLayout->addWidget( lb_fitctrl,    row++, 0, 1, 6 );
   finmodelLayout->addWidget( lb_curvtype,   row,   0, 1, 3 );
   finmodelLayout->addWidget( cb_curvtype,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_s_lower,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_s_lower,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_s_upper,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_s_upper,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_k_lower,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_k_lower,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_k_upper,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_k_upper,    row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_sigmpar1,   row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_sigmpar1,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_sigmpar2,   row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_sigmpar2,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_crpoints,   row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_crpoints,   row++, 3, 1, 3 );
   finmodelLayout->addWidget( lb_mciters,    row,   0, 1, 3 );
   finmodelLayout->addWidget( ct_mciters,    row++, 3, 1, 3 );

   finmodelLayout->addWidget( lb_bfmstat,    row++, 0, 1, 6 );
   finmodelLayout->addWidget( te_bfmstat,    row,   0, 8, 6 );
   row         += 8;
   finmodelLayout->addWidget( lb_space1,     row,   0, 1, 6 );

   // Lay out the right side, of Model Record controls and status
DbgLv(1) << "AA: populate mreclistLayout";
   row          = 0;
   mreclistLayout->addWidget( lb_mrecctrl,   row++, 0, 1, 6 );
   mreclistLayout->addWidget( pb_loadmrs,    row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_storemrs,   row++, 3, 1, 3 );
   mreclistLayout->addWidget( pb_loadbfm,    row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_storebfm,   row++, 3, 1, 3 );
   mreclistLayout->addWidget( pb_resetbfm,   row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_resetmrs,   row++, 3, 1, 3 );
   mreclistLayout->addWidget( pb_buildbfm,   row,   0, 1, 3 );
   mreclistLayout->addWidget( pb_mciters,    row++, 3, 1, 3 );

   mreclistLayout->addWidget( lb_mrecstat,   row++, 0, 1, 6 );
   mreclistLayout->addWidget( te_mrecstat,   row,   0, 8, 6 );
   mreclistLayout->setRowStretch( row, 2 );
   row         += 8;
   mreclistLayout->addWidget( lb_progress,   row,   0, 1, 1 );
   mreclistLayout->addWidget( b_progress,    row++, 1, 1, 5 );
   mreclistLayout->addWidget( pb_help,       row,   0, 1, 2 );
   mreclistLayout->addWidget( pb_cancel,     row,   2, 1, 2 );
   mreclistLayout->addWidget( pb_accept,     row++, 4, 1, 2 );
   mreclistLayout->addWidget( lb_space2,     row,   0, 1, 6 );

   // Set defaults and status values based on the initial model records
   mrecs        = *p_mrecs;
   nmrecs       = mrecs.size();
   mciters      = 0;
   kciters      = 0;
   bfm0_exists  = false;
   mrs0_exists  = false;
   bfm_new      = false;
   mrs_new      = false;
   mc_done      = false;
   mc_running   = false;
   ctype        = 1;
   nisols       = 0;
   ncsols       = ( nmrecs > 0 ) ? mrecs[ 0 ].csolutes.size() : 0;

   if ( ncsols > 0 )
   {  // We are starting with models already computed
      mrec         = mrecs[ 0 ];
      nisols       = mrec.isolutes.size();
      ncsols       = mrec.csolutes.size();

      for ( int js = 0; js < nisols; js++ )
      {
         smin         = qMin( smin, mrec.isolutes[ js ].s * 1.e13 );
         smax         = qMax( smax, mrec.isolutes[ js ].s * 1.e13 );
         kmin         = qMin( kmin, mrec.isolutes[ js ].k );
         kmax         = qMax( kmax, mrec.isolutes[ js ].k );
      }

      ctype        = ( mrec.par1 == mrec.str_k ) ? 0 :
                     ( ( mrec.str_k < mrec.end_k ) ? 1 : 2 );

      ct_s_lower ->setValue( smin );
      ct_s_upper ->setValue( smax );
      ct_k_lower ->setValue( kmin );
      ct_k_upper ->setValue( kmax );
      ct_sigmpar1->setValue( mrec.par1  );
      ct_sigmpar2->setValue( mrec.par2  );
      ct_crpoints->setValue( nisols );
DbgLv(1) << "AA: mr p1 p2  m0 p1 p2" << mrec.par1 << mrec.par2 << mrecs[0].par1
 << mrecs[0].par2 << "  typ ni nc" << ctype << nisols << ncsols;

      te_bfmstat ->setText(
         tr( "An initial best final model, with RMSD of %1,\n"
             "  has been read.\n" ).arg( mrec.rmsd ) );

      te_mrecstat->setText(
         tr( "An initial model records list, with %1 fits,\n"
             "  has been read." ).arg( nmrecs ) );

      mrec0        = mrec;          // Save initial model records
      mrecs0       = mrecs;
      bfm0_exists  = true;
      mrs0_exists  = true;
   }

   else
   {  // We are starting with models not yet computed
      mrec         = ( nmrecs > 0 ) ? mrecs[ 0 ] : mrec;
      mrec0        = mrec;          // Save initial model records
      mrecs0       = mrecs;
DbgLv(1) << "AA: nmrecs" << nmrecs << "ncsols" << ncsols;
      te_bfmstat ->setText(
         tr( "No initial best final model has been read" ) );
      te_mrecstat->setText(
         tr( "No initial model records list has been read" ) );
   }

   // Define connections
DbgLv(1) << "AA: connect buttons";
   connect( cb_curvtype, SIGNAL( currentIndexChanged( int ) ),
            this,        SLOT(   curvtypeChanged    ( int ) ) );

   connect( ct_s_lower,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   slowerChanged( double ) ) );
   connect( ct_s_upper,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   supperChanged( double ) ) );
   connect( ct_k_lower,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   klowerChanged( double ) ) );
   connect( ct_k_upper,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   kupperChanged( double ) ) );
   connect( ct_sigmpar1, SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   sipar1Changed( double ) ) );
   connect( ct_sigmpar2, SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   sipar2Changed( double ) ) );
   connect( ct_crpoints, SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   pointsChanged( double ) ) );
   connect( ct_mciters,  SIGNAL( valueChanged ( double ) ),
            this,        SLOT(   mciterChanged( double ) ) );

   connect( pb_loadmrs,  SIGNAL( clicked()          ),
            this,        SLOT(   load_mrecs()       ) );
   connect( pb_storemrs, SIGNAL( clicked()          ),
            this,        SLOT(   store_mrecs()      ) );
   connect( pb_loadbfm,  SIGNAL( clicked()          ),
            this,        SLOT(   load_bfm()         ) );
   connect( pb_storebfm, SIGNAL( clicked()          ),
            this,        SLOT(   store_bfm()        ) );
   connect( pb_resetbfm, SIGNAL( clicked()          ),
            this,        SLOT(   reset_bfm()        ) );
   connect( pb_resetmrs, SIGNAL( clicked()          ),
            this,        SLOT(   reset_mrecs()      ) );
   connect( pb_buildbfm, SIGNAL( clicked()          ),
            this,        SLOT(   build_bfm()        ) );
   connect( pb_mciters,  SIGNAL( clicked()          ),
            this,        SLOT(   start_montecarlo() ) );

   connect( pb_help,     SIGNAL( clicked() ),
            this,        SLOT(   help()    ) );
   connect( pb_cancel,   SIGNAL( clicked() ),
            this,        SLOT(   cancel()  ) );
   connect( pb_accept,   SIGNAL( clicked() ),
            this,        SLOT(   select()  ) );

   curvtypeChanged( ctype );
   resize( 780, 400 );
DbgLv(1) << "Post-resize size" << size();
   qApp->processEvents();
}

// Return state flag from advanced actions and, possibly, MC models
int US_AdvAnalysis::advanced_results( QVector< ModelRecord >* p_mrecsmc )
{
   // Set state flag reflecting new-bfm, new-mrs, montecarlo
   int state    = bfm_new ?           msk_bfnew   : 0;
   state        = mrs_new ? ( state | msk_mrnew ) : state;
   state        = mc_done ? ( state | msk_mcarl ) : state;
DbgLv(1) << "advanced_results - state=" << state;

   if ( mc_done  &&  p_mrecsmc != 0 )
   {  // If MonteCarlo was done, return its model records
      *p_mrecsmc   = mrecs_mc;
   }

   return state;
}

// Accept button clicked
void US_AdvAnalysis::select()
{
   if ( mrs_new  &&  p_mrecs != 0 )
   {  // If model records are new, return them to the caller
      *p_mrecs     = mrecs;
   }

   accept();
   close();
}

// Cancel button clicked
void US_AdvAnalysis::cancel()
{
   reject();
   close();
}

// Slot to handle a change in curve type
void US_AdvAnalysis::curvtypeChanged( int ivalue )
{
DbgLv(1) << "curvtypeChanged" << ivalue;
   ctype          = ivalue;
   bool is_sigm   = ( ctype != 0 );

   lb_sigmpar1->setVisible( is_sigm );
   ct_sigmpar1->setVisible( is_sigm );
   lb_sigmpar2->setVisible( is_sigm );
   ct_sigmpar2->setVisible( is_sigm );
}

// Slot to handle a change in S lower bound
void US_AdvAnalysis::slowerChanged( double value )
{
DbgLv(1) << "slowerChanged" << value;
}

// Slot to handle a change in S upper bound
void US_AdvAnalysis::supperChanged( double value )
{
DbgLv(1) << "supperChanged" << value;
}

// Slot to handle a change in sigmoid par 1
void US_AdvAnalysis::sipar1Changed( double value )
{
DbgLv(1) << "sipar1Changed" << value;
}

// Slot to handle a change in sigmoid par 2
void US_AdvAnalysis::sipar2Changed( double value )
{
DbgLv(1) << "sipar2Changed" << value;
}

// Slot to handle a change in K(f/f0) lower bound
void US_AdvAnalysis::klowerChanged( double value )
{
DbgLv(1) << "klowerChanged" << value;
}

// Slot to handle a change in K(f/f0) upper bound
void US_AdvAnalysis::kupperChanged( double value )
{
DbgLv(1) << "kupperChanged" << value;
}

// Slot to handle a change in curve points
void US_AdvAnalysis::pointsChanged( double value )
{
DbgLv(1) << "pointsChanged" << value;
}

// Slot to handle a change in monte carlo iterations
void US_AdvAnalysis::mciterChanged( double value )
{
DbgLv(1) << "mciterChanged" << value;
   mciters           = (int)value;
}

// Slot to load a model records list from disk
void US_AdvAnalysis::load_mrecs()
{
DbgLv(1) << "load_mrecs";
//under_construct( "Load Model Records" );
   // Query and get the file for loading
   QString load_file  = store_dir + "/pcsa-mrs-old_mrecs.xml";
   load_file        = QFileDialog::getOpenFileName( this,
      tr( "Select XML File Name for Model Records Load" ), store_dir,
      tr( "Model Records files (*pcsa-mrs-*.xml);;"
          "Any XML files (*.xml);;Any files (*)" ) );

   if ( load_file.isEmpty() )
   {
      return;
   }

   load_file        = load_file.replace( "\\", "/" );
   QString fdir     = load_file.section( "/",  0, -2 ) + "/";
   QString fname    = load_file.section( "/", -1, -1 );

   // Open the specified input file
   QFile filei( load_file );

   if ( !filei.open( QIODevice::ReadOnly  ) )
   {
      QMessageBox::critical( this, tr( "Open Error" ),
         tr( "Cannot open file %1 ." ).arg( load_file ) );
      return;
   }

   // Read in and parse the XML file to generate a new mrecs list
   mrecs.clear();
   int    ctype     = cb_curvtype->currentIndex();
   int    nisols    = 0;
   int    kisols    = 0;
   int    nmrecs    = 0;
   double smin      = 1e99;
   double smax      = -1e99;
   double kmin      = 1e99;
   double kmax      = -1e99;
   bool   is_mrsf   = false;
   QString xmlname  = "";
   QXmlStreamReader xmli( &filei );

   while ( ! xmli.atEnd() )
   {
      xmli.readNext();

      if ( xmli.isComment() )
      {  // Verify DOCTYPE PcsaModelRecords
         QString comm     = xmli.text().toString();
//DbgLv(1) << "LM:xml: comm" << comm;

         if ( comm.contains( "PcsaModelRecords" ) )
            is_mrsf          = true;

         else
         {
            QMessageBox::critical( this, tr( "File Type Error" ),
               tr( "File \"%1\" is not a PcsaModelRecords XML file." )
               .arg( fname ) );
            filei.close();
            return;
         }
      }

      xmlname          = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
//DbgLv(1) << "LM:xml: start name" << xmlname;
         QXmlStreamAttributes xattrs = xmli.attributes();

         if ( xmlname == "modelrecords" )
         {
            ctype            = xattrs.value( "type" ).toString().toInt();
            smin             = xattrs.value( "smin" ).toString().toDouble();
            smax             = xattrs.value( "smax" ).toString().toDouble();
            kmin             = xattrs.value( "kmin" ).toString().toDouble();
            kmax             = xattrs.value( "kmax" ).toString().toDouble();
            nisols           = xattrs.value( "curve_points" )
                               .toString().toInt();
//DbgLv(1) << "LM:xml:   nisols" << nisols;
         }

         else if ( xmlname == "modelrecord" )
         {
            mrec.taskx       = xattrs.value( "taskx"   ).toString().toInt();
            mrec.str_k       = xattrs.value( "start_k" ).toString().toDouble();
            mrec.end_k       = xattrs.value( "end_k"   ).toString().toDouble();
            mrec.par1        = xattrs.value( "par1"    ).toString().toDouble();
            mrec.par2        = xattrs.value( "par2"    ).toString().toDouble();
            mrec.rmsd        = xattrs.value( "rmsd"    ).toString().toDouble();
            QString cvpt     = xattrs.value( "curve_points" ).toString();
            kisols           = cvpt.isEmpty() ? nisols : cvpt.toInt();
            mrec.isolutes.resize( kisols );
            mrec.csolutes.clear();
            ncsols           = 0;
            kmin             = qMin( kmin, mrec.str_k );
            kmax             = qMax( kmax, mrec.end_k );
//DbgLv(1) << "LM:xml:    nmrecs" << nmrecs << "kisols" << kisols;
         }

         else if ( xmlname == "c_solute" )
         {
            US_Solute csolute;
            csolute.s        = xattrs.value( "s" ).toString().toDouble();
            csolute.k        = xattrs.value( "k" ).toString().toDouble();
            csolute.c        = xattrs.value( "c" ).toString().toDouble();
            smin             = qMin( smin, csolute.s );
            smax             = qMax( smax, csolute.s );
            kmin             = qMin( kmin, csolute.k );
            kmax             = qMax( kmax, csolute.k );
            csolute.s       *= 1.e-13;

            mrec.csolutes << csolute;
            ncsols++;
         }
      }

      else if ( xmli.isEndElement()  &&  xmlname == "modelrecord" )
      {
         mrecs << mrec;
         nmrecs++;
//DbgLv(1) << "LM:xml: End rec name" << xmlname << "nm" << nmrecs
// << "ncsols" << ncsols << "csize" << mrec.csolutes.size();
         mrec.csolutes.clear();
      }
   }
DbgLv(1) << "LM:xml: End ALL: nmrecs" << nmrecs << "last ncsols" << ncsols;

   if ( xmli.hasError() )
   {
      QMessageBox::critical( this, tr( "XML Invalid" ),
         tr( "File \"%1\" is not a valid XML file." ).arg( fname ) );
   }

   else if ( ! is_mrsf )
   {
      QMessageBox::critical( this, tr( "File Type Error" ),
         tr( "File \"%1\" is not a PcsaModelRecords XML file." )
         .arg( fname ) );
   }

   filei.close();

   // Report on loaded file
   te_mrecstat->setText(
      tr( "Model Records have been loaded from file\n"
          "  \"%1\", of directory\n  \"%2\".\n"
          "There are %3  %4-solute records.\n" )
      .arg( fname ).arg( fdir).arg( nmrecs ).arg( nisols ) );

   // Re-generate curve points for every model record
   for ( int mr = 0; mr < nmrecs; mr++ )
   {
      curve_points( ctype, smin, smax, kmin, kmax, mrecs[ mr ] );
   }

   mrec             = mrecs[ 0 ];
   ncsols           = mrec.isolutes.size();
   QString sctype   = ( ctype == 1 ) ? "Increasing Sigmoid" :
                    ( ( ctype == 0 ) ? "Straight Line" :
                                       "Decreasing Sigmoid" );

   // Build the model that goes along with the BFM
   bfm_model();

   te_bfmstat->setText(
      tr( "A new Best Final Model derives from the top spot\n"
          "  of the just-loaded Model Records list.\n"
          "The %1 model has %2 computed solutes\n"
          "  and an RMSD of %3" )
      .arg( sctype ).arg( ncsols ).arg( mrec.rmsd ) );

   bfm_new          = true;
   mrs_new          = true;
}

// Slot to store a model records list to disk
void US_AdvAnalysis::store_mrecs()
{
DbgLv(1) << "store_mrecs";
//under_construct( "Store Model Records" );
   // Query and get the file for storing
   QString store_file = store_dir + "/pcsa-mrs-new_mrecs.xml";
   store_file       = QFileDialog::getSaveFileName( this,
      tr( "Specify XML File Name for Model Records Store" ), store_dir,
      tr( "Model Records files (*pcsa-mrs-*.xml);;"
          "Any XML files (*.xml);;Any files (*)" ) );

   if ( store_file.isEmpty() )
   {
DbgLv(1) << "store_mrecs - FILE NAME EMPTY";
      return;
   }
else DbgLv(1) << "store_mrecs - FILE NAME *NOT* EMPTY" << store_file;

   store_file       = store_file.replace( "\\", "/" );
   QString fdir     = store_file.section( "/",  0, -2 ) + "/";
   QString fname    = store_file.section( "/", -1, -1 );

   // Massage the name to be in "mrs_<name>.xml" form
   if ( store_file.endsWith( "." ) )
   {  // Ends with ".":  no ".xml" is to be added
      store_file       = store_file.left( store_file.length() - 1 );
      fname            = fname     .left( fname     .length() - 1 );
   }

   else if ( ! store_file.endsWith( ".xml" ) )
   {  // If no ".xml" extension, add one
      store_file       = store_file + ".xml";
      fname            = fname      + ".xml";
   }

   if ( fname.startsWith( "." ) )
   {  // Starts with ".":  no "pcsa-mrs-" prefix is to be added
      store_file       = fdir + fname.mid( 1 );
   }

   else if ( ! fname.startsWith( "pcsa-mrs-" ) )
   {  // If no "pcsa-mrs-" prefix, add one
      store_file       = fdir + "pcsa-mrs-" + fname;
   }

   // Open the specified output file
   QFile fileo( store_file );

   if ( !fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::critical( this, tr( "Open Error" ),
         tr( "Cannot open file %1 ." ).arg( store_file ) );
      return;
   }

   // Write out the XML file
   int    ctype     = cb_curvtype->currentIndex();
   int    nisols    = (int)ct_crpoints->value();
   int    nmrecs    = mrecs.size();
   double smin      = ct_s_lower ->value();
   double smax      = ct_s_upper ->value();
   double kmin      = ct_k_lower ->value();
   double kmax      = ct_k_upper ->value();
   QXmlStreamWriter xmlo( &fileo );
   xmlo.setAutoFormatting( true );
   xmlo.writeStartDocument( "1.0" );
   xmlo.writeComment( "DOCTYPE PcsaModelRecords" );
   xmlo.writeCharacters( "\n" );
   xmlo.writeStartElement( "modelrecords" );
   xmlo.writeAttribute( "version",      "1.0" );
   xmlo.writeAttribute( "type",         QString::number( ctype  ) );
   xmlo.writeAttribute( "smin",         QString::number( smin  ) ); 
   xmlo.writeAttribute( "smax",         QString::number( smax  ) ); 
   xmlo.writeAttribute( "kmin",         QString::number( kmin  ) ); 
   xmlo.writeAttribute( "kmax",         QString::number( kmax  ) ); 
   xmlo.writeAttribute( "curve_points", QString::number( nisols ) ); 

   for ( int mr = 0; mr < nmrecs; mr++ )
   {
      ModelRecord mrec = mrecs[ mr ];
      int    kisols    = mrec.isolutes.size();
      int    ncsols    = mrec.csolutes.size();
      xmlo.writeStartElement( "modelrecord" );
      xmlo.writeAttribute( "taskx",   QString::number( mrec.taskx ) );
      xmlo.writeAttribute( "start_k", QString::number( mrec.str_k ) );
      xmlo.writeAttribute( "end_k",   QString::number( mrec.end_k ) );
      xmlo.writeAttribute( "par1",    QString::number( mrec.par1  ) );
      xmlo.writeAttribute( "par2",    QString::number( mrec.par2  ) );
      xmlo.writeAttribute( "rmsd",    QString::number( mrec.rmsd  ) );
      if ( kisols != nisols )
         xmlo.writeAttribute( "curve_points", QString::number( kisols ) );

      for ( int cc = 0; cc < ncsols; cc++ )
      {
         xmlo.writeStartElement( "c_solute" );
         double sval      = mrec.csolutes[ cc ].s * 1.e13;
         xmlo.writeAttribute( "s", QString::number( sval ) );
         xmlo.writeAttribute( "k", QString::number( mrec.csolutes[ cc ].k ) );
         xmlo.writeAttribute( "c", QString::number( mrec.csolutes[ cc ].c ) );
         xmlo.writeEndElement();
      }

      xmlo.writeEndElement();
   }

   xmlo.writeEndElement();
   xmlo.writeEndDocument();
   fileo.close();

   // Report on saved file
   fdir             = store_file.section( "/",  0, -2 ) + "/";
   fname            = store_file.section( "/", -1, -1 );
   QString sttext   = te_mrecstat->toPlainText();
   te_mrecstat->setText( sttext +
      tr( "\nModel Records have been stored in file\n"
          "  \"%1\", of directory\n  \"%2\".\n" )
      .arg( fname ).arg( fdir) );
}

// Slot to load a best final model from disk
void US_AdvAnalysis::load_bfm()
{
DbgLv(1) << "load_bfm";
//under_construct( "Load Final Model" );
   // Query and get the file for loading
   QString load_file  = store_dir + "/pcsa-bfm-old_mrecs.xml";
   load_file        = QFileDialog::getOpenFileName( this,
      tr( "Select XML File Name for Best Final Model Load" ), store_dir,
      tr( "Best Final Model files (*pcsa-bfm-*.xml);;"
          "Any XML files (*.xml);;Any files (*)" ) );

   if ( load_file.isEmpty() )
   {
      return;
   }

   load_file        = load_file.replace( "\\", "/" );
   QString fdir     = load_file.section( "/",  0, -2 ) + "/";
   QString fname    = load_file.section( "/", -1, -1 );

   // Open the specified input file
   QFile filei( load_file );

   if ( !filei.open( QIODevice::ReadOnly  ) )
   {
      QMessageBox::critical( this, tr( "Open Error" ),
         tr( "Cannot open file %1 ." ).arg( load_file ) );
      return;
   }

   // Read in and parse the XML file to generate a new BFM
   int    ctype     = cb_curvtype->currentIndex();
   int    nisols    = 0;
   double smin      = 1e99;
   double smax      = -1e99;
   double kmin      = 1e99;
   double kmax      = -1e99;
   bool   is_bfmf   = false;
   QString xmlname  = "";
   QXmlStreamReader xmli( &filei );

   while ( ! xmli.atEnd() )
   {
      xmli.readNext();

      if ( xmli.isComment() )
      {  // Verify DOCTYPE PcsaBestFinalModel
         QString comm     = xmli.text().toString();
//DbgLv(1) << "LM:xml: comm" << comm;

         if ( comm.contains( "PcsaBestFinalModel" ) )
            is_bfmf          = true;

         else
         {
            QMessageBox::critical( this, tr( "File Type Error" ),
               tr( "File \"%1\" is not a PcsaBestFinalModel XML file." )
               .arg( fname ) );
            filei.close();
            return;
         }
      }

      xmlname          = xmli.name().toString();

      if ( xmli.isStartElement() )
      {
//DbgLv(1) << "LM:xml: start name" << xmlname;
         QXmlStreamAttributes xattrs = xmli.attributes();

         if ( xmlname == "modelrecord" )
         {
            ctype            = xattrs.value( "type" ).toString().toInt();
            smin             = xattrs.value( "smin" ).toString().toDouble();
            smax             = xattrs.value( "smax" ).toString().toDouble();
            kmin             = xattrs.value( "kmin" ).toString().toDouble();
            kmax             = xattrs.value( "kmax" ).toString().toDouble();
            nisols           = xattrs.value( "curve_points" )
                               .toString().toInt();
            mrec.taskx       = xattrs.value( "taskx"   ).toString().toInt();
            mrec.str_k       = xattrs.value( "start_k" ).toString().toDouble();
            mrec.end_k       = xattrs.value( "end_k"   ).toString().toDouble();
            mrec.par1        = xattrs.value( "par1"    ).toString().toDouble();
            mrec.par2        = xattrs.value( "par2"    ).toString().toDouble();
            mrec.rmsd        = xattrs.value( "rmsd"    ).toString().toDouble();
            mrec.isolutes.resize( nisols );
            mrec.csolutes.clear();
            ncsols           = 0;
            kmin             = qMin( kmin, mrec.str_k );
            kmax             = qMax( kmax, mrec.end_k );
//DbgLv(1) << "LM:xml:    nmrecs" << nmrecs << "kisols" << kisols;
         }

         else if ( xmlname == "c_solute" )
         {
            US_Solute csolute;
            csolute.s        = xattrs.value( "s" ).toString().toDouble();
            csolute.k        = xattrs.value( "k" ).toString().toDouble();
            csolute.c        = xattrs.value( "c" ).toString().toDouble();
            smin             = qMin( smin, csolute.s );
            smax             = qMax( smax, csolute.s );
            kmin             = qMin( kmin, csolute.k );
            kmax             = qMax( kmax, csolute.k );
            csolute.s       *= 1.e-13;

            mrec.csolutes << csolute;
            ncsols++;
         }
      }
   }
DbgLv(1) << "LM:xml: End ALL: nmrecs" << nmrecs << "last ncsols" << ncsols;
   filei.close();

   if ( xmli.hasError() )
   {
      QMessageBox::critical( this, tr( "XML Invalid" ),
         tr( "File \"%1\" is not a valid XML file." ).arg( fname ) );
      return;
   }

   else if ( ! is_bfmf )
   {
      QMessageBox::critical( this, tr( "File Type Error" ),
         tr( "File \"%1\" is not a PcsaBestFinalModel XML file." )
         .arg( fname ) );
      return;
   }

   // Re-generate curve points for the model
   curve_points( ctype, smin, smax, kmin, kmax, mrec );

   QString sctype   = ( ctype == 1 ) ? "Increasing Sigmoid" :
                    ( ( ctype == 0 ) ? "Straight Line" :
                                       "Decreasing Sigmoid" );

   // Build the model that goes along with the BFM
   bfm_model();

   // Report on loaded file
   te_bfmstat->setText(
      tr( "A new Best Final Model has been loaded from file\n"
          "  \"%1\", of directory\n  \"%2\".\n"
          "The %3 model has %4 computed solutes\n"
          "  and an RMSD of %5" )
      .arg( fname ).arg( fdir).arg( sctype ).arg( ncsols ).arg( mrec.rmsd ) );

   bfm_new          = true;
   mrecs[ 0 ]       = mrec;
}

// Slot to store a best final model to disk
void US_AdvAnalysis::store_bfm()
{
DbgLv(1) << "store_bfm";
   // Query and get the file for storing
   QString store_file = store_dir + "/pcsa-bfm-new_bfm.xml";
   store_file       = QFileDialog::getSaveFileName( this,
      tr( "Specify XML File Name for Best Final Model Store" ), store_dir,
      tr( "Best Final Model files (*pcsa-bfm-*.xml);;"
          "Any XML files (*.xml);;Any files (*)" ) );

   if ( store_file.isEmpty() )
   {
      return;
   }

   store_file       = store_file.replace( "\\", "/" );
   QString fdir     = store_file.section( "/",  0, -2 ) + "/";
   QString fname    = store_file.section( "/", -1, -1 );

   // Massage the name to be in "pcsa-bfm-<name>.xml" form
   if ( store_file.endsWith( "." ) )
   {  // Ends with ".":  no ".xml" is to be added
      store_file       = store_file.left( store_file.length() - 1 );
      fname            = fname     .left( fname     .length() - 1 );
   }

   else if ( ! store_file.endsWith( ".xml" ) )
   {  // If no ".xml" extension, add one
      store_file       = store_file + ".xml";
      fname            = fname      + ".xml";
   }

   if ( fname.startsWith( "." ) )
   {  // Starts with ".":  no "pcsa-bfm-" prefix is to be added
      store_file       = fdir + fname.mid( 1 );
   }

   else if ( ! fname.startsWith( "pcsa-bfm-" ) )
   {  // If no "pcsa-bfm-" prefix, add one
      store_file       = fdir + "pcsa-bfm-" + fname;
   }

   // Open the specified output file
   QFile fileo( store_file );

   if ( !fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      QMessageBox::critical( this, tr( "Open Error" ),
         tr( "Cannot open file %1 ." ).arg( store_file ) );
      return;
   }

   // Write out the XML file
   int    ctype     = cb_curvtype->currentIndex();
//   int    nisols    = (int)ct_crpoints->value();
   int    kisols    = mrec.isolutes.size();
   int    ncsols    = mrec.csolutes.size();
   double smin      = ct_s_lower ->value();
   double smax      = ct_s_upper ->value();
   double kmin      = ct_k_lower ->value();
   double kmax      = ct_k_upper ->value();
   QXmlStreamWriter xmlo;
   xmlo.setDevice( &fileo );
   xmlo.setAutoFormatting( true );
   xmlo.writeStartDocument( "1.0" );
   xmlo.writeComment( "DOCTYPE PcsaBestFinalModel" );
   xmlo.writeCharacters( "\n" );
   xmlo.writeStartElement( "modelrecord" );
   xmlo.writeAttribute( "version",      "1.0" );
   xmlo.writeAttribute( "type",         QString::number( ctype      ) );
   xmlo.writeAttribute( "smin",         QString::number( smin       ) ); 
   xmlo.writeAttribute( "smax",         QString::number( smax       ) ); 
   xmlo.writeAttribute( "kmin",         QString::number( kmin       ) ); 
   xmlo.writeAttribute( "kmax",         QString::number( kmax       ) ); 
   xmlo.writeAttribute( "curve_points", QString::number( kisols     ) ); 
   xmlo.writeAttribute( "taskx",        QString::number( mrec.taskx ) );
   xmlo.writeAttribute( "start_k",      QString::number( mrec.str_k ) );
   xmlo.writeAttribute( "end_k",        QString::number( mrec.end_k ) );
   xmlo.writeAttribute( "par1",         QString::number( mrec.par1  ) );
   xmlo.writeAttribute( "par2",         QString::number( mrec.par2  ) );
   xmlo.writeAttribute( "rmsd",         QString::number( mrec.rmsd  ) );

   for ( int cc = 0; cc < ncsols; cc++ )
   {
      xmlo.writeStartElement( "c_solute" );
      double sval      = mrec.csolutes[ cc ].s * 1.e13;
      xmlo.writeAttribute( "s", QString::number( sval ) );
      xmlo.writeAttribute( "k", QString::number( mrec.csolutes[ cc ].k ) );
      xmlo.writeAttribute( "c", QString::number( mrec.csolutes[ cc ].c ) );
      xmlo.writeEndElement();
   }

   xmlo.writeEndElement();
   xmlo.writeEndDocument();
   fileo.close();

   // Report on the saved file
   fdir             = store_file.section( "/",  0, -2 ) + "/";
   fname            = store_file.section( "/", -1, -1 );
   QString sttext   = te_bfmstat->toPlainText();

   te_bfmstat->setText( sttext +
      tr( "\nBest Final Model has been stored in file\n"
          "  \"%1\", of directory\n  \"%2\".\n" )
      .arg( fname ).arg( fdir) );
}

// Slot to reset the best final model to its initial state
void US_AdvAnalysis::reset_bfm()
{
DbgLv(1) << "reset_bfm";
}

// Slot to reset the list of model records to its initial state
void US_AdvAnalysis::reset_mrecs()
{
DbgLv(1) << "reset_mrecs";
}

// Slot to build a new best final model from specified fitting controls
void US_AdvAnalysis::build_bfm()
{
DbgLv(1) << "build_bfm";
under_construct( "Build Final Model" );
}

// Slot to start and process monte carlo iterations
void US_AdvAnalysis::start_montecarlo()
{
DbgLv(1) << "start_montecarlo";
   wdata          = dset0->run_data;
   edata          = &wdata;
   mciters        = (int)ct_mciters->value();
   QString sline1 = tr( "%1 Monte Carlo iterations are being computed...\n" )
                    .arg( mciters );
   ksiters        = 0;
   kciters        = 0;
   mrecs_mc.clear();
   ModelRecord          mrec_mc = mrec;
   US_SolveSim::DataSet dset    = *dset0;
   QList< US_SolveSim::DataSet* > dsets;
   dsets << &dset;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   b_progress->reset();
   b_progress->setMaximum( mciters );

   if ( nthr == 1 )
   {  // Monte Carlo with single thread
      for ( int iter = 0; iter < mciters; iter++ )
      {
         ksiters++;
         US_SolveSim::Simulation sim_vals;
         sim_vals.solutes          = mrec.isolutes;
         dsets[ 0 ]->run_data      = wdata;

         US_SolveSim* solvesim     = new US_SolveSim( dsets, 0, false );

         solvesim->calc_residuals( 0, 1, sim_vals );

         kciters          = ksiters;
         mrec_mc.variance = sim_vals.variance;
         mrec_mc.rmsd     = sqrt( sim_vals.variance );
         mrec_mc.csolutes = sim_vals.solutes;
         ncsols           = mrec_mc.csolutes.size();
DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd
 << "ncsols" << ncsols;

         mrecs_mc << mrec_mc;

         // Set up new data modified by a gaussian distribution
         if ( iter == 0 )
         {
            set_gaussians( sim_vals );
         }

         // Except on last iteration, we must create new "experiment" data
         if ( kciters < mciters )
         {
            apply_gaussians();
         }

         te_bfmstat ->setText( sline1 +
            tr( "%1 are completed ( last:  %2-solute,  RMSD=%3 )" )
            .arg( kciters ).arg( ncsols ).arg( mrec_mc.rmsd ) );
         b_progress->setValue( kciters );
         qApp->processEvents();
      }

      // Complete the MonteCarlo process
      b_progress->setValue( mciters );
      qApp->processEvents();

      montecarlo_done();

      QApplication::restoreOverrideCursor();
      qApp->processEvents();
   }

   else
   {  // Monte Carlo in threads
      sline1 = tr( "%1 Monte Carlo iterations are being"
                   " computed in %2 threads...\n" )
                    .arg( mciters ).arg( nthr );

      // Do the first iteration computation and set gaussians
      US_SolveSim::Simulation sim_vals;
      sim_vals.solutes          = mrec.isolutes;
      dsets[ 0 ]->run_data      = wdata;
      ksiters++;

      US_SolveSim* solvesim     = new US_SolveSim( dsets, 0, false );

      solvesim->calc_residuals( 0, 1, sim_vals );

      kciters          = ksiters;
      mrec_mc.variance = sim_vals.variance;
      mrec_mc.rmsd     = sqrt( sim_vals.variance );
      mrec_mc.csolutes = sim_vals.solutes;
      ncsols           = mrec_mc.csolutes.size();
DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd
 << "ncsols" << ncsols;
      te_bfmstat ->setText( sline1 +
         tr( "%1 are completed ( last:  %2-solute,  RMSD=%3 )" )
         .arg( kciters ).arg( ncsols ).arg( mrec_mc.rmsd ) );
      b_progress->setValue( kciters );
      qApp->processEvents();

      mrecs_mc << mrec_mc;
      set_gaussians( sim_vals );

      WorkPacket    wtbase;
      wtbase.par1      = mrec.par1;
      wtbase.par2      = mrec.par2;
      wtbase.str_k     = mrec.str_k;
      wtbase.end_k     = mrec.end_k;
      wtbase.isolutes  = mrec.isolutes;
      wtbase.csolutes.clear();
      wtbase.noisf     = 0;
      wtbase.dsets     = dsets;
      wtbase.depth     = 0;
      US_SolveSim::DataSet wkdset  = *(dsets[ 0 ]);
      wkdsets.clear();

      for ( int jt = 0; jt < nthr; jt++ )
      {  // Build up a list of datasets for each thread
         wkdsets << wkdset;
      }

      // Start the next few iterations in available threads
      for ( int jt = 0; jt < nthr; jt++ )
      {
         apply_gaussians();

         WorkPacket wtask       = wtbase;
         US_SolveSim::Simulation        sim_vals;
         sim_vals.solutes       = mrec.isolutes;

         wtask.thrn             = jt + 1;
         wtask.taskx            = jt + 1;
         wtask.str_k            = mrec.str_k;
         wtask.end_k            = mrec.end_k;
         wtask.sim_vals         = sim_vals;
         wtask.dsets[ 0 ]       = &wkdsets[ jt ];

         wtask.dsets[ 0 ]->run_data = wdata;

         WorkerThread* wthrd    = new WorkerThread( this );
         connect( wthrd, SIGNAL( work_complete( WorkerThread* ) ),
                  this,  SLOT  ( process_job  ( WorkerThread* ) ) );

         wthrd->define_work( wtask );

         wthrd->start();

         ksiters++;
DbgLv(1) << "    ksiters" << ksiters << "dsets[0]" << wtask.dsets[0];
      }

   }

}

// Set gaussian distribution: sigmas and iteration 1 simulation data
void US_AdvAnalysis::set_gaussians( US_SolveSim::Simulation& sim_vals )
{
   bool gausmoo  = US_Settings::debug_match( "MC-GaussianSmooth" );
   int  nscans   = edata->scanCount();
   int  npoints  = edata->pointCount();
DbgLv(1) << "AA: set_gaus: gausmoo" << gausmoo;
   US_DataIO::RawData*   sdata = &sim_vals.sim_data;
   US_DataIO::RawData*   rdata = &sim_vals.residuals;

   sigmas.clear();

   for ( int ss = 0; ss < nscans; ss++ )
   {  // Loop to accumulate the residuals from iteration 1
      QVector< double > vv( npoints );

      for ( int rr = 0; rr < npoints; rr++ )
      {  // Get all residuals points from a scan
         vv[ rr ]   = qAbs( rdata->value( ss, rr ) );
      }

      if ( gausmoo )
      {  // Do a 5-point gaussian smooth of each scan's residuals
         US_Math2::gaussian_smoothing( vv, 5 );
      }

      // Append residuals to build total sigmas array
      sigmas << vv;
   }

   if ( gausmoo )
   {  // Scale the sigmas so they are at the same level as original residuals
      double rmsdi    = mrecs_mc[ 0 ].rmsd;
      double rmsds    = 0.0;
      int    ntpoints = nscans * npoints;

      for ( int rr = 0; rr < ntpoints; rr++ )
         rmsds          += sq( sigmas[ rr ] );            // Sum of squares

      rmsds           = sqrt( rmsds / (double)ntpoints ); // Sigmas RMSD
      double sigscl   = rmsdi / rmsds;                    // Sigma scale factor

      for ( int rr = 0; rr < ntpoints; rr++ )
         sigmas[ rr ]   *= sigscl;                        // Scaled sigmas
DbgLv(1) << "AA: gausmoo: rmsd-i rmsc-s sigscl" << rmsdi << rmsds << sigscl;
   }

   // Save the simulation data set from iteration 1
   sdata1       = *sdata;
}

// Apply gaussians: add in random variations of sigmas to base simulation
void US_AdvAnalysis::apply_gaussians()
{
   int nscans   = edata->scanCount();
   int npoints  = edata->pointCount();
   int kk       = 0;

   // Add box-muller portion of each sigma to the base simulation
   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double svari  = US_Math2::box_muller( 0.0, sigmas[ kk++ ] );
         wdata.setValue( ss, rr, ( sdata1.value( ss, rr ) + svari ) );
      }
   }
}

// Process the completion of an MC worker thread
void US_AdvAnalysis::process_job( WorkerThread* wthr )
{
   WorkPacket wresult;
   wthr->get_result( wresult );
   kciters++;

   ModelRecord mrec_mc = mrecs_mc[ 0 ];
   mrec_mc.variance = wresult.sim_vals.variance;
   mrec_mc.rmsd     = sqrt( mrec_mc.variance );
   mrec_mc.csolutes = wresult.sim_vals.solutes;
   ncsols           = mrec_mc.csolutes.size();
DbgLv(1) << "  kciters" << kciters << "rmsd" << mrec_mc.rmsd
 << "ncsols" << ncsols;
   QString sline1 = tr( "%1 Monte Carlo iterations are being"
                         " computed in %2 threads...\n" )
                    .arg( mciters ).arg( nthr );
   te_bfmstat ->setText( sline1 +
      tr( "%1 are completed ( last:  %2-solute,  RMSD=%3 )" )
      .arg( kciters ).arg( ncsols ).arg( mrec_mc.rmsd ) );
   b_progress->setValue( kciters );
   qApp->processEvents();

   mrecs_mc << mrec_mc;

   if ( kciters == mciters )
   {  // Complete the MonteCarlo process
      b_progress->setValue( mciters );

      montecarlo_done();

      QApplication::restoreOverrideCursor();
      qApp->processEvents();
   }

   else if ( ksiters < mciters )
   {  // Submit a new task
//DbgLv(1) << "    ksiters" << ksiters << "   apply_gaussians";
      apply_gaussians();
//DbgLv(1) << "    ksiters" << ksiters << "    retn: apply_gaussians";

      WorkerThread* wthrd        = wthr;
      WorkPacket    wtask        = wresult;
      wtask.dsets[ 0 ]->run_data = wdata;
      wtask.taskx                = ksiters;

      wthrd->disconnect();
      connect( wthrd, SIGNAL( work_complete( WorkerThread* ) ),
               this,  SLOT  ( process_job  ( WorkerThread* ) ) );

//DbgLv(1) << "    ksiters" << ksiters << "     define,start";
      wthrd->define_work( wtask );
      wthrd->start();

      ksiters++;
DbgLv(1) << "    ksiters" << ksiters << "dsets[0]" << wtask.dsets[0];
   }
}

// Complete model records and the final model after monte carlo completion
void US_AdvAnalysis::montecarlo_done( void )
{
DbgLv(1) << "==montecarlo_done()==";
   te_bfmstat ->setText( te_bfmstat->toPlainText() +
      tr( "\nBuilding MC models and final composite..." ) );
   int     nccsol   = 0;
   QVector< US_Solute > compsols;
   QStringList sortlst;
   US_Model::SimulationComponent zcomponent;
   zcomponent.vbar20   = dset0->vbar20;

   // Build individual models and append all solutes to one composite
   for ( int jmc = 0; jmc < mciters; jmc++ )
   {
      mrec             = mrecs_mc[ jmc ];
      QVector< US_Solute > csolutes = mrec.csolutes;
      int     nsols    = csolutes.size();
      model.components.resize( nsols );

      for ( int cc = 0; cc < nsols; cc++ )
      {
         // Get component values and sorting string
         double  sol_s    = csolutes[ cc ].s;
         double  sol_k    = csolutes[ cc ].k;
         double  sol_c    = csolutes[ cc ].c;
         QString sol_id   = QString().sprintf( "%.4f:%.4f:%d",
            sol_s * 1.e13, sol_k, nccsol++ );
DbgLv(1) << "MCD: cc" << cc << "sol_id" << sol_id;

         // Save unsorted/summed solute and sorting string
         US_Solute compsol;
         compsol.s        = sol_s;
         compsol.k        = sol_k;
         compsol.c        = sol_c;
         compsols << compsol;
         sortlst  << sol_id;

         // Build the model component
         model.components[ cc ]                       = zcomponent;
         model.components[ cc ].s                     = sol_s;
         model.components[ cc ].f_f0                  = sol_k;
         model.components[ cc ].signal_concentration  = sol_c;
         model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );
         model.calc_coefficients( model.components[ cc ] );
      }

      // Save the individual MC model
      model.monteCarlo = true;
      mrec.model       = model;
      mrecs_mc[ jmc ]  = mrec;
   }

   // Now sort the solute id strings to create sorted composite
   qSort( sortlst );
   US_Solute pcompsol;
   US_Solute ccompsol;
   QString pskmatch  = QString();
   mrec.csolutes.clear();
   double  cnorm     = 1.0 / (double)mciters;
   ncsols            = 0;

   for ( int cc = 0; cc < nccsol; cc++ )
   {
      QString sol_id    = sortlst[ cc ];
      QString skmatch   = sol_id.section( ":", 0, 1 );
      int     ccin      = sol_id.section( ":", 2, 2 ).toInt();
      ccompsol          = compsols[ ccin ];

      if ( skmatch != pskmatch )
      {  // New s,k combination:  output previous component, start new sum
         if ( cc > 0 )
         {
            pcompsol.c    *= cnorm;
            mrec.csolutes << pcompsol;
            ncsols++;
         }

         pcompsol       = ccompsol;
         pskmatch       = skmatch;
DbgLv(1) << "MCD: cc ccin ncsols" << cc << ccin << ncsols;
      }

      else
      {  // Identical s,k to previous:  sum concentration;
         pcompsol.c    += ccompsol.c;
      }
   }

   // Output last component
   pcompsol.c    *= cnorm;
   mrec.csolutes << pcompsol;
   ncsols++;
   US_Model modela;

   model          = mrec.model;           // Model to pass back to main
   model.components.resize( ncsols );
   modela         = model;                // Model for application (corrected)
   double sfactor = 1.0 / dset0->s20w_correction;
   double dfactor = 1.0 / dset0->D20w_correction;

   // Build the model that goes along with the new composite model record
   for ( int cc = 0; cc < ncsols; cc++ )
   {
      model.components[ cc ]                       = zcomponent;
      model.components[ cc ].s                     = mrec.csolutes[ cc ].s;
      model.components[ cc ].f_f0                  = mrec.csolutes[ cc ].k;
      model.components[ cc ].signal_concentration  = mrec.csolutes[ cc ].c;
      model.components[ cc ].name = QString().sprintf( "SC%04d", cc + 1 );
      model.calc_coefficients( model.components[ cc ] );

      modela.components[ cc ]                      = model.components[ cc ];
      modela.components[ cc ].s                   *= sfactor;
      modela.components[ cc ].D                   *= dfactor;
   }

   // Do a fit with the composite model and get the RMSD
                       edata   = &dset0->run_data;
   US_DataIO::RawData* sdata   = &mrec.sim_data;
   US_AstfemMath::initSimData( *sdata, *edata, 0.0 );

   US_Astfem_RSA astfem_rsa( modela, dset0->simparams );
   astfem_rsa.calculate( *sdata );

   int    nscans  = edata->scanCount();
   int    npoints = edata->pointCount();
   double varia   = 0.0;

   for ( int ss = 0; ss < nscans; ss++ )
      for ( int rr = 0; rr < npoints; rr++ )
         varia   += sq( ( edata->value( ss, rr ) - sdata->value( ss, rr ) ) );

   varia         /= (double)( nscans * npoints );
   mrec.variance  = varia;
   mrec.rmsd      = sqrt( varia );
   mrec.model     = model;
   mrecs[ 0 ]     = mrec;

   // Report MC completion status
   te_bfmstat ->setText( te_bfmstat->toPlainText() +
      tr( "\nMC models and final %1-solute composite model are built." )
      .arg( ncsols ) );

   te_mrecstat->setText(
      tr( "A best final %1-solute model ( RMSD = %2 )\n"
          "  now occupies the top curve model records list spot." )
      .arg( ncsols ).arg( mrec.rmsd ) );

   bfm_new        = true;
   mrs_new        = true;
   mc_done        = true;
}

// Pop up an under-construction message dialog
void US_AdvAnalysis::under_construct( QString proc )
{
   QMessageBox::information( this, tr( "UNDER CONSTRUCTION" ),
      tr( "Implementation of <b>%1</b> is not yet complete." ).arg( proc ) );
}

// Re-generate the curve points for a model record
void US_AdvAnalysis::curve_points( int ctype, double str_s, double end_s,
      double str_k, double end_k, ModelRecord& mrec )
{
   int    nisols  = mrec.isolutes.size();
   double prng    = (double)( nisols - 1 );
   double srng    = end_s - str_s;
   double sinc    = srng / prng;
   double par1    = mrec.par1;
   double par2    = mrec.par2;
   double xinc    = 1.0 / prng;
   double xval    = 0.0;
DbgLv(1) << "AA:CP: xinc" << xinc << "ctype" << ctype;

   if ( ctype == 1 )       // Increasing Sigmoid
   {
      double kdif    = end_k - str_k;
      double p1rt    = sqrt( 2.0 * par1 );

      for ( int kk = 0; kk < nisols; kk++ )
      {
         double sval    = str_s + xval * srng;
         double efac    = 0.5 * erf( ( xval - par2 ) / p1rt ) + 0.5;
         double kval    = str_k + kdif * efac;
         mrec.isolutes[ kk ].s = sval * 1.e-13;
         mrec.isolutes[ kk ].k = kval;
         xval          += xinc;
      }
   }

   else if ( ctype == 0 )  // Straight Line
   {
      double kval    = mrec.str_k;
      double kinc    = ( mrec.end_k - mrec.str_k ) / prng;
      double sval    = str_s;

      for ( int kk = 0; kk < nisols; kk++ )
      {
         mrec.isolutes[ kk ].s = sval * 1.e-13;
         mrec.isolutes[ kk ].k = kval;
         sval          += sinc;
         kval          += kinc;
      }
   }

   else if ( ctype == 2 )  // Decreasing Sigmoid
   {
      double kdif    = str_k - end_k;
      double p1rt    = sqrt( 2.0 * par1 );

      for ( int kk = 0; kk < nisols; kk++ )
      {
         double sval    = str_s + xval * srng;
         double efac    = 0.5 * erf( ( xval - par2 ) / p1rt ) + 0.5;
         double kval    = end_k + kdif * efac;
         mrec.isolutes[ kk ].s = sval * 1.e-13;
         mrec.isolutes[ kk ].k = kval;
         xval          += xinc;
      }
   }
DbgLv(1) << "AA:CP: sol0 s,k" << mrec.isolutes[0].s << mrec.isolutes[0].k;
}

// Generate the model that goes with the BFM record
void US_AdvAnalysis::bfm_model( void )
{
   US_Model modela;
   US_Model::SimulationComponent zcomponent;
   zcomponent.vbar20   = dset0->vbar20;
   nisols         = mrec.isolutes.size();
   ncsols         = mrec.csolutes.size();

   model          = mrec.model;           // Model to pass back to main
   model.components.resize( ncsols );
   modela         = model;                // Model for application (corrected)
   double sfactor = 1.0 / dset0->s20w_correction;
   double dfactor = 1.0 / dset0->D20w_correction;

   // Build the model that goes along with the new composite model record
   for ( int cc = 0; cc < ncsols; cc++ )
   {
      model.components[ cc ]        = zcomponent;
      model.components[ cc ].s      = mrec.csolutes[ cc ].s;
      model.components[ cc ].f_f0   = mrec.csolutes[ cc ].k;
      model.components[ cc ].signal_concentration
                                    = mrec.csolutes[ cc ].c;
      model.components[ cc ].name   = QString().sprintf( "SC%04d", cc + 1 );

      model.calc_coefficients( model.components[ cc ] );

      modela.components[ cc ]       = model.components[ cc ];
      modela.components[ cc ].s    *= sfactor;
      modela.components[ cc ].D    *= dfactor;
   }

   // Do a fit with the composite model and get the RMSD
                       edata   = &dset0->run_data;
   US_DataIO::RawData* sdata   = &mrec.sim_data;
   US_DataIO::RawData* rdata   = &mrec.residuals;
   US_AstfemMath::initSimData( *sdata, *edata, 0.0 );
   US_AstfemMath::initSimData( *rdata, *edata, 0.0 );

   US_Astfem_RSA astfem_rsa( modela, dset0->simparams );
   astfem_rsa.calculate( *sdata );

   int    nscans  = edata->scanCount();
   int    npoints = edata->pointCount();
   double varia   = 0.0;

   for ( int ss = 0; ss < nscans; ss++ )
   {
      for ( int rr = 0; rr < npoints; rr++ )
      {
         double rval    = edata->value( ss, rr ) - sdata->value( ss, rr );
         varia         += sq( rval );
         rdata->setValue( ss, rr, rval );
      }
   }

   varia         /= (double)( nscans * npoints );
   mrec.variance  = varia;
   mrec.rmsd      = sqrt( varia );
   mrec.model     = model;
   mrecs[ 0 ]     = mrec;
}


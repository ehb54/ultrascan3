//! \file us_combine_models.cpp

#include <QApplication>

#include "us_combine_models.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_editor.h"
#include "us_select_runs.h"
#include "us_model_loader.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_sleep.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_CombineModels w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_CombineModels class constructor
US_CombineModels::US_CombineModels() : US_Widgets()
{
   // set up the GUI
   setWindowTitle( tr( "Create Global Model" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level   = US_Settings::us_debug();
   mfilter     = QString( "" );

   // primary layout
   QGridLayout* mainLayout  = new QGridLayout( this );
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   // fill in the GUI components
   QLabel* lb_main    = us_label(      tr( "Combine Models for Global Fit" ) );
           pb_prefilt = us_pushbutton( tr( "Select PreFilter" ) );
           le_prefilt = us_lineedit  ( "", -1, true );
           pb_add     = us_pushbutton( tr( "Add Models" ) );
           pb_reset   = us_pushbutton( tr( "Reset List" ) );
           pb_save    = us_pushbutton( tr( "Save Global Model" ) );
           pb_help    = us_pushbutton( tr( "Help" ) );
           pb_close   = us_pushbutton( tr( "Close" ) );
   dkdb_cntrls        = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   lw_models          = us_listwidget();

   int row       = 0;
   mainLayout->addWidget( lb_main,     row++, 0, 1, 4 );
   mainLayout->addWidget( lw_models,   row++, 0, 1, 4 );
   mainLayout->addLayout( dkdb_cntrls, row++, 0, 1, 4 );
   mainLayout->addWidget( pb_prefilt,  row,   0, 1, 1 );
   mainLayout->addWidget( le_prefilt,  row++, 1, 1, 3 );
   mainLayout->addWidget( pb_add,      row,   0, 1, 2 );
   mainLayout->addWidget( pb_reset,    row++, 2, 1, 2 );
   mainLayout->addWidget( pb_save,     row,   0, 1, 2 );
   mainLayout->addWidget( pb_help,     row,   2, 1, 1 );
   mainLayout->addWidget( pb_close,    row++, 3, 1, 1 );

   connect( pb_prefilt, SIGNAL( clicked()      ),
            this,       SLOT(   select_filt()  ) );
   connect( pb_add,     SIGNAL( clicked()      ),
            this,       SLOT(   add_models()   ) );
   connect( pb_reset,   SIGNAL( clicked()      ),
            this,       SLOT(   reset()        ) );
   connect( pb_close,   SIGNAL( clicked()      ),
            this,       SLOT(   close()        ) );
   connect( pb_help,    SIGNAL( clicked()      ),
            this,       SLOT(   help()         ) );
   connect( pb_save,    SIGNAL( clicked()      ),
            this,       SLOT(   save()         ) );

   lw_models ->setToolTip(
      tr( "List of models to combine for a global model" ) );
   pb_prefilt->setToolTip(
      tr( "Choose RunIDs to pre-filter the models list" ) );
   le_prefilt->setToolTip(
      tr( "Chosen RunID pre-filter to the models list" ) );
   pb_add    ->setToolTip(
      tr( "Add to the list of component models" ) );
   pb_reset  ->setToolTip(
      tr( "Clear the models list to allow re-selection" ) );
   pb_help   ->setToolTip(
      tr( "Display detailed US_CombineModels documentation text and images" ) );
   pb_close  ->setToolTip(
      tr( "Close the US_CombineModels window and exit" ) );
   pb_save   ->setToolTip(
      tr( "Create a global model from the listed component models" ) );
   pb_reset->setEnabled( false );
   pb_save ->setEnabled( false );

   runsel     = true;
   latest     = true;

   setMinimumSize( 480, 200 );
   adjustSize();
   show();
}

// Add selected model(s) to list of component models
void US_CombineModels::add_models()
{
   bool loadDB = dkdb_cntrls->db();
   QList< US_Model > tmodels;
   QStringList       tmdescs;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, tmodels, tmdescs, pfilts );
   dialog.move( this->pos() + QPoint( 200, 200 ) );

   connect( &dialog, SIGNAL(   changed( bool ) ),
            this, SLOT( update_disk_db( bool ) ) );
   QApplication::restoreOverrideCursor();

   if ( dialog.exec() != QDialog::Accepted )
      return;   // No selection was made


   for ( int jj = 0; jj < tmodels.count(); jj++ )
   {
      QString desc  = tmdescs[ jj ];
      QString sepa  = desc.left( 1 );
      QString mdesc = desc.section( sepa, 1, 1 );
      lw_models->addItem( mdesc );
      this->models << tmodels[ jj ];
      this->mdescs << mdesc;
   }

   pb_reset->setEnabled( true );
   pb_save ->setEnabled( models.size() > 1 );
}

// View
void US_CombineModels::reset()
{
   models    .clear();
   mdescs    .clear();
   lw_models->clear();
   pb_reset ->setEnabled( false );
   pb_save  ->setEnabled( false );
}

// Save
void US_CombineModels::save()
{
   const int mxtotrl = 60;
   QStringList runIDs;
   int      nmodels = models.size();
qDebug() << "SAVE:  nmodels" << nmodels;
   // Initialize output combo model from first input
   US_Model cmodel  = models[ 0 ];
   int      ncomps  = cmodel.components.size();
   int      ndupc   = 0;
qDebug() << "SAVE:   m0 ncomps" << ncomps;
   QString  runID   = mdescs[ 0 ].section( ".", 0, -4 );
   runIDs << runID;
   cmodel.update_coefficients();

   for ( int ii = 1; ii < nmodels; ii++ )
   {  // Add components from the remainder of the models
      US_Model* imodel = &models[ ii ];

      imodel->update_coefficients();
      int kcomps = imodel->components.size();
      runID      = mdescs[ ii ].section( ".", 0, -4 );
      bool cnstv = imodel->constant_vbar();
      bool cnstk = imodel->constant_ff0 ();
qDebug() << "SAVE:   im" << ii << "kcomps" << kcomps;

      if ( !runIDs.contains( runID ) )
         runIDs << runID;       // Build the list of unique run IDs

      for ( int jj = 0; jj < kcomps; jj++ )
      {  // Add model components to the combo model
         bool dupc    = false;
         US_Model::SimulationComponent sc = imodel->components[ jj ];
         double conc  = sc.signal_concentration;
         int kksv     = 0;

         if ( cnstv )
         {  // Test for duplicate where vbar20 is constant
            for ( int kk = 0; kk < ncomps; kk++ )
            {  // See if this component already exists
               if ( sc.s    == cmodel.components[ kk ].s  &&
                    sc.f_f0 == cmodel.components[ kk ].f_f0 )
               {  // This component is a duplicate, so break
                  kksv   = kk;
                  dupc   = true;
                  break;
               }
            }
         }
         else if ( cnstk )
         {  // Test for duplicate where f/f0 is constant
            for ( int kk = 0; kk < ncomps; kk++ )
            {  // See if this component already exists
               if ( sc.s      == cmodel.components[ kk ].s  &&
                    sc.vbar20 == cmodel.components[ kk ].vbar20 )
               {  // This component is a duplicate, so break
                  kksv   = kk;
                  dupc   = true;
                  break;
               }
            }
         }
         else
         {  // Test for duplicate where neither f/f0 nor vbar20 is constant
            for ( int kk = 0; kk < ncomps; kk++ )
            {  // See if this component already exists
               if ( sc.s      == cmodel.components[ kk ].s     &&
                    sc.f_f0   == cmodel.components[ kk ].f_f0  &&
                    sc.vbar20 == cmodel.components[ kk ].vbar20 )
               {  // This component is a duplicate, so break
                  kksv   = kk;
                  dupc   = true;
                  break;
               }
            }
         }

         if ( dupc )
         {  // Update concentration sum if duplicate component
            cmodel.components[ kksv ].signal_concentration += conc;
            ndupc++;
            continue;
         }
            
         // Add in any new unique component and bump count
         ncomps++;
         sc.name = QString::asprintf( "SC%04d", ncomps );
         cmodel.components << sc;
qDebug() << "SAVE:      NEW comp: ncomps" << ncomps;
      }
qDebug() << "SAVE:       ncomps" << ncomps;
   }
qDebug() << "SAVE:    ncomps" << ncomps << cmodel.components.size()
 << "ndupc" << ndupc << "nrunIDs" << runIDs.size();
   // Loop to scale concentrations by dividing by number of input models
   double cscale      = 1.0 / (double)nmodels;
   for ( int jj = 0; jj < ncomps; jj++ )
      cmodel.components[ jj ].signal_concentration *= cscale;

   // Default output name derives from the name of the first input
   cmodel_name        = "global-" + mdescs[ 0 ];
   QString mdlbnam    = cmodel_name.section( ".",  0, -3 ) + ".";
   QString mdlanno    = cmodel_name.section( ".", -2, -2 );
   QString mdlaedt    = mdlanno.section( "_", 0, 0 ) + "_";
   QString mdlaanl    = "a" + QDateTime::currentDateTime()
                        .toString( "yyMMddhhmm" ) + "_";
   QString mdlatyp    = mdlanno.section( "_", 2, 2 ).section( "-", 0, 0 )
                        + "-GL";
   QString mdliter    = "_local_i01.model";
   cmodel_name        = mdlbnam + mdlaedt + mdlaanl + mdlatyp + mdliter;
qDebug() << "SAVE:     cmodel_name" << cmodel_name;
   QString mdlguid    = US_Util::new_guid();
   cmodel.modelGUID   = mdlguid;
   cmodel.requestGUID = mdlguid;
   cmodel.global      = US_Model::GLOBAL;
   cmodel.monteCarlo  = false;

   // Open a dialog that reports and allows modification of description
   runID           = cmodel_name.section( ".",  0, -4 );
   QString odesc   = cmodel_name.section( ".", -3, -1 );
   QMessageBox mbox;

   QString msg1    = tr( "An output combined model has been created. "
                         "It's description is:<br/><b>" )
      + cmodel_name + "</b>.<br/><br/>"
      + tr( "It combines %1 models with a total of %2 unique components. "
            "Click:<br/><br/>" )
      .arg( nmodels ).arg( ncomps )
      + tr( "  <b>OK</b>     to output the model as is;<br/>"
            "  <b>Edit</b>   to modify the model description (runID);<br/>"
            "  <b>Cancel</b> to abort model creation.<br/>" );

   mbox.setWindowTitle( tr( "Save Global Model" ) );
   mbox.setText       ( msg1 );
   QPushButton *pb_ok   = mbox.addButton( tr( "OK" ),
         QMessageBox::YesRole );
   QPushButton *pb_edit = mbox.addButton( tr( "Edit" ) ,
         QMessageBox::AcceptRole );
   QPushButton *pb_canc = mbox.addButton( tr( "Cancel" ),
         QMessageBox::RejectRole );
   mbox.setEscapeButton ( pb_canc );
   mbox.setDefaultButton( pb_ok   );

   mbox.exec();

   if ( mbox.clickedButton() == pb_canc )  return;

   if ( mbox.clickedButton() == pb_edit )
   {  // Open another dialog to get a modified runID
      bool ok;
      bool getruni  = true;

      while ( getruni )
      {
         int curriln   = runID.length();
         QString msg2  = tr( "The default run ID for the output combined"
                             " model is <br/><b>" ) + runID + "</b>.<br/><br/>"
            + tr( "You may modify this part of the model description"
                  " (currently %1 characters).<br/>"
                  "Use alphanumeric characters, underscores, or hyphens"
                  " (no spaces).<br/>"
                  "Enter so the total is from 3 to %2 characters.<br/><br/>"
                  "It is <b>strongly</b> recommended that you only"
                  " <b>append</b> any descriptive string to the original<br/>"
                  "runID in order to facilitate finding the model in"
                  " subsequent model loader dialogs." )
                  .arg( curriln ).arg( mxtotrl );
         runID         = QInputDialog::getText( this,
               tr( "Modify Global Model Description RunID" ),
               msg2, QLineEdit::Normal, runID, &ok );

         if ( !ok )  return;

         runID.remove( QRegExp( "[^\\w\\d_-]" ) );
         int slen   = runID.length();
         getruni    = false;

         if ( slen < 3 )
            runID     += QString( "GLO" ).left( 3 - slen ); 

         else if ( slen > mxtotrl )
         {
            runID      = runID.left( mxtotrl );
            getruni    = true;
         }
      }

      cmodel_name = runID + "." + odesc;
qDebug() << "SAVE:     (2)cmodel_name" << cmodel_name;
   }

   cmodel.description = cmodel_name;

   // Output the combined model
   if ( dkdb_cntrls->db() )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      cmodel.write( &db );
qDebug() << "SAVE:      DB";
   }

   else
   {
      bool newFile;
      QString mdlpath;
      US_Model::model_path( mdlpath );
      QString fnamo = US_Model::get_filename( mdlpath, mdlguid, newFile );
      cmodel.write( fnamo );
qDebug() << "SAVE:      fnamo" << fnamo;
   }

}

// Update DB/Disk setting from the model loader dialog
void US_CombineModels::update_disk_db( bool isDB )
{
   if ( isDB )
      dkdb_cntrls->set_db();
   else
      dkdb_cntrls->set_disk();
}

// Select RunID/Edit prefilter of models list
void US_CombineModels::select_filt( void )
{
   QString pfmsg;
   int nruns     = 0;
   pfilts.clear();

   US_SelectRuns srdiag( dkdb_cntrls->db(), pfilts );
   srdiag.move( this->pos() + QPoint( 200, 200 ) );
   connect( &srdiag, SIGNAL( dkdb_changed  ( bool ) ),
            this,    SLOT  ( update_disk_db( bool ) ) );

   if ( srdiag.exec() == QDialog::Accepted )
      nruns         = pfilts.size();
   else
      pfilts.clear();

   if ( nruns == 0 )
      pfmsg = tr( "(none chosen)" );

   else if ( nruns == 1 )
      pfmsg = tr( "RunID prefilter - 1 run: " )
              + QString( pfilts[ 0 ] ).left( 20 ) + "...";

   else
      pfmsg = tr( "RunID prefilter - %1 runs: " ).arg( nruns )
              + QString( pfilts[ 0 ] ).left( 20 ) + "*,...";

   le_prefilt->setText( pfmsg );
}


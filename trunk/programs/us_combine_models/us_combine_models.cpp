//! \file us_combmodels.cpp

#include <QApplication>

#include "us_combine_models.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_passwd.h"
#include "us_editor.h"
#include "us_model_loader.h"
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
   mainLayout->addWidget( pb_add,      row,   0, 1, 2 );
   mainLayout->addWidget( pb_reset,    row++, 2, 1, 2 );
   mainLayout->addWidget( pb_save,     row,   0, 1, 2 );
   mainLayout->addWidget( pb_help,     row,   2, 1, 1 );
   mainLayout->addWidget( pb_close,    row++, 3, 1, 1 );

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

   lw_models->setToolTip(
      tr( "List of models to combine for a global model" ) );
   pb_add   ->setToolTip(
      tr( "Add to the list of component models" ) );
   pb_reset ->setToolTip(
      tr( "Clear the models list to allow re-selection" ) );
   pb_help  ->setToolTip(
      tr( "Display detailed US_CombineModels documentation text and images" ) );
   pb_close ->setToolTip(
      tr( "Close the US_CombineModels window and exit" ) );
   pb_save  ->setToolTip(
      tr( "Create a global model from the listed component models" ) );
   pb_reset->setEnabled( false );
   pb_save ->setEnabled( false );

   rbtn_click = false;

   show();
}

// Filter events to catch right-mouse-button-click on tree widget
bool US_CombineModels::eventFilter( QObject *obj, QEvent *e )
{
   if ( obj->objectName() == "tree-widget"  &&
        e->type() == QEvent::ContextMenu )
   {  // catch tree row right-mouse click
      rbtn_click = true;
DbgLv(1) << "eventFilter   rbtn_click" << rbtn_click;
      return false;
   }

   else
   {  // pass all others for normal handling
      return US_Widgets::eventFilter( obj, e );
   }
}

// Add selected model(s) to list of component models
void US_CombineModels::add_models()
{
   bool loadDB = dkdb_cntrls->db();
   QList< US_Model > tmodels;
   QStringList       tmdescs;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, tmodels, tmdescs );
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
   lw_models->clear();
   pb_reset->setEnabled( false );
   pb_save ->setEnabled( false );
}

// Save
void US_CombineModels::save()
{
   QStringList runIDs;
   int      nmodels = models.size();
qDebug() << "SAVE:  nmodels" << nmodels;
   // Initialize output combo model from first input
   US_Model cmodel  = models[ 0 ];
   int      ncomps  = cmodel.components.size();
   QString  runID   = mdescs[ 0 ].section( ".", 0, -4 );
   runIDs << runID;
   cmodel.update_coefficients();

   for ( int ii = 1; ii < nmodels; ii++ )
   {  // Add components from the remainder of the models
      US_Model* imodel = &models[ ii ];

      imodel->update_coefficients();
      int kcomps = imodel->components.size();
      runID      = mdescs[ ii ].section( ".", 0, -4 );

      if ( !runIDs.contains( runID ) )
         runIDs << runID;       // Build the list of unique run IDs

      for ( int jj = 0; jj < kcomps; jj++ )
      {  // Add model components to the combo model
         bool dupc = false;
         US_Model::SimulationComponent sc = imodel->components[ jj ];

         for ( int kk = 0; kk < ncomps; kk++ )
         {  // See if this component already exists
            if ( sc.s    == cmodel.components[ kk ].s  &&
                 sc.f_f0 == cmodel.components[ kk ].f_f0 )
            {  // This component is a duplicate, so break
               dupc = true;
               break;
            }
         }

         if ( dupc )  continue;     // Skip adding a duplicate component

         ncomps++;
         sc.name = QString().sprintf( "SC%04d", ncomps );
         cmodel.components << sc;   // Add a component and bump count
      }
   }
qDebug() << "SAVE:    ncomps" << ncomps << cmodel.components.size();
qDebug() << "SAVE:    nrunIDs" << runIDs.size();

   // Default output name derives from the name of the first input
   cmodel_name = "global-" + mdescs[ 0 ];
qDebug() << "SAVE:     cmodel_name" << cmodel_name;
   QString mdlpath;
   US_Model::model_path( mdlpath );
   QString mdlguid    = US_Util::new_guid();
   cmodel.modelGUID   = mdlguid;
   cmodel.global      = US_Model::GLOBAL;
   QString fnamo      = US_Model::get_filename( mdlpath, mdlguid );

   // Open a dialog that reports and allows modification of description
   runID           = cmodel_name.section( ".",  0, -4 );
   QString odesc   = cmodel_name.section( ".", -3, -1 );
   QMessageBox mbox;

   QString msg1    = tr( "An output combined model has been created. "
                         "It's description is:<br/><b>" )
      + cmodel_name + "</b>.<br/><br/>"
      + tr( "It combines %1 models with a total of %2 components. "
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
      bool    ok;
      QString msg2    = tr( "The default run ID for the output combined<br/>"
                            "model is <b>" ) + runID + "</b>.<br/><br/>"
         + tr( "You may modify this part of the model description.<br/>"
               "Use alphanumeric characters, underscores, or hyphens<br/>"
               "(no spaces). Enter 3 to 40 characters." );
      runID           = QInputDialog::getText( this,
            tr( "Modify Model Description RunID" ),
            msg2,
            QLineEdit::Normal,
            runID,
            &ok );

      if ( !ok )  return;

      runID.remove( QRegExp( "[^\\w\\d_-]" ) );
      int slen = runID.length();

      if ( slen < 3 )
         runID += QString( "GLO" ).left( 3 - slen ); 

      else if ( slen > 40 )
         runID  = runID.left( 40 );

      cmodel_name = runID + "." + odesc;
qDebug() << "SAVE:     (2)cmodel_name" << cmodel_name;
   }

   cmodel.description = cmodel_name;

   // Output the combined model
   cmodel.write( fnamo );
qDebug() << "SAVE:      fnamo" << fnamo;

}

// Update DB/Disk setting from the model loader dialog
void US_CombineModels::update_disk_db( bool isDB )
{
   if ( isDB )
      dkdb_cntrls->set_db();
   else
      dkdb_cntrls->set_disk();
}



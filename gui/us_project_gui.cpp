//! \file us_project_gui.cpp

#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_investigator.h"
#include "us_project.h"
#include "us_project_gui.h"

US_ProjectGui::US_ProjectGui( 
      bool  signal_wanted,
      int   select_db_disk,
      const US_Project& dataIn 
      ) : US_WidgetsDialog( 0, 0 ), project( dataIn )
{
   signal         = signal_wanted;
   investigatorID = US_Settings::us_inv_ID();

   setWindowTitle( tr( "Project Management" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* main      = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QFontMetrics fm( QFont( US_GuiSettings::fontFamily(),
                           US_GuiSettings::fontSize() ) );

   QStringList DB = US_Settings::defaultDB();
   if ( DB.isEmpty() ) DB << "Undefined";

   QLabel* lb_DB = us_banner( tr( "Database: " ) + DB.at( 0 ) );
//   lb_DB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_DB );

   // Second row - tab widget
   tabWidget           = us_tabwidget();
   generalTab          = new US_ProjectGuiGeneral( &investigatorID, select_db_disk );
   goalsTab            = new US_ProjectGuiGoals( );
   moleculesTab        = new US_ProjectGuiMolecules( );
   purityTab           = new US_ProjectGuiPurity( );
   expenseTab          = new US_ProjectGuiExpense( );
   bufferComponentsTab = new US_ProjectGuiBufferComponents( );
   saltInformationTab  = new US_ProjectGuiSaltInformation( );
   auc_questionsTab    = new US_ProjectGuiAUC_questions( );
   expDesignTab        = new US_ProjectGuiExpDesign( );
   notesTab            = new US_ProjectGuiNotes( );

   tabWidget -> addTab( generalTab,          tr( "1: General"           ) );
   tabWidget -> addTab( goalsTab,            tr( "2: Goals"             ) );
   tabWidget -> addTab( moleculesTab,        tr( "3: Molecules"         ) );
   tabWidget -> addTab( purityTab,           tr( "4: Purity"            ) );
   tabWidget -> addTab( expenseTab,          tr( "5: Expense"           ) );
   tabWidget -> addTab( bufferComponentsTab, tr( "6: Buffer Components" ) );
   tabWidget -> addTab( saltInformationTab,  tr( "7: Sample Handling"   ) );
   tabWidget -> addTab( auc_questionsTab,    tr( "8: AUC Questions"     ) );
   tabWidget -> addTab( expDesignTab,        tr( "9: Experiment Design" ) );
   tabWidget -> addTab( notesTab,            tr( "10: Notes"             ) );

   main->addWidget( tabWidget );

   // Some pushbuttons
   QHBoxLayout* buttons = new QHBoxLayout;

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( resetAll() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   pb_accept = us_pushbutton( tr( "Close" ) );

   if ( signal )
   {
      QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
      connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );
      buttons->addWidget( pb_cancel );

      pb_accept -> setText( tr( "Accept" ) );
   }

   connect( pb_accept, SIGNAL( clicked() ), SLOT( accept() ) );
   buttons->addWidget( pb_accept );
   
   // Global signals
   connect( generalTab, SIGNAL( source_changed ( bool ) ),
                        SLOT  ( source_changed ( bool ) ) );

   connect( generalTab, SIGNAL( newProject ( ) ),
                        SLOT  ( newProject ( ) ) );

   connect( generalTab, SIGNAL( load       ( ) ),
                        SLOT  ( load       ( ) ) );

   connect( generalTab, SIGNAL( selectProject( QListWidgetItem* ) ),
                        SLOT  ( selectProject( QListWidgetItem* ) ) );

   connect( generalTab, SIGNAL( saveDescription( const QString& ) ),
                        SLOT  ( saveDescription( const QString& ) ) );

   connect( generalTab, SIGNAL( save           ( ) ),
                        SLOT  ( saveProject    ( ) ) );

   connect( generalTab, SIGNAL( deleteProject  ( ) ),
                        SLOT  ( deleteProject  ( ) ) );

   connect( goalsTab,   SIGNAL( goalsTabChanged ( ) ),
                        SLOT  ( tabTextChanged  ( ) ) );

   connect( moleculesTab,SIGNAL( moleculesTabChanged ( ) ),
                         SLOT  ( tabTextChanged      ( ) ) );

   connect( purityTab,   SIGNAL( purityTabChanged ( const QString& ) ),
                         SLOT  ( tabTextChanged   ( const QString& ) ) );

   connect( expenseTab,  SIGNAL( expenseTabChanged ( ) ),
                         SLOT  ( tabTextChanged    ( ) ) );

   connect( bufferComponentsTab,SIGNAL( bufferComponentsTabChanged ( ) ),
                                SLOT  ( tabTextChanged             ( ) ) );

   connect( saltInformationTab, SIGNAL( saltInformationTabChanged ( ) ),
                                SLOT  ( tabTextChanged            ( ) ) );

   connect( auc_questionsTab,   SIGNAL( AUC_questionsTabChanged ( ) ),
                                SLOT  ( tabTextChanged          ( ) ) );

   connect( expDesignTab,SIGNAL( expDesignTabChanged ( ) ),
                         SLOT  ( tabTextChanged      ( ) ) );

   connect( notesTab,    SIGNAL( notesTabChanged ( ) ),
                         SLOT  ( tabTextChanged  ( ) ) );

   // Now let's assemble the page
   
   main->addLayout( buttons );
   
   // Load the project descriptions
   load();

   // Select the current one if we know what it is
   if ( project.projectID > 0 )
   {
      QList< QListWidgetItem* > items 
        = generalTab->lw_projects->findItems( project.projectDesc, Qt::MatchExactly );

      // Should be exactly 1, but let's make sure
      if ( items.size() == 1 )
      {
         selectProject( items[ 0 ] );
         generalTab->lw_projects->setCurrentItem( items[ 0 ] );
      }
   }
}

// Function to clean up all the tab pointers
US_ProjectGui::~US_ProjectGui()
{
   delete generalTab                  ;
   delete goalsTab                    ; 
   delete moleculesTab                ;
   delete purityTab                   ;
   delete expenseTab                  ;
   delete bufferComponentsTab         ;
   delete saltInformationTab          ;
   delete auc_questionsTab            ;
   delete expDesignTab                ;
   delete notesTab                    ;
   delete tabWidget                   ;
}

// Function to enable/disable buttons
void US_ProjectGui::reset( void )
{
   generalTab          ->setGUID             ( project.projectGUID      );
   generalTab          ->setDesc             ( project.projectDesc      );
   goalsTab            ->setGoals            ( project.goals            );
   moleculesTab        ->setMolecules        ( project.molecules        );
   purityTab           ->setPurity           ( project.purity           );
   expenseTab          ->setExpense          ( project.expense          );
   bufferComponentsTab ->setBufferComponents ( project.bufferComponents );
   saltInformationTab  ->setSaltInformation  ( project.saltInformation  );
   auc_questionsTab    ->setAUC_questions    ( project.AUC_questions    );
   expDesignTab        ->setExpDesign        ( project.expDesign        );
   notesTab            ->setNotes            ( project.notes            );
   generalTab          ->setLastUpd          ( project.lastUpdated.toString( "yyyy/MM/dd hh:mm UTC" ) );

   text_changed = false;

   enableButtons();

   generalTab->reset();
}

// Function to determine which buttons should be enabled
void US_ProjectGui::enableButtons( void )
{
   // Let's calculate if we're eligible to save this project
   generalTab->pb_save-> setEnabled( text_changed );

   // We can always delete something, even if it's just what's in the dialog
   generalTab->pb_del->setEnabled( false );

   if ( generalTab->lw_projects->currentRow() != -1 )
   {
      generalTab->pb_del->setEnabled( true );
   }

   // Figure out if the accept button should be enabled
   if ( ! signal )      // Then it's just a close button
      pb_accept->setEnabled( true );

   else if ( project.saveStatus == US_Project::BOTH )
      pb_accept->setEnabled( true );

   else if ( ! generalTab->disk_controls->db() && project.saveStatus == US_Project::HD_ONLY )
      pb_accept->setEnabled( true );

   else if (   generalTab->disk_controls->db() && project.saveStatus == US_Project::DB_ONLY )
      pb_accept->setEnabled( true );

   else
      pb_accept->setEnabled( false );
}

// Function to clear out all data
void US_ProjectGui::resetAll( void )
{
   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();
   lastUpds .clear();

   projectMap.clear();

   info.clear();
   project.clear();

   generalTab->lw_projects->clear();
   reset();
}

// Function to accept the current set of projects and return
void US_ProjectGui::accept( void )
{
   if ( signal )
   {
      if ( generalTab->le_projectDesc->text().isEmpty() )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "Please enter a description for\n"
                   "your project before accepting." ) );
         return;
      }

      emit updateProjectGuiSelection( project );
   }

   close();
}

// Function to cancel the current dialog and return
void US_ProjectGui::cancel( void )
{
   if ( signal )
      emit cancelProjectGuiSelection();

   close();
}

// Function to create a new project
void US_ProjectGui::newProject( void )
{
   resetAll();

   generalTab          ->setDesc             ( tr( "New Project" ) );

   goalsTab            ->setGoals            ( tr( "Please replace with an introduction to your "
                                                   "research project and explain the goals "
                                                   "of your research. We will use this "
                                                   "information to optimally design your "
                                                   "experiment. Please provide enough background "
                                                   "so we can assess the biological significance "
                                                   "of this research." ) );
   moleculesTab        ->setMolecules        ( tr( "Please include their approximate molecular weights" ) );
   purityTab           ->setPurity           ( tr( "%" ) );
   expenseTab          ->setExpense          ( tr( "Would this expense be acceptable? If not, what amount "
                                                   "would you feel comfortable with?" ) );
   bufferComponentsTab ->setBufferComponents ( tr( "What buffers do you plan to use? Is phosphate or "
                                                   "MOPS buffer an option?\n"
                                                   "To minimize absorbance we prefer to run phosphate or "
                                                   "MOPS buffers in low concentration (~ 5-10 mM). Salts "
                                                   "also absorb and should be kept to a minimum, although "
                                                   "a certain ionic sliength (25-50 mM) is desired to aid "
                                                   "with the hydrodynamic ideality behavior.\n\n"
                                                   "Do you need to have drugs in your sample, such as "
                                                   "reductants and nucleotide analogs?\n"
                                                   "Please list all components in your buffer. If reductants "
                                                   "are required it is essential that you use TCEP, which "
                                                   "can be used at 280 nm, but not lower wavelengths.\n\n"
                                                   "Please use this space to list all buffer components:" ) );
   saltInformationTab  ->setSaltInformation  ( tr( "If not, please explain why not." ) );
   auc_questionsTab    ->setAUC_questions    ( tr( "How do you propose to approach the research with "
                                                   "AUC experiments?" ) );
   expDesignTab        ->setExpDesign        ( tr( "Please enter any notes about the experiment design." ) );
   notesTab            ->setNotes            ( tr( "Special instructions, questions, and notes (optional)" ) );

   enableButtons();
}

// Function to load projects into projects list widget
void US_ProjectGui::load( void )
{
   if ( generalTab->disk_controls->db() )
      loadDB();

   else
      loadDisk();

   text_changed = false;
   enableButtons();

}

// Function to load projects from disk
void US_ProjectGui::loadDisk( void )
{
   QString path;
   if ( ! project.diskPath( path ) ) return;

   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();
   lastUpds .clear();
   QDateTime lastUpd;

   QDir dir( path );
   QStringList filter( "P*.xml" );
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );

   QFile a_file;

   for ( int i = 0; i < names.size(); i++ )
   {
      QString xfn = path + "/" + names[ i ];
      a_file.setFileName( xfn );

      if ( ! a_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

      lastUpd = QFileInfo( xfn ).lastModified().toUTC();
qDebug() << "i" << i << "xfn" << xfn << "lastUpd" << lastUpd;
      QXmlStreamReader xml( &a_file );

      while ( ! xml.atEnd() )
      {
         xml.readNext();

         if ( xml.isStartElement() )
         {
            if ( xml.name() == "project" )
            {
               QXmlStreamAttributes a = xml.attributes();

               IDs          << a.value( "id" ).toString();
               GUIDs        << a.value( "guid" ).toString();
               filenames    << xfn;
               lastUpds     << lastUpd;

            }

            else if ( xml.name() == "description" )
            {
               xml.readNext();
               descriptions << xml.text().toString();
            }
         }
      }

      a_file.close();
   }

   loadProjects();
}

// Function to load projects from db
void US_ProjectGui::loadDB( void )
{
   US_Passwd pw;
   QString masterPW = pw.getPasswd();
   IUS_DB2 db( masterPW );

   if ( db.lastErrno() != IUS_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   if ( investigatorID < 1 ) investigatorID = US_Settings::us_inv_ID();

   QStringList q( "get_project_desc" );
   q << QString::number( investigatorID );
   db.query( q );

   if ( db.lastErrno() != IUS_DB2::OK ) return;

   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();
   lastUpds .clear();

   while ( db.next() )
   {
      QString newID = db.value( 0 ).toString();
      IDs          << newID;
      descriptions << db.value( 1 ).toString();
      GUIDs        << QString( "" );
      filenames    << QString( "" );
      lastUpds     << db.value( 13 ).toDateTime();
   }

   loadProjects();
}

// Function to load the projects list widget from the projects data structure
void US_ProjectGui::loadProjects( void )
{
   generalTab->lw_projects->clear();
   info.clear();
   projectMap.clear();

   for ( int i = 0; i < descriptions.size(); i++ )
   {
      ProjectInfo si;
      si.projectID   = IDs         [ i ].toInt();
      si.description = descriptions[ i ];
      si.GUID        = GUIDs       [ i ];
      si.filename    = filenames   [ i ];
      si.lastUpdated = lastUpds    [ i ];
      si.index       = i;
      info << si;
qDebug() << "index" << si.index << "lastUpd" << si.lastUpdated;

      // Create a map to account for automatic sorting of the list
      QListWidgetItem* item = new QListWidgetItem( descriptions[ i ], generalTab->lw_projects );
      projectMap[ item ] = i;

      generalTab->lw_projects->addItem( item );
   }
}

// Function to handle when project listwidget item is selected
void US_ProjectGui::selectProject( QListWidgetItem* item )
{
   // Account for the fact that the list has been sorted
   int     ndx         = projectMap[ item ];
   int     projectID   = info[ ndx ].projectID;
   QString projectGUID = info[ ndx ].GUID;

   project.clear();

   int status = IUS_DB2::OK;

   if ( generalTab->disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      IUS_DB2 db( masterPW );
   
      if ( db.lastErrno() != IUS_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      status = project.readFromDB  ( projectID, &db );

      // Error reporting 
      if ( status == IUS_DB2::NO_PROJECT )
      { 
         QMessageBox::information( this, 
               tr( "Attention" ), 
               tr( "The project was not found.\n" 
                   "Please restore and try again.\n" ) ); 
      } 
      
      else if ( status != IUS_DB2::OK )
         db_error( db.lastError() );
   }

   else
   {
      status = project.readFromDisk( projectGUID );

      // Error reporting 
      if ( status == IUS_DB2::NO_PROJECT )
      { 
         QMessageBox::information( this, 
               tr( "Attention" ), 
               tr( "The project was not found.\n" 
                   "Please select an existing project and try again.\n" ) ); 
      } 
      
      else if ( status != IUS_DB2::OK )
      { 
         QMessageBox::information( this, 
               tr( "Disk Read Problem" ), 
               tr( "Could not read data from the disk.\n" 
                   "Disk status: " ) + QString::number( status ) ); 
      }
   }

   reset();
}

// Function to update the description associated with the current project
void US_ProjectGui::saveDescription( const QString& )
{
   project.projectDesc = generalTab->le_projectDesc->text();

   // Find the description in the lw; first making sure an item is selected
   if ( generalTab->lw_projects->currentRow() > 0 )
   {
      QListWidgetItem* item = generalTab->lw_projects->currentItem();
      item->setText( project.projectDesc );
      generalTab->lw_projects->setCurrentItem( item );
   }

   // Enable save
   text_changed = true;
   generalTab->pb_save-> setEnabled( true );
}

// Function to save project information from all tabs to disk or db
void US_ProjectGui::saveProject( void )
{
   // Load information from tabs
   project.projectDesc      = generalTab          ->getDesc();
   project.goals            = goalsTab            ->getGoals();
   project.molecules        = moleculesTab        ->getMolecules();
   project.purity           = purityTab           ->getPurity();
   project.expense          = expenseTab          ->getExpense();
   project.bufferComponents = bufferComponentsTab ->getBufferComponents();
   project.saltInformation  = saltInformationTab  ->getSaltInformation();
   project.AUC_questions    = auc_questionsTab    ->getAUC_questions();
   project.expDesign        = expDesignTab        ->getExpDesign();
   project.notes            = notesTab            ->getNotes();

   if ( generalTab->disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      IUS_DB2 db( masterPW );
   
      if ( db.lastErrno() != IUS_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      int status = project.saveToDB( &db );

      if ( status == IUS_DB2::NO_PROJECT )
      {
         QMessageBox::information( this,
               tr( "Attention" ),
               tr( "There was a problem saving the project to the database.\n" ) );
         return;
      }

      else if ( status != IUS_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

   }

   else
      project.saveToDisk();

   QMessageBox::information( this,
         tr( "Save results" ),
         tr( "Project saved" ) );

   // Refresh project list
   load();
   reset();

   // Select the current item in the list
   QList< QListWidgetItem* > items 
     = generalTab->lw_projects->findItems( project.projectDesc, Qt::MatchExactly );

   // should be exactly 1, but let's make sure
   if ( items.size() == 1 )
   {
      selectProject( items[ 0 ] );
      generalTab->lw_projects->setCurrentItem( items[ 0 ] );
   }
}

// Function to delete a project from disk, db, or in the current form
void US_ProjectGui::deleteProject( void )
{
   if ( generalTab->disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      IUS_DB2 db( masterPW );
   
      if ( db.lastErrno() != IUS_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      project.deleteFromDB( &db );
   }

   else
      project.deleteFromDisk();

   project.clear();
   load();
   reset();

   QMessageBox::information( this,
         tr( "Delete results" ),
         tr( "Project Deleted" ) );
}

// Function to change the data source (disk/db)
void US_ProjectGui::source_changed( bool db )
{
   QStringList DB = US_Settings::defaultDB();

   if ( DB.size() < 5 )
   {
      QMessageBox::warning( this,
         tr( "Attention" ),
         tr( "There is no default database set." ) );
   }

   emit use_db( db );
   qApp->processEvents();

   // Clear out project list
   projectMap.clear();
   generalTab->lw_projects->clear();

   load();
   reset();
}

void US_ProjectGui::tabTextChanged( const QString& )
{
   text_changed = true;
   generalTab->pb_save-> setEnabled( true );
}

// Function to display an error returned from the database
void US_ProjectGui::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

US_ProjectGuiGeneral::US_ProjectGuiGeneral( int* invID,
                                            int  select_db_disk
                                            ) : US_Widgets() 
{
   investigatorID = invID;

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* general      = new QGridLayout( this );
   general->setSpacing         ( 2 );
   general->setContentsMargins ( 2, 2, 2, 2 );
   general->setColumnStretch( 0, 3.0 );
   general->setColumnStretch( 1, 1.0 );
   general->setColumnStretch( 2, 1.0 );

   int row = 0;

   // First row
   if ( US_Settings::us_inv_level() > 2 )
   {
      QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
      connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
      general->addWidget( pb_investigator, row, 0 );
   }
   else
   {
      QLabel* lb_investigator = us_label( tr( "Investigator:" ) );
      general->addWidget( lb_investigator, row, 0 );
   }
      
   le_investigator = us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   general->addWidget( le_investigator, row++, 1, 1, 2 );

   // Row 2 - Available projects
   QLabel* lb_banner2 = us_banner( tr( "Click on project to select" ), -2  );
   general->addWidget( lb_banner2, row, 0 );

   // Radio buttons
   disk_controls = new US_Disk_DB_Controls( select_db_disk );
   connect( disk_controls, SIGNAL( changed       ( bool ) ),
                           SIGNAL( source_changed( bool ) ) );
   general->addLayout( disk_controls, row++, 1, 1, 2 );

   // Row 3
   lw_projects = us_listwidget();
   lw_projects-> setSortingEnabled( true );
   connect( lw_projects, SIGNAL( itemClicked    ( QListWidgetItem* ) ),
                         SIGNAL( selectProject  ( QListWidgetItem* ) ) );

   int add_rows = ( US_Settings::us_debug() == 0 ) ? 6 : 8;

   general->addWidget( lw_projects, row, 0, add_rows, 1 );

   // Row 4
   pb_query = us_pushbutton( tr( "Query Projects" ), true );
   connect( pb_query, SIGNAL( clicked() ), SIGNAL( load() ) );
   general->addWidget( pb_query, row, 1 );

   pb_save = us_pushbutton( tr( "Save Project" ), false );
   connect( pb_save, SIGNAL( clicked() ), SIGNAL( save() ) );
   general->addWidget( pb_save, row++, 2 );

   // Row 5
   pb_newProject = us_pushbutton( tr( "New Project" ), true );
   connect( pb_newProject, SIGNAL( clicked() ), SIGNAL( newProject() ) );
   general->addWidget( pb_newProject, row, 1 );

   pb_del = us_pushbutton( tr( "Delete Project" ), false );
   connect( pb_del, SIGNAL( clicked() ), SIGNAL( deleteProject() ) );
   general->addWidget( pb_del, row++, 2 );

   // Row 6
   QLabel* lb_projectDesc = us_label( tr( "Project Name:" ) );
   general->addWidget( lb_projectDesc, row++, 1, 1, 2 );

   // Row 7
   le_projectDesc = us_lineedit( "", 1 );
   connect( le_projectDesc, SIGNAL( textEdited      ( const QString&   ) ),
                            SIGNAL( saveDescription ( const QString&   ) ) );
   general->addWidget( le_projectDesc, row++, 1, 1, 2 );

   // Row 8
   QLabel* lbl_lastupd = us_label( tr( "Last Updated:" ) );
   general->addWidget( lbl_lastupd, row++, 1, 1, 2 );

   // Row 9
   le_lastupd = us_lineedit( "" );
   le_lastupd->setPalette ( gray );
   le_lastupd->setReadOnly( true );
   general->addWidget( le_lastupd, row++, 1, 1, 2 );

   // Row 8
   QLabel* lb_guid = us_label( tr( "Global Identifier:" ) );
   general->addWidget( lb_guid, row++, 1, 1, 2 );

   // Row 9
   le_guid = us_lineedit( "" ); 
   le_guid->setPalette ( gray );
   le_guid->setReadOnly( true );
   general->addWidget( le_guid, row++, 1, 1, 2 );

   if ( US_Settings::us_debug() == 0 )
   {
     lb_guid->setVisible( false );
     le_guid->setVisible( false );
   }

   reset();       // This is the GeneralTab reset();
}

// Function to refresh the display with values from the project structure,
//  and to enable/disable features
void US_ProjectGuiGeneral::reset( void )
{
   // Display investigator
   if ( *investigatorID == 0 )
      *investigatorID = US_Settings::us_inv_ID();
   
   QString number = ( *investigatorID > 0 )
      ? QString::number( *investigatorID ) + ": " 
      : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
}

// Function to select the current investigator
void US_ProjectGuiGeneral::sel_investigator( void )
{
   US_Investigator* inv_dialog = new US_Investigator( true, *investigatorID );

   connect( inv_dialog,
      SIGNAL( investigator_accepted( int ) ),
      SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Function to assign the selected investigator as current
void US_ProjectGuiGeneral::assign_investigator( int invID )
{
   *investigatorID = invID;

   QString number = ( *investigatorID > 0 )
      ? QString::number( *investigatorID ) + ": " 
      : "";

   le_investigator->setText( number + US_Settings::us_inv_name() );
}

void US_ProjectGuiGeneral::setGUID( QString newGUID )
{
   le_guid->setText( newGUID );
}

void US_ProjectGuiGeneral::setLastUpd( QString lastupd )
{
   le_lastupd->setText( lastupd );
}

void US_ProjectGuiGeneral::setDesc( QString newDesc )
{
   le_projectDesc->setText( newDesc );
}

QString US_ProjectGuiGeneral::getDesc( void )
{
   return( le_projectDesc->text() );
}

US_ProjectGuiGoals::US_ProjectGuiGoals( void ) : US_Widgets()
{
   QVBoxLayout* goals = new QVBoxLayout;

   QLabel* lb_goals = 
      us_label( tr( "Please provide a detailed description"
                    " of your research.\n"
                    "Include an abstract and explain"
                    " the goals of your research.\n"
                    "We will use this information to optimally"
                    " design your experiment:" ) ); 

   goals->addWidget( lb_goals );

   te_goals = us_textedit();
   connect( te_goals, SIGNAL( textChanged   () ), 
                      SIGNAL( goalsTabChanged() ) );
   goals->addWidget( te_goals );
   te_goals->setReadOnly( false );
   setLayout( goals );
}

void US_ProjectGuiGoals::setGoals( QString newGoals )
{
   te_goals->setText( newGoals );
}

QString US_ProjectGuiGoals::getGoals( void )
{
   return( te_goals->toPlainText() );
}

US_ProjectGuiMolecules::US_ProjectGuiMolecules( void ) : US_Widgets()
{
   QVBoxLayout* molecules = new QVBoxLayout;

   QLabel* lb_molecules = 
      us_label( tr( "For each analyte to be measured,"
                    " please provide a name,\n"
                    "appropriate molar mass, partial specific volume"
                    " (if it was measured),\n"
                    "and, for proteins, the sequence in single-letter code.\n"
                    "For each submitted sample, please enter a detailed"
                    " sample description\n"
                    "key for the label used on each tube:" ) );

   molecules->addWidget( lb_molecules );

   te_molecules = us_textedit();
   connect( te_molecules, SIGNAL( textChanged        () ), 
                          SIGNAL( moleculesTabChanged() ) );
   molecules->addWidget( te_molecules );
   te_molecules->setReadOnly( false );
   setLayout( molecules );
}

void US_ProjectGuiMolecules::setMolecules( QString newMolecules )
{
   te_molecules->setText( newMolecules );
}

QString US_ProjectGuiMolecules::getMolecules( void )
{
   return( te_molecules->toPlainText() );
}

US_ProjectGuiPurity::US_ProjectGuiPurity( void ) : US_Widgets()
{
   QVBoxLayout* purity = new QVBoxLayout;

   QLabel* lb_purity = 
      us_label( tr( "Please indicate the approximate purity of your sample(s).\n"
                    "You can express it in percent:" ) );

   purity->addWidget( lb_purity );

   le_purity = us_lineedit();
   connect( le_purity, SIGNAL( textChanged     ( const QString& ) ), 
                       SIGNAL( purityTabChanged( const QString& ) ) );
   purity->addWidget( le_purity );
   le_purity->setReadOnly( false );
   setLayout( purity );
}

void US_ProjectGuiPurity::setPurity( QString newPurity )
{
   le_purity->setText( newPurity );
}

QString US_ProjectGuiPurity::getPurity( void )
{
   return( le_purity->text() );
}

US_ProjectGuiExpense::US_ProjectGuiExpense( void ) : US_Widgets()
{
   QVBoxLayout* expense = new QVBoxLayout;

   QLabel* lb_expense = 
      us_label( tr( "How much material (volume, concentration) is"
                    " available for your research?\n"
                    "Please identify concentration in absorbance units,"
                    " wavelength, or\n"
                    "type of fluorescense label and its molar concentration.\n"
                    "For samples to be measured with UV/visible"
                    " absorbance optics,\n"
                    "provide an absorbance scan against buffer from 215-700 nm for each sample:" ) );

   expense->addWidget( lb_expense );

   te_expense = us_textedit();
   connect( te_expense, SIGNAL( textChanged      () ), 
                        SIGNAL( expenseTabChanged() ) );
   expense->addWidget( te_expense );
   te_expense->setReadOnly( false );
   setLayout( expense );
}

void US_ProjectGuiExpense::setExpense( QString newExpense )
{
   te_expense->setText( newExpense );
}

QString US_ProjectGuiExpense::getExpense( void )
{
   return( te_expense->toPlainText() );
}

US_ProjectGuiBufferComponents::US_ProjectGuiBufferComponents( void ) : US_Widgets()
{
   QVBoxLayout* bufferComponents = new QVBoxLayout;

   QLabel* lb_bufferComponents = 
      us_label( tr( "Please list the molar concentrations of each component"
                    " in your buffer,\n"
                    "including reductants and nucleotides.\n"
                    "Provide a buffer scan against ddH2O from 215-700 nm.\n"
                    "To minimize absorbance, we prefer to use phosphate or optically"
                    " pure TRIS.\n"
                    "To avoid hydrodynamic non-ideality, a minimum salt concentration"
                    " of 20 mM is desired.\n"
                    "Please explain if this is not possible. If reductants are required,"
                    " please use TCEP\n"
                    "which can be measured at wavelengths > 260 nm:" ) );

   bufferComponents->addWidget( lb_bufferComponents );

   te_bufferComponents = us_textedit();
   connect( te_bufferComponents, SIGNAL( textChanged               () ), 
                                 SIGNAL( bufferComponentsTabChanged() ) );
   bufferComponents->addWidget( te_bufferComponents );
   te_bufferComponents->setReadOnly( false );
   setLayout( bufferComponents );
}

void US_ProjectGuiBufferComponents::setBufferComponents( QString newBufferComponents )
{
   te_bufferComponents->setText( newBufferComponents );
}

QString US_ProjectGuiBufferComponents::getBufferComponents( void )
{
   return( te_bufferComponents->toPlainText() );
}

US_ProjectGuiSaltInformation::US_ProjectGuiSaltInformation( void ) : US_Widgets()
{
   QVBoxLayout* saltInformation = new QVBoxLayout;

   QLabel* lb_saltInformation = 
      us_label( tr( "Please indicate the ideal storage conditions"
                    " (room temperature,\n"
                    "4, -20, and/or -80 degrees Celcius), the shelf"
                    " life of your sample,\n"
                    "any health/safety concerns (is it toxic if ingested or"
                    " injected, etc.)\n"
                    "and, if so, proper care to be taken when working with and\n"
                    "disposing of sample.\n"
                    "State if you would like to have the remaining used or unused"
                    " sample returned\n"
                    "to you (you will need to pay for return shipment)"
                    " and indicate how to\n"
                    "properly dispose of the sample:" ) );

   saltInformation->addWidget( lb_saltInformation );

   te_saltInformation = us_textedit();
   connect( te_saltInformation, SIGNAL( textChanged              () ), 
                                SIGNAL( saltInformationTabChanged() ) );
   saltInformation->addWidget( te_saltInformation );
   te_saltInformation->setReadOnly( false );
   setLayout( saltInformation );
}

void US_ProjectGuiSaltInformation::setSaltInformation( QString newSaltInformation )
{
   te_saltInformation->setText( newSaltInformation );
}

QString US_ProjectGuiSaltInformation::getSaltInformation( void )
{
   return( te_saltInformation->toPlainText() );
}

US_ProjectGuiAUC_questions::US_ProjectGuiAUC_questions( void ) : US_Widgets()
{
   QVBoxLayout* auc_questions = new QVBoxLayout;

   QLabel* lb_auc_questions = 
      us_label( tr( "What questions are you trying to answer with AUC?\n"
                    "How do you propose to approach the research"
                    " with AUC experiments?" ) );
   auc_questions->addWidget( lb_auc_questions );

   te_auc_questions = us_textedit();
   connect( te_auc_questions, SIGNAL( textChanged            () ), 
                              SIGNAL( AUC_questionsTabChanged() ) );
   auc_questions->addWidget( te_auc_questions );
   te_auc_questions->setReadOnly( false );
   setLayout( auc_questions );
}

void US_ProjectGuiAUC_questions::setAUC_questions( QString newAUC_questions )
{
   te_auc_questions->setText( newAUC_questions );
}

QString US_ProjectGuiAUC_questions::getAUC_questions( void )
{
   return( te_auc_questions->toPlainText() );
}

US_ProjectGuiExpDesign::US_ProjectGuiExpDesign( void ) : US_Widgets()
{
   QVBoxLayout* exp_design = new QVBoxLayout;

   QLabel* lb_exp_design = 
      us_label( tr( "The AUC experimental design:" ) );
   exp_design->addWidget( lb_exp_design );

   te_exp_design = us_textedit();
   connect( te_exp_design, SIGNAL( textChanged        () ), 
                           SIGNAL( expDesignTabChanged() ) );
   exp_design->addWidget( te_exp_design );
   te_exp_design->setReadOnly( false );

   setLayout( exp_design );
}

void US_ProjectGuiExpDesign::setExpDesign( QString newExpDesign )
{
   te_exp_design->setText( newExpDesign );
}

QString US_ProjectGuiExpDesign::getExpDesign( void )
{
   return( te_exp_design->toPlainText() );
}

US_ProjectGuiNotes::US_ProjectGuiNotes( void ) : US_Widgets()
{
   QVBoxLayout* notes = new QVBoxLayout;

   QLabel* lb_notes = us_label( tr( "Special instructions, questions, and notes (optional):" ) );
   notes->addWidget( lb_notes );

   te_notes = us_textedit();
   connect( te_notes, SIGNAL( textChanged    () ), 
                      SIGNAL( notesTabChanged() ) );
   notes->addWidget( te_notes );
   te_notes->setReadOnly( false );

   setLayout( notes );
}

void US_ProjectGuiNotes::setNotes( QString newNotes )
{
   te_notes->setText( newNotes );
}

QString US_ProjectGuiNotes::getNotes( void )
{
   return( te_notes->toPlainText() );
}

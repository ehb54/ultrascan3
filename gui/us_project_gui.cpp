//! \file us_project_gui.cpp

#include <QtGui>

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
   lb_DB->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   main->addWidget( lb_DB );

   // Second row - tab widget
   tabWidget           = us_tabwidget();
   generalTab          = new US_ProjectGuiGeneral( &investigatorID, select_db_disk );
   goalsTab            = new GoalsTab( );
   moleculesTab        = new MoleculesTab( );
   purityTab           = new PurityTab( );
   expenseTab          = new ExpenseTab( );
   bufferComponentsTab = new BufferComponentsTab( );
   saltInformationTab  = new SaltInformationTab( );
   auc_questionsTab    = new AUC_questionsTab( );
   notesTab            = new NotesTab( );

   tabWidget -> addTab( generalTab,          tr( "1: General"           ) );
   tabWidget -> addTab( goalsTab,            tr( "2: Goals"             ) );
   tabWidget -> addTab( moleculesTab,        tr( "3: Molecules"         ) );
   tabWidget -> addTab( purityTab,           tr( "4: Purity"            ) );
   tabWidget -> addTab( expenseTab,          tr( "5: Expense"           ) );
   tabWidget -> addTab( bufferComponentsTab, tr( "6: Buffer Components" ) );
   tabWidget -> addTab( saltInformationTab,  tr( "7: Salt Information"  ) );
   tabWidget -> addTab( auc_questionsTab,    tr( "8: AUC Questions"     ) );
   tabWidget -> addTab( notesTab,            tr( "9: Notes"             ) );

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

   // Now let's assemble the page
   
   main->addLayout( buttons );
   
   // Load the project descriptions
   load();

   // Select the current one if we know what it is
   if ( project.projectID > 0 )
   {
      QList< QListWidgetItem* > items 
        = generalTab->lw_projects->findItems( project.projectDesc, Qt::MatchExactly );

      // should be exactly 1, but let's make sure
      if ( items.size() == 1 )
      {
         selectProject( items[ 0 ] );
         generalTab->lw_projects->setCurrentItem( items[ 0 ] );
      }
   }
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
   notesTab            ->setNotes            ( project.notes            );

   enableButtons();

   generalTab->reset();
}

// Function to determine which buttons should be enabled
void US_ProjectGui::enableButtons( void )
{
   // Let's calculate if we're eligible to save this project
   generalTab->pb_save-> setEnabled( false );
   if ( ! goalsTab            ->getGoals().isEmpty()            &&
        ! moleculesTab        ->getMolecules().isEmpty()        &&
        ! purityTab           ->getPurity().isEmpty()           &&
        ! expenseTab          ->getExpense().isEmpty()          &&
        ! bufferComponentsTab ->getBufferComponents().isEmpty() &&
        ! saltInformationTab  ->getSaltInformation().isEmpty()  &&
        ! auc_questionsTab    ->getAUC_questions().isEmpty()    &&
        ! notesTab            ->getNotes().isEmpty()            &&
        ! generalTab          ->getDesc().isEmpty()             )
   {
      generalTab->pb_save->setEnabled( true );
   }

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

   QDir dir( path );
   QStringList filter( "P*.xml" );
   QStringList names = dir.entryList( filter, QDir::Files, QDir::Name );

   QFile a_file;

   for ( int i = 0; i < names.size(); i++ )
   {
      a_file.setFileName( path + "/" + names[ i ] );

      if ( ! a_file.open( QIODevice::ReadOnly | QIODevice::Text) ) continue;

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
               filenames    << path + "/" + names[ i ];

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
   US_DB2 db( masterPW );

   if ( db.lastErrno() != US_DB2::OK )
   {
      db_error( db.lastError() );
      return;
   }

   if ( investigatorID < 1 ) investigatorID = US_Settings::us_inv_ID();

   QStringList q( "get_project_desc" );
   q << QString::number( investigatorID );
   db.query( q );

   if ( db.lastErrno() != US_DB2::OK ) return;

   IDs.clear();
   descriptions.clear();
   GUIDs.clear();
   filenames.clear();

   while ( db.next() )
   {
      QString newID = db.value( 0 ).toString();
      IDs          << newID;
      descriptions << db.value( 1 ).toString();
      GUIDs        << QString( "" );
      filenames    << QString( "" );
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
      si.index       = i;
      info << si;

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

   if ( generalTab->disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      project.readFromDB  ( projectID, &db );
   }

   else
      project.readFromDisk( projectGUID );

   reset();
}

// Function to update the description associated with the current project
void US_ProjectGui::saveDescription( const QString& )
{
   project.projectDesc = generalTab->le_projectDesc->text();

   // Find the description in the lw
   if ( generalTab->lw_projects->currentItem() > 0 ) // Make sure an item is selected first
   {
      QListWidgetItem* item = generalTab->lw_projects->currentItem();
      item->setText( project.projectDesc );
      generalTab->lw_projects->setCurrentItem( item );
   }
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
   project.notes            = notesTab            ->getNotes();

   if ( generalTab->disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
      {
         db_error( db.lastError() );
         return;
      }

      project.saveToDB( &db );
   }

   else
   {
      project.saveToDisk();
   }

   QMessageBox::information( this,
         tr( "Save results" ),
         tr( "Project saved" ) );
}

// Function to delete a project from disk, db, or in the current form
void US_ProjectGui::deleteProject( void )
{
   if ( generalTab->disk_controls->db() )
   {
      US_Passwd pw;
      QString masterPW = pw.getPasswd();
      US_DB2 db( masterPW );
   
      if ( db.lastErrno() != US_DB2::OK )
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

// Function to display an error returned from the database
void US_ProjectGui::db_error( const QString& error )
{
   QMessageBox::warning( this, tr( "Database Problem" ),
         tr( "Database returned the following error: \n" ) + error );
}

US_ProjectGuiGeneral::US_ProjectGuiGeneral( int* invID,
                                            int  select_db_disk,
                                            QWidget* parent ) : QWidget( parent )
{
   investigatorID = invID;

   // Very light gray, for read-only line edits
   QPalette gray = US_GuiSettings::editColor();
   gray.setColor( QPalette::Base, QColor( 0xe0, 0xe0, 0xe0 ) );

   QGridLayout* general      = new QGridLayout( this );
   general->setSpacing         ( 2 );
   general->setContentsMargins ( 2, 2, 2, 2 );

   US_Widgets* usWidget = new US_Widgets();

   int row = 0;

   // First row
   if ( US_Settings::us_inv_level() > 2 )
   {
      QPushButton* pb_investigator = usWidget->us_pushbutton( tr( "Select Investigator" ) );
      connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
      general->addWidget( pb_investigator, row, 0 );
   }
   else
   {
      QLabel* lb_investigator = usWidget->us_label( tr( "Investigator:" ) );
      general->addWidget( lb_investigator, row, 0 );
   }
      
   le_investigator = usWidget->us_lineedit( tr( "Not Selected" ) );
   le_investigator->setReadOnly( true );
   general->addWidget( le_investigator, row++, 1, 1, 2 );

   // Row 2 - Available projects
   QLabel* lb_banner2 = usWidget->us_banner( tr( "Click on project to select" ), -2  );
   lb_banner2->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
   lb_banner2->setMinimumWidth( 400 );
   general->addWidget( lb_banner2, row, 0 );

   // Radio buttons
   disk_controls = new US_Disk_DB_Controls( select_db_disk );
   connect( disk_controls, SIGNAL( changed       ( bool ) ),
                           SIGNAL( source_changed( bool ) ) );
   general->addLayout( disk_controls, row++, 1, 1, 2 );

   // Row 3
   lw_projects = usWidget->us_listwidget();
   lw_projects-> setSortingEnabled( true );
   connect( lw_projects, SIGNAL( itemClicked    ( QListWidgetItem* ) ),
                         SIGNAL( selectProject  ( QListWidgetItem* ) ) );

   int add_rows = ( US_Settings::us_debug() == 0 ) ? 5 : 7;

   general->addWidget( lw_projects, row, 0, add_rows, 1 );

   // Row 4
   pb_query = usWidget->us_pushbutton( tr( "Query Projects" ), true );
   connect( pb_query, SIGNAL( clicked() ), SIGNAL( load() ) );
   general->addWidget( pb_query, row, 1 );

   pb_save = usWidget->us_pushbutton( tr( "Save Project" ), false );
   connect( pb_save, SIGNAL( clicked() ), SIGNAL( save() ) );
   general->addWidget( pb_save, row++, 2 );

   // Row 5
   pb_newProject = usWidget->us_pushbutton( tr( "New Project" ), true );
   connect( pb_newProject, SIGNAL( clicked() ), SIGNAL( newProject() ) );
   general->addWidget( pb_newProject, row, 1 );

   pb_del = usWidget->us_pushbutton( tr( "Delete Project" ), false );
   connect( pb_del, SIGNAL( clicked() ), SIGNAL( deleteProject() ) );
   general->addWidget( pb_del, row++, 2 );

   // Row 6
   QLabel* lb_projectDesc = usWidget->us_label( tr( "Project Name:" ) );
   general->addWidget( lb_projectDesc, row++, 1, 1, 2 );

   // Row 7
   le_projectDesc = usWidget->us_lineedit( "", 1 );
   connect( le_projectDesc, SIGNAL( textEdited      ( const QString&   ) ),
                            SIGNAL( saveDescription ( const QString&   ) ) );
   general->addWidget( le_projectDesc, row++, 1, 1, 2 );

   // Row 8
   QLabel* lb_guid = usWidget->us_label( tr( "Global Identifier:" ) );
   general->addWidget( lb_guid, row++, 1, 1, 2 );

   // Row 9
   le_guid = usWidget->us_lineedit( "" ); 
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
      SIGNAL( investigator_accepted( int, const QString&, const QString& ) ),
      SLOT  ( assign_investigator  ( int, const QString&, const QString& ) ) );

   inv_dialog->exec();
}

// Function to assign the selected investigator as current
void US_ProjectGuiGeneral::assign_investigator( int invID,
      const QString& /*lname*/, const QString& /*fname*/ )
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

void US_ProjectGuiGeneral::setDesc( QString newDesc )
{
   le_projectDesc->setText( newDesc );
}

QString US_ProjectGuiGeneral::getDesc( void )
{
   return( le_projectDesc->text() );
}

US_ProjectGui::GoalsTab::GoalsTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* goals = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_goals = usWidget->us_label( tr( "Please provide a detailed description of your research, \n"
                                              "including an introduction and goals:" ) );
   goals->addWidget( lb_goals );

   te_goals = usWidget->us_textedit();
   goals->addWidget( te_goals );
   te_goals->setMinimumHeight( 200 );
   te_goals->setReadOnly( false );

   goals -> addStretch( 1 );
   setLayout( goals );
}

void US_ProjectGui::GoalsTab::setGoals( QString newGoals )
{
   te_goals->setText( newGoals );
}

QString US_ProjectGui::GoalsTab::getGoals( void )
{
   return( te_goals->toPlainText() );
}

US_ProjectGui::MoleculesTab::MoleculesTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* molecules = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_molecules = usWidget->us_label( tr( "What proteins/DNA molecules are involved in the research?" ) );
   molecules->addWidget( lb_molecules );

   te_molecules = usWidget->us_textedit();
   molecules->addWidget( te_molecules );
   te_molecules->setMinimumHeight( 200 );
   te_molecules->setReadOnly( false );

   molecules -> addStretch( 1 );
   setLayout( molecules );
}

void US_ProjectGui::MoleculesTab::setMolecules( QString newMolecules )
{
   te_molecules->setText( newMolecules );
}

QString US_ProjectGui::MoleculesTab::getMolecules( void )
{
   return( te_molecules->toPlainText() );
}

US_ProjectGui::PurityTab::PurityTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* purity = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_purity = usWidget->us_label( tr( "Please indicate the approximate purity of your sample(s)." ) );
   purity->addWidget( lb_purity );

   le_purity = usWidget->us_lineedit();
   purity->addWidget( le_purity );
//   le_purity->setMinimumHeight( 200 );
   le_purity->setReadOnly( false );

   purity -> addStretch( 1 );
   setLayout( purity );
}

void US_ProjectGui::PurityTab::setPurity( QString newPurity )
{
   le_purity->setText( newPurity );
}

QString US_ProjectGui::PurityTab::getPurity( void )
{
   return( le_purity->text() );
}

US_ProjectGui::ExpenseTab::ExpenseTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* expense = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_expense = 
      usWidget->us_label( tr( "Please rate the expense of providing 5 ml at 1 OD 280 concentration:" ) );
   expense->addWidget( lb_expense );

   te_expense = usWidget->us_textedit();
   expense->addWidget( te_expense );
   te_expense->setMinimumHeight( 200 );
   te_expense->setReadOnly( false );

   expense -> addStretch( 1 );
   setLayout( expense );
}

void US_ProjectGui::ExpenseTab::setExpense( QString newExpense )
{
   te_expense->setText( newExpense );
}

QString US_ProjectGui::ExpenseTab::getExpense( void )
{
   return( te_expense->toPlainText() );
}

US_ProjectGui::BufferComponentsTab::BufferComponentsTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* bufferComponents = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_bufferComponents = 
      usWidget->us_label( tr( "What buffers do you plan to use?" ) );
   bufferComponents->addWidget( lb_bufferComponents );

   te_bufferComponents = usWidget->us_textedit();
   bufferComponents->addWidget( te_bufferComponents );
   te_bufferComponents->setMinimumHeight( 200 );
   te_bufferComponents->setReadOnly( false );

   bufferComponents -> addStretch( 1 );
   setLayout( bufferComponents );
}

void US_ProjectGui::BufferComponentsTab::setBufferComponents( QString newBufferComponents )
{
   te_bufferComponents->setText( newBufferComponents );
}

QString US_ProjectGui::BufferComponentsTab::getBufferComponents( void )
{
   return( te_bufferComponents->toPlainText() );
}

US_ProjectGui::SaltInformationTab::SaltInformationTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* saltInformation = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_saltInformation = 
      usWidget->us_label( tr( "Is a salt concentration between 20-50 mM for your experiment acceptable?" ) );
   saltInformation->addWidget( lb_saltInformation );

   te_saltInformation = usWidget->us_textedit();
   saltInformation->addWidget( te_saltInformation );
   te_saltInformation->setMinimumHeight( 200 );
   te_saltInformation->setReadOnly( false );

   saltInformation -> addStretch( 1 );
   setLayout( saltInformation );
}

void US_ProjectGui::SaltInformationTab::setSaltInformation( QString newSaltInformation )
{
   te_saltInformation->setText( newSaltInformation );
}

QString US_ProjectGui::SaltInformationTab::getSaltInformation( void )
{
   return( te_saltInformation->toPlainText() );
}

US_ProjectGui::AUC_questionsTab::AUC_questionsTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* auc_questions = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_auc_questions = 
      usWidget->us_label( tr( "What questions are you trying to answer with AUC?" ) );
   auc_questions->addWidget( lb_auc_questions );

   te_auc_questions = usWidget->us_textedit();
   auc_questions->addWidget( te_auc_questions );
   te_auc_questions->setMinimumHeight( 200 );
   te_auc_questions->setReadOnly( false );

   auc_questions -> addStretch( 1 );
   setLayout( auc_questions );
}

void US_ProjectGui::AUC_questionsTab::setAUC_questions( QString newAUC_questions )
{
   te_auc_questions->setText( newAUC_questions );
}

QString US_ProjectGui::AUC_questionsTab::getAUC_questions( void )
{
   return( te_auc_questions->toPlainText() );
}

US_ProjectGui::NotesTab::NotesTab( QWidget* parent ) : QWidget( parent )
{
   QVBoxLayout* notes = new QVBoxLayout;

   US_Widgets* usWidget = new US_Widgets();
   QLabel* lb_notes = usWidget->us_label( tr( "Notes:" ) );
   notes->addWidget( lb_notes );

   te_notes = usWidget->us_textedit();
   notes->addWidget( te_notes );
   te_notes->setMinimumHeight( 200 );
   te_notes->setReadOnly( false );

   notes -> addStretch( 1 );
   setLayout( notes );
}

void US_ProjectGui::NotesTab::setNotes( QString newNotes )
{
   te_notes->setText( newNotes );
}

QString US_ProjectGui::NotesTab::getNotes( void )
{
   return( te_notes->toPlainText() );
}

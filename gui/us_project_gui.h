//! \file us_project_gui.h
#ifndef US_PROJECT_GUI_H
#define US_PROJECT_GUI_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_project.h"
#include "us_help.h"

class US_EXTERN GeneralTab : public QWidget
{
   Q_OBJECT

   public:
      GeneralTab( int*,
                  QWidget* parent = 0 );

      //! A centralized place to reset all the controls in all the tabs
      void reset    ( void );

      QLineEdit*    le_investigator;
      QLineEdit*    le_projectDesc;
      QLineEdit*    le_guid;
   
      QListWidget*  lw_projects;
   
      QPushButton*  pb_query;
      QPushButton*  pb_save;
      QPushButton*  pb_newProject;
      QPushButton*  pb_del;
      QPushButton*  pb_resetAll;
   
      QRadioButton* rb_db;
      QRadioButton* rb_disk;
      
      int*          investigatorID;

      void          setGUID( QString );
      void          setDesc( QString );
      QString       getDesc( void    );

   signals:
      void check_db           ( void );       //!< \brief Emitted when the user clicks the db radio button
      void check_disk         ( void );       //!< \brief Emitted when the user clicks the disk radio button
      void newProject         ( void );       //!< \brief Emitted when the user clicks the new project button
      void load               ( void );       //!< \brief Emitted when the user clicks the Query Projects button
      void selectProject      ( QListWidgetItem* ); //!< \brief Emitted when the user selects a project
      void saveDescription    ( const QString& );   //!< \brief Emitted when the description changes
      void save               ( void );       //!< \brief Emitted when the user saves the project
      void deleteProject      ( void );       //!< \brief Emitted when the user deletes the project

   private slots:
      void sel_investigator   ( void );
      void assign_investigator( int, const QString&, const QString& );
};
    
class US_EXTERN GoalsTab : public QWidget
{
   Q_OBJECT

   public:
      GoalsTab                ( QWidget* parent = 0 );
      QString getGoals        ( void );
      void    setGoals        ( QString );

   private:
      QTextEdit*              te_goals;
   
};

class US_EXTERN MoleculesTab : public QWidget
{
   Q_OBJECT

   public:
      MoleculesTab            ( QWidget* parent = 0 );
      QString getMolecules    ( void );
      void    setMolecules    ( QString );

   private:
      QTextEdit*              te_molecules;
   
};

class US_EXTERN PurityTab : public QWidget
{
   Q_OBJECT

   public:
      PurityTab               ( QWidget* parent = 0 );
      QString getPurity       ( void );
      void    setPurity       ( QString );

   private:
      QTextEdit*              te_purity;
   
};

class US_EXTERN ExpenseTab : public QWidget
{
   Q_OBJECT

   public:
      ExpenseTab              ( QWidget* parent = 0 );
      QString getExpense      ( void );
      void    setExpense      ( QString );

   private:
      QTextEdit*              te_expense;
   
};

class US_EXTERN BufferComponentsTab : public QWidget
{
   Q_OBJECT

   public:
      BufferComponentsTab         ( QWidget* parent = 0 );
      QString getBufferComponents ( void );
      void    setBufferComponents ( QString );

   private:
      QTextEdit*                  te_bufferComponents;
   
};

class US_EXTERN SaltInformationTab : public QWidget
{
   Q_OBJECT

   public:
      SaltInformationTab          ( QWidget* parent = 0 );
      QString getSaltInformation  ( void );
      void    setSaltInformation  ( QString );

   private:
      QTextEdit*                  te_saltInformation;
   
};

class US_EXTERN AUC_questionsTab : public QWidget
{
   Q_OBJECT

   public:
      AUC_questionsTab            ( QWidget* parent = 0 );
      QString getAUC_questions    ( void );
      void    setAUC_questions    ( QString );

   private:
      QTextEdit*                  te_auc_questions;
   
};

class US_EXTERN NotesTab : public QWidget
{
   Q_OBJECT

   public:
      NotesTab                    ( QWidget* parent = 0 );
      QString getNotes            ( void );
      void    setNotes            ( QString );

   private:
      QTextEdit*                  te_notes;
   
};

/*! \class US_ProjectGui
           This class provides the ability to enter/edit the project
           parameters.
*/
           
class US_EXTERN US_ProjectGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_ProjectGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param invID   An integer value that indicates the ID of
                         the current user
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param dataIn  A reference to a structure that contains
                         the currently selected project information.
      */
      US_ProjectGui( int = 0,
                     bool = false,
                     const US_Project& = US_Project() );

      //! A null destructor. 
      ~US_ProjectGui() {};

      class ProjectInfo
      {
         public:
         int        projectID;
         QString    GUID;
         QString    description;
         QString    filename;
         int        index;
      };
   
      QList< ProjectInfo >       info;
      int           investigatorID;
      US_Project    project;
   
      QStringList   IDs;
      QStringList   descriptions;
      QStringList   GUIDs;
      QStringList   filenames;
   
      QMap< QListWidgetItem*, int > projectMap;
   
   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param project A reference to a structure that contains the
                        project information
      */
      void updateProjectGuiSelection( US_Project& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered information is erased.
      */
      void cancelProjectGuiSelection( void );

   private:
      bool          signal;

      US_Help       showHelp;

      QTabWidget*   tabWidget;

      GeneralTab*          generalTab;
      GoalsTab*            goalsTab;
      MoleculesTab*        moleculesTab;
      PurityTab*           purityTab;
      ExpenseTab*          expenseTab;
      BufferComponentsTab* bufferComponentsTab;
      SaltInformationTab*  saltInformationTab;
      AUC_questionsTab*    auc_questionsTab;
      NotesTab*            notesTab;

      QPushButton*  pb_accept;

   public slots:
      void check_db           ( void );
      void check_disk         ( void );
      void newProject         ( void );
      void load               ( void );
      void selectProject      ( QListWidgetItem* );
      void saveDescription    ( const QString& );
      void saveProject        ( void );
      void deleteProject      ( void );

   private slots:
      void reset              ( void );
      void enableButtons      ( void );
      void resetAll           ( void );
      void accept             ( void );
      void cancel             ( void );
      void loadDisk           ( void );
      void loadDB             ( void );
      void loadProjects       ( void );
      void db_error           ( const QString& );
      void help               ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif

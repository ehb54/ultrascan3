//! \file us_project_gui.h
#ifndef US_PROJECT_GUI_H
#define US_PROJECT_GUI_H

#include "us_extern.h"
#include "us_help.h"
#include "us_project.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

/*! \class US_ProjectGuiGeneral
           This class forms the content of the general tab in the US_Project GUI.
           Most management of the information itself takes place here.
*/
class US_ProjectGuiGeneral : public US_Widgets {
      Q_OBJECT

   public:
      /*! \brief Generic constructor for the US_ProjectGuiGeneral class. To 
              instantiate the class a calling function must
              provide the ID of the investigator.
       \param invID   A pointer to the current investigator ID
       \param select_db_disk Indicates whether the default search is on
                      the local disk or in the DB
   */
      US_ProjectGuiGeneral(int *, int);

      //! \brief Resets all the controls on the general tab
      void reset(void);

      //! \brief Sets the GUID line edit value
      //! \param newGUID The new GUID value to load into the form
      void setGUID(QString);

      //! \brief Sets the Last Updated line edit value
      //! \param lastupd The Last Updated value to load into the form
      void setLastUpd(QString);

      //! \brief Sets the Description textbox value
      //! \param newDesc The new description to load into the text box
      void setDesc(QString);

      //! \brief Returns the description from the form
      QString getDesc(void);

      QLineEdit *le_investigator; //!< Stores the investigator information
      QLineEdit *le_projectDesc; //!< A description of the project
      QLineEdit *le_guid; //!< The unique GUID of the project
      QLineEdit *le_lastupd; //!< Last update time of the project

      int *investigatorID; //!< Stores the investiator ID locally

      QListWidget *lw_projects; //!< Lists the projects retrieved

      QPushButton *pb_query; //!< The query projects button
      QPushButton *pb_save; //!< The save projects button
      QPushButton *pb_newProject; //!< The new project button
      QPushButton *pb_del; //!< The delete project button
      QPushButton *pb_resetAll; //!< Resets everything on the form

      US_Disk_DB_Controls *disk_controls; //!< Radiobuttons for disk/db choice

   signals:
      //! \brief Emitted when the new project button is clicked
      void newProject(void);

      //! \brief Emitted when the Query Projects button is clicked
      void load(void);

      //! \brief Emitted when the user selects a project
      //! \param item The currently selected item in the project list widget
      void selectProject(QListWidgetItem *item);

      //! \brief Emitted when the description changes
      void saveDescription(const QString &);

      //! \brief Emitted when the user saves the project
      void save(void);

      //! \brief Emitted when the user deletes the project
      void deleteProject(void);

      //! \brief Emitted when the user changes data source (disk/db)
      //! \param db True for db, false for disk
      void source_changed(bool db);

   private:
   private slots:
      void sel_investigator(void);
      void assign_investigator(int);
};

/*! \class US_ProjectGuiGoals
    \brief This class forms the content of the Goals tab in the US_Project GUI.
*/
class US_ProjectGuiGoals : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiGoals class
      US_ProjectGuiGoals(void);

      //! \brief Returns the content entered by the user
      QString getGoals(void);

      //! \brief Loads the content of this tab
      //! \param newGoals The text to load into the text widget
      void setGoals(QString);

   signals:
      //! \brief Emitted when the text in the goals tab changes
      void goalsTabChanged(void);

   private:
      QTextEdit *te_goals;
};

/*! \class US_ProjectGuiMolecules
    \brief This class forms the content of the molecules tab in the US_Project GUI.
*/
class US_ProjectGuiMolecules : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiMolecules class
      US_ProjectGuiMolecules(void);

      //! \brief Returns the content entered by the user
      QString getMolecules(void);

      //! \brief Loads the content of this tab
      //! \param newMolecules The text to load into the text widget
      void setMolecules(QString);

   signals:
      //! \brief Emitted when the text in the molecules tab changes
      void moleculesTabChanged(void);

   private:
      QTextEdit *te_molecules;
};

/*! \class US_ProjectGuiPurity
    \brief This class forms the content of the purity tab in the US_Project GUI.
*/
class US_ProjectGuiPurity : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiPurity class
      US_ProjectGuiPurity(void);

      //! \brief Returns the content entered by the user
      QString getPurity(void);

      //! \brief Loads the content of this tab
      //! \param newPurity The text to load into the text widget
      void setPurity(QString);

   signals:
      //! \brief Emitted when the text in the purity tab changes
      void purityTabChanged(const QString &);

   private:
      QLineEdit *le_purity;
};

/*! \class US_ProjectGuiExpense
    \brief This class forms the content of the expense tab in the US_Project GUI.
*/
class US_ProjectGuiExpense : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiExpense class
      US_ProjectGuiExpense(void);

      //! \brief Returns the content entered by the user
      QString getExpense(void);

      //! \brief Loads the content of this tab
      //! \param newExpense The text to load into the text widget
      void setExpense(QString);

   signals:
      //! \brief Emitted when the text in the expense tab changes
      void expenseTabChanged(void);

   private:
      QTextEdit *te_expense;
};

/*! \class US_ProjectGuiBufferComponents
    \brief This class forms the content of the buffer components tab in the US_Project GUI.
*/
class US_ProjectGuiBufferComponents : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiBufferComponents class
      US_ProjectGuiBufferComponents(void);

      //! \brief Returns the content entered by the user
      QString getBufferComponents(void);

      //! \brief Loads the content of this tab
      //! \param newBufferComponents The text to load into the text widget
      void setBufferComponents(QString);

   signals:
      //! \brief Emitted when the text in the bufferComponents tab changes
      void bufferComponentsTabChanged(void);

   private:
      QTextEdit *te_bufferComponents;
};

/*! \class US_ProjectGuiSaltInformation
    \brief This class forms the content of the salt information tab in the US_Project GUI.
*/
class US_ProjectGuiSaltInformation : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiSaltInformation class
      US_ProjectGuiSaltInformation(void);

      //! \brief Returns the content entered by the user
      QString getSaltInformation(void);

      //! \brief Loads the content of this tab
      //! \param newSaltInformation The text to load into the text widget
      void setSaltInformation(QString);

   signals:
      //! \brief Emitted when the text in the saltInformation tab changes
      void saltInformationTabChanged(void);

   private:
      QTextEdit *te_saltInformation;
};

/*! \class US_ProjectGuiAUC_questions
    \brief This class forms the content of the AUC_questions tab in the US_Project GUI.
*/
class US_ProjectGuiAUC_questions : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiAUC_questions class
      US_ProjectGuiAUC_questions(void);

      //! \brief Returns the content entered by the user
      QString getAUC_questions(void);

      //! \brief Loads the content of this tab
      //! \param newAUC_questions The text to load into the text widget
      void setAUC_questions(QString);

   signals:
      //! \brief Emitted when the text in the AUC_questions tab changes
      void AUC_questionsTabChanged(void);

   private:
      QTextEdit *te_auc_questions;
};

/*! \class US_ProjectGuiExpDesign
    \brief This class forms the content of the expDesign tab in the US_Project GUI.
*/
class US_ProjectGuiExpDesign : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the expDesignTab class
      US_ProjectGuiExpDesign(void);

      //! \brief Returns the content entered by the user
      QString getExpDesign(void);

      //! \brief Loads the content of this tab
      //! \param newExpDesign The text to load into the text widget
      void setExpDesign(QString);

   signals:
      //! \brief Emitted when the text in the expDesign tab changes
      void expDesignTabChanged(void);

   private:
      QTextEdit *te_exp_design;
};

/*! \class US_ProjectGuiNotes
    \brief This class forms the content of the notes tab in the US_Project GUI.
*/
class US_ProjectGuiNotes : public US_Widgets {
      Q_OBJECT

   public:
      //! \brief Generic constructor for the US_ProjectGuiNotes class
      US_ProjectGuiNotes(void);

      //! \brief Returns the content entered by the user
      QString getNotes(void);

      //! \brief Loads the content of this tab
      //! \param newNotes The text to load into the text widget
      void setNotes(QString);

   signals:
      //! \brief Emitted when the text in the notes tab changes
      void notesTabChanged(void);

   private:
      QTextEdit *te_notes;
};

/*! \class US_ProjectGui
           This class provides the ability to enter/edit the project
           parameters.
*/

class US_GUI_EXTERN US_ProjectGui : public US_WidgetsDialog {
      Q_OBJECT

   public:
      /*! \brief Generic constructor for the US_ProjectGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
          \param dataIn  A reference to a structure that contains
                         the currently selected project information.
      */
      US_ProjectGui(bool = false, int = US_Disk_DB_Controls::Default, const US_Project & = US_Project());

      //! \brief Cleans up when the project dialog is closed
      ~US_ProjectGui();

      //! \class ProjectInfo
      //! \brief Information that is useful throughout US_ProjectGui
      class ProjectInfo {
         public:
            int projectID; //!< The ID of the project
            QString GUID; //!< The GUID associated with the project
            QString description; //!< The overall description of the project
            QString filename; //!< The filename if saved to disk
            int index; //!< Used when indexing the information
            QDateTime lastUpdated; //!< Last update date-time of project
      };

      QList<ProjectInfo> info; //!< Information to populate the project list widget
      int investigatorID; //!< Stores the ID of the current investigator
      US_Project project; //!< Stores all the information about the project

      QStringList IDs; //!< The IDs of the items in the project list
      QStringList descriptions; //!< The descriptions in the project list
      QStringList GUIDs; //!< The GUIDs of the items in the project list
      QStringList filenames; //!< The filenames associated with disk saves
      QList<QDateTime> lastUpds; //!< The last updates of projects

      QMap<QListWidgetItem *, int> projectMap; //!< Maps the IDs with the listwidget items

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param project A reference to a structure that contains the
                        project information
      */
      void updateProjectGuiSelection(US_Project &project);

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered information is erased.
      */
      void cancelProjectGuiSelection(void);

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db(bool DB);

   private:
      bool signal;
      bool text_changed;

      US_Help showHelp;

      QTabWidget *tabWidget;

      US_ProjectGuiGeneral *generalTab;
      US_ProjectGuiGoals *goalsTab;
      US_ProjectGuiMolecules *moleculesTab;
      US_ProjectGuiPurity *purityTab;
      US_ProjectGuiExpense *expenseTab;
      US_ProjectGuiBufferComponents *bufferComponentsTab;
      US_ProjectGuiSaltInformation *saltInformationTab;
      US_ProjectGuiAUC_questions *auc_questionsTab;
      US_ProjectGuiExpDesign *expDesignTab;
      US_ProjectGuiNotes *notesTab;

      QPushButton *pb_accept;

   public slots:
      //! \brief Creates a new project
      void newProject(void);

      //! \brief Loads projects into the projects list widget
      void load(void);

      //! \brief Handles when project listwidget item is selected
      void selectProject(QListWidgetItem *);

      //! \brief Updates the description associated with the current project
      void saveDescription(const QString &);

      //! \brief Saves the project information from all tabs to disk or db
      void saveProject(void);

      //! \brief Deletes the currently selected project from disk or db
      void deleteProject(void);

      //! \brief Change data source (disk/db)
      void source_changed(bool);

      //! \brief Helps to keep track if the text in any tab has changed
      void tabTextChanged(const QString & = "");

   private slots:
      void reset(void);
      void enableButtons(void);
      void resetAll(void);
      void accept(void);
      void cancel(void);
      void loadDisk(void);
      void loadDB(void);
      void loadProjects(void);
      void db_error(const QString &);
      void help(void) { showHelp.show_help("manual/us_project.html"); };
};
#endif

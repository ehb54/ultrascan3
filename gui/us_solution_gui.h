//! \file us_solution_gui.h
#ifndef US_SOLUTION_GUI_H
#define US_SOLUTION_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "us_help.h"

/*! \class US_SolutionGui
           This class provides the ability to associate the solution
           parameters, buffers, and analytes with a single
           cell / channel / wavelength combination in the experiment.
*/
           
class US_GUI_EXTERN US_SolutionGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_SolutionGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param expID   An integer value that indicates the ID of
                         the associated experiment
          \param chID    An integer value that indicates the ID of
                         the channel used
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param select_db_disk Indicates whether the default search is on
                         the local disk or in the DB
          \param dataIn  A reference to a structure that contains
                         the currently selected c/c/w dataset.
          \param auto_save A boolean value indicating whether the caller
                         wants an automatic save at Accept.
      */
      US_SolutionGui( int  = 1,
                      int  = 1,
                      bool = false,
                      int  = US_Disk_DB_Controls::Default,
                      const US_Solution& = US_Solution(),
                      bool = true );

      //! A null destructor. 
      ~US_SolutionGui() {};

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param solution A reference to a structure that contains the
                        solution, buffer and analyte choices for a 
                        single c/c/w combination 
      */
      void updateSolutionGuiSelection( US_Solution& solution );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelSolutionGuiSelection( void );

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:
      class SolutionInfo
      {
         public:
         int        solutionID;
         QString    GUID;
         QString    description;
         QString    filename;
         int        index;
      };

      QList< SolutionInfo >       info;
      int                         investigatorID;
      int                         experimentID;
      int                         channelID;
      bool                        signal;
      US_Solution                 solution;
      bool                        autosave;
      bool                        changed;

      QStringList   IDs;
      QStringList   descriptions;
      QStringList   GUIDs;
      QStringList   filenames;

      US_Help       showHelp;

      QLabel*       lb_amount;

      QLineEdit*    le_investigator;
      QLineEdit*    le_bufferInfo;
      QLineEdit*    le_solutionDesc;
      QLineEdit*    le_commonVbar20;
      QLineEdit*    le_density;
      QLineEdit*    le_viscosity;
      QLineEdit*    le_storageTemp;
      QLineEdit*    le_guid;
      QwtCounter*   ct_amount;

      QListWidget*  lw_solutions;
      QListWidget*  lw_analytes;

      QTextEdit*    te_notes;

      US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

      QPushButton*  pb_query;
      QPushButton*  pb_buffer;
      QPushButton*  pb_newSolution;
      QPushButton*  pb_save;
      QPushButton*  pb_del;
      QPushButton*  pb_addAnalyte;
      QPushButton*  pb_removeAnalyte;
      QPushButton*  pb_accept;

      QMap< QListWidgetItem*, int > solutionMap;
      QMap< QListWidgetItem*, int > analyteMap;

  private slots:
      void reset              ( void );
      void accept             ( void );
      void cancel             ( void );
      void sel_investigator   ( void );
      void assign_investigator( int, const QString&, const QString& );
      void load               ( void );
      void loadDisk           ( void );
      void loadDB             ( void );
      void loadSolutions      ( void );
      void selectSolution     ( QListWidgetItem* );
      void addAnalyte         ( void );
      void assignAnalyte      ( US_Analyte );
      void selectAnalyte      ( QListWidgetItem* );
      void removeAnalyte      ( void );
      void calcCommonVbar20   ( void );
      void selectBuffer       ( void );
      void assignBuffer       ( US_Buffer );
      void saveAmount         ( double );
      void saveDescription    ( const QString& );
      void saveTemperature    ( const QString& );
      void saveNotes          ( void );
      void newSolution        ( void );
      void save               ( bool = true );
      void delete_solution    ( void );
      void source_changed     ( bool );
      void update_disk_db     ( bool );
      void db_error           ( const QString& );
      void help               ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif

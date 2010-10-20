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
           
class US_EXTERN US_SolutionGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_SolutionGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param invID   An integer value that indicates the ID of
                         the current user
          \param expID   An integer value that indicates the ID of
                         the associated experiment
          \param chID    An integer value that indicates the ID of
                         the channel used
          \param signal_wanted A boolean value indicating whether the caller
                         wants a signal to be emitted
          \param dataIn  A reference to a structure that contains
                         the currently selected c/c/w dataset.
      */
      US_SolutionGui( int = 0,
                      int = 1,
                      int = 1,
                      bool = false,
                      const US_Solution& = US_Solution() );

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
      void updateSolutionGuiSelection( US_Solution& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelSolutionGuiSelection( void );

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

      QStringList   IDs;
      QStringList   descriptions;
      QStringList   GUIDs;
      QStringList   filenames;

      US_Help       showHelp;

      QLabel*       lb_amount;

      QLineEdit*    le_investigator;
      QLineEdit*    le_bufferInfo;
      QLineEdit*    le_solutionDesc;
      QLineEdit*    le_storageTemp;
      QLineEdit*    le_guid;
      QwtCounter*   ct_amount;

      QListWidget*  lw_solutions;
      QListWidget*  lw_analytes;

      QTextEdit*    te_notes;

      QPushButton*  pb_query;
      QPushButton*  pb_buffer;
      QPushButton*  pb_newSolution;
      QPushButton*  pb_save;
      QPushButton*  pb_del;
      QPushButton*  pb_addAnalyte;
      QPushButton*  pb_removeAnalyte;
      QPushButton*  pb_accept;

      QRadioButton* rb_db;
      QRadioButton* rb_disk;

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
      void selectBuffer       ( void );
      void assignBuffer       ( US_Buffer );
      void saveAmount         ( double );
      void saveDescription    ( const QString& );
      void saveTemperature    ( const QString& );
      void saveNotes          ( void );
      void newSolution        ( void );
      void save               ( void );
      void delete_solution    ( void );
      void check_db           ( void );
      void check_disk         ( void );
      void db_error           ( const QString& );
      void help               ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif

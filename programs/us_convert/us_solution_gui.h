//! \file us_solutioninfo.h
#ifndef US_SOLUTION_GUI_H
#define US_SOLUTION_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_selectbox.h"
#include "us_analyte.h"
#include "us_help.h"

/*! \class US_SolutionGui
           This class provides the ability to associate raw data with
           the relevant experiment parameters, such as the lab,
           instrument, and rotor on which the experiment was run. 
*/
           
class US_EXTERN US_SolutionGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief  Class that contains information about relevant 
      //!         cell/channel/wavelength combinations
      class TripleInfo
      {
         public:
         int              tripleID;           //!< The ID of this c/c/w combination
         QString          tripleDesc;         //!< The description of this triple ( e.g., "2 / A / 260" )
         QString          description;        //!< A text description of this triple
         bool             excluded;           //!< Whether this triple has been dropped or not
         int              centerpiece;        //!< The ID of the centerpiece that was used
         int              bufferID;           //!< The ID of the buffer that was associated
         QString          bufferGUID;         //!< The GUID of the buffer
         QString          bufferDesc;         //!< The corresponding buffer description
         int              analyteID;          //!< The ID of the analyte that was associated
         QString          analyteGUID;        //!< The GUID of the analyte
         QString          analyteDesc;        //!< The corresponding analyte description
         char             tripleGUID[16];     //!< The GUID of this triple
         QString          tripleFilename;     //!< The filename of this auc file
         char             solutionGUID[16];   //!< The GUID of the solution for this triple
         QString          solutionDesc;       //!< A description of the solution 
         bool             storageTemp;        //!< A boolean value
         QString          notes;              //!< Notes on the channel solution
         TripleInfo();                        //!< A generic constructor
         void             clear( void );
         void             show( void );       // temporary
      };

      /*! \brief Generic constructor for the US_SolutionGui class. To 
                 instantiate the class a calling function must
                 provide a structure to contain all the data.

          \param dataIn  A reference to a structure that contains
                         previously selected experiment data, if any.
          \param current An integer value that indicates which c/c/w
                         combination is currently selected
          \param ID      An integer value the indicates the ID of
                         the current user
      */
      US_SolutionGui( QList< TripleInfo >&, int&, int& );

      //! A null destructor. 
      ~US_SolutionGui() {};

      //! \brief   Some status codes to keep track of where solution data has been saved to
      enum solutionStatus
      {
         NOT_SAVED,                           //!< The file has not been saved
         EDITING,                             //!< Data is being edited; certain operations not permitted
         HD_ONLY,                             //!< The file has been saved to the HD
         DB_ONLY,                             //!< The file has been saved to the DB
         BOTH                                 //!< The file has been saved to both HD and DB
      };

   signals:

      /*! \brief The signal that is emitted when the user chooses
                 to accept the current choices. This information is
                 passed back to the calling function.

          \param triples A reference to a structure that contains all
                         the current experiment data
          \param currentTriple A reference to an integer containing the 
                         current c/c/w combination
      */
      void updateSolutionGuiSelection( QList< US_SolutionGui::TripleInfo >&, int& );

      /*! \brief The signal that is emitted when the user chooses
                 to cancel the current selection. In this case all
                 previously-entered experiment parameter associations
                 are erased.
      */
      void cancelSolutionGuiSelection( void );

   private:
      QList< TripleInfo >&   triples;
      int&                   currentTriple;
      int&                   invID;
      QList< solutionStatus > saveStatus;

      US_Help                showHelp;

      QListWidget*  lw_triple;                        // cell, channel, wavelength

      US_SelectBox* cb_triple;
      QLineEdit*    le_description;
      QLineEdit*    le_bufferInfo;
      QLineEdit*    le_analyteInfo;
      QLineEdit*    le_solutionDesc;

      QTextEdit*    te_notes;

      QPushButton*  pb_applyAll;
      QPushButton*  pb_buffer;
      QPushButton*  pb_analyte;
      QPushButton*  pb_load;
      QPushButton*  pb_save;
      QPushButton*  pb_del;

      QRadioButton* rb_db;
      QRadioButton* rb_disk;

      void setTripleList     ( void );

  private slots:
      void reset             ( void );
      void accept            ( void );
      void cancel            ( void );
      void change_triple     ( int );
      void selectBuffer      ( void );
      void assignBuffer      ( const QString&  );
      void selectAnalyte     ( void );
      void assignAnalyte     ( US_Analyte  );
      void tripleApplyAll    ( void );
      void saveFormData      ( void );
      void load              ( void );
      void save              ( void );
      void delete_solution   ( void );
      void db_error          ( const QString& );
      void help              ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
